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


#ifndef _FLUID_LIMITER_H
#define _FLUID_LIMITER_H

#ifdef LIMITER_SUPPORT

// how many channel buffers to process at once
// maybe if we want to parallelize, we might set this to 1
#define FLUID_LIMITER_NUM_CHANNELS_AT_ONCE 2

typedef struct {
    fluid_real_t input_gain;
    fluid_real_t output_limit;
    fluid_real_t attack_ms;
    fluid_real_t hold_ms;
    fluid_real_t release_ms;
    int smoothing_stages;
    fluid_real_t link_channels;
} fluid_limiter_settings_t;

typedef void fluid_limiter_t;

fluid_limiter_t*
new_fluid_limiter(fluid_real_t sample_rate, fluid_limiter_settings_t* settings);

void delete_fluid_limiter(fluid_limiter_t* lim);

int fluid_limiter_samplerate_change(fluid_limiter_t* lim, fluid_real_t sample_rate);

void
fluid_limiter_run(fluid_limiter_t *lim, fluid_real_t *buf_l, fluid_real_t *buf_r, int block_count);

#endif /* LIMITER_SUPPORT */

#endif /* _FLUID_LIMITER_H */
