/******************************************************************************
 * FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2025  Neoharp development team
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
 *
 */

#ifndef _FLUID_LIMITER_IMPL_H
#define _FLUID_LIMITER_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

void fluid_limiter_impl_set_sample_rate(fluid_limiter_t*lim, fluid_real_t sample_rate, unsigned int block_size);
fluid_limiter_t* fluid_limiter_impl_new(fluid_real_t sample_rate, fluid_limiter_settings_t* settings, unsigned int block_size);
void fluid_limiter_impl_delete(fluid_limiter_t* lim);
void fluid_limiter_impl_process_buffers(fluid_limiter_t* lim, fluid_real_t* bufs[FLUID_LIMITER_NUM_CHANNELS_AT_ONCE], unsigned int block_size);

#ifdef __cplusplus
}
#endif

#endif /* _FLUID_LIMITER_IMPL_H */
