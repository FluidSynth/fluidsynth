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
 * Internal audio conversion helpers.
 *
 * This module provides shared conversion routines for translating
 * planar float audio buffers into interleaved PCM formats.
 * These helpers centralize float-to-PCM conversion so synth-write and
 * driver2 paths share identical conversion logic.
 *
 * Guardrail:
 *  - Not part of the public FluidSynth API.
 *  - Intended for use by audio drivers and synth-write paths only.
 *  - Do not include from application code.
 */

#include "drivers/fluid_audio_convert.h"
#include "fluid_sys.h"
#include <cstdlib> /* rand(), RAND_MAX */

/* --------------------------------------------------------------------------
 * Shared kernels storage/initialization (single definition)
 * -------------------------------------------------------------------------- */

extern "C" {

/* Definition (storage) for the shared dither table. */
float rand_table[DITHER_CHANNELS][DITHER_SIZE];

void init_dither(void)
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

} /* extern "C" */

static int fluid_audio_convert_validate_args(const char *func_name,
                                             const void *dst_interleaved,
                                             int dst_stride,
                                             const float *const *src_planar,
                                             int channels,
                                             int frames)
{
    /* Log at most once per call: return on first failing condition. */

    if (dst_interleaved == NULL)
    {
        FLUID_LOG(FLUID_ERR, "%s: dst_interleaved is NULL", func_name);
        return FLUID_FAILED;
    }

    if (src_planar == NULL)
    {
        FLUID_LOG(FLUID_ERR, "%s: src_planar is NULL", func_name);
        return FLUID_FAILED;
    }

    if (channels <= 0)
    {
        FLUID_LOG(FLUID_ERR, "%s: invalid channels=%d (must be > 0)", func_name, channels);
        return FLUID_FAILED;
    }

    if (frames < 0)
    {
        FLUID_LOG(FLUID_ERR, "%s: invalid frames=%d (must be >= 0)", func_name, frames);
        return FLUID_FAILED;
    }

    /* Typical interleaved stride is 'channels'. Contract: at least channels (and thus > 0). */
    if (dst_stride < channels)
    {
        FLUID_LOG(FLUID_ERR, "%s: invalid dst_stride=%d (must be >= channels=%d)", func_name, dst_stride, channels);
        return FLUID_FAILED;
    }

    for (int ch = 0; ch < channels; ch++)
    {
        if (src_planar[ch] == NULL)
        {
            FLUID_LOG(FLUID_ERR, "%s: src_planar[%d] is NULL", func_name, ch);
            return FLUID_FAILED;
        }
    }

    return FLUID_OK;
}

extern "C" int fluid_audio_planar_float_to_s16(int16_t *dst_interleaved,
                                               int dst_stride,
                                               const float *const *src_planar,
                                               int channels,
                                               int frames)
{
    if (fluid_audio_convert_validate_args(
        "fluid_audio_planar_float_to_s16", dst_interleaved, dst_stride, src_planar, channels, frames) != FLUID_OK)
    {
        return FLUID_FAILED;
    }

    /* Return semantics: conversion of 0 frames is a valid no-op. */
    if (frames == 0)
    {
        return FLUID_OK;
    }

    int di = 0;

    for (int f = 0; f < frames; ++f)
    {
        /* One interleaved frame: write channel samples then advance by stride. */
        for (int ch = 0; ch < channels; ++ch)
        {
            dst_interleaved[ch] = round_clip_to_i16(src_planar[ch][f] * 32766.0f + rand_table[ch & 1][di]);
        }

        if (++di >= DITHER_SIZE)
        {
            di = 0;
        }

        dst_interleaved += dst_stride;
    }

    return FLUID_OK;
}

extern "C" int fluid_audio_planar_float_to_s24(int32_t *dst_interleaved,
                                               int dst_stride,
                                               const float *const *src_planar,
                                               int channels,
                                               int frames)
{
    if (fluid_audio_convert_validate_args(
        "fluid_audio_planar_float_to_s24", dst_interleaved, dst_stride, src_planar, channels, frames) != FLUID_OK)
    {
        return FLUID_FAILED;
    }

    /* Return semantics: conversion of 0 frames is a valid no-op. */
    if (frames == 0)
    {
        return FLUID_OK;
    }

    for (int f = 0; f < frames; ++f)
    {
        for (int ch = 0; ch < channels; ++ch)
        {
            int32_t s = round_clip_to_i32(src_planar[ch][f] * 2147483646.0f);
            dst_interleaved[ch] = (int32_t)(s & 0xFFFFFF00);
        }

        dst_interleaved += dst_stride;
    }

    return FLUID_OK;
}

extern "C" int fluid_audio_planar_float_to_s32(int32_t *dst_interleaved,
                                               int dst_stride,
                                               const float *const *src_planar,
                                               int channels,
                                               int frames)
{
    if (fluid_audio_convert_validate_args(
        "fluid_audio_planar_float_to_s32", dst_interleaved, dst_stride, src_planar, channels, frames) != FLUID_OK)
    {
        return FLUID_FAILED;
    }

    /* Return semantics: conversion of 0 frames is a valid no-op. */
    if (frames == 0)
    {
        return FLUID_OK;
    }

    for (int f = 0; f < frames; ++f)
    {
        for (int ch = 0; ch < channels; ++ch)
        {
            dst_interleaved[ch] = round_clip_to_i32(src_planar[ch][f] * 2147483646.0f);
        }

        dst_interleaved += dst_stride;
    }

    return FLUID_OK;
}
