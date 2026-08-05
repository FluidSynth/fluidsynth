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

#pragma once

#include "fluid_sys.h"
#include <array>
#include <cmath>

/**
 * Normalized windowed-sinc interpolation kernel of order SINC_ORDER.
 *
 * The caller fills an array s[] of SINC_ORDER samples where:
 *   s[j] == waveform[ dsp_phase_index + (j - half) ]
 *   half == SINC_ORDER / 2
 *
 * @param s  Input sample array of length SINC_ORDER.
 * @param x  Fractional phase in [0, 1].  This is the raw fractional part of
 *           dsp_phase *after* the +0.5-sample advance applied by
 *           fluid_rvoice_dsp_interpolate_sinc_local().  The true playback
 *           position within s[] is therefore:
 *
 *               center = half + (x - 0.5)  =  half - 0.5 + x
 *
 *           When x == 0.5 the center falls exactly on s[half] (for odd orders only), so
 *           fluid_interp_sinc_kernel returns s[half] unmodified (sinc(0) = 1,
 *           sinc(n*pi) = 0 for all non-zero integers n).
 */
template<int SINC_ORDER>
static inline fluid_real_t fluid_interp_sinc_kernel(const std::array<fluid_real_t, SINC_ORDER>& s, fluid_real_t x)
{
    static_assert(SINC_ORDER >= 1 && SINC_ORDER != 2, "SINC_ORDER must be at least 1 and not equal to 2");

    constexpr int half = SINC_ORDER / 2;

    const auto center = (SINC_ORDER % 2 == 0)
        ? (fluid_real_t)(half - 1) + x // == half + (x - 1)
        : (fluid_real_t)half - 0.5f + x; // == half + (x - 0.5f)


    std::array<fluid_real_t, SINC_ORDER> coeffs;
    fluid_real_t sum = 0.0f;

    for(int i = 0; i < SINC_ORDER; i++)
    {
        fluid_real_t v;
        const fluid_real_t i_shifted = (fluid_real_t)i - center;
        const fluid_real_t arg = FLUID_M_PI * i_shifted;
        if(std::fabs(arg) > 1e-6f)
        {
            v = std::sin(arg) / arg;
            /* Hanning window: */
            // 0.5f * (1.0f + std::cos(arg * (fluid_real_t)(2.0 / SINC_ORDER))) == cos²(arg / SINC_ORDER)
            const fluid_real_t wnd = std::cos(arg / SINC_ORDER);
            v *= wnd * wnd;
        }
        else
        {
            v = 1.0f;
        }

        coeffs[i] = v;
        sum += v;
    }

    /* Normalize so coefficients always sum to 1.0, preventing amplitude
     * modulation artifacts (harmonic distortion) as fractional phase varies. */
    for(int i = 0; i < SINC_ORDER; i++) coeffs[i] /= sum;

    fluid_real_t result = 0.0f;
    for(int i = 0; i < SINC_ORDER; i++) result += coeffs[i] * s[i];
    return result;
}
