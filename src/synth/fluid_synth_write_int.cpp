/*
 * FluidSynth - A Software Synthesizer
 *
 * C++11 integer output rendering implementation for s16.
 * This file contains the full, bit-identical logic formerly implemented in
 * fluid_synth.c: fluid_synth_write_s16() and fluid_synth_write_s16_channels().
 */

#include "fluid_synth_write_int.h"

/* Include the same internal headers that the original C bodies relied on.
 * Adjust includes to match upstream commit 2d07b6b.
 */
#include "fluid_sys.h"
/* + any other needed internal headers */

#include <cstdint>
#include <climits> // INT32_MIN / INT32_MAX

/* --------------------------------------------------------------------------
 * Local static helpers/tables
 *
 * -------------------------------------------------------------------------- */

static FLUID_INLINE unsigned int fluid_synth_get_ticks(fluid_synth_t *synth)
{
    return fluid_atomic_int_get(&synth->ticks_since_start);
}

static FLUID_INLINE void fluid_synth_add_ticks(fluid_synth_t *synth, int val)
{
    fluid_atomic_int_add(&synth->ticks_since_start, val);
}


/***************************************************************
 *                    FLUID SAMPLE TIMERS
 *    Timers that use written audio data as timing reference
 */
struct _fluid_sample_timer_t
{
    fluid_sample_timer_t *next; /* Single linked list of timers */
    unsigned long starttick;
    fluid_timer_callback_t callback;
    void *data;
    int isfinished;
};

/*
 * fluid_sample_timer_process - called when synth->ticks is updated
 */
static void fluid_sample_timer_process(fluid_synth_t *synth)
{
    fluid_sample_timer_t *st;
    long msec;
    int cont;
    unsigned int ticks = fluid_synth_get_ticks(synth);

    for (st = synth->sample_timers; st; st = st->next)
    {
        if (st->isfinished)
        {
            continue;
        }

        msec = (long)(1000.0 * ((double)(ticks - st->starttick)) / synth->sample_rate);
        cont = (*st->callback)(st->data, msec);

        if (cont == 0)
        {
            st->isfinished = 1;
        }
    }
}

/* Dither table */
#define DITHER_SIZE 48000
#define DITHER_CHANNELS 2

static float rand_table[DITHER_CHANNELS][DITHER_SIZE];

/* Init dither table */
static void init_dither(void)
{
    float d, dp;
    int c, i;

    for (c = 0; c < DITHER_CHANNELS; c++)
    {
        dp = 0;

        for (i = 0; i < DITHER_SIZE - 1; i++)
        {
            d = rand() / (float)RAND_MAX - 0.5f;
            rand_table[c][i] = d - dp;
            dp = d;
        }

        rand_table[c][DITHER_SIZE - 1] = 0 - dp;
    }
}

/* A portable replacement for roundf(), seems it may actually be faster too! */
static FLUID_INLINE int16_t round_clip_to_i16(float x)
{
    long i;

    if (x >= 0.0f)
    {
        i = (long)(x + 0.5f);

        if (FLUID_UNLIKELY(i > 32767))
        {
            i = 32767;
        }
    }
    else
    {
        i = (long)(x - 0.5f);

        if (FLUID_UNLIKELY(i < -32768))
        {
            i = -32768;
        }
    }

    return (int16_t)i;
}

/* Round + clip to signed 32-bit PCM. No dithering. */
static FLUID_INLINE int32_t round_clip_to_i32(float x)
{
    int64_t i;

    if (x >= 0.0f)
    {
        i = (int64_t)(x + 0.5f);

        if (FLUID_UNLIKELY(i > INT32_MAX))
        {
            i = INT32_MAX;
        }
    }
    else
    {
        i = (int64_t)(x - 0.5f);

        if (FLUID_UNLIKELY(i < INT32_MIN))
        {
            i = INT32_MIN;
        }
    }

    return (int32_t)i;
}

/**
 * Process blocks (FLUID_BUFSIZE) of audio.
 * Must be called from renderer thread only!
 * @return number of blocks rendered. Might (often) return less than requested
 */
static int fluid_synth_render_blocks(fluid_synth_t *synth, int blockcount)
{
    int i, maxblocks;
    fluid_profile_ref_var(prof_ref);

    /* Assign ID of synthesis thread */
    //  synth->synth_thread_id = fluid_thread_get_id ();

    fluid_check_fpe("??? Just starting up ???");

    fluid_rvoice_eventhandler_dispatch_all(synth->eventhandler);

    /* do not render more blocks than we can store internally */
    maxblocks = fluid_rvoice_mixer_get_bufcount(synth->eventhandler->mixer);

    if (blockcount > maxblocks)
    {
        blockcount = maxblocks;
    }

    for (i = 0; i < blockcount; i++)
    {
        fluid_sample_timer_process(synth);
        fluid_synth_add_ticks(synth, FLUID_BUFSIZE);

        /* If events have been queued waiting for fluid_rvoice_eventhandler_dispatch_all()
         * (should only happen with parallel render) stop processing and go for rendering
         */
        if (fluid_rvoice_eventhandler_dispatch_count(synth->eventhandler))
        {
            // Something has happened, we can't process more
            blockcount = i + 1;
            break;
        }
    }

    fluid_check_fpe("fluid_sample_timer_process");

    blockcount = fluid_rvoice_mixer_render(synth->eventhandler->mixer, blockcount);

    /* Testcase, that provokes a denormal floating point error */
#if 0
    {
        float num = 1;

        while(num != 0)
        {
            num *= 0.5;
        };
    };
#endif
    fluid_check_fpe("??? Remainder of synth_one_block ???");
    fluid_profile(FLUID_PROF_ONE_BLOCK,
                  prof_ref,
                  fluid_rvoice_mixer_get_active_voices(synth->eventhandler->mixer),
                  blockcount * FLUID_BUFSIZE);
    return blockcount;
}

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
        return round_clip_to_i16(x * 32766.0f + rand_table[ch][di]);
    }

    static FLUID_INLINE void advance_di(int &di)
    {
        if (++di >= DITHER_SIZE)
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

/**
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
        int32_t s = round_clip_to_i32(x * 2147483646.0f);
        return (int32_t)(s & 0xFFFFFF00);
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
        return round_clip_to_i32(x * 2147483646.0f);
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
    } while (i);

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
        if (cur >= synth->curmax)
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
        if (n > size)
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
            } while (i);

            traits_t::advance_di(di);
        } while (++n < 0);
    } while (size);

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

/* --------------------------------------------------------------------------
 * C ABI entrypoints called by the C forwarders
 * -------------------------------------------------------------------------- */

/**
 * Synthesize a block of 16 bit audio samples channels to audio buffers.
 * The function is convenient for audio driver to render multiple stereo
 * channels pairs on multi channels audio cards (i.e 2, 4, 6, 8,.. channels).
 *
 * @param synth FluidSynth instance.
 * @param len Count of audio frames to synthesize.
 * @param channels_count Count of channels in a frame.
 *  must be multiple of 2 and  channel_count/2 must not exceed the number
 *  of internal mixer buffers (synth->audio_groups)
 * @param channels_out Array of channels_count pointers on 16 bit words to
 *  store sample channels. Modified on return.
 * @param channels_off Array of channels_count offset index to add to respective pointer
 *  in channels_out for first sample.
 * @param channels_incr Array of channels_count increment between consecutive
 *  samples channels.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 *
 * Useful for storing:
 * - interleaved channels in a unique buffer.
 * - non interleaved channels in an unique buffer (or in distinct buffers).
 *
 * Example for interleaved 4 channels (c1, c2, c3, c4) and n samples (s1, s2,..sn)
 * in a unique buffer:
 * { s1:c1, s1:c2, s1:c3, s1:c4,  s2:c1, s2:c2, s2:c3, s2:c4, ....
 *   sn:c1, sn:c2, sn:c3, sn:c4 }.
 *
 * @note Should only be called from synthesis thread.
 * @note Reverb and Chorus are mixed to \c lout resp. \c rout.
 * @note Dithering is performed when converting from internal floating point to
 * 16 bit audio.
 */
extern "C" int 
fluid_synth_write_s16_channels_cpp(
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

/**
 * Synthesize a block of 24 bit audio samples channels to audio buffers.
 * Same shape as s16_channels, but outputs left aligned, signed 24-bit PCM (24-in-32),
 * and performs round+clip+mask conversion from internal floating point. No dithering.
 */
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

/**
 * Synthesize a block of 32 bit audio samples channels to audio buffers.
 * Same shape as s16_channels, but outputs signed 32-bit PCM (int32_t),
 * and performs round+clip conversion from internal floating point. No dithering.
 */
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

/**
 * Synthesize a block of 16 bit audio samples to audio buffers.
 * @param synth FluidSynth instance
 * @param len Count of audio frames to synthesize
 * @param lout Array of 16 bit words to store left channel of audio
 * @param loff Offset index in 'lout' for first sample
 * @param lincr Increment between samples stored to 'lout'
 * @param rout Array of 16 bit words to store right channel of audio
 * @param roff Offset index in 'rout' for first sample
 * @param rincr Increment between samples stored to 'rout'
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * Useful for storing interleaved stereo (lout = rout, loff = 0, roff = 1,
 * lincr = 2, rincr = 2).
 *
 * @note Should only be called from synthesis thread.
 * @note Reverb and Chorus are mixed to \c lout resp. \c rout.
 * @note Dithering is performed when converting from internal floating point to
 * 16 bit audio.
 */
extern "C" int
fluid_synth_write_s16_cpp(fluid_synth_t *synth, int len, void *lout, int loff, int lincr, void *rout, int roff, int rincr)
{
    void *channels_out[2] = { lout, rout };
    int channels_off[2] = { loff, roff };
    int channels_incr[2] = { lincr, rincr };

    return fluid_synth_write_s16_channels_cpp(synth, len, 2, channels_out, channels_off, channels_incr);
}

/**
 * Synthesize a block of 24 bit audio samples channels to audio buffers.
 * Same shape as s16_channels, but outputs left aligned, signed 24-bit PCM (24-in-32),
 * and performs round+clip+mask conversion from internal floating point. No dithering.
 */
extern "C" int
fluid_synth_write_s24_cpp(fluid_synth_t *synth, int len, void *lout, int loff, int lincr, void *rout, int roff, int rincr)
{
    void *channels_out[2] = { lout, rout };
    int channels_off[2] = { loff, roff };
    int channels_incr[2] = { lincr, rincr };

    return fluid_synth_write_s24_channels_cpp(synth, len, 2, channels_out, channels_off, channels_incr);
}

/**
 * Synthesize a block of 32 bit audio samples to audio buffers.
 * Same shape as s16, but outputs signed 32-bit PCM (int32_t),
 * and performs round+clip conversion from internal floating point. No dithering.
 */
extern "C" int
fluid_synth_write_s32_cpp(fluid_synth_t *synth, int len, void *lout, int loff, int lincr, void *rout, int roff, int rincr)
{
    void *channels_out[2] = { lout, rout };
    int channels_off[2] = { loff, roff };
    int channels_incr[2] = { lincr, rincr };

    return fluid_synth_write_s32_channels_cpp(synth, len, 2, channels_out, channels_off, channels_incr);
}
