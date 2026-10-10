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

#define __STDCPP_WANT_MATH_SPEC_FUNCS__
#include "fluid_sys.h"
#include <array>
#include <cmath>

/* 0 = Hann window (default)
 * 1 = Custom Kaiser window (sfizz-derived, uses std::cyl_bessel_i)
 * 2 = Signalsmith Kaiser window (requires signalsmith-dsp headers)
 *
 * In case of 2 adding
    target_include_directories ( libfluidsynth-OBJ PRIVATE
        $<TARGET_PROPERTY:signalsmith-dsp,INTERFACE_INCLUDE_DIRECTORIES> )
 * to src/CMakeLists.txt will be required
 */
#define USE_KAISER_WINDOW 0

#if USE_KAISER_WINDOW == 1
// Code borrowed from sfizz: https://github.com/sfztools/sfizz/blob/f5c6e29f23b8057867c08e88f5f6ac6738baa30b/src/sfizz/Interpolators.hpp#L146-L157
constexpr fluid_real_t get_beta(int sinc_order)
{
    constexpr size_t PointsMin = 6;
    constexpr size_t PointsMax = 32;

    /* Kaiser shape parameter. beta ~5.658 targets ~60 dB stopband attenuation,
     * a good compromise for short kernels: deeper stopbands can't be realized
     * with few taps and only widen the transition band (dulling the passband). */
    constexpr double BetaMin = 5.658;
    constexpr double BetaMax = 10.0;

    sinc_order = sinc_order < PointsMin ? PointsMin : sinc_order;

    return BetaMin + (BetaMax - BetaMin) * (double(sinc_order - PointsMin) / double(PointsMax - PointsMin));
}

template<int SINC_ORDER>
static inline fluid_real_t kaiser(fluid_real_t i_shifted)
{
    constexpr fluid_real_t beta = get_beta(SINC_ORDER);

    /* Precompute 1 / I0(beta) once per call (normalizes the Kaiser window peak to 1). */
    static const fluid_real_t inv_i0_beta = (fluid_real_t)1.0 / std::cyl_bessel_i(0.f, beta);

    /* Half-width of the window in "tap" units. The kernel spans SINC_ORDER taps,
     * so the window runs from -N/2 .. +N/2 around the reconstruction point. */
    constexpr fluid_real_t halfD = (fluid_real_t)(SINC_ORDER / 2.0);

    /* Kaiser window, centered on the reconstruction point.
     * w(t) = I0(beta * sqrt(1 - (t/halfN)^2)) / I0(beta),  for |t| <= halfN.
     * ratio can slightly exceed 1 for the outermost tap at some fractional
     * phases; clamp the radicand to zero so we never take sqrt of a negative.
     */
    fluid_real_t ratio = i_shifted / halfD;
    fluid_real_t rad = (fluid_real_t)1.0 - ratio * ratio;
    fluid_real_t w = (rad > 0.0)
                     ? std::cyl_bessel_i(0.f, beta * std::sqrt(rad)) * inv_i0_beta
                     : (fluid_real_t)0.0;

    return w;
}
#endif // USE_KAISER_WINDOW == 1

#if USE_KAISER_WINDOW == 2
#include "signalsmith-dsp/windows.h"

template<int SINC_ORDER>
static inline fluid_real_t kaiser_signalsmith(fluid_real_t i_shifted)
{
    /* Bandwidth scaled to SINC_ORDER, targeting the expected range [8, 72].
     * bw = 4.0 at order 8  → beta ≈ 5.4  (matches sfizz BetaMin ≈ 5.66)
     * bw = 7.0 at order 72 → beta ≈ 9.3  (approaches sfizz BetaMax = 10.0)
     * heuristicOptimal=true further tunes beta for minimal sidelobe energy. */
    constexpr double bw_t = SINC_ORDER <= 8 ? 0.0 : SINC_ORDER >= 72 ? 1.0 : (SINC_ORDER - 8.0) / 64.0;
    constexpr double bw = 4.0 + 3.0 * bw_t;
    static signalsmith::windows::Kaiser w = signalsmith::windows::Kaiser::withBandwidth(bw, /*heuristicOptimal=*/true);

    /* Map tap offset i_shifted ∈ [-halfD, +halfD] to unit ∈ [0, 1].
     * signalsmith Kaiser uses unit = 0.5 for center (window peak),
     * unit = 0 or 1 for the edges. Clamp for the outermost fractional taps. */
    constexpr fluid_real_t halfD = (fluid_real_t)(SINC_ORDER / 2.0);
    const double unit = (double)(i_shifted / halfD) * 0.5 + 0.5;
    const double clamped = unit < 0.0 ? 0.0 : unit > 1.0 ? 1.0 : unit;
    return (fluid_real_t)w(clamped);
}
#endif // USE_KAISER_WINDOW == 2

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
static inline fluid_real_t fluid_interp_sinc_kernel(const std::array<fluid_real_t, SINC_ORDER> &s, fluid_real_t x)
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
#if USE_KAISER_WINDOW == 1
            const fluid_real_t k = kaiser<SINC_ORDER>(i_shifted);
            v *= k;
#elif USE_KAISER_WINDOW == 2
            const fluid_real_t k = kaiser_signalsmith<SINC_ORDER>(i_shifted);
            v *= k;
#else
            /* Hanning window: */
            // 0.5f * (1.0f + std::cos(arg * (fluid_real_t)(2.0 / SINC_ORDER))) == cos²(arg / SINC_ORDER)
            const fluid_real_t hann = std::cos(arg / SINC_ORDER);
            v *= hann * hann;
#endif
        }
        else
        {
            v = 1.0f;
        }

        coeffs[i] = v;
    }

    fluid_real_t result = 0.0f;

    for(int i = 0; i < SINC_ORDER; i++)
    {
        result += coeffs[i] * s[i];
        sum += coeffs[i];
    }

    /* Normalize so coefficients always sum to 1.0, preventing amplitude
     * modulation artifacts (harmonic distortion) as fractional phase varies. */
    return result / sum;
}
