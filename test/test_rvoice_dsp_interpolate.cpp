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
#include "rvoice/fluid_rvoice.h"
#include "rvoice/fluid_rvoice_dsp_tables.h"
#include "sfloader/fluid_sfont.h"

#include <cstring>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <limits>

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

/* ----- tolerances (in original sample units, before DSP scaling) -------- */

/* Polynomial interpolators (NONE, LINEAR, 4TH ORDER) are exact for their
 * respective polynomial degrees. Tolerance covers table quantization. */
static const fluid_real_t TOL_POLY = (fluid_real_t)0.01;

/* 7th-order sinc has energy smearing at fractional positions.
 * At integer positions it's more accurate, but fractional positions
 * can deviate by ~0.15% of sample value. */
static const fluid_real_t TOL_SINC_INT = (fluid_real_t)5.0;    /* integer phase */
static const fluid_real_t TOL_SINC_FRAC = (fluid_real_t)15.0;  /* fractional phase */

/* Loop boundary tolerance - larger to account for boundary handling. */
static const fluid_real_t TOL_LOOP = (fluid_real_t)50.0;

/* ----- helpers ---------------------------------------------------------- */

static void setup_rvoice(fluid_rvoice_t* rvoice,
    fluid_sample_t* sample,
    short* data,
    char* data24,
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
    fluid_sample_t* sample,
    short* data,
    double           phase_float,
    double           phase_incr,
    int              loop_start,
    int              loop_end,
    int              has_looped,
    fluid_interp     interp_method)
{
    setup_rvoice(rvoice, sample, data, NULL,
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

    if (diff > tol)
    {
        if (index >= 0)
            fprintf(stderr, "FAIL at index %d: ", index);
        else
            fprintf(stderr, "FAIL: ");
        fprintf(stderr, "dsp_output=%.1f, actual=%.1f, expected=%.1f, diff=%.1f, tol=%.1f\n",
            (double)dsp_output, (double)actual, (double)expected, (double)diff, (double)tol);
        TEST_ASSERT(0);
    }
}

static const char* interp_name(fluid_interp mode)
{
    switch (mode)
    {
    case FLUID_INTERP_NONE:     return "NONE";
    case FLUID_INTERP_LINEAR:   return "LINEAR";
    case FLUID_INTERP_4THORDER: return "4TH_ORDER";
    case FLUID_INTERP_7THORDER: return "7TH_ORDER";
    default: throw std::logic_error("Unknown interpolation mode");
    }
}

static fluid_real_t get_tolerance(fluid_interp mode, bool fractional_phase = false)
{
    if (mode == FLUID_INTERP_7THORDER)
        return fractional_phase ? TOL_SINC_FRAC : TOL_SINC_INT;
    return TOL_POLY;
}

/**
 * Get the number of boundary samples needed by each interpolator.
 * These are samples needed AFTER the current index for interpolation.
 */
static int get_boundary_samples(fluid_interp mode)
{
    switch (mode)
    {
    case FLUID_INTERP_NONE:     return 0;
    case FLUID_INTERP_LINEAR:   return 1;
    case FLUID_INTERP_4THORDER: return 2;
    case FLUID_INTERP_7THORDER: return 3;
    default: throw std::logic_error("Unknown interpolation mode");
    }
}

/**
 * Calculate how many output samples can be safely verified for non-looping playback.
 */
static int safe_verify_count(fluid_interp mode, int sample_end, double phase_start, double phase_incr)
{
    int boundary = get_boundary_samples(mode);
    double max_safe_phase = (double)(sample_end - boundary);

    if (phase_start > max_safe_phase)
		throw std::runtime_error("Phase start exceeds safe limit for verification");

    int count = (int)((max_safe_phase - phase_start) / phase_incr) + 1;
    return count > 0 ? count : 0;
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
    short data[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++)
        data[i] = (short)(i * RAMP_STEP_SMALL);

    /* Test NONE, LINEAR, and 4TH ORDER */
    const fluid_interp modes[] = { FLUID_INTERP_NONE,
                                   FLUID_INTERP_LINEAR,
                                   FLUID_INTERP_4THORDER };

    for (size_t m = 0; m < sizeof(modes) / sizeof(modes[0]); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        fluid_real_t    buf[FLUID_BUFSIZE];
        memset(buf, 0, sizeof(buf));

        setup_rvoice_16bit(&rvoice, &samp, data,
            0.0, 1.0,
            0, SAMPLE_SIZE,
            0, modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, 0);
        TEST_ASSERT(count > 0);

        int verify_count = safe_verify_count(modes[m], SAMPLE_SIZE - 1, 0.0, 1.0);
        if (verify_count > count) verify_count = count;

        for (int i = 0; i < verify_count; i++)
        {
            test_sample_eq(buf[i], (fluid_real_t)data[i], TOL_POLY, i);
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
    short data[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++)
        data[i] = (short)(i * ramp_step);

    const double phase_start = 5.0;
    const double phase_incr = 0.5;

    /* B1: NONE - nearest-neighbour */
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        fluid_real_t    buf[FLUID_BUFSIZE];
        memset(buf, 0, sizeof(buf));

        setup_rvoice_16bit(&rvoice, &samp, data,
            phase_start, phase_incr,
            0, SAMPLE_SIZE,
            0, FLUID_INTERP_NONE);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, 0);
        TEST_ASSERT(count > 0);

        int verify_count = safe_verify_count(FLUID_INTERP_NONE, SAMPLE_SIZE - 1, phase_start, phase_incr);
        if (verify_count > count) verify_count = count;

        for (int i = 0; i < verify_count; i++)
        {
            double phase = phase_start + i * phase_incr;
            int nearest = (int)(phase + 0.5);

            test_sample_eq(buf[i], (fluid_real_t)data[nearest], TOL_POLY, i);
        }
        printf("  NONE (nearest): PASS (%d samples verified)\n", verify_count);
    }

    /* B2: LINEAR and 4TH ORDER - exact linear reproduction */
    const fluid_interp poly_modes[] = { FLUID_INTERP_LINEAR, FLUID_INTERP_4THORDER };

    for (size_t m = 0; m < sizeof(poly_modes) / sizeof(poly_modes[0]); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        fluid_real_t    buf[FLUID_BUFSIZE];
        memset(buf, 0, sizeof(buf));

        setup_rvoice_16bit(&rvoice, &samp, data,
            phase_start, phase_incr,
            0, SAMPLE_SIZE,
            0, poly_modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, 0);
        TEST_ASSERT(count > 0);

        int verify_count = safe_verify_count(poly_modes[m], SAMPLE_SIZE - 1, phase_start, phase_incr);
        if (verify_count > count) verify_count = count;

        for (int i = 0; i < verify_count; i++)
        {
            double phase = phase_start + i * phase_incr;
            fluid_real_t expected = (fluid_real_t)(phase * ramp_step);
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

    short data[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++)
        data[i] = CONST_VAL;

    const fluid_interp modes[] = { FLUID_INTERP_NONE,
                                   FLUID_INTERP_LINEAR,
                                   FLUID_INTERP_4THORDER,
                                   FLUID_INTERP_7THORDER };

    /* C1: Integer phase, phase_incr = 1.0 */
    printf("  C1: Integer phase\n");
    for (size_t m = 0; m < sizeof(modes) / sizeof(modes[0]); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        fluid_real_t    buf[FLUID_BUFSIZE];
        memset(buf, 0, sizeof(buf));

        /* Start a bit into the sample for modes that need history */
        double start = (modes[m] == FLUID_INTERP_7THORDER) ? 3.0 :
            (modes[m] == FLUID_INTERP_4THORDER) ? 1.0 : 0.0;

        setup_rvoice_16bit(&rvoice, &samp, data,
            start, 1.0,
            0, SAMPLE_SIZE,
            0, modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, 0);
        TEST_ASSERT(count > 0);

        int verify_count = safe_verify_count(modes[m], SAMPLE_SIZE - 1, start, 1.0);
        if (verify_count > count) verify_count = count;

        fluid_real_t tol = get_tolerance(modes[m], false);
        for (int i = 0; i < verify_count; i++)
        {
            test_sample_eq(buf[i], (fluid_real_t)CONST_VAL, tol, i);
        }
        printf("    %s: PASS (%d samples)\n", interp_name(modes[m]), verify_count);
    }

    /* C2: Fractional phase */
    printf("  C2: Fractional phase\n");
    for (size_t m = 0; m < sizeof(modes) / sizeof(modes[0]); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        fluid_real_t    buf[FLUID_BUFSIZE];
        memset(buf, 0, sizeof(buf));

        /* Start well into interior for all modes */
        setup_rvoice_16bit(&rvoice, &samp, data,
            10.0, 0.7,
            0, SAMPLE_SIZE,
            0, modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, 0);
        TEST_ASSERT(count > 0);

        int verify_count = safe_verify_count(modes[m], SAMPLE_SIZE - 1, 10.0, 0.7);
        if (verify_count > count) verify_count = count;

        fluid_real_t tol = get_tolerance(modes[m], true);
        for (int i = 0; i < verify_count; i++)
        {
            test_sample_eq(buf[i], (fluid_real_t)CONST_VAL, tol, i);
        }
        printf("    %s: PASS (%d samples)\n", interp_name(modes[m]), verify_count);
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

    short data[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++)
        data[i] = CONST_VAL;

    const double phase_start = (double)(LOOP_END - 4);
    const double phase_incr = 0.6;

    const fluid_interp modes[] = { FLUID_INTERP_NONE,
                                   FLUID_INTERP_LINEAR,
                                   FLUID_INTERP_4THORDER,
                                   FLUID_INTERP_7THORDER };

    const char* sub_names[] = { "first loop crossing (has_looped=0)",
                                "steady-state (has_looped=1)" };

    for (int sub = 0; sub < 2; sub++)
    {
        printf("  D%d: %s\n", sub + 1, sub_names[sub]);

        for (size_t m = 0; m < sizeof(modes) / sizeof(modes[0]); m++)
        {
            fluid_rvoice_t  rvoice;
            fluid_sample_t  samp;
            fluid_real_t    buf[FLUID_BUFSIZE];
            memset(buf, 0, sizeof(buf));

            setup_rvoice_16bit(&rvoice, &samp, data,
                phase_start, phase_incr,
                LOOP_START, LOOP_END,
                sub, modes[m]);

            int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, /*looping=*/1);
            TEST_ASSERT(count == FLUID_BUFSIZE);

            fluid_real_t tol = (modes[m] == FLUID_INTERP_7THORDER) ? TOL_LOOP : TOL_POLY;

            for (int i = 0; i < count; i++)
            {
                test_sample_eq(buf[i], (fluid_real_t)CONST_VAL, tol, i);
            }

            TEST_ASSERT(rvoice.dsp.has_looped == 1);
            printf("    %s: PASS\n", interp_name(modes[m]));
        }
    }
}

/* =========================================================================
 * Test E – Loop-boundary wrap with ramp (LINEAR)
 *
 * Verify that wrap-around samples come from the correct loop positions.
 * ========================================================================= */
extern "C" const fluid_real_t* const sinc_table7;
static void test_E_loop_boundary_ramp_wrap(void)
{
    printf("Test E: Loop boundary wrap (periodic ramp)\n");

    const int loop_len = LOOP_END - LOOP_START;
    const int ramp_step = RAMP_STEP_LOOP;  /* Safe: max = 15 * RAMP_STEP_LOOP = 30000 < 32767 */

    short data[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++)
    {
        int rel = ((i - LOOP_START) % loop_len + loop_len) % loop_len;
        data[i] = (short)(rel * ramp_step);
    }

    const double phase_start = (double)(LOOP_END - 2);
    const double phase_incr = 0.25;

    const fluid_interp modes[] = { FLUID_INTERP_LINEAR,
                                   FLUID_INTERP_4THORDER,FLUID_INTERP_NONE,
                                   FLUID_INTERP_7THORDER };

    for (size_t m = 0; m < sizeof(modes) / sizeof(modes[0]); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        fluid_real_t    buf[FLUID_BUFSIZE];
        memset(buf, 0, sizeof(buf));

        setup_rvoice_16bit(&rvoice, &samp, data,
            phase_start, phase_incr,
            LOOP_START, LOOP_END,
            1, modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, /*looping=*/1);
        TEST_ASSERT(count == FLUID_BUFSIZE);

		fluid_real_t sample_ring_buffer[FLUID_INTERP_HIGHEST + 1];
		fluid_real_t* s = &sample_ring_buffer[FLUID_N_ELEMENTS(sample_ring_buffer) / 2];
        double phase_d = phase_start;
        if(modes[m] == FLUID_INTERP_7THORDER)
        {
            /* The 7th-order DSP adds 0.5 to the phase before computing the
             * index and fractional part (to center the filter on the 4th tap).
             * See: fluid_phase_incr(dsp_phase, 0x80000000)
             * Mirror that here so we use the same index, samples, and table row. */
            phase_d += 0.5f;
        }
        for (int i = 0; i < count; i++)
        {
            int idx = (int)phase_d;
            double frac = phase_d - idx;

            while (idx >= LOOP_END)  idx -= loop_len;
            while (idx < LOOP_START) idx += loop_len;

            /* Helper: get a sample at position idx + k, loop-wrapped */
            auto s = [&](int k) -> fluid_real_t {
                    k += idx;
                    while (k >= LOOP_END)  k -= loop_len;
                    while (k < LOOP_START) k += loop_len;
                    return (fluid_real_t)data[k];
            };

            fluid_real_t expected;
            switch (modes[m])
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
				/* Use precomputed sinc table for 7th-order interpolation */
				const int table_row = (int)(frac * FLUID_INTERP_MAX);
				const fluid_real_t* coeffs = &sinc_table7[table_row * SINC_INTERP_ORDER];
				expected =
					coeffs[0] * s(-3) +
					coeffs[1] * s(-2) +
					coeffs[2] * s(-1) +
					coeffs[3] * s(0) +
					coeffs[4] * s(1) +
					coeffs[5] * s(2) +
					coeffs[6] * s(3);
			}
			break;
            }

            test_sample_eq(buf[i], expected, TOL_POLY, i);

            phase_d += phase_incr;
            while (phase_d >= (double)LOOP_END) phase_d -= loop_len;
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

    short data16[SAMPLE_SIZE];
    char  data24[SAMPLE_SIZE];

    /* Create 24-bit constant sample: (data16 << 8) | (data24 & 0xFF)
     * Use value 0x123456 = 1193046
     * After left-shift by 8 in get_sample24: 0x12345600 = 305419776 */
    const int32_t val24_input = 0x1234;  /* Use smaller value to avoid overflow issues */
    for (int i = 0; i < SAMPLE_SIZE; i++)
    {
        data16[i] = (short)val24_input;
        data24[i] = (char)0x56;
    }

    /* The 24-bit value assembled is: (0x1234 << 8) | 0x56 = 0x123456 */
    const fluid_real_t expected_24bit = (fluid_real_t)0x123456;

    const fluid_interp modes[] = { FLUID_INTERP_NONE,
                                   FLUID_INTERP_LINEAR,
                                   FLUID_INTERP_4THORDER,
                                   FLUID_INTERP_7THORDER };

    for (size_t m = 0; m < sizeof(modes) / sizeof(modes[0]); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        fluid_real_t    buf[FLUID_BUFSIZE];
        memset(buf, 0, sizeof(buf));

        double start = (modes[m] == FLUID_INTERP_7THORDER) ? 5.0 :
            (modes[m] == FLUID_INTERP_4THORDER) ? 2.0 : 1.0;

        setup_rvoice(&rvoice, &samp, data16, data24,
            start, 1.0,
            0, SAMPLE_SIZE - 1,
            0, SAMPLE_SIZE,
            0, modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, 0);
        TEST_ASSERT(count > 0);

        /* For 24-bit, the output is the assembled 24-bit value (no additional scaling
         * beyond what's already in get_sample24 which just assembles the bits) */
        fluid_real_t tol = (modes[m] == FLUID_INTERP_7THORDER) ?
            (fluid_real_t)5000.0 : (fluid_real_t)1.0;

        int verify_count = safe_verify_count(modes[m], SAMPLE_SIZE - 1, start, 1.0);
        if (verify_count > count) verify_count = count;

        for (int i = 0; i < verify_count; i++)
        {
            /* 24-bit output is NOT divided by DSP_SCALE - compare directly */
            fluid_real_t diff = std::abs(buf[i] - expected_24bit);
            if (diff > tol)
            {
                fprintf(stderr, "FAIL at index %d: dsp_output=%.1f, expected=%.1f, diff=%.1f, tol=%.1f\n",
                    i, (double)buf[i], (double)expected_24bit, (double)diff, (double)tol);
                TEST_ASSERT(0);
            }
        }
        printf("  %s: PASS (%d samples)\n", interp_name(modes[m]), verify_count);
    }
}

/* =========================================================================
 * Test G – High playback rate (pitch up)
 * ========================================================================= */
static void test_G_high_playback_rate(void)
{
    printf("Test G: High playback rate (pitch up)\n");

    short data[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++)
        data[i] = CONST_VAL;

    const double rates[] = { 2.0, 3.0, 4.0, 16.0, 128.0, 8192.0 };

    const fluid_interp modes[] = { FLUID_INTERP_NONE,
                                   FLUID_INTERP_LINEAR,
                                   FLUID_INTERP_4THORDER,
                                   FLUID_INTERP_7THORDER };

    for (size_t r = 0; r < sizeof(rates) / sizeof(rates[0]); r++)
    {
        printf("  Rate %.1fx:\n", rates[r]);

        for (size_t m = 0; m < sizeof(modes) / sizeof(modes[0]); m++)
        {
            fluid_rvoice_t  rvoice;
            fluid_sample_t  samp;
            fluid_real_t    buf[FLUID_BUFSIZE];
            memset(buf, 0, sizeof(buf));

            double start = (modes[m] == FLUID_INTERP_7THORDER) ? 3.0 :
                (modes[m] == FLUID_INTERP_4THORDER) ? 1.0 : 0.0;

            setup_rvoice_16bit(&rvoice, &samp, data,
                start, rates[r],
                0, SAMPLE_SIZE,
                0, modes[m]);

            int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, 0);
            TEST_ASSERT(count > 0);

            int verify_count = safe_verify_count(modes[m], SAMPLE_SIZE - 1, start, rates[r]);
            if (verify_count > count) verify_count = count;

            fluid_real_t tol = get_tolerance(modes[m], false);
            for (int i = 0; i < verify_count; i++)
            {
                test_sample_eq(buf[i], (fluid_real_t)CONST_VAL, tol, i);
            }
            printf("    %s: PASS (%d samples)\n", interp_name(modes[m]), count);
        }
    }
}

/* =========================================================================
 * Test H – Short loops
 * ========================================================================= */
static void test_H_short_loops(void)
{
    printf("Test H: Short loops\n");

    short data[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++)
        data[i] = CONST_VAL;

    struct {
        fluid_interp mode;
        int min_loop_len;
    } tests[] = {
        { FLUID_INTERP_NONE,     2 },
        { FLUID_INTERP_LINEAR,   3 },
        { FLUID_INTERP_4THORDER, 5 },
        { FLUID_INTERP_7THORDER, 8 },
    };

    for (size_t t = 0; t < sizeof(tests) / sizeof(tests[0]); t++)
    {
        int loop_start = 10;
        int loop_end = loop_start + tests[t].min_loop_len;

        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        fluid_real_t    buf[FLUID_BUFSIZE];
        memset(buf, 0, sizeof(buf));

        setup_rvoice_16bit(&rvoice, &samp, data,
            (double)loop_start, 0.5,
            loop_start, loop_end,
            1, tests[t].mode);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, /*looping=*/1);
        TEST_ASSERT(count == FLUID_BUFSIZE);

        fluid_real_t tol = (tests[t].mode == FLUID_INTERP_7THORDER) ? TOL_LOOP : TOL_POLY;

        for (int i = 0; i < count; i++)
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

    short data[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++)
        data[i] = (short)(i * RAMP_STEP_SMALL);

    /* I1: Non-looping */
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        fluid_real_t    buf[FLUID_BUFSIZE];

        for (int i = 0; i < FLUID_BUFSIZE; i++)
            buf[i] = 12345.0f;

        setup_rvoice_16bit(&rvoice, &samp, data,
            0.0, 1.0,
            0, SAMPLE_SIZE,
            0, FLUID_INTERP_NONE);

        int count = fluid_rvoice_dsp_silence(&rvoice, buf, /*looping=*/0);
        TEST_ASSERT(count > 0);

        for (int i = 0; i < count; i++)
        {
            TEST_ASSERT(buf[i] == 0.0f);
        }
        printf("  Non-looping: PASS (%d samples)\n", count);
    }

    /* I2: Looping */
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        fluid_real_t    buf[FLUID_BUFSIZE];

        for (int i = 0; i < FLUID_BUFSIZE; i++)
            buf[i] = 12345.0f;

        setup_rvoice_16bit(&rvoice, &samp, data,
            (double)(LOOP_END - 2), 0.5,
            LOOP_START, LOOP_END,
            0, FLUID_INTERP_NONE);

        int count = fluid_rvoice_dsp_silence(&rvoice, buf, /*looping=*/1);
        TEST_ASSERT(count == FLUID_BUFSIZE);

        for (int i = 0; i < count; i++)
        {
            TEST_ASSERT(buf[i] == 0.0f);
        }

        TEST_ASSERT(rvoice.dsp.has_looped == 1);
        printf("  Looping: PASS\n");
    }
}

/* =========================================================================
 * Test J – Phase accumulation over multiple buffers
 * ========================================================================= */
static void test_J_phase_accumulation(void)
{
    printf("Test J: Phase accumulation\n");

    short data[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++)
        data[i] = CONST_VAL;

    const double phase_incr = 0.123456789;

    fluid_rvoice_t  rvoice;
    fluid_sample_t  samp;

    setup_rvoice_16bit(&rvoice, &samp, data,
        (double)LOOP_START, phase_incr,
        LOOP_START, LOOP_END,
        1, FLUID_INTERP_LINEAR);

    for (int iter = 0; iter < 100; iter++)
    {
        fluid_real_t buf[FLUID_BUFSIZE];
        memset(buf, 0, sizeof(buf));

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, /*looping=*/1);
        TEST_ASSERT(count == FLUID_BUFSIZE);

        for (int i = 0; i < count; i++)
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

    short data[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++)
        data[i] = CONST_VAL;

    const fluid_interp modes[] = { FLUID_INTERP_NONE,
                                   FLUID_INTERP_LINEAR,
                                   FLUID_INTERP_4THORDER,
                                   FLUID_INTERP_7THORDER };

    for (size_t m = 0; m < sizeof(modes) / sizeof(modes[0]); m++)
    {
        fluid_rvoice_t  rvoice;
        fluid_sample_t  samp;
        fluid_real_t    buf[FLUID_BUFSIZE];
        memset(buf, 0, sizeof(buf));

        double start = SAMPLE_SIZE - 10.0;

        setup_rvoice_16bit(&rvoice, &samp, data,
            start, 1.0,
            0, SAMPLE_SIZE,
            0, modes[m]);

        int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, /*looping=*/0);

        TEST_ASSERT(count > 0);
        TEST_ASSERT(count < FLUID_BUFSIZE);
        TEST_ASSERT(count <= 12);

        printf("  %s: PASS (returned %d samples from position %.0f)\n",
            interp_name(modes[m]), count, start);
    }
}

/* =========================================================================
 * Test L – 7th order at integer phase positions
 * ========================================================================= */
static void test_L_7th_order_integer_phase(void)
{
    printf("Test L: 7th order at integer phase\n");

    /* Use safe ramp: max = 63 * RAMP_STEP_SMALL = 31500 */
    short data[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++)
        data[i] = (short)(i * RAMP_STEP_SMALL);

    fluid_rvoice_t  rvoice;
    fluid_sample_t  samp;
    fluid_real_t    buf[FLUID_BUFSIZE];
    memset(buf, 0, sizeof(buf));

    setup_rvoice_16bit(&rvoice, &samp, data,
        4.0, 1.0,
        0, SAMPLE_SIZE,
        0, FLUID_INTERP_7THORDER);

    int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, 0);
    TEST_ASSERT(count > 0);

    int verify_count = safe_verify_count(FLUID_INTERP_7THORDER, SAMPLE_SIZE - 1, 4.0, 1.0);
    if (verify_count > count) verify_count = count;

    for (int i = 0; i < verify_count; i++)
    {
        int idx = 4 + i;
        test_sample_eq(buf[i], (fluid_real_t)data[idx], TOL_SINC_INT, i);
    }
    printf("  Integer phase ramp: PASS (%d samples)\n", verify_count);
}

/* =========================================================================
 * Test M – Verify sample overflow protection
 *
 * Ensure test data stays within short range to catch any future regressions.
 * ========================================================================= */
static void test_M_sample_range_validation(void)
{
    printf("Test M: Sample range validation\n");

    const int SHORT_MAX = std::numeric_limits<short>::max();
    const int SHORT_MIN = std::numeric_limits<short>::min();

    /* Test A, B, L: ramp with step RAMP_STEP_SMALL, max index SAMPLE_SIZE-1 */
    int max_ramp_500 = (SAMPLE_SIZE - 1) * RAMP_STEP_SMALL;
    TEST_ASSERT(max_ramp_500 <= SHORT_MAX);
    TEST_ASSERT(max_ramp_500 >= SHORT_MIN);
    printf("  Ramp step %d (max=%d): OK\n", RAMP_STEP_SMALL, max_ramp_500);

    /* Test C, D, G, H, J, K: constant value */
    TEST_ASSERT(CONST_VAL <= SHORT_MAX);
    TEST_ASSERT(CONST_VAL >= SHORT_MIN);
    printf("  CONST_VAL (%d): OK\n", CONST_VAL);

    /* Test E: periodic loop ramp with step RAMP_STEP_LOOP, max index loop_len-1 */
    int loop_len = LOOP_END - LOOP_START;
    int max_loop_ramp = (loop_len - 1) * RAMP_STEP_LOOP;
    TEST_ASSERT(max_loop_ramp <= SHORT_MAX);
    TEST_ASSERT(max_loop_ramp >= SHORT_MIN);
    printf("  Loop ramp step %d (loop_len=%d, max=%d): OK\n",
        RAMP_STEP_LOOP, loop_len, max_loop_ramp);

    /* Test F: 24-bit MSB stored in short (0x1234 = 4660) */
    TEST_ASSERT(VAL_24BIT_MSB <= SHORT_MAX);
    TEST_ASSERT(VAL_24BIT_MSB >= SHORT_MIN);
    printf("  24-bit MSB (0x%04X = %d): OK\n", VAL_24BIT_MSB, VAL_24BIT_MSB);

    printf("  All sample values within short range: PASS\n");
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

    test_L_7th_order_integer_phase();
    printf("\n");

    test_M_sample_range_validation();
    printf("\n");

    printf("========================================\n");
    printf("All tests PASSED\n");

    return EXIT_SUCCESS;
}
