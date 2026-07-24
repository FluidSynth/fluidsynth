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

#include "fluid_sys.h"
#include "fluid_phase.h"
#include "fluid_rvoice.h"
#include "fluid_rvoice_dsp_tables.h"

/* Purpose:
 *
 * Interpolates audio data (obtains values between the samples of the original
 * waveform data).
 *
 * Variables loaded from the voice structure (assigned in fluid_rvoice_write()):
 * - dsp_data: Pointer to the original waveform data
 * - dsp_phase: The position in the original waveform data.
 *              This has an integer and a fractional part (between samples).
 * - dsp_phase_incr: For each output sample, the position in the original
 *              waveform advances by dsp_phase_incr. This also has an integer
 *              part and a fractional part.
 *              If a sample is played at root pitch (no pitch change),
 *              dsp_phase_incr is integer=1 and fractional=0.
 * - dsp_amp: The current amplitude envelope value.
 * - dsp_amp_incr: The changing rate of the amplitude envelope.
 *
 * A couple of variables are used internally, their results are discarded:
 * - dsp_i: Index through the output buffer
 * - dsp_buf: Output buffer of floating point values (FLUID_BUFSIZE in length)
 */

/* Interpolation (find a value between two samples of the original waveform) */

extern "C" const fluid_real_t *const interp_coeff_linear;
extern "C" const fluid_real_t *const interp_coeff;
extern "C" const fluid_real_t *const sinc_table7;

template<bool IS_24BIT>
static FLUID_INLINE fluid_real_t
fluid_rvoice_get_float_sample(const short int *FLUID_RESTRICT dsp_msb, const char *FLUID_RESTRICT dsp_lsb, unsigned int idx)
{
    int32_t sample;
    if (IS_24BIT)
    {
        sample = fluid_rvoice_get_sample24(dsp_msb, dsp_lsb, idx);
    }
    else
    {
        sample = fluid_rvoice_get_sample16(dsp_msb, idx);
    }
    
    return (fluid_real_t)sample;
}

/* Special case of interpolate_none for rendering silent voices, i.e. in delay phase or zero volume */
template<bool LOOPING>
static int fluid_rvoice_dsp_silence_local(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf)
{
    fluid_rvoice_dsp_t *voice = &rvoice->dsp;
    fluid_phase_t dsp_phase = voice->phase;
    fluid_phase_t dsp_phase_incr;
    unsigned short dsp_i = 0;
    unsigned int dsp_phase_index;
    unsigned int end_index;

    /* Convert playback "speed" floating point value to phase index/fract */
    fluid_phase_set_float(dsp_phase_incr, voice->phase_incr);

    end_index = LOOPING ? voice->loopend - 1 : voice->end;

    while (1)
    {
        dsp_phase_index = fluid_phase_index_round(dsp_phase); /* round to nearest point */

        /* interpolate sequence of sample points */
        for (; dsp_i < FLUID_BUFSIZE && dsp_phase_index <= end_index; dsp_i++)
        {
            fluid_real_t sample = 0;
            dsp_buf[dsp_i] = sample;

            /* increment phase and amplitude */
            fluid_phase_incr(dsp_phase, dsp_phase_incr);
        }

        /* break out if not looping (buffer may not be full) */
        if (!LOOPING)
        {
            break;
        }

        dsp_phase_index = fluid_phase_index_round(dsp_phase); /* round to nearest point */
        /* go back to loop start */
        if (dsp_phase_index > end_index)
        {
            fluid_phase_sub_int(dsp_phase, voice->loopend - voice->loopstart);
            voice->has_looped = 1;
        }

        /* break out if filled buffer */
        if (dsp_i >= FLUID_BUFSIZE)
        {
            break;
        }
    }

    voice->phase = dsp_phase;
    // Note, there is no need to update the amplitude here. When the voice becomes audible again, the amp will be updated anyway in fluid_rvoice_calc_amp().
    // voice->amp = dsp_amp;

    return (dsp_i);
}

/* No interpolation. Just take the sample, which is closest to
  * the playback pointer.  Questionable quality, but very
  * efficient. */
template<bool IS_24BIT, bool LOOPING>
static int
fluid_rvoice_dsp_interpolate_none_local(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf)
{
    fluid_rvoice_dsp_t *voice = &rvoice->dsp;
    fluid_phase_t dsp_phase = voice->phase;
    fluid_phase_t dsp_phase_incr;
    const short int *FLUID_RESTRICT dsp_data = voice->sample->data;
    const char *FLUID_RESTRICT dsp_data24 = voice->sample->data24;
    unsigned short dsp_i = 0;
    unsigned int dsp_phase_index;
    unsigned int end_index;

    /* Convert playback "speed" floating point value to phase index/fract */
    fluid_phase_set_float(dsp_phase_incr, voice->phase_incr);

    end_index = LOOPING ? voice->loopend - 1 : voice->end;

    while(1)
    {
        dsp_phase_index = fluid_phase_index_round(dsp_phase);	/* round to nearest point */

        /* interpolate sequence of sample points */
        for(; dsp_i < FLUID_BUFSIZE && dsp_phase_index <= end_index; dsp_i++)
        {
            fluid_real_t sample = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index);
            
            dsp_buf[dsp_i] = sample;

            /* increment phase and amplitude */
            fluid_phase_incr(dsp_phase, dsp_phase_incr);
            dsp_phase_index = fluid_phase_index_round(dsp_phase);	/* round to nearest point */
        }

        /* break out if not looping (buffer may not be full) */
        if(!LOOPING)
        {
            break;
        }

        /* go back to loop start */
        if(dsp_phase_index > end_index)
        {
            fluid_phase_sub_int(dsp_phase, voice->loopend - voice->loopstart);
            voice->has_looped = 1;
        }

        /* break out if filled buffer */
        if(dsp_i >= FLUID_BUFSIZE)
        {
            break;
        }
    }

    voice->phase = dsp_phase;

    return (dsp_i);
}

/* Straight line interpolation.
 * Returns number of samples processed (usually FLUID_BUFSIZE but could be
 * smaller if end of sample occurs).
 */
template<bool IS_24BIT, bool LOOPING>
static int
fluid_rvoice_dsp_interpolate_linear_local(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf)
{
    fluid_rvoice_dsp_t *voice = &rvoice->dsp;
    fluid_phase_t dsp_phase = voice->phase;
    fluid_phase_t dsp_phase_incr;
    const short int *FLUID_RESTRICT dsp_data = voice->sample->data;
    const char *FLUID_RESTRICT dsp_data24 = voice->sample->data24;
    unsigned short dsp_i = 0;
    unsigned int dsp_phase_index;
    unsigned int end_index;
    fluid_real_t point;
    const fluid_real_t *FLUID_RESTRICT coeffs;

    /* Convert playback "speed" floating point value to phase index/fract */
    fluid_phase_set_float(dsp_phase_incr, voice->phase_incr);

    /* last index before 2nd interpolation point must be specially handled */
    end_index = (LOOPING ? voice->loopend - 1 : voice->end) - 1;

    /* 2nd interpolation point to use at end of loop or sample */
    if(LOOPING)
    {
        point = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopstart);    /* loop start */
    }
    else
    {
        point = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->end);    /* duplicate end for samples no longer looping */
    }

    while(1)
    {
        dsp_phase_index = fluid_phase_index(dsp_phase);

        /* interpolate the sequence of sample points */
        for(; dsp_i < FLUID_BUFSIZE && dsp_phase_index <= end_index; dsp_i++)
        {
            fluid_real_t sample;
            coeffs = &interp_coeff_linear[fluid_phase_fract_to_tablerow(dsp_phase) * LINEAR_INTERP_ORDER];
            
            sample =  (coeffs[0] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index)
                     + coeffs[1] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 1));
                        
            dsp_buf[dsp_i] = sample;

            /* increment phase and amplitude */
            fluid_phase_incr(dsp_phase, dsp_phase_incr);
            dsp_phase_index = fluid_phase_index(dsp_phase);
        }

        /* break out if buffer filled */
        if(dsp_i >= FLUID_BUFSIZE)
        {
            break;
        }

        end_index++;	/* we're now interpolating the last point */

        /* interpolate within last point */
        for(; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
        {
            fluid_real_t sample;
            coeffs = &interp_coeff_linear[fluid_phase_fract_to_tablerow(dsp_phase) * LINEAR_INTERP_ORDER];
            
            sample =  (coeffs[0] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index)
                     + coeffs[1] * point);

            dsp_buf[dsp_i] = sample;

            /* increment phase and amplitude */
            fluid_phase_incr(dsp_phase, dsp_phase_incr);
            dsp_phase_index = fluid_phase_index(dsp_phase);
        }

        if(!LOOPING)
        {
            break;    /* break out if not looping (end of sample) */
        }

        /* go back to loop start (if past */
        if(dsp_phase_index > end_index)
        {
            fluid_phase_sub_int(dsp_phase, voice->loopend - voice->loopstart);
            voice->has_looped = 1;
        }

        /* break out if filled buffer */
        if(dsp_i >= FLUID_BUFSIZE)
        {
            break;
        }

        end_index--;	/* set end back to second to last sample point */
    }

    voice->phase = dsp_phase;

    return (dsp_i);
}

/* 4th order (cubic) interpolation.
 * Returns number of samples processed (usually FLUID_BUFSIZE but could be
 * smaller if end of sample occurs).
 */
template<bool IS_24BIT, bool LOOPING>
static int
fluid_rvoice_dsp_interpolate_4th_order_local(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf)
{
    fluid_rvoice_dsp_t *voice = &rvoice->dsp;
    fluid_phase_t dsp_phase = voice->phase;
    fluid_phase_t dsp_phase_incr;
    const short int *FLUID_RESTRICT dsp_data = voice->sample->data;
    const char *FLUID_RESTRICT dsp_data24 = voice->sample->data24;
    unsigned int dsp_i = 0;
    unsigned int start_index, loop_end_index;
    fluid_real_t start_point, end_point1, end_point2;

    fluid_phase_set_float(dsp_phase_incr, voice->phase_incr);

    /* Loop region is [loopstart, loopend-1] for LOOPING, [start, end] otherwise */
    loop_end_index = LOOPING ? voice->loopend - 1 : voice->end;

    if(voice->has_looped)
    {
        start_index = voice->loopstart;
        start_point = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopend - 1);
    }
    else
    {
        start_index = voice->start;
        start_point = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->start);
    }

    if(LOOPING)
    {
        end_point1 = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopstart);
        end_point2 = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopstart + 1);
    }
    else
    {
        end_point1 = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->end);
        end_point2 = end_point1;
    }

    /* Fixed-point fraction to float conversion */
    constexpr fluid_real_t FRACT_SCALE = 1.0f / 4294967296.0f;  /* 1 / 2^32 */
    
    /* Loop length in fixed-point (32.32 format) */
    const fluid_phase_t loop_length = LOOPING 
        ? static_cast<fluid_phase_t>(voice->loopend - voice->loopstart) << 32 
        : 0;

    /* Helper lambda for phase-to-index with loop wrap */
    auto wrap_phase = [&](fluid_phase_t phase) -> fluid_phase_t {
        if(LOOPING && voice->has_looped)
        {
            const fluid_phase_t loop_start_phase = static_cast<fluid_phase_t>(voice->loopstart) << 32;
            const fluid_phase_t loop_end_phase = static_cast<fluid_phase_t>(voice->loopend) << 32;
            
            while(phase >= loop_end_phase)
            {
                phase -= loop_length;
            }
            /* Safety: ensure we're not below loop start after wrapping */
            while(phase < loop_start_phase && loop_length > 0)
            {
                phase += loop_length;
            }
        }
        return phase;
    };

    /* Process single sample with full boundary handling */
    auto process_single_sample = [&](fluid_phase_t phase) -> fluid_real_t {
        const unsigned int idx = static_cast<unsigned int>(phase >> 32);
        const fluid_real_t f = static_cast<fluid_real_t>(phase & 0xFFFFFFFF) * FRACT_SCALE;

        /* Clamped indices for safe memory access */
        const unsigned int idx_m1 = (idx > start_index) ? (idx - 1) : start_index;
        const unsigned int idx_p1 = (idx < loop_end_index) ? (idx + 1) : loop_end_index;
        const unsigned int idx_p2 = (idx + 1 < loop_end_index) ? (idx + 2) : loop_end_index;

        const fluid_real_t raw_m1 = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, idx_m1);
        const fluid_real_t raw_0  = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, idx);
        const fluid_real_t raw_p1 = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, idx_p1);
        const fluid_real_t raw_p2 = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, idx_p2);

        const bool at_start = (idx == start_index);
        const bool at_end   = (idx >= loop_end_index);
        const bool near_end = (idx + 1 >= loop_end_index);

        const fluid_real_t s0 = at_start ? start_point : raw_m1;
        const fluid_real_t s1 = raw_0;
        const fluid_real_t s2 = at_end ? end_point1 : raw_p1;
        const fluid_real_t s3 = at_end ? end_point2 : (near_end ? end_point1 : raw_p2);

        const fluid_real_t f2 = f * f;
        const fluid_real_t f3 = f2 * f;

        return (-0.5f * f3 + f2 - 0.5f * f) * s0
             + (1.5f * f3 - 2.5f * f2 + 1.0f) * s1
             + (-1.5f * f3 + 2.0f * f2 + 0.5f * f) * s2
             + (0.5f * f3 - 0.5f * f2) * s3;
    };

    /* Calculate samples until phase index reaches target (exclusive) */
    auto samples_until_index = [&](fluid_phase_t phase, unsigned int target_index) -> unsigned int {
        const fluid_phase_t target_phase = static_cast<fluid_phase_t>(target_index) << 32;
        if(phase >= target_phase)
            return 0;
        return static_cast<unsigned int>((target_phase - phase) / dsp_phase_incr);
    };

    /* Calculate samples while phase index stays <= target */
    auto samples_while_at_or_below = [&](fluid_phase_t phase, unsigned int target_index) -> unsigned int {
        const fluid_phase_t limit_phase = static_cast<fluid_phase_t>(target_index + 1) << 32;
        if(phase >= limit_phase)
            return 0;
        return static_cast<unsigned int>((limit_phase - phase) / dsp_phase_incr);
    };

    while(dsp_i < FLUID_BUFSIZE)
    {
        /* Apply any pending loop wrap */
        dsp_phase = wrap_phase(dsp_phase);
        
        const unsigned int current_index = static_cast<unsigned int>(dsp_phase >> 32);

        /* Check if we've gone past the end (non-looping case) */
        if(current_index > loop_end_index)
        {
            if(!LOOPING)
                break;
            
            /* This shouldn't happen after wrap_phase, but safety first */
            dsp_phase = wrap_phase(dsp_phase);
            continue;
        }

        /* Update loop state if we've just entered the loop region */
        if(LOOPING && !voice->has_looped && current_index >= voice->loopstart)
        {
            voice->has_looped = 1;
            start_index = voice->loopstart;
            start_point = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopend - 1);
        }

        /* Safe region boundaries: [safe_start, safe_end]
         * Within this range, all four taps (idx-1, idx, idx+1, idx+2) are valid */
        const unsigned int safe_start = start_index + 1;
        const unsigned int safe_end = (loop_end_index >= 2) ? loop_end_index - 2 : 0;

        /* Determine which region we're in and how many samples to process */
        unsigned int region_count = 0;
        enum { REGION_BEFORE_SAFE, REGION_SAFE, REGION_AFTER_SAFE } region;

        if(current_index < safe_start)
        {
            region = REGION_BEFORE_SAFE;
            region_count = samples_until_index(dsp_phase, safe_start);
            region_count = std::min(region_count, FLUID_BUFSIZE - dsp_i);
            
            /* Ensure at least 1 sample if we're still before safe region */
            if(region_count == 0)
                region_count = 1;
        }
        else if(current_index <= safe_end)
        {
            region = REGION_SAFE;
            region_count = samples_while_at_or_below(dsp_phase, safe_end);
            region_count = std::min(region_count, FLUID_BUFSIZE - dsp_i);
        }
        else
        {
            region = REGION_AFTER_SAFE;
            region_count = samples_while_at_or_below(dsp_phase, loop_end_index);
            region_count = std::min(region_count, FLUID_BUFSIZE - dsp_i);
            
            /* Ensure at least 1 sample if we're in the boundary region */
            if(region_count == 0)
                region_count = 1;
        }

        if(region == REGION_SAFE && region_count > 0)
        {
            /*
             * VECTORIZABLE LOOP - Middle region
             * 
             * - No loop-carried dependencies
             * - Fixed iteration count known at loop entry  
             * - No branches inside loop
             * - All memory accesses guaranteed valid
             */
            const fluid_phase_t phase_base = dsp_phase;
            const unsigned int loop_end = dsp_i + region_count;

#if defined(__clang__)
            #pragma clang loop vectorize(enable) interleave(enable)
#elif defined(__GNUC__)
            #pragma GCC ivdep
#elif defined(_OPENMP)
            #pragma omp simd
#endif
            for(unsigned int i = dsp_i; i < loop_end; i++)
            {
                /* Phase computed entirely from loop index */
                const fluid_phase_t phase = phase_base + static_cast<fluid_phase_t>(i - dsp_i) * dsp_phase_incr;
                const unsigned int idx = static_cast<unsigned int>(phase >> 32);
                const fluid_real_t f = static_cast<fluid_real_t>(phase & 0xFFFFFFFF) * FRACT_SCALE;

                /* Direct sample access - all guaranteed in bounds */
                const fluid_real_t s0 = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, idx - 1);
                const fluid_real_t s1 = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, idx);
                const fluid_real_t s2 = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, idx + 1);
                const fluid_real_t s3 = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, idx + 2);

                /* Catmull-Rom interpolation */
                const fluid_real_t f2 = f * f;
                const fluid_real_t f3 = f2 * f;

                dsp_buf[i] = (-0.5f * f3 + f2 - 0.5f * f) * s0
                           + (1.5f * f3 - 2.5f * f2 + 1.0f) * s1
                           + (-1.5f * f3 + 2.0f * f2 + 0.5f * f) * s2
                           + (0.5f * f3 - 0.5f * f2) * s3;
            }

            dsp_phase = phase_base + static_cast<fluid_phase_t>(region_count) * dsp_phase_incr;
            dsp_i = loop_end;
        }
        else
        {
            /*
             * SCALAR LOOP - Boundary regions (before safe_start or after safe_end)
             * 
             * Uses full boundary handling with conditional sample selection
             */
            const unsigned int loop_end = dsp_i + region_count;

            for(unsigned int i = dsp_i; i < loop_end; i++)
            {
                /* Handle loop wrap for each sample in boundary region */
                dsp_phase = wrap_phase(dsp_phase);
                
                const unsigned int idx = static_cast<unsigned int>(dsp_phase >> 32);
                
                /* Stop if we've gone past the end in non-looping mode */
                if(idx > loop_end_index)
                {
                    if(!LOOPING)
                    {
                        dsp_i = i;
                        goto done;
                    }
                    continue;
                }

                dsp_buf[i] = process_single_sample(dsp_phase);
                dsp_phase += dsp_phase_incr;
            }

            dsp_i = loop_end;
        }
    }

done:
    voice->phase = dsp_phase;
    return static_cast<int>(dsp_i);
}

/* 7th order interpolation.
 * Returns number of samples processed (usually FLUID_BUFSIZE but could be
 * smaller if end of sample occurs).
 */
template<bool IS_24BIT, bool LOOPING>
static int
fluid_rvoice_dsp_interpolate_7th_order_local(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf)
{
    fluid_rvoice_dsp_t *voice = &rvoice->dsp;
    fluid_phase_t dsp_phase = voice->phase;
    fluid_phase_t dsp_phase_incr;
    const short int *FLUID_RESTRICT dsp_data = voice->sample->data;
    const char *FLUID_RESTRICT dsp_data24 = voice->sample->data24;
    unsigned short dsp_i = 0;
    unsigned int dsp_phase_index;
    unsigned int start_index, end_index;
    fluid_real_t start_points[3], end_points[3];
    const fluid_real_t *FLUID_RESTRICT coeffs;

    /* Convert playback "speed" floating point value to phase index/fract */
    fluid_phase_set_float(dsp_phase_incr, voice->phase_incr);

    /* add 1/2 sample to dsp_phase since 7th order interpolation is centered on
     * the 4th sample point */
    fluid_phase_incr(dsp_phase, (fluid_phase_t)0x80000000);

    /* last index before 7th interpolation point must be specially handled */
    end_index = (LOOPING ? voice->loopend - 1 : voice->end) - 3;

    if(voice->has_looped)	/* set start_index and start point if looped or not */
    {
        start_index = voice->loopstart;
        start_points[0] = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopend - 1);
        start_points[1] = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopend - 2);
        start_points[2] = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopend - 3);
    }
    else
    {
        start_index = voice->start;
        start_points[0] = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->start);	/* just duplicate the start point */
        start_points[1] = start_points[0];
        start_points[2] = start_points[0];
    }

    /* get the 3 points off the end (loop start if looping, duplicate point if end) */
    if(LOOPING)
    {
        end_points[0] = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopstart);
        end_points[1] = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopstart + 1);
        end_points[2] = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopstart + 2);
    }
    else
    {
        end_points[0] = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->end);
        end_points[1] = end_points[0];
        end_points[2] = end_points[0];
    }

    while(1)
    {
        dsp_phase_index = fluid_phase_index(dsp_phase);

        /* interpolate first sample point (start or loop start) if needed */
        for(; dsp_phase_index == start_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
        {
            fluid_real_t sample;
            coeffs = &sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase) * SINC_INTERP_ORDER];

            sample =  (coeffs[0] * start_points[2]
                     + coeffs[1] * start_points[1]
                     + coeffs[2] * start_points[0]
                     + coeffs[3] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index)
                     + coeffs[4] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 1)
                     + coeffs[5] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 2)
                     + coeffs[6] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 3));

            dsp_buf[dsp_i] = sample;

            /* increment phase and amplitude */
            fluid_phase_incr(dsp_phase, dsp_phase_incr);
            dsp_phase_index = fluid_phase_index(dsp_phase);
        }

        start_index++;

        /* interpolate 2nd to first sample point (start or loop start) if needed */
        for(; dsp_phase_index == start_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
        {
            fluid_real_t sample;
            coeffs = &sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase) * SINC_INTERP_ORDER];

            sample =  (coeffs[0] * start_points[1]
                     + coeffs[1] * start_points[0]
                     + coeffs[2] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 1)
                     + coeffs[3] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index)
                     + coeffs[4] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 1)
                     + coeffs[5] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 2)
                     + coeffs[6] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 3));

            dsp_buf[dsp_i] = sample;

            /* increment phase and amplitude */
            fluid_phase_incr(dsp_phase, dsp_phase_incr);
            dsp_phase_index = fluid_phase_index(dsp_phase);
        }

        start_index++;

        /* interpolate 3rd to first sample point (start or loop start) if needed */
        for(; dsp_phase_index == start_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
        {
            fluid_real_t sample;
            coeffs = &sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase) * SINC_INTERP_ORDER];

            sample =  (coeffs[0] * start_points[0]
                     + coeffs[1] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 2)
                     + coeffs[2] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 1)
                     + coeffs[3] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index)
                     + coeffs[4] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 1)
                     + coeffs[5] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 2)
                     + coeffs[6] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 3));

            dsp_buf[dsp_i] = sample;

            /* increment phase and amplitude */
            fluid_phase_incr(dsp_phase, dsp_phase_incr);
            dsp_phase_index = fluid_phase_index(dsp_phase);
        }

        start_index -= 2;	/* set back to original start index */


        /* interpolate the sequence of sample points */
        for(; dsp_i < FLUID_BUFSIZE && dsp_phase_index <= end_index; dsp_i++)
        {
            fluid_real_t sample;
            coeffs = &sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase) * SINC_INTERP_ORDER];

            sample =  (coeffs[0] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 3)
                     + coeffs[1] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 2)
                     + coeffs[2] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 1)
                     + coeffs[3] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index)
                     + coeffs[4] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 1)
                     + coeffs[5] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 2)
                     + coeffs[6] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 3));

            dsp_buf[dsp_i] = sample;

            /* increment phase and amplitude */
            fluid_phase_incr(dsp_phase, dsp_phase_incr);
            dsp_phase_index = fluid_phase_index(dsp_phase);
        }

        /* break out if buffer filled */
        if(dsp_i >= FLUID_BUFSIZE)
        {
            break;
        }

        end_index++;	/* we're now interpolating the 3rd to last point */

        /* interpolate within 3rd to last point */
        for(; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
        {
            fluid_real_t sample;
            coeffs = &sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase) * SINC_INTERP_ORDER];

            sample =  (coeffs[0] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 3)
                     + coeffs[1] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 2)
                     + coeffs[2] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 1)
                     + coeffs[3] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index)
                     + coeffs[4] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 1)
                     + coeffs[5] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 2)
                     + coeffs[6] * end_points[0]);

            dsp_buf[dsp_i] = sample;

            /* increment phase and amplitude */
            fluid_phase_incr(dsp_phase, dsp_phase_incr);
            dsp_phase_index = fluid_phase_index(dsp_phase);
        }

        end_index++;	/* we're now interpolating the 2nd to last point */

        /* interpolate within 2nd to last point */
        for(; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
        {
            fluid_real_t sample;
            coeffs = &sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase) * SINC_INTERP_ORDER];

            sample =  (coeffs[0] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 3)
                     + coeffs[1] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 2)
                     + coeffs[2] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 1)
                     + coeffs[3] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index)
                     + coeffs[4] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index + 1)
                     + coeffs[5] * end_points[0]
                     + coeffs[6] * end_points[1]);

            dsp_buf[dsp_i] = sample;

            /* increment phase and amplitude */
            fluid_phase_incr(dsp_phase, dsp_phase_incr);
            dsp_phase_index = fluid_phase_index(dsp_phase);
        }

        end_index++;	/* we're now interpolating the last point */

        /* interpolate within last point */
        for(; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
        {
            fluid_real_t sample;
            coeffs = &sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase) * SINC_INTERP_ORDER];

            sample =  (coeffs[0] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 3)
                     + coeffs[1] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 2)
                     + coeffs[2] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index - 1)
                     + coeffs[3] * fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, dsp_phase_index)
                     + coeffs[4] * end_points[0]
                     + coeffs[5] * end_points[1]
                     + coeffs[6] * end_points[2]);

            dsp_buf[dsp_i] = sample;

            /* increment phase and amplitude */
            fluid_phase_incr(dsp_phase, dsp_phase_incr);
            dsp_phase_index = fluid_phase_index(dsp_phase);
        }

        if(!LOOPING)
        {
            break;    /* break out if not looping (end of sample) */
        }

        /* go back to loop start */
        if(dsp_phase_index > end_index)
        {
            fluid_phase_sub_int(dsp_phase, voice->loopend - voice->loopstart);

            if(!voice->has_looped)
            {
                voice->has_looped = 1;
                start_index = voice->loopstart;
                start_points[0] = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopend - 1);
                start_points[1] = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopend - 2);
                start_points[2] = fluid_rvoice_get_float_sample<IS_24BIT>(dsp_data, dsp_data24, voice->loopend - 3);
            }
        }

        /* break out if filled buffer */
        if(dsp_i >= FLUID_BUFSIZE)
        {
            break;
        }

        end_index -= 3;	/* set end back to 4th to last sample point */
    }

    /* sub 1/2 sample from dsp_phase since 7th order interpolation is centered on
     * the 4th sample point (correct back to real value) */
    fluid_phase_decr(dsp_phase, (fluid_phase_t)0x80000000);

    voice->phase = dsp_phase;

    return (dsp_i);
}

struct ProcessSilence
{
    template<bool IS_24BIT, bool LOOPING>
    int operator()(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf) const
    {
        return fluid_rvoice_dsp_silence_local<LOOPING>(rvoice, dsp_buf);
    }
};

struct InterpolateNone
{
    template<bool IS_24BIT, bool LOOPING>
    int operator()(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf) const
    {
        return fluid_rvoice_dsp_interpolate_none_local<IS_24BIT, LOOPING>(rvoice, dsp_buf);
    }
};

struct InterpolateLinear
{
    template<bool IS_24BIT, bool LOOPING>
    int operator()(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf) const
    {
        return fluid_rvoice_dsp_interpolate_linear_local<IS_24BIT, LOOPING>(rvoice, dsp_buf);
    }
};

struct Interpolate4thOrder
{
    template<bool IS_24BIT, bool LOOPING>
    int operator()(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf) const
    {
        return fluid_rvoice_dsp_interpolate_4th_order_local<IS_24BIT, LOOPING>(rvoice, dsp_buf);
    }
};

struct Interpolate7thOrder
{
    template<bool IS_24BIT, bool LOOPING>
    int operator()(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf) const
    {
        return fluid_rvoice_dsp_interpolate_7th_order_local<IS_24BIT, LOOPING>(rvoice, dsp_buf);
    }
};

template<typename T>
int dsp_invoker(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf, int looping)
{
    T func;
    bool is_24bit = rvoice->dsp.sample->data24 != NULL;

    if (is_24bit)
    {
        if (looping)
        {
            return func.template operator()<true, true>(rvoice, dsp_buf);
        }
        else
        {
            return func.template operator()<true, false>(rvoice, dsp_buf);
        }
    }
    else
    {
        // This case is most common, thanks to templating it will also become the fastest one
        if (looping)
        {
            return func.template operator()<false, true>(rvoice, dsp_buf);
        }
        else
        {
            return func.template operator()<false, false>(rvoice, dsp_buf);
        }
    }
}

extern "C" int
fluid_rvoice_dsp_silence(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf, int looping)
{
    return dsp_invoker<ProcessSilence>(rvoice, dsp_buf, looping);
}

extern "C" int
fluid_rvoice_dsp_interpolate(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf, int looping)
{
    switch (rvoice->dsp.interp_method)
    {
        case FLUID_INTERP_NONE:
            return dsp_invoker<InterpolateNone>(rvoice, dsp_buf, looping);

        case FLUID_INTERP_LINEAR:
            return dsp_invoker<InterpolateLinear>(rvoice, dsp_buf, looping);

        case FLUID_INTERP_4THORDER:
        default:
            return dsp_invoker<Interpolate4thOrder>(rvoice, dsp_buf, looping);

        case FLUID_INTERP_7THORDER:
            return dsp_invoker<Interpolate7thOrder>(rvoice, dsp_buf, looping);
    }
}