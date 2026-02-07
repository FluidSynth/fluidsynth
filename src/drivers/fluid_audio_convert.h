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

#ifndef FLUID_AUDIO_CONVERT_H
#define FLUID_AUDIO_CONVERT_H

/**
 * @file
 * @internal
 *
 * Internal-only declarations for shared audio conversion helpers.
 * Not installed; not part of the public FluidSynth API.
 */

#ifdef __cplusplus

#include <cstdint>
#include <limits>


/* A portable replacement for roundf(), seems it may actually be faster too! */
template<typename T>
T round_clip_to(float x)
{
    if(x >= 0.0f)
    {
        x += 0.5f;

        if(x > std::numeric_limits<T>::max())
        {
            return std::numeric_limits<T>::max();
        }
    }
    else
    {
        x -= 0.5f;

        if(x < std::numeric_limits<T>::lowest())
        {
            return std::numeric_limits<T>::lowest();
        }
    }

    return static_cast<T>(x);
}

extern "C" {
#else
#include <stdint.h>
#endif

    /* --------------------------------------------------------------------------
     * Shared float->integer PCM kernels (internal).
     *
     * These kernels are used by:
     *  - synth integer write path (fluid_synth_write_int.cpp)
     *  - planar->interleaved conversion (fluid_audio_convert.cpp)
     *
     * Not public API. Not installed.
     * -------------------------------------------------------------------------- */

    /* Dither table */
#define DITHER_SIZE 48000
#define DITHER_CHANNELS 2

    extern float rand_table[DITHER_CHANNELS][DITHER_SIZE];

    /* Init dither table */
    void init_dither(void);

    /*
     * Conversion helpers shared by synth-write and driver2 paths.
     *
     * Convert planar float buffers (src[ch][frame]) into interleaved PCM buffers.
     *
     * - dst_stride is in samples (not bytes). Typical interleaved stride is 'channels'.
     * - s24 means 24-bit PCM stored in 32-bit signed containers (24-in-32).
     *
     * Return FLUID_OK / FLUID_FAILED.
     */

    int fluid_audio_planar_float_to_s16(int16_t *dst_interleaved,
                                        int dst_stride,
                                        const float *const *src_planar,
                                        int channels,
                                        int frames);

    int fluid_audio_planar_float_to_s24(int32_t *dst_interleaved,
                                        int dst_stride,
                                        const float *const *src_planar,
                                        int channels,
                                        int frames);

    int fluid_audio_planar_float_to_s32(int32_t *dst_interleaved,
                                        int dst_stride,
                                        const float *const *src_planar,
                                        int channels,
                                        int frames);

    void fluid_synth_dither_s16(int *dither_index, int len, const float *lin, const float *rin,
                                int16_t *lout, int loff, int lincr,
                                int16_t *rout, int roff, int rincr);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FLUID_AUDIO_CONVERT_H */
