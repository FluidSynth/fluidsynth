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

/**
 * @file
 * @internal
 *
 * Internal C/C++ bridge for integer output rendering.
 * Not installed. Not part of the public API.
 */
#ifndef FLUID_SYNTH_WRITE_INT_H
#define FLUID_SYNTH_WRITE_INT_H

#include "fluid_synth.h" /* fluid_synth_t */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Stereo (L/R) entrypoints
 */

/*
 * Synthesize a block of 16-bit audio samples to audio buffers.
 */
int fluid_synth_write_s16_cpp(fluid_synth_t *synth, int len,
                              void *lout, int loff, int lincr,
                              void *rout, int roff, int rincr);

/*
 * Synthesize a block of 24-bit audio samples to audio buffers.
 * Output type is int32_t with 24 valid bits left-aligned (low 8 bits zero).
 */
int fluid_synth_write_s24_cpp(fluid_synth_t *synth, int len,
                              void *lout, int loff, int lincr,
                              void *rout, int roff, int rincr);

/*
 * Synthesize a block of 32-bit audio samples to audio buffers.
 */
int fluid_synth_write_s32_cpp(fluid_synth_t *synth, int len,
                              void *lout, int loff, int lincr,
                              void *rout, int roff, int rincr);

/*
 * Multi-channel entrypoints
 * The output buffers are interleaved per channel, e.g.
 * for stereo: L0, R0, L1, R1, L2, R2, ...
 */

/*
 * Synthesize a block of 16-bit audio sample channels to audio buffers.
 */
int fluid_synth_write_s16_channels_cpp(fluid_synth_t *synth,
                                       int len, int channels_count,
                                       void *channels_out[], int channels_off[],
                                       int channels_incr[]);

/*
 * Synthesize a block of 24-bit audio sample channels to audio buffers.
 * Output type is int32_t with 24 valid bits left-aligned (low 8 bits zero).
 */
int fluid_synth_write_s24_channels_cpp(fluid_synth_t *synth,
                                       int len, int channels_count,
                                       void *channels_out[], int channels_off[],
                                       int channels_incr[]);

/*
 * Synthesize a block of 32-bit audio sample channels to audio buffers.
 */
int fluid_synth_write_s32_channels_cpp(fluid_synth_t *synth,
                                       int len, int channels_count,
                                       void *channels_out[], int channels_off[],
                                       int channels_incr[]);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FLUID_SYNTH_WRITE_INT_H */
