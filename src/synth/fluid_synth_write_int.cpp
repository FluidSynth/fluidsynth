/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */

/*
 * C++11 integer output rendering implementation (s16/s24/s32).
 *
 * Contains the full, bit-identical legacy s16 rendering logic formerly in
 * fluid_synth.c, plus shared integer output infrastructure used for s24 and
 * s32.
 */

#define NOMINMAX // to prevent windows.h from defining min/max macros
#include "fluid_synth_write_int.h"

/* Include the same internal headers that the original C bodies relied on.
 */
#include "fluid_sys.h"
#include "fluid_audio_convert.h"


/* --------------------------------------------------------------------------
 * Integer write core: tag dispatch + traits (C++11)
 * -------------------------------------------------------------------------- */

struct s16_tag
{
};

struct s24_tag
{
};

struct s32_tag
{
};

template<typename Tag> struct int_write_traits;

/* s16: dithered int16 output, bit-identical to legacy C path */
template<> struct int_write_traits<s16_tag>
{
    typedef int16_t sample_t;

    static FLUID_INLINE sample_t emit(fluid_real_t x, int ch, int di)
    {
        /* Preserve exact arithmetic order:
         * (sample * 32766.0f) + rand_table[ch][di], then round_clip_to_i16().
         */
        return round_clip_to<sample_t>((float)x * 32766.0f + rand_table[ch][di]);
    }

    static FLUID_INLINE void advance_di(int &di)
    {
        if(++di >= DITHER_SIZE)
        {
            di = 0;
        }
    }

    static FLUID_INLINE int load_di(const fluid_synth_t *synth)
    {
        return synth->dither_index;
    }

    static FLUID_INLINE void store_di(fluid_synth_t *synth, int di)
    {
        synth->dither_index = di;
    }
};

/*
 * s24: 24-bit PCM in an int32_t container.
 * - 24 valid bits are left-aligned in the 32-bit word.
 * - The low 8 bits are always zero (mask 0xFFFFFF00).
 * - Non-dithered conversion: same scale and round+clip as s32, then apply the mask.
 *
 * Implemented as s32 conversion with the low 8 bits cleared, to keep s24 and s32
 * directly comparable and deterministic.
 */
template<> struct int_write_traits<s24_tag>
{
    typedef int32_t sample_t;

    static FLUID_INLINE sample_t emit(fluid_real_t x, int /*ch*/, int /*di*/)
    {
        /* 24 valid bits left-aligned in 32-bit container:
         * - Convert exactly like s32 (full 32-bit scale)
         * - Then clear the lowest 8 bits (transport truncation)
         */
        sample_t s = round_clip_to<sample_t>(x * 2147483646.0f);
        return (sample_t)(s & 0xFFFFFF00);
    }

    static FLUID_INLINE void advance_di(int & /*di*/)
    {
    }

    static FLUID_INLINE int load_di(const fluid_synth_t * /*synth*/)
    {
        return 0;
    }

    static FLUID_INLINE void store_di(fluid_synth_t * /*synth*/, int /*di*/)
    {
    }
};

/* s32: non-dithered int32 output, round + clip from internal float */
template<> struct int_write_traits<s32_tag>
{
    typedef int32_t sample_t;

    static FLUID_INLINE sample_t emit(fluid_real_t x, int /*ch*/, int /*di*/)
    {
        /* No dithering. Convert using round+clip only.
         * Keep scale convention parallel to s16's 32766.0f:
         * INT32_MAX-1 => 2147483646.0f
         */
        return round_clip_to<sample_t>(x * 2147483646.0f);
    }

    static FLUID_INLINE void advance_di(int & /*di*/)
    {
        /* no-op: s32 has no dither index */
    }

    static FLUID_INLINE int load_di(const fluid_synth_t * /*synth*/)
    {
        return 0;
    }

    static FLUID_INLINE void store_di(fluid_synth_t * /*synth*/, int /*di*/)
    {
        /* no-op */
    }
};

template<typename Tag>
static int fluid_synth_write_int_channels_impl(fluid_synth_t *synth,
        int len,
        int channels_count,
        void *channels_out[],
        int channels_off[],
        int channels_incr[])
{
    typedef int_write_traits<Tag> traits_t;
    typedef typename traits_t::sample_t sample_t;

    sample_t **chan_out = (sample_t **)channels_out;
    int di, n, cur, size;

    /* pointers on first input mixer buffer */
    fluid_real_t *left_in;
    fluid_real_t *right_in;
    int bufs_in_count; /* number of stereo input buffers */
    int i;

    /* start average cpu load probe */
    double time = fluid_utime();
    float cpu_load;

    /* start profiling duration probe (if profiling is enabled) */
    fluid_profile_ref_var(prof_ref);

    /* check parameters */
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);

    fluid_return_val_if_fail(len >= 0, FLUID_FAILED);
    fluid_return_val_if_fail(len != 0, FLUID_OK); // to avoid raising FE_DIVBYZERO below

    /* check for valid channel_count: must be multiple of 2 and
       channel_count/2 must not exceed the number of internal mixer buffers
       (synth->audio_groups)
    */
    fluid_return_val_if_fail(!(channels_count & 1) /* must be multiple of 2 */
                             && channels_count >= 2,
                             FLUID_FAILED);

    bufs_in_count = (unsigned int)channels_count >> 1; /* channels_count/2 */
    fluid_return_val_if_fail(bufs_in_count <= synth->audio_groups, FLUID_FAILED);

    fluid_return_val_if_fail(channels_out != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(channels_off != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(channels_incr != NULL, FLUID_FAILED);

    /* initialize output channels buffers on first sample position */
    i = channels_count;

    do
    {
        i--;
        chan_out[i] += channels_off[i];
    }
    while(i);

    /* Conversely to fluid_synth_process(),
       we want rendered audio effect mixed in internal audio dry buffers.
       TRUE instructs the mixer that internal audio effects will be mixed in internal
       audio dry buffers.
    */
    fluid_rvoice_mixer_set_mix_fx(synth->eventhandler->mixer, TRUE);
    /* get first internal mixer audio dry buffer's pointer (left and right channel) */
    fluid_rvoice_mixer_get_bufs(synth->eventhandler->mixer, &left_in, &right_in);

    size = len;
    /* synth->cur indicates if available samples are still in internal mixer buffer */
    cur = synth->cur; /* get previous sample position in internal buffer (due to prvious call) */
    di = traits_t::load_di(synth);

    do
    {
        /* fill up the buffers as needed */
        if(cur >= synth->curmax)
        {
            /* render audio (dry and effect) to internal dry buffers */
            /* always render full blocks multiple of FLUID_BUFSIZE */
            int blocksleft = (size + FLUID_BUFSIZE - 1) / FLUID_BUFSIZE;
            synth->curmax = FLUID_BUFSIZE * fluid_synth_render_blocks(synth, blocksleft);

            /* get first internal mixer audio dry buffer's pointer (left and right channel) */
            fluid_rvoice_mixer_get_bufs(synth->eventhandler->mixer, &left_in, &right_in);
            cur = 0;
        }

        /* calculate amount of available samples */
        n = synth->curmax - cur;

        /* keep track of emitted samples */
        if(n > size)
        {
            n = size;
        }

        size -= n;

        /* update pointers to current position */
        left_in += cur + n;
        right_in += cur + n;

        /* set final cursor position */
        cur += n;

        /* reverse index */
        n = 0 - n;

        do
        {
            i = bufs_in_count;

            do
            {
                /* input sample index in stereo buffer i */
                int in_idx = --i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + n;
                int c = i << 1; /* channel index c to write */

                /* write left input sample to channel sample */
                *chan_out[c] = traits_t::emit(left_in[in_idx], 0, di);

                /* write right input sample to next channel sample */
                *chan_out[c + 1] = traits_t::emit(right_in[in_idx], 1, di);

                /* advance output pointers */
                chan_out[c] += channels_incr[c];
                chan_out[c + 1] += channels_incr[c + 1];
            }
            while(i);

            traits_t::advance_di(di);
        }
        while(++n < 0);
    }
    while(size);

    synth->cur = cur; /* save current sample position. It will be used on next call */
    traits_t::store_di(synth, di);

    /* save average cpu load, used by API for real time cpu load meter */
    time = fluid_utime() - time;
    cpu_load =
        0.5f * (fluid_atomic_float_get(&synth->cpu_load) + (float)(time * synth->sample_rate / len / 10000.0));
    fluid_atomic_float_set(&synth->cpu_load, cpu_load);

    /* stop duration probe and save performance measurement (if profiling is enabled) */
    fluid_profile_write(FLUID_PROF_WRITE,
                        prof_ref,
                        fluid_rvoice_mixer_get_active_voices(synth->eventhandler->mixer),
                        len);
    return FLUID_OK;
}

/*
 * C ABI entrypoints called by the C forwarders.
 *
 * These wrappers are internal plumbing; their public-facing documentation
 * (if any) lives with the declarations in fluid_synth_write_int.h.
 */
extern "C" int fluid_synth_write_s16_channels_cpp(
    fluid_synth_t *synth,
    int len,
    int channels_count,
    void *channels_out[],
    int channels_off[],
    int channels_incr[])
{
    return fluid_synth_write_int_channels_impl<s16_tag>(
               synth,
               len,
               channels_count,
               channels_out,
               channels_off,
               channels_incr);
}

extern "C" int
fluid_synth_write_s24_channels_cpp(
    fluid_synth_t *synth,
    int len,
    int channels_count,
    void *channels_out[],
    int channels_off[],
    int channels_incr[])
{
    return fluid_synth_write_int_channels_impl<s24_tag>(
               synth,
               len,
               channels_count,
               channels_out,
               channels_off,
               channels_incr);
}

extern "C" int
fluid_synth_write_s32_channels_cpp(
    fluid_synth_t *synth,
    int len,
    int channels_count,
    void *channels_out[],
    int channels_off[],
    int channels_incr[])
{
    return fluid_synth_write_int_channels_impl<s32_tag>(
               synth,
               len,
               channels_count,
               channels_out,
               channels_off,
               channels_incr);
}

extern "C" int
fluid_synth_write_s16_cpp(fluid_synth_t *synth, int len, void *lout, int loff, int lincr, void *rout, int roff, int rincr)
{
    void *channels_out[2] = { lout, rout };
    int channels_off[2] = { loff, roff };
    int channels_incr[2] = { lincr, rincr };

    return fluid_synth_write_s16_channels_cpp(synth, len, 2, channels_out, channels_off, channels_incr);
}

extern "C" int
fluid_synth_write_s24_cpp(fluid_synth_t *synth, int len, void *lout, int loff, int lincr, void *rout, int roff, int rincr)
{
    void *channels_out[2] = { lout, rout };
    int channels_off[2] = { loff, roff };
    int channels_incr[2] = { lincr, rincr };

    return fluid_synth_write_s24_channels_cpp(synth, len, 2, channels_out, channels_off, channels_incr);
}

extern "C" int
fluid_synth_write_s32_cpp(fluid_synth_t *synth, int len, void *lout, int loff, int lincr, void *rout, int roff, int rincr)
{
    void *channels_out[2] = { lout, rout };
    int channels_off[2] = { loff, roff };
    int channels_incr[2] = { lincr, rincr };

    return fluid_synth_write_s32_channels_cpp(synth, len, 2, channels_out, channels_off, channels_incr);
}
