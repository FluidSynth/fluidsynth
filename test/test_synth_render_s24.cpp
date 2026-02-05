/*
 * FluidSynth - A Software Synthesizer
 *
 * Lightweight s24 render identity test (float-oracle).
 *
 * Verifies that fluid_synth_write_s24() matches a local reference conversion
 * computed from fluid_synth_write_float() using the same scale + round+clip
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

/*
 * Must match the s24 renderer's scale convention in fluid_synth_write_int.cpp.
 *
 * For s24, we quantize to signed 24-bit range and then store as 24-in-32
 * (left-aligned), i.e. final int32 sample has low 8 bits cleared.
 */
#define S24_SCALE (8388606.0f) /* (2^23 - 2) matches the renderer convention */

/* Round+clip to signed 24-bit integer (range [-8388608, 8388607]). */
static int32_t round_clip_to_i24_ref(float x)
{
    int64_t i;

    if (x >= 0.0f)
    {
        i = (int64_t)(x + 0.5f);
        if (i > 8388607)
        {
            i = 8388607;
        }
    }
    else
    {
        i = (int64_t)(x - 0.5f);
        if (i < -8388608)
        {
            i = -8388608;
        }
    }

    return (int32_t)i;
}

/* Convert float samples to s24-in-32 (left aligned): (i24 << 8). */
static void float_to_s24_ref(const float *in, int32_t *out, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        int32_t i24 = round_clip_to_i24_ref(in[i] * S24_SCALE);

        /* Shift in unsigned domain to avoid UB on negative values. */
        uint32_t u = (uint32_t)i24;
        u <<= 8;
        out[i] = (int32_t)u;
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
    fluid_synth_t *synth_s24 = NULL; /* s24 render synth */

    /* Enough frames to span multiple internal blocks, but still fast. */
    const int len = 4096;

    float *out_f = NULL;     /* interleaved float: L,R,L,R,... */
    int32_t *out_s24 = NULL; /* interleaved s24-in-32 from API */
    int32_t *exp_s24 = NULL; /* interleaved s24-in-32 oracle */

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

    /* Deterministic program + note (apply identically to both synths) */
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
#if defined(__i386__) && !defined(__SSE2__)
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

    /* Compare */
    for (i = 0; i < 2 * len; ++i)
    {
        int64_t delta = (int64_t)out_s24[i] - (int64_t)exp_s24[i];
        int64_t ad = abs_i64(delta);

        if (ad > maxAbsDelta)
        {
            maxAbsDelta = ad;
            worstIdx = i;
            worstDelta = delta;
        }

        if (ad == 0)
        {
            continue;
        }

        if (ad > kTol)
        {
            fprintf(stderr, "s24 mismatch @%d: exp=%d got=%d delta=%ld\n", i, (int)exp_s24[i], (int)out_s24[i], (long)delta);
            TEST_ASSERT(0);
        }

        /* ad is non-zero and within tolerance */
        tolCount++;
        if (tolCount > kMaxTol)
        {
            fprintf(stderr, "Too many tolerated mismatches (count=%d), maxAbsDelta=%ld\n", tolCount, (long)maxAbsDelta);
            TEST_ASSERT(0);
        }
    }

#if defined(__i386__) && !defined(__SSE2__)
    if (tolCount > 0)
    {
        fprintf(stderr,
                "x87 tolerated mismatches: count=%d, maxAbsDelta=%ld at idx=%d (delta=%ld)\n",
                tolCount,
                (long)maxAbsDelta,
                worstIdx,
                (long)worstDelta);
    }
#endif

    free(out_f);
    free(out_s24);
    free(exp_s24);

    delete_fluid_synth(synth_f);
    delete_fluid_synth(synth_s24);
    delete_fluid_settings(settings);

    return 0;
}
