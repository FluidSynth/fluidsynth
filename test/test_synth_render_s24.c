/*
 * FluidSynth - A Software Synthesizer
 *
 * Lightweight s24 render identity test (float-oracle).
 *
 * Verifies that fluid_synth_write_s24() matches a local reference conversion
 * computed from fluid_synth_write_float() using the same s32 scale + round + clip + mask
 * semantics as the s24 renderer. No golden EXPECTED buffer yet.
 *
 * Note: On 32-bit x86 builds without SSE2 (x87 FPU), floating-point evaluation
 * can differ slightly between render paths (excess precision / double rounding),
 * leading to rare, small LSB-level deltas at the quantization boundary. This test
 * tolerates a bounded number of small mismatches on such builds.
 */

#include "test.h"
#include "fluidsynth.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

/* Must match the s32 scale used by the integer renderer (s24 uses s32 scale + mask). */
#define S32_SCALE (2147483646.0f)
#define S24_MASK (0xFFFFFF00u) /* 24 valid bits left-aligned in 32-bit container */

/* Local float->i32 reference conversion: round+clip, no dithering. */
static int32_t round_clip_to_i32_ref(float x)
{
    int64_t i;

    if (x >= 0.0f)
    {
        i = (int64_t)(x + 0.5f);
        if (i > INT32_MAX)
        {
            i = INT32_MAX;
        }
    }
    else
    {
        i = (int64_t)(x - 0.5f);
        if (i < INT32_MIN)
        {
            i = INT32_MIN;
        }
    }

    return (int32_t)i;
}

/* Convert float samples to s24-in-32 (left aligned). */
static void float_to_s24_ref(const float *in, int32_t *out, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        /* s24 is transported as int32 with the lowest 8 bits cleared (left-aligned 24-bit PCM). */
        const int32_t s32 = round_clip_to_i32_ref(in[i] * S32_SCALE);
        out[i] = (int32_t)((uint32_t)s32 & (uint32_t)S24_MASK);
    }
}

static int64_t abs_i64(int64_t x)
{
    return (x < 0) ? -x : x;
}

int main(void)
{
    fluid_settings_t *settings = NULL;
    fluid_synth_t *synth_f = NULL;   /* float oracle synth */
    fluid_synth_t *synth_s24 = NULL; /* s24 render synth (int32 container, low 8 bits zero) */

    /* Enough frames to span multiple internal blocks, but still fast. */
    const int len = 4096;

    float *out_f = NULL;     /* interleaved float: L,R,L,R,... */
    int32_t *out_s24 = NULL; /* interleaved s24 from API (int32 container) */
    int32_t *exp_s24 = NULL; /* interleaved s24 oracle (int32 container) */

    int i;

    settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    /* Make test deterministic. */
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.reverb.active", 0));
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.chorus.active", 0));

    synth_f = new_fluid_synth(settings);
    TEST_ASSERT(synth_f != NULL);

    synth_s24 = new_fluid_synth(settings);
    TEST_ASSERT(synth_s24 != NULL);

    /* Load known test soundfont and select it for all channels (reset=1). */
    TEST_SUCCESS(fluid_synth_sfload(synth_f, TEST_SOUNDFONT, 1));
    TEST_SUCCESS(fluid_synth_sfload(synth_s24, TEST_SOUNDFONT, 1));

    /* Deterministic program + note (apply identically to both synths). */
    TEST_SUCCESS(fluid_synth_program_change(synth_f, 0, 0));
    TEST_SUCCESS(fluid_synth_program_change(synth_s24, 0, 0));

    TEST_SUCCESS(fluid_synth_noteon(synth_f, 0, 60, 100));
    TEST_SUCCESS(fluid_synth_noteon(synth_s24, 0, 60, 100));

    out_f = (float *)malloc(sizeof(float) * 2 * len);
    out_s24 = (int32_t *)malloc(sizeof(int32_t) * 2 * len);
    exp_s24 = (int32_t *)malloc(sizeof(int32_t) * 2 * len);

    TEST_ASSERT(out_f != NULL);
    TEST_ASSERT(out_s24 != NULL);
    TEST_ASSERT(exp_s24 != NULL);

    memset(out_f, 0, sizeof(float) * 2 * len);
    memset(out_s24, 0, sizeof(int32_t) * 2 * len);
    memset(exp_s24, 0, sizeof(int32_t) * 2 * len);

    /* Render float (oracle source). Interleaved stereo. */
    TEST_SUCCESS(fluid_synth_write_float(synth_f, len, out_f, 0, 2, out_f, 1, 2));

    /* Convert float oracle -> expected s24-in-32 */
    float_to_s24_ref(out_f, exp_s24, 2 * len);

    /* Render s24. Interleaved stereo. */
    TEST_SUCCESS(fluid_synth_write_s24(synth_s24, len, out_s24, 0, 2, out_s24, 1, 2));

    /* Tolerances */
    /* x87/excess-precision detection (prefer this over __SSE2__) */
#if (defined(__i386__) || defined(_M_IX86)) && defined(__FLT_EVAL_METHOD__) && (__FLT_EVAL_METHOD__ != 0)
    const int64_t kTol = 256; /* x87 tolerance for s24-in-32 (1 << 8) */
    const int kMaxTol = 16;   /* allow a few borderline samples */
#else
    const int64_t kTol = 0; /* strict everywhere else */
    const int kMaxTol = 0;
#endif

    /* Track mismatches */
    int tolCount = 0;
    int64_t maxAbsDelta = 0;
    int worstIdx = -1;
    int64_t worstDelta = 0;

    /* Iterate over frames */
    for (i = 0; i < 2 * len; ++i)
    {
        /* Check low byte is zero (s24-in-32 format) */
        if ((((uint32_t)out_s24[i]) & 0xFFu) != 0u) /* FAIL: non-zero low byte */
        {
            fprintf(stderr, "s24 low-byte nonzero @%d: got=0x%08X\n", i, (unsigned)((uint32_t)out_s24[i]));

            TEST_ASSERT(0);
        }

        /* Compute delta and compare */
        int64_t delta = (int64_t)out_s24[i] - (int64_t)exp_s24[i];
        int64_t absDelta = abs_i64(delta);

        if (absDelta == 0)
        {
            continue;
        }

        if (absDelta > kTol) /* FAIL: large delta */
        {
            fprintf(stderr,
                    "s24 mismatch @%d (interleaved index): exp=%d got=%d delta=%lld\n",
                    i,
                    (int)exp_s24[i],
                    (int)out_s24[i],
                    (long long)delta);

            TEST_ASSERT(0);
        }

        /* absDelta is non-zero and within tolerance, increment small delta count */
        tolCount++;

        /* Track worst delta */
        if (absDelta > maxAbsDelta)
        {
            maxAbsDelta = absDelta;
            worstIdx = i;
            worstDelta = delta;
        }

        if (tolCount > kMaxTol) /* FAIL: too many small deltas */
        {
            fprintf(stderr,
                    "%d non-zero deltas <= %lld, largest delta = %lld at idx = %d\n",
                    tolCount,
                    (long long)kTol,
                    (long long)worstDelta,
                    worstIdx);

            TEST_ASSERT(0);
        }
    }

    free(out_f);
    free(out_s24);
    free(exp_s24);

    delete_fluid_synth(synth_f);
    delete_fluid_synth(synth_s24);
    delete_fluid_settings(settings);

    return 0;
}
