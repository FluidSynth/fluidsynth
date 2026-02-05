/*
 * FluidSynth - A Software Synthesizer
 *
 * Lightweight s32 render identity test (float-oracle).
 *
 * Verifies that fluid_synth_write_s32() matches a local reference conversion
 * computed from fluid_synth_write_float() using the same scale + round+clip
 * semantics as the s32 renderer. No golden EXPECTED buffer yet.
 */

#include "test.h"
#include "fluidsynth.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

/* Must match the s32 renderer's scale convention in fluid_synth_write_int.cpp */
#define S32_SCALE (2147483646.0f)

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

static void float_to_s32_ref(const float *in, int32_t *out, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        out[i] = round_clip_to_i32_ref(in[i] * S32_SCALE);
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
    fluid_synth_t *synth_s32 = NULL; /* s32 render synth */

    /* Enough frames to span multiple internal blocks, but still fast. */
    const int len = 4096;

    float *out_f = NULL;     /* interleaved float: L,R,L,R,... */
    int32_t *out_s32 = NULL; /* interleaved s32 from API */
    int32_t *exp_s32 = NULL; /* interleaved s32 oracle */

    int i;

    settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    /* Make test deterministic. */
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.reverb.active", 0));
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.chorus.active", 0));

    synth_f = new_fluid_synth(settings);
    TEST_ASSERT(synth_f != NULL);

    synth_s32 = new_fluid_synth(settings);
    TEST_ASSERT(synth_s32 != NULL);

    /* Load known test soundfont and select it for all channels (reset=1). */
    TEST_SUCCESS(fluid_synth_sfload(synth_f, TEST_SOUNDFONT, 1));
    TEST_SUCCESS(fluid_synth_sfload(synth_s32, TEST_SOUNDFONT, 1));

    /* Deterministic program + note (apply identically to both synths) */
    TEST_SUCCESS(fluid_synth_program_change(synth_f, 0, 0));
    TEST_SUCCESS(fluid_synth_program_change(synth_s32, 0, 0));

    TEST_SUCCESS(fluid_synth_noteon(synth_f, 0, 60, 100));
    TEST_SUCCESS(fluid_synth_noteon(synth_s32, 0, 60, 100));

    out_f = (float *)malloc(sizeof(float) * 2 * len);
    out_s32 = (int32_t *)malloc(sizeof(int32_t) * 2 * len);
    exp_s32 = (int32_t *)malloc(sizeof(int32_t) * 2 * len);

    TEST_ASSERT(out_f != NULL);
    TEST_ASSERT(out_s32 != NULL);
    TEST_ASSERT(exp_s32 != NULL);

    memset(out_f, 0, sizeof(float) * 2 * len);
    memset(out_s32, 0, sizeof(int32_t) * 2 * len);
    memset(exp_s32, 0, sizeof(int32_t) * 2 * len);

    /* Render float (oracle source). Interleaved stereo. */
    TEST_SUCCESS(fluid_synth_write_float(synth_f, len, out_f, 0, 2, out_f, 1, 2));

    /* Convert float oracle -> expected s32 */
    float_to_s32_ref(out_f, exp_s32, 2 * len);

    /* Render s32. Interleaved stereo. */
    TEST_SUCCESS(fluid_synth_write_s32(synth_s32, len, out_s32, 0, 2, out_s32, 1, 2));

    /* Tolerances */
#if defined(__i386__) && !defined(__SSE2__)
    const int64_t kTol = 8; /* x87 tolerance for s32 */
    const int kMaxTol = 16; /* allow a few borderline samples */
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
        int64_t delta = (int64_t)out_s32[i] - (int64_t)exp_s32[i];
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
            fprintf(stderr, "s32 mismatch @%d: exp=%d got=%d delta=%ld\n", i, (int)exp_s32[i], (int)out_s32[i], (long)delta);
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
    free(out_s32);
    free(exp_s32);

    delete_fluid_synth(synth_f);
    delete_fluid_synth(synth_s32);
    delete_fluid_settings(settings);

    return 0;
}
