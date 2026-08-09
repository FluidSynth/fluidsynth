/* FluidSynth - A Software Synthesizer
 *
 * Unit test for fluid_rvoice_dsp_interpolate()
 *
 * Covers all four interpolation modes:
 *   FLUID_INTERP_NONE      - nearest-neighbour
 *   FLUID_INTERP_LINEAR    - linear (2-point)
 *   FLUID_INTERP_4THORDER  - cubic (4-point Catmull-Rom)
 *   FLUID_INTERP_7THORDER  - 7-point windowed sinc
 *
 * Key implementation details:
 *   - Sample values are left-shifted by 8 bits (both 16-bit and 24-bit)
 *   - Output is therefore scaled by 256 relative to input sample values
 *   - Boundary handling uses pre-loaded wrap-around values
 *   - Phase is stored as fixed-point with 32-bit fractional part
 */

#define NOMINMAX

#include "test.h"
#include "Wave_Sine_093.h"
#include "rvoice/fluid_rvoice.h"
#include "rvoice/fluid_rvoice_dsp_tables.h"
#include "rvoice/fluid_rvoice_dsp_sinc.hpp"
#include "sfloader/fluid_sfont.h"

#include <cstring>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <limits>
#include <array>

/* ----- constants -------------------------------------------------------- */

static const int SAMPLE_SIZE = 64;
static const int LOOP_START = 8;
static const int LOOP_END = 24;
static const short CONST_VAL = 10000;

static const int RAMP_STEP_SMALL = 500;   /* Used in Test A, B, L */
static const int RAMP_STEP_LOOP = 2000;  /* Used in Test E */
static const short VAL_24BIT_MSB = 0x1234; /* Used in Test F */

/* The DSP implementation left-shifts samples by 8 bits (multiplies by 256) */
static const fluid_real_t DSP_SCALE = (fluid_real_t)256.0;

/* Compile-time range checks — sample values must not overflow short */
static_assert((SAMPLE_SIZE - 1) * RAMP_STEP_SMALL <= std::numeric_limits<short>::max(),
              "Ramp (small step) overflows short");
static_assert((LOOP_END - LOOP_START - 1) * RAMP_STEP_LOOP <= std::numeric_limits<short>::max(),
              "Loop ramp overflows short");
static_assert(CONST_VAL <= std::numeric_limits<short>::max(), "CONST_VAL overflows short");
static_assert(VAL_24BIT_MSB <= std::numeric_limits<short>::max(), "24-bit MSB overflows short");

/* ----- tolerances (in original sample units, before DSP scaling) -------- */

/* Polynomial interpolators (NONE, LINEAR, 4TH ORDER) are exact for their
 * respective polynomial degrees. */
static const fluid_real_t TOL_POLY = (fluid_real_t)0.01;

/* 7th-order sinc has energy smearing at fractional positions.
 * At integer positions it's more accurate, but fractional positions
 * can deviate. */
static const fluid_real_t TOL_SINC_INT = (fluid_real_t)2.5;    /* integer phase */
static const fluid_real_t TOL_SINC_FRAC = (fluid_real_t)13.0;  /* fractional phase */

/* ----- helpers ---------------------------------------------------------- */

static void setup_rvoice(fluid_rvoice_t* rvoice,
                         fluid_sample_t *sample,
                         short *data,
                         char *data24,
                         double           phase_float,
                         double           phase_incr,
                         int              start,
                         int              end,
                         int              loop_start,
                         int              loop_end,
                         int              has_looped,
                         fluid_interp     interp_method)
{
    memset(rvoice, 0, sizeof(*rvoice));
    memset(sample, 0, sizeof(*sample));

    sample->data = data;
    sample->data24 = data24;

    rvoice->dsp.sample = sample;
    rvoice->dsp.start = start;
    rvoice->dsp.end = end;
    rvoice->dsp.loopstart = loop_start;
    rvoice->dsp.loopend = loop_end;
    rvoice->dsp.has_looped = (char)has_looped;
    rvoice->dsp.interp_method = interp_method;

    fluid_phase_set_float(rvoice->dsp.phase, phase_float);
    rvoice->dsp.phase_incr = (fluid_real_t)phase_incr;
}

/* Simplified setup for common 16-bit case */
static void setup_rvoice_16bit(fluid_rvoice_t* rvoice,
                               fluid_sample_t *sample,
                               short *data,
                               double           phase_float,
                               double           phase_incr,
                               int              loop_start,
                               int              loop_end,
                               int              has_looped,
                               fluid_interp     interp_method)
{
    setup_rvoice(rvoice, sample, data, nullptr,
                 phase_float, phase_incr,
                 0, SAMPLE_SIZE - 1,
                 loop_start, loop_end,
                 has_looped, interp_method);
}

/**
 * Compare DSP output to expected value.
 * DSP output is scaled by 256 (left-shifted by 8 bits).
 */
static void test_sample_eq(fluid_real_t dsp_output,
                           fluid_real_t expected,
                           fluid_real_t tol,
                           int index = -1)
{
    fluid_real_t actual = dsp_output / DSP_SCALE;
    fluid_real_t diff = std::abs(actual - expected);

    if(diff > tol)
    {
        if(index >= 0)
        {
            fprintf(stderr, "FAIL at index %d: ", index);
        }
        else
        {
            fprintf(stderr, "FAIL: ");
        }

        fprintf(stderr, "dsp_output=%.1f, actual=%.1f, expected=%.1f, diff=%.1f, tol=%.1f\n",
                (double)dsp_output, (double)actual, (double)expected, (double)diff, (double)tol);
        TEST_ASSERT(0);
    }
}

static const char *interp_name(fluid_interp mode)
{
    switch(mode)
    {
    case FLUID_INTERP_NONE:
        return "NONE";

    case FLUID_INTERP_LINEAR:
        return "LINEAR";

    case FLUID_INTERP_4THORDER:
        return "4TH_ORDER";

    case FLUID_INTERP_7THORDER:
        return "7TH_ORDER";

    default:
        throw std::logic_error("Unknown interpolation mode");
    }
}

static fluid_real_t get_tolerance(fluid_interp mode, bool fractional_phase = false)
{
    if(mode == FLUID_INTERP_7THORDER)
    {
        return fractional_phase ? TOL_SINC_FRAC : TOL_SINC_INT;
    }

    return TOL_POLY;
}

/**
 * Calculate how many output samples can be safely interpolated by a single interpolation call for non-looping samples.
 */
static int safe_verify_count(fluid_interp mode, int sample_end, double phase_start, double phase_incr)
{
    if(phase_start > sample_end)
    {
        throw std::runtime_error("Phase start exceeds safe limit for verification");
    }

    int count = (int)((sample_end - phase_start) / phase_incr) + 1;
    fluid_clip(count, 0, FLUID_BUFSIZE);
    return count;

}

/* =========================================================================
 * Test A – Integer-phase passthrough
 *
 * At 1:1 playback speed (phase_incr = 1.0, starting at integer phase),
 * the output must equal the input sample value (times DSP_SCALE).
 * ========================================================================= */
static void test_A_integer_phase_passthrough(void)
{
    printf("Test A: Integer-phase passthrough\n");

    /* Use small values to avoid overflow: max value = 63 * RAMP_STEP_SMALL = 31500 */
    std::array<short, SAMPLE_SIZE> data;

    for(int i = 0; i < SAMPLE_SIZE; i++)
    {
        data[i] = (short)(i * RAMP_STEP_SMALL);
    }

    /* Test all interpolation modes */
    constexpr const std::array<fluid_interp, 4> modes = { FLUID_INTERP_NONE,
                                                          FLUID_INTERP_LINEAR,
                                                          FLUID_INTERP_4THORDER,
                                                          FLUID_INTERP_7THORDER
                                                        };

    for(size_t m = 0; m < FLUID_N_ELEMENTS(modes); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        std::array<fluid_real_t, FLUID_BUFSIZE> buf;
        buf.fill(0);

        /* Start far enough into the sample for modes that need look-behind samples */
        double start = (modes[m] == FLUID_INTERP_7THORDER) ? 4.0 :
                       (modes[m] == FLUID_INTERP_4THORDER) ? 1.0 : 0.0;

        setup_rvoice_16bit(&rvoice, &samp, data.data(),
                           start, 1.0,
                           0, SAMPLE_SIZE,
                           0, modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), 0);
        TEST_ASSERT(count > 0);

        int verify_count = safe_verify_count(modes[m], SAMPLE_SIZE - 1, start, 1.0);
        TEST_ASSERT(count == verify_count);

        fluid_real_t tol = get_tolerance(modes[m], false);

        for(int i = 0; i < verify_count; i++)
        {
            test_sample_eq(buf[i], (fluid_real_t)data[(int)start + i], tol, i);
        }

        printf("  %s: PASS (%d samples verified)\n", interp_name(modes[m]), verify_count);
    }
}

/* =========================================================================
 * Test B – Fractional-phase accuracy
 *
 * B1: NONE uses nearest-neighbor (round to nearest integer index)
 * B2: LINEAR and 4TH ORDER exactly reproduce linear ramps
 *
 * IMPORTANT: Use small ramp values to avoid short overflow!
 * Max short = 32767, so with 64 samples, max step = 32767/63 ≈ 520
 * ========================================================================= */
static void test_B_fractional_linear_ramp(void)
{
    printf("Test B: Fractional-phase linear ramp\n");

    /* Use step of RAMP_STEP_SMALL to stay well within short range: max = 63 * RAMP_STEP_SMALL = 31500 */
    const int ramp_step = RAMP_STEP_SMALL;
    std::array<short, SAMPLE_SIZE> data;

    for(int i = 0; i < SAMPLE_SIZE; i++)
    {
        data[i] = (short)(i * ramp_step);
    }

    const double phase_start = 5.0;
    const double phase_incr = 0.5;

    /* B1: NONE - nearest-neighbour */
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        std::array<fluid_real_t, FLUID_BUFSIZE> buf;
        buf.fill(0);

        setup_rvoice_16bit(&rvoice, &samp, data.data(),
                           phase_start, phase_incr,
                           0, SAMPLE_SIZE,
                           0, FLUID_INTERP_NONE);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), 0);
        TEST_ASSERT(count > 0);

        int verify_count = safe_verify_count(FLUID_INTERP_NONE, SAMPLE_SIZE - 1, phase_start, phase_incr);
        TEST_ASSERT(count == verify_count);

        for(int i = 0; i < verify_count; i++)
        {
            double phase = phase_start + i * phase_incr;
            int nearest = (int)(phase + 0.5);

            test_sample_eq(buf[i], (fluid_real_t)data[nearest], TOL_POLY, i);
        }

        printf("  NONE (nearest): PASS (%d samples verified)\n", verify_count);
    }

    /* B2: LINEAR and 4TH ORDER - exact linear reproduction */
    constexpr const std::array<fluid_interp, 2> poly_modes = { FLUID_INTERP_LINEAR, FLUID_INTERP_4THORDER };

    for(size_t m = 0; m < FLUID_N_ELEMENTS(poly_modes); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        std::array<fluid_real_t, FLUID_BUFSIZE> buf;
        buf.fill(0);

        setup_rvoice_16bit(&rvoice, &samp, data.data(),
                           phase_start, phase_incr,
                           0, SAMPLE_SIZE,
                           0, poly_modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), 0);
        TEST_ASSERT(count > 0);

        int verify_count = safe_verify_count(poly_modes[m], SAMPLE_SIZE - 1, phase_start, phase_incr);
        TEST_ASSERT(count == verify_count);

        for(int i = 0; i < verify_count; i++)
        {
            double phase = phase_start + i * phase_incr;
            auto expected = (fluid_real_t)(phase * ramp_step);
            test_sample_eq(buf[i], expected, TOL_POLY, i);
        }

        printf("  %s: PASS (%d samples verified)\n", interp_name(poly_modes[m]), verify_count);
    }
}

/* =========================================================================
 * Test C – Constant-sample accuracy (DC gain = 1)
 *
 * All interpolators must have unity DC gain: constant input → constant output.
 * ========================================================================= */
static void test_C_constant_sample(void)
{
    printf("Test C: Constant sample (DC gain = 1)\n");

    std::array<short, SAMPLE_SIZE> data;
    data.fill(CONST_VAL);

    constexpr const std::array<fluid_interp, 4> modes = { FLUID_INTERP_NONE,
                                                          FLUID_INTERP_LINEAR,
                                                          FLUID_INTERP_4THORDER,
                                                          FLUID_INTERP_7THORDER
                                                        };

    /* Fractional phase */
    printf("  Fractional phase\n");

    for(size_t m = 0; m < FLUID_N_ELEMENTS(modes); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        std::array<fluid_real_t, FLUID_BUFSIZE> buf;
        buf.fill(0);

        /* Start well into interior for all modes */
        setup_rvoice_16bit(&rvoice, &samp, data.data(),
                           10.0, 0.7,
                           0, SAMPLE_SIZE,
                           0, modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), 0);
        TEST_ASSERT(count > 0);

        int verify_count = safe_verify_count(modes[m], SAMPLE_SIZE - 1, 10.0, 0.7);
        TEST_ASSERT(count == verify_count);

        fluid_real_t tol = get_tolerance(modes[m], true);

        for(int i = 0; i < verify_count; i++)
        {
            test_sample_eq(buf[i], (fluid_real_t)CONST_VAL, tol, i);
        }

        printf("    %s: PASS (%d samples verified)\n", interp_name(modes[m]), verify_count);
    }
}

/* =========================================================================
 * Test D – Loop-boundary correctness with constant sample
 *
 * When looping, the interpolators must fetch wrap-around samples from the
 * loop start when they need samples beyond loop end. The implementation
 * pre-loads boundary values into local variables (end_point1, end_point2).
 * ========================================================================= */
static void test_D_loop_boundary_constant_sample(void)
{
    printf("Test D: Loop boundary (constant sample)\n");

    std::array<short, SAMPLE_SIZE> data;
    data.fill(CONST_VAL);

    const double phase_start = (double)(LOOP_END - 4);
    const double phase_incr = 0.6;

    constexpr const std::array<fluid_interp, 4> modes = { FLUID_INTERP_NONE,
                                                          FLUID_INTERP_LINEAR,
                                                          FLUID_INTERP_4THORDER,
                                                          FLUID_INTERP_7THORDER
                                                        };

    printf("  D1: first loop crossing (has_looped=0)\n");

    for(size_t m = 0; m < FLUID_N_ELEMENTS(modes); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        std::array<fluid_real_t, FLUID_BUFSIZE> buf;
        buf.fill(0);

        setup_rvoice_16bit(&rvoice, &samp, data.data(),
                           phase_start, phase_incr,
                           LOOP_START, LOOP_END,
                           0, modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), /*looping=*/1);
        TEST_ASSERT(count == FLUID_BUFSIZE);

        fluid_real_t tol = get_tolerance(modes[m], true);

        for(int i = 0; i < count; i++)
        {
            test_sample_eq(buf[i], (fluid_real_t)CONST_VAL, tol, i);
        }

        TEST_ASSERT(rvoice.dsp.has_looped == 1);
        printf("    %s: PASS\n", interp_name(modes[m]));
    }
}

/* =========================================================================
 * Test E – Loop-boundary wrap with ramp (LINEAR)
 *
 * Verify that wrap-around samples come from the correct loop positions.
 * ========================================================================= */
extern "C" const fluid_real_t *const sinc_table7;
static void test_E_loop_boundary_ramp_wrap(void)
{
    printf("Test E: Loop boundary wrap (periodic ramp)\n");

    const int loop_len = LOOP_END - LOOP_START;
    const int ramp_step = RAMP_STEP_LOOP;  /* Safe: max = 15 * RAMP_STEP_LOOP = 30000 < 32767 */

    std::array<short, SAMPLE_SIZE> data;

    for(int i = 0; i < SAMPLE_SIZE; i++)
    {
        int rel = ((i - LOOP_START) % loop_len + loop_len) % loop_len;
        data[i] = (short)(rel * ramp_step);
    }

    const double phase_start = (double)(LOOP_END - 2);
    const double phase_incr = 0.25;

    constexpr const std::array<fluid_interp, 4> modes = { FLUID_INTERP_LINEAR,
                                                          FLUID_INTERP_4THORDER, FLUID_INTERP_NONE,
                                                          /*FLUID_INTERP_7THORDER*/
                                                        };

    for(size_t m = 0; m < FLUID_N_ELEMENTS(modes); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        std::array<fluid_real_t, FLUID_BUFSIZE> buf;
        buf.fill(0);

        setup_rvoice_16bit(&rvoice, &samp, data.data(),
                           phase_start, phase_incr,
                           LOOP_START, LOOP_END,
                           1, modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), /*looping=*/1);
        TEST_ASSERT(count == FLUID_BUFSIZE);

        fluid_real_t sample_ring_buffer[FLUID_INTERP_HIGHEST + 1];
        double phase_d = phase_start;

        if(modes[m] == FLUID_INTERP_7THORDER)
        {
            /* The 7th-order DSP adds 0.5 to the phase before computing the
             * index and fractional part (to center the filter on the 4th tap).
             * See: fluid_phase_incr(dsp_phase, 0x80000000)
             * Mirror that here so we use the same index, samples, and table row. */
            phase_d += 0.5f;
        }

        for(int i = 0; i < count; i++)
        {
            int idx = (int)phase_d;
            double frac = phase_d - idx;

            while(idx >= LOOP_END)
            {
                idx -= loop_len;
            }

            while(idx < LOOP_START)
            {
                idx += loop_len;
            }

            /* Helper: get a sample at position idx + k, loop-wrapped */
            auto s = [&](int k) -> fluid_real_t
            {
                k += idx;

                while(k >= LOOP_END)
                {
                    k -= loop_len;
                }

                while(k < LOOP_START)
                {
                    k += loop_len;
                }

                return (fluid_real_t)data[k];
            };

            fluid_real_t expected = 0;

            switch(modes[m])
            {
            case FLUID_INTERP_NONE:
                expected = (frac < 0.5) ? s(0) : s(1);
                break;

            case FLUID_INTERP_LINEAR:
                expected = s(0) + (fluid_real_t)frac * (s(1) - s(0));
                break;

            case FLUID_INTERP_4THORDER:
            {
                /* Compute Catmull-Rom spline coefficients, aka cubic interpolation */
                const fluid_real_t x = frac;
                const fluid_real_t x2 = x * x;
                const fluid_real_t x3 = x2 * x;
                const fluid_real_t halfx = 0.5f * x;

                expected =
                    (-halfx + x2 - 0.5f * x3) * s(-1)
                    + (1.0f - 2.5f * x2 + 1.5f * x3) * s(0)
                    + (halfx + 2.0f * x2 - 1.5f * x3) * s(1)
                    + (-0.5f * x2 + 0.5f * x3) * s(2);
            }
            break;

            case FLUID_INTERP_7THORDER:
            {
#if 0
                /* Use precomputed sinc table for 7th-order interpolation */
                const auto table_row = (int)(frac * FLUID_INTERP_MAX);
                const fluid_real_t *coeffs = &sinc_table7[table_row * SINC_INTERP_ORDER];
                expected =
                    coeffs[0] * s(-3) +
                    coeffs[1] * s(-2) +
                    coeffs[2] * s(-1) +
                    coeffs[3] * s(0) +
                    coeffs[4] * s(1) +
                    coeffs[5] * s(2) +
                    coeffs[6] * s(3);
#else
                double sum = 0;

                for(int j = 0; j < SINC_INTERP_ORDER; j++)
                {
                    double v, j_shifted = (double)j - ((double)(SINC_INTERP_ORDER / 2) - 0.5 + frac);

                    if(fabs(j_shifted) > 0.000001)
                    {
                        double arg = M_PI * j_shifted;
                        v = sin(arg) / arg;
                        /* Hanning window */
                        v *= 0.5 * (1.0 + cos(2.0 * arg / (double)SINC_INTERP_ORDER));
                    }
                    else
                    {
                        v = 1.0;
                    }

                    expected += v * s(j - (SINC_INTERP_ORDER / 2));
                    sum += v;
                }

                expected /= sum;
#endif
            }
            break;
            }

            test_sample_eq(buf[i], expected, TOL_POLY, i);

            phase_d += phase_incr;

            while(phase_d >= (double)LOOP_END)
            {
                phase_d -= loop_len;
            }
        }

        printf("  %s: PASS (%d samples)\n", interp_name(modes[m]), count);
    }
}

/* =========================================================================
 * Test F – 24-bit sample path
 *
 * Verifies that 24-bit samples (16-bit MSB + 8-bit LSB) are correctly
 * combined and interpolated. Both 16-bit and 24-bit paths shift left by 8.
 * ========================================================================= */
static void test_F_24bit_samples(void)
{
    printf("Test F: 24-bit sample path\n");

    std::array<short, SAMPLE_SIZE> data16;
    std::array<char, SAMPLE_SIZE>  data24;

    /* Create 24-bit constant sample: (data16 << 8) | (data24 & 0xFF)
     * Use value 0x123456 = 1193046
     * After left-shift by 8 in get_sample24: 0x12345600 = 305419776 */
    const int32_t val24_input = 0x1234;  /* Use smaller value to avoid overflow issues */
    data16.fill((short)val24_input);
    data24.fill((char)0x56);

    /* The 24-bit value assembled is: (0x1234 << 8) | 0x56 = 0x123456 */
    constexpr const auto expected_24bit = (fluid_real_t)0x123456;

    constexpr const std::array<fluid_interp, 4> modes = { FLUID_INTERP_NONE,
                                                          FLUID_INTERP_LINEAR,
                                                          FLUID_INTERP_4THORDER,
                                                          FLUID_INTERP_7THORDER
                                                        };

    for(size_t m = 0; m < FLUID_N_ELEMENTS(modes); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        std::array<fluid_real_t, FLUID_BUFSIZE> buf;
        buf.fill(0);

        double start = (modes[m] == FLUID_INTERP_7THORDER) ? 5.0 :
                       (modes[m] == FLUID_INTERP_4THORDER) ? 2.0 : 1.0;

        setup_rvoice(&rvoice, &samp, data16.data(), data24.data(),
                     start, 1.0,
                     0, SAMPLE_SIZE - 1,
                     0, SAMPLE_SIZE,
                     0, modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), 0);
        TEST_ASSERT(count > 0);

        /* For 24-bit, the output is the assembled 24-bit value (no additional scaling
         * beyond what's already in get_sample24 which just assembles the bits) */
        fluid_real_t tol = (modes[m] == FLUID_INTERP_7THORDER) ?
                           (fluid_real_t)5000.0 : (fluid_real_t)1.0;

        int verify_count = safe_verify_count(modes[m], SAMPLE_SIZE - 1, start, 1.0);
        TEST_ASSERT(count == verify_count);

        for(int i = 0; i < verify_count; i++)
        {
            /* 24-bit output is NOT divided by DSP_SCALE - compare directly */
            fluid_real_t diff = std::abs(buf[i] - expected_24bit);

            if(diff > tol)
            {
                fprintf(stderr, "FAIL at index %d: dsp_output=%.1f, expected=%.1f, diff=%.1f, tol=%.1f\n",
                        i, (double)buf[i], (double)expected_24bit, (double)diff, (double)tol);
                TEST_ASSERT(0);
            }
        }

        printf("  %s: PASS (%d samples verified)\n", interp_name(modes[m]), verify_count);
    }
}

/* =========================================================================
 * Test G – High playback rate (pitch up)
 * ========================================================================= */
static void test_G_high_playback_rate(void)
{
    printf("Test G: High playback rate (pitch up)\n");

    std::array<short, SAMPLE_SIZE> data;
    data.fill(CONST_VAL);

    constexpr const std::array<double, 6> rates = { 2.0, 3.0, 4.0, 16.0, 128.0, 8192.0 };

    constexpr const std::array<fluid_interp, 4> modes = { FLUID_INTERP_NONE,
                                                          FLUID_INTERP_LINEAR,
                                                          FLUID_INTERP_4THORDER,
                                                          FLUID_INTERP_7THORDER
                                                        };

    /* G1: Non-looping */
    printf("  G1: Non-looping\n");

    for(size_t r = 0; r < FLUID_N_ELEMENTS(rates); r++)
    {
        printf("    Rate %.1fx:\n", rates[r]);

        for(size_t m = 0; m < FLUID_N_ELEMENTS(modes); m++)
        {
            fluid_rvoice_t  rvoice;
            fluid_sample_t  samp;
            std::array<fluid_real_t, FLUID_BUFSIZE> buf;
            buf.fill(0);

            double start = (modes[m] == FLUID_INTERP_7THORDER) ? 3.0 :
                           (modes[m] == FLUID_INTERP_4THORDER) ? 1.0 : 0.0;

            setup_rvoice_16bit(&rvoice, &samp, data.data(),
                               start, rates[r],
                               0, SAMPLE_SIZE,
                               0, modes[m]);

            int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), 0);
            TEST_ASSERT(count > 0);
            TEST_ASSERT(count < FLUID_BUFSIZE);

            int verify_count = safe_verify_count(modes[m], SAMPLE_SIZE - 1, start, rates[r]);
            TEST_ASSERT(count == verify_count);

            fluid_real_t tol = get_tolerance(modes[m], false);

            for(int i = 0; i < verify_count; i++)
            {
                test_sample_eq(buf[i], (fluid_real_t)CONST_VAL, tol, i);
            }

            printf("      %s: PASS (%d samples verified)\n", interp_name(modes[m]), count);
        }
    }

    /* G2: Looping – same rates, but with a loop region (steady-state, has_looped=1).
     * All data is CONST_VAL so the output must be CONST_VAL regardless of how many
     * times the phase wraps around the loop per output sample. */
    printf("  G2: Looping\n");

    for(size_t r = 0; r < FLUID_N_ELEMENTS(rates); r++)
    {
        printf("    Rate %.1fx:\n", rates[r]);

        for(size_t m = 0; m < FLUID_N_ELEMENTS(modes); m++)
        {
            fluid_rvoice_t  rvoice;
            fluid_sample_t  samp;
            std::array<fluid_real_t, FLUID_BUFSIZE> buf;
            buf.fill(0);

            setup_rvoice_16bit(&rvoice, &samp, data.data(),
                               (double)LOOP_START, rates[r],
                               LOOP_START, LOOP_END,
                               1, modes[m]);

            int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), /*looping=*/1);
            TEST_ASSERT(count == FLUID_BUFSIZE);

            fluid_real_t tol = get_tolerance(modes[m], false);

            for(int i = 0; i < count; i++)
            {
                test_sample_eq(buf[i], (fluid_real_t)CONST_VAL, tol, i);
            }

            printf("      %s: PASS (%d samples verified)\n", interp_name(modes[m]), count);
        }
    }
}

/* =========================================================================
 * Test H – Short loops
 * ========================================================================= */
static void test_H_short_loops(void)
{
    printf("Test H: Short loops\n");

    std::array<short, SAMPLE_SIZE> data;
    data.fill(CONST_VAL);

    struct
    {
        fluid_interp mode;
        int min_loop_len;
    } tests[] =
    {
        { FLUID_INTERP_NONE,     2 },
        { FLUID_INTERP_LINEAR,   3 },
        { FLUID_INTERP_4THORDER, 5 },
        { FLUID_INTERP_7THORDER, 8 },
    };

    for(size_t t = 0; t < FLUID_N_ELEMENTS(tests); t++)
    {
        int loop_start = 10;
        int loop_end = loop_start + tests[t].min_loop_len;

        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        std::array<fluid_real_t, FLUID_BUFSIZE> buf;
        buf.fill(0);

        setup_rvoice_16bit(&rvoice, &samp, data.data(),
                           (double)loop_start, 0.5,
                           loop_start, loop_end,
                           1, tests[t].mode);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), /*looping=*/1);
        TEST_ASSERT(count == FLUID_BUFSIZE);

        fluid_real_t tol = get_tolerance(tests[t].mode, true);

        for(int i = 0; i < count; i++)
        {
            test_sample_eq(buf[i], (fluid_real_t)CONST_VAL, tol, i);
        }

        printf("  %s (loop_len=%d): PASS\n",
               interp_name(tests[t].mode), tests[t].min_loop_len);
    }
}

/* =========================================================================
 * Test I – Silence rendering
 * ========================================================================= */
static void test_I_silence_rendering(void)
{
    printf("Test I: Silence rendering\n");

    std::array<short, SAMPLE_SIZE> data;

    for(int i = 0; i < SAMPLE_SIZE; i++)
    {
        data[i] = (short)(i * RAMP_STEP_SMALL);
    }

    /* I1: Non-looping */
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        std::array<fluid_real_t, FLUID_BUFSIZE> buf;
        buf.fill(12345.0f);

        setup_rvoice_16bit(&rvoice, &samp, data.data(),
                           0.0, 1.0,
                           0, SAMPLE_SIZE,
                           0, FLUID_INTERP_NONE);

        int count = fluid_rvoice_dsp_silence(&rvoice, buf.data(), /*looping=*/0);
        TEST_ASSERT(count == SAMPLE_SIZE);

        for(int i = 0; i < count; i++)
        {
            TEST_ASSERT(buf[i] == 0.0f);
        }

        printf("  Non-looping: PASS (%d samples verified)\n", count);
    }

    /* I2: Looping */
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        std::array<fluid_real_t, FLUID_BUFSIZE> buf;
        buf.fill(12345.0f);

        setup_rvoice_16bit(&rvoice, &samp, data.data(),
                           (double)(LOOP_END - 2), 0.5,
                           LOOP_START, LOOP_END,
                           0, FLUID_INTERP_NONE);

        int count = fluid_rvoice_dsp_silence(&rvoice, buf.data(), /*looping=*/1);
        TEST_ASSERT(count == FLUID_BUFSIZE);

        for(int i = 0; i < count; i++)
        {
            TEST_ASSERT(buf[i] == 0.0f);
        }

        TEST_ASSERT(rvoice.dsp.has_looped == 1);
        printf("  Looping: PASS (%d samples verified)\n", count);
    }
}

/* =========================================================================
 * Test J – Phase accumulation over multiple buffers
 * ========================================================================= */
static void test_J_phase_accumulation(void)
{
    printf("Test J: Phase accumulation\n");

    std::array<short, SAMPLE_SIZE> data;
    data.fill(CONST_VAL);

    const double phase_incr = 0.123456789;

    fluid_rvoice_t  rvoice;
    fluid_sample_t  samp;

    setup_rvoice_16bit(&rvoice, &samp, data.data(),
                       (double)LOOP_START, phase_incr,
                       LOOP_START, LOOP_END,
                       1, FLUID_INTERP_LINEAR);

    for(int iter = 0; iter < 100; iter++)
    {
        std::array<fluid_real_t, FLUID_BUFSIZE> buf;
        buf.fill(0);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), /*looping=*/1);
        TEST_ASSERT(count == FLUID_BUFSIZE);

        for(int i = 0; i < count; i++)
        {
            test_sample_eq(buf[i], (fluid_real_t)CONST_VAL, TOL_POLY, i);
        }
    }

    unsigned int phase_idx = fluid_phase_index(rvoice.dsp.phase);
    TEST_ASSERT(phase_idx >= (unsigned)LOOP_START);
    TEST_ASSERT(phase_idx < (unsigned)LOOP_END);

    printf("  100 iterations: PASS (final phase index: %u)\n", phase_idx);
}

/* =========================================================================
 * Test K – Non-looping end-of-sample behavior
 * ========================================================================= */
static void test_K_end_of_sample(void)
{
    printf("Test K: End-of-sample behavior\n");

    std::array<short, SAMPLE_SIZE> data;
    data.fill(CONST_VAL);

    const fluid_interp modes[] = { FLUID_INTERP_NONE,
                                   FLUID_INTERP_LINEAR,
                                   FLUID_INTERP_4THORDER,
                                   FLUID_INTERP_7THORDER
                                 };

    for(size_t m = 0; m < FLUID_N_ELEMENTS(modes); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        std::array<fluid_real_t, FLUID_BUFSIZE> buf;
        buf.fill(0);

        double start = SAMPLE_SIZE - 10.0;

        setup_rvoice_16bit(&rvoice, &samp, data.data(),
                           start, 1.0,
                           0, SAMPLE_SIZE,
                           0, modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), /*looping=*/0);

        TEST_ASSERT(count > 0);
        TEST_ASSERT(count < FLUID_BUFSIZE);
        TEST_ASSERT(count <= 12);

        printf("  %s: PASS (returned %d samples from position %.0f)\n",
               interp_name(modes[m]), count, start);
    }
}

/* =========================================================================
 * Test L – Sinc kernel centering
 *
 * Directly exercises fluid_interp_sinc_kernel<N>() to verify that the center
 * position is computed correctly for both odd and even SINC_ORDERs.
 *
 * Core property exploited:
 *   After the +0.5 phase advance in the DSP loop, x = fract(phase) = 0.5
 *   means the true playback position falls exactly on s[half]... at least for odd orders.
 *   For a unit impulse at s[half] the kernel must therefore return exactly 1.0:
 *     sinc(0) = 1,  sinc(n*pi) = 0  for all non-zero integers n.
 *
 * Sub-tests per order:
 *   L1 – Unit impulse at s[half], x=0.5   →  result == 1.0
 *   L2 – Unit impulse, x=0.0              →  result < 1.0  (sanity: non-trivial)
 *   L3 – Constant (DC) input              →  result == DC for any x  (unity gain)
 *   L4 – Impulse at s[half], sweep x      →  peak is at x=0.5, not elsewhere
 * ========================================================================= */
template<int N>
static void test_L_order()
{
    constexpr int half = N / 2;
    auto get_center = []()
    {
        // A dsp_phase of 0 can be seen as interpolating for the center sample
        double center = 0;

        if(N % 2 == 0)
        {
            // For even orders, there is no true "center" sample, because the center falls in between two samples.
            // The kernel compensates for that by subtracting an additional 0.5 in this case, hence we add 0.5
            // to the center
            center += 0.5;
        }

        // The sinc interpolator adds an unconditional 1/2 sample shift, which we have to account for here as well.
        center += 0.5;
        return (fluid_real_t)center;
    };

    /* L1: unit impulse at s[half], x=0.5 must give 1.0 */
    {
        std::array<fluid_real_t, N> s;
        s.fill(0.0f);
        s[half] = 1.0f;
        fluid_real_t result = fluid_interp_sinc_kernel<N>(s, get_center());
        fluid_real_t diff = std::fabs(result - 1.0f);

        if(diff > (fluid_real_t)1e-5)
        {
            fprintf(stderr, "FAIL L1 (order=%d): impulse@s[%d] x=0.5 -> %.8f (expected 1.0, diff=%.2e)\n",
                    N, half, (double)result, (double)diff);
            TEST_ASSERT(0);
        }

        printf("    L1 (order=%d) impulse@s[half] x=0.5: PASS (result=%.8f)\n", N, (double)result);
    }

    /* L2: same impulse at x=0.0 must give something < 1.0 (non-trivial kernel) */
    if(N > 1)
    {
        std::array<fluid_real_t, N> s;
        s.fill(0.0f);
        s[half] = 1.0f;
        fluid_real_t result = fluid_interp_sinc_kernel<N>(s, 0.0f);

        if(result >= 1.0f - (fluid_real_t)1e-5)
        {
            fprintf(stderr, "FAIL L2 (order=%d): kernel appears trivial at x=0.0 (result=%.8f)\n",
                    N, (double)result);
            TEST_ASSERT(0);
        }

        printf("    L2 (order=%d) impulse@s[half] x=0.0: PASS (result=%.8f, confirms non-trivial)\n",
               N, (double)result);
    }

    /* L3: constant (DC) input must reproduce the constant for all x */
    {
        const fluid_real_t DC = 12345.0f;
        std::array<fluid_real_t, N> s;
        s.fill(DC);
        const float x_vals[] = { 0.0f, 0.1f, 0.25f, 0.5f, 0.75f, 0.9f, 0.999f };

        for(float xv : x_vals)
        {
            fluid_real_t result = fluid_interp_sinc_kernel<N>(s, (fluid_real_t)xv);
            fluid_real_t diff = std::fabs(result - DC);

            if(diff > (fluid_real_t)1e-2 * DC)
            {
                fprintf(stderr, "FAIL L3 (order=%d): DC x=%.3f -> %.1f (expected %.1f, diff=%.2e)\n",
                        N, (double)xv, (double)result, (double)DC, (double)diff);
                TEST_ASSERT(0);
            }
        }

        printf("    L3 (order=%d) DC gain: PASS\n", N);
    }

    /* L4: sweep x in [0, 1) — the peak response to an impulse at s[half]
     * must occur at x=0.5, not at any other tabulated position. */
    {
        std::array<fluid_real_t, N> s;
        s.fill(0.0f);
        s[half] = 1.0f;

        const fluid_real_t peak = fluid_interp_sinc_kernel<N>(s, get_center());
        bool found_higher = false;

        for(int step = 0; step <= 100; step++)
        {
            float xv = step / 100.0f;

            if(xv >= 1.0f)
            {
                xv = 0.999f;
            }

            fluid_real_t v = fluid_interp_sinc_kernel<N>(s, (fluid_real_t)xv);

            if(v > peak + (fluid_real_t)1e-5)
            {
                fprintf(stderr, "FAIL L4 (order=%d): peak not at x=0.5 "
                                "(x=%.3f gives %.8f > peak %.8f)\n",
                        N, (double)xv, (double)v, (double)peak);
                found_higher = true;
            }
        }

        TEST_ASSERT(!found_higher);
        printf("    L4 (order=%d) peak at x=0.5: PASS\n", N);
    }
}

static void test_L_sinc_kernel_centering(void)
{
    printf("Test L: Sinc kernel centering\n");
    printf("  Odd orders:\n");
    test_L_order<1>();
    test_L_order<3>();
    test_L_order<5>();
    test_L_order<7>();
    test_L_order<9>();
    test_L_order<11>();
    test_L_order<13>();
    test_L_order<15>();
    test_L_order<17>();
    test_L_order<19>();

    printf("  Even orders:\n");
    /* N=2 is omitted: with only 2 taps the windowed-sinc degenerates into a
     * near-linear extrapolator whose normalized coefficients overshoot for
     * x > 0.5.  This is a property of the kernel shape (not a centering
     * bug) and N=2 is not a realistic interpolation order. */
    test_L_order<4>();
    test_L_order<6>();
    test_L_order<8>();
    test_L_order<10>();
    test_L_order<12>();
    test_L_order<14>();
    test_L_order<16>();
    test_L_order<18>();
    test_L_order<20>();
}

/**
 * Fit  y(p) = A * sin(2π·p/loop_len + φ)  to samples[loop_start .. loop_start+loop_len-1].
 *
 * Uses the DFT identity for a single frequency:
 *   B = (2/N) Σ y[p] sin(ω·p),  C = (2/N) Σ y[p] cos(ω·p)
 *   A = √(B²+C²),  φ = atan2(C, B)
 *
 * This is an exact LSQ fit when the data contains exactly one period.
 */
static void sine_fit_loop(const short* samples, int loop_start, int loop_len,
                          double *out_A, double *out_phi)
{
    const double omega = 2.0 * M_PI / loop_len;
    double B = 0.0, C = 0.0;

    for(int p = 0; p < loop_len; p++)
    {
        double s = (double)samples[loop_start + p];
        B += s * std::sin(omega * p);
        C += s * std::cos(omega * p);
    }

    B *= 2.0 / loop_len;
    C *= 2.0 / loop_len;
    *out_A   = std::sqrt(B * B + C * C);
    *out_phi = std::atan2(C, B);
}

/* =========================================================================
 * Test M – Sine wave interpolation (Wave_Sine_093.h)
 *
 * The sample data is a real 44 kHz, 16-bit sine wave with a forward loop
 * spanning samples 38–62 inclusive (25 samples = one period).
 *
 * M1: Constant phase_incr = 1.0 (1:1 playback) across 3 buffer calls.
 *     At every integer phase position the DSP output must equal the stored
 *     sample value, exercising loop wrap-around across all four modes.
 *
 * M2: Phase ramp (pitch sweep) — phase_incr steps from 1.0 to 2.0 in 11
 *     calls of 0.1 each, simulating a one-octave upward glide.  Each output
 *     sample is compared against a procedurally computed sine whose
 *     amplitude and phase are determined by a least-squares fit to the loop
 *     region.
 * ========================================================================= */
static void test_M_sine_wave_interpolation(void)
{
    printf("Test M: Sine wave interpolation (Wave_Sine_093)\n");

    /* Audio data from Wave_Sine_093.h (wave_sine_093[71]).
     * Copy to a short[] buffer accepted by the DSP helpers. */
    constexpr int SINE_N        = 71;
    constexpr int SINE_LS       = (int)AUDIO_SAMPLES_LOOP0_START;      /* 38, inclusive */
    constexpr int SINE_LE       = (int)AUDIO_SAMPLES_LOOP0_END + 1;    /* 63, exclusive */
    constexpr int SINE_LOOP_LEN = SINE_LE - SINE_LS;                   /* 25            */

    std::array<short, SINE_N> data;

    for(int i = 0; i < SINE_N; i++)
    {
        data[i] = (short)wave_sine_093[i];
    }

    /* Least-squares sine fit to the loop region (used by M2). */
    double sine_A, sine_phi;
    sine_fit_loop(wave_sine_093, SINE_LS, SINE_LOOP_LEN, &sine_A, &sine_phi);
    const double sine_omega = 2.0 * M_PI / SINE_LOOP_LEN;
    printf("  Fitted sine: A=%.1f  phi=%.4f rad\n", sine_A, sine_phi);

    /* ---- M1: Constant phase_incr = 1.0 --------------------------------- */
    printf("  M1: Constant phase_incr=1.0, 3 buffer iterations (all modes)\n");
    {
        constexpr const std::array<fluid_interp, 4> modes = { FLUID_INTERP_NONE,
                                                              FLUID_INTERP_LINEAR,
                                                              FLUID_INTERP_4THORDER,
                                                              FLUID_INTERP_7THORDER
                                                            };

        for(size_t m = 0; m < FLUID_N_ELEMENTS(modes); m++)
        {
            fluid_rvoice_t rvoice;
            fluid_sample_t samp;

            /* Loop starts at index 38.  The widest stencil (7th-order) needs
             * data[n-3] .. data[n+4].  At n=38 that is data[35..42], all
             * within the 71-element array, so we can start all modes at
             * loop_start without any additional offset. */
            const double start = (double)SINE_LS;

            setup_rvoice(&rvoice, &samp, data.data(), /*data24=*/nullptr,
                         start, /*phase_incr=*/1.0,
                         /*start=*/0, /*end=*/SINE_N - 1,
                         SINE_LS, SINE_LE,
                         /*has_looped=*/1, modes[m]);

            /* At integer-only positions the tolerance for 7th-order is the
             * tighter "integer phase" value; other modes use TOL_POLY. */
            const fluid_real_t tol = get_tolerance(modes[m], /*fractional=*/false);

            double shadow = start;   /* mirrors the DSP phase (integer positions only) */
            int total = 0;

            for(int iter = 0; iter < 3; iter++)
            {
                std::array<fluid_real_t, FLUID_BUFSIZE> buf;
                buf.fill(0);

                int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), /*is_looping=*/1);
                TEST_ASSERT(count == FLUID_BUFSIZE);

                for(int i = 0; i < count; i++)
                {
                    /* shadow is always an integer value; map it into the loop. */
                    int idx = (int)shadow;

                    while(idx >= SINE_LE)
                    {
                        idx -= SINE_LOOP_LEN;
                    }

                    test_sample_eq(buf[i], (fluid_real_t)data[idx], tol, total + i);

                    shadow += 1.0;

                    if(shadow >= SINE_LE)
                    {
                        shadow -= SINE_LOOP_LEN;
                    }
                }

                total += count;
            }

            printf("    %s: PASS (%d samples verified)\n", interp_name(modes[m]), total);
        }
    }

    /* ---- M2: Phase ramp — pitch sweep 1.0x -> 2.0x --------------------- */
    printf("  M2: Phase ramp (phase_incr 1.0 -> 2.0 in 11 buffer calls)\n");
    {
        /* Tolerance budget (in original sample units, before DSP scaling).
         * All modes get a little extra headroom to keep the test robust. */
        struct
        {
            fluid_interp mode;
            fluid_real_t tol;
        } tests[] =
        {
            { FLUID_INTERP_LINEAR, (fluid_real_t)300.0 },
            { FLUID_INTERP_4THORDER, (fluid_real_t)50.0 },
            { FLUID_INTERP_7THORDER, (fluid_real_t)100.0 },
        };

        for(size_t m = 0; m < FLUID_N_ELEMENTS(tests); m++)
        {
            fluid_rvoice_t rvoice;
            fluid_sample_t samp;

            const double start = (double)SINE_LS;

            setup_rvoice(&rvoice, &samp, data.data(), /*data24=*/nullptr,
                         start, /*phase_incr=*/1.0,
                         /*start=*/0, /*end=*/SINE_N - 1,
                         SINE_LS, SINE_LE,
                         /*has_looped=*/1, tests[m].mode);

            /* shadow tracks the exact fractional loop position consumed by
             * each output sample, mirroring the DSP phase accumulator. */
            double shadow = start;
            int total = 0;

            for(int step = 0; step <= 10; step++)
            {
                const double incr = 1.0 + step * 0.1;
                rvoice.dsp.phase_incr = (fluid_real_t)incr;

                std::array<fluid_real_t, FLUID_BUFSIZE> buf;
                buf.fill(0);

                int count = fluid_rvoice_dsp_interpolate(&rvoice, buf.data(), /*is_looping=*/1);
                TEST_ASSERT(count == FLUID_BUFSIZE);

                for(int i = 0; i < count; i++)
                {
                    /* Fractional position within the loop (0-based). */
                    const double loop_pos = shadow - SINE_LS;
                    const double expected = sine_A * std::sin(sine_omega * loop_pos + sine_phi);

                    test_sample_eq(buf[i], (fluid_real_t)expected, tests[m].tol, total + i);

                    /* Advance and wrap shadow to match DSP behaviour. */
                    shadow += incr;

                    if(shadow >= SINE_LE)
                    {
                        shadow -= SINE_LOOP_LEN;
                    }
                }

                total += count;
            }

            printf("    %s: PASS (%d samples verified)\n",
                   interp_name(tests[m].mode), total);
        }
    }
}

/* ========================================================================= */

int main(void)
{
    printf("FluidSynth DSP Interpolation Unit Tests\n");
    printf("========================================\n\n");

    test_A_integer_phase_passthrough();
    printf("\n");

    test_B_fractional_linear_ramp();
    printf("\n");

    test_C_constant_sample();
    printf("\n");

    test_D_loop_boundary_constant_sample();
    printf("\n");

    test_E_loop_boundary_ramp_wrap();
    printf("\n");

    test_F_24bit_samples();
    printf("\n");

    test_G_high_playback_rate();
    printf("\n");

    test_H_short_loops();
    printf("\n");

    test_I_silence_rendering();
    printf("\n");

    test_J_phase_accumulation();
    printf("\n");

    test_K_end_of_sample();
    printf("\n");

    test_L_sinc_kernel_centering();
    printf("\n");

    test_M_sine_wave_interpolation();
    printf("\n");

    printf("========================================\n");
    printf("All tests PASSED\n");

    return EXIT_SUCCESS;
}
