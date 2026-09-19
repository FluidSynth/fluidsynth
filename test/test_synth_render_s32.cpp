/*
 * FluidSynth - A Software Synthesizer
 *
 * Lightweight s32 render identity test (float-oracle).
 *
 * Verifies that fluid_synth_write_s32() matches a local reference conversion
 * computed from fluid_synth_write_float() using the same scale + round + clip
 * semantics as the s32 renderer. No golden EXPECTED buffer yet.
 *
 * Note: On 32-bit x86 builds without SSE2 (x87 FPU), floating-point evaluation
 * can differ slightly between render paths (excess precision / double rounding),
 * leading to rare, small LSB-level deltas at the quantization boundary. This test
 * tolerates a bounded number of small mismatches on such builds.
 */

#include "test.h"
#include "fluidsynth.h"
#include "fluid_audio_convert.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cstdio>

/* Must match the s32 renderer's scale convention in fluid_synth_write_int.cpp */
#define S32_SCALE (2147483646.0f)

static void float_to_s32_ref(const float *in, int32_t *out, int count)
{
    int i;

    for(i = 0; i < count; ++i)
    {
        out[i] = round_clip_to<int32_t>(in[i] * S32_SCALE);
    }
}

static int64_t abs_i64(int64_t x)
{
    return (x < 0) ? -x : x;
}

int main(void)
{
    fluid_settings_t *settings = nullptr;
    fluid_synth_t *synth_f = nullptr;   /* float oracle synth */
    fluid_synth_t *synth_s32 = nullptr; /* s32 render synth */

    /* Enough frames to span multiple internal blocks, but still fast. */
    const int len = 4096;

    float *out_f = nullptr;     /* interleaved float: L,R,L,R,... */
    int32_t *out_s32 = nullptr; /* interleaved s32 from API */
    int32_t *exp_s32 = nullptr; /* interleaved s32 oracle */

    int i;

    settings = new_fluid_settings();
    TEST_ASSERT(settings != nullptr);

    /* Make test deterministic. */
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.reverb.active", 0));
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.chorus.active", 0));

    synth_f = new_fluid_synth(settings);
    TEST_ASSERT(synth_f != nullptr);

    synth_s32 = new_fluid_synth(settings);
    TEST_ASSERT(synth_s32 != nullptr);

    /* Load known test soundfont and select it for all channels (reset=1). */
    TEST_SUCCESS(fluid_synth_sfload(synth_f, TEST_SOUNDFONT, 1));
    TEST_SUCCESS(fluid_synth_sfload(synth_s32, TEST_SOUNDFONT, 1));

    /* Deterministic program + note (apply identically to both synths). */
    TEST_SUCCESS(fluid_synth_program_change(synth_f, 0, 0));
    TEST_SUCCESS(fluid_synth_program_change(synth_s32, 0, 0));

    TEST_SUCCESS(fluid_synth_noteon(synth_f, 0, 60, 100));
    TEST_SUCCESS(fluid_synth_noteon(synth_s32, 0, 60, 100));

    out_f = (float *)malloc(sizeof(float) * 2 * len);
    out_s32 = (int32_t *)malloc(sizeof(int32_t) * 2 * len);
    exp_s32 = (int32_t *)malloc(sizeof(int32_t) * 2 * len);

    TEST_ASSERT(out_f != nullptr);
    TEST_ASSERT(out_s32 != nullptr);
    TEST_ASSERT(exp_s32 != nullptr);

    memset(out_f, 0, sizeof(float) * 2 * len);
    memset(out_s32, 0, sizeof(int32_t) * 2 * len);
    memset(exp_s32, 0, sizeof(int32_t) * 2 * len);

    /* Render float (oracle source). Interleaved stereo. */
    TEST_SUCCESS(fluid_synth_write_float(synth_f, len, out_f, 0, 2, out_f, 1, 2));

    /* Convert float oracle -> expected s32. */
    float_to_s32_ref(out_f, exp_s32, 2 * len);

    /* Render s32. Interleaved stereo. */
    TEST_SUCCESS(fluid_synth_write_s32(synth_s32, len, out_s32, 0, 2, out_s32, 1, 2));

    /* Tolerances */
    /* x87/excess-precision detection (prefer this over __SSE2__) */
#if (defined(__i386__) || defined(_M_IX86)) && defined(__FLT_EVAL_METHOD__) && (__FLT_EVAL_METHOD__ != 0)
    const int64_t kTol = 16; /* x87 FPU tolerance for s32 */
    const int kMaxTol = 512; /* allow some borderline samples, 414 observed */
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
    for(i = 0; i < 2 * len; ++i)
    {
        int64_t delta = (int64_t)out_s32[i] - (int64_t)exp_s32[i];
        int64_t absDelta = abs_i64(delta);

        if(absDelta == 0)
        {
            continue;
        }

        if(absDelta > kTol)  /* FAIL: large delta */
        {
            fprintf(stderr,
                    "s32 mismatch @%d (interleaved index): exp=%d got=%d delta=%lld\n",
                    i,
                    (int)exp_s32[i],
                    (int)out_s32[i],
                    (long long)delta);
            TEST_ASSERT(0);
        }

        /* absDelta is non-zero and within tolerance, increment small delta count */
        tolCount++;

        /* Track worst delta */
        if(absDelta > maxAbsDelta)
        {
            maxAbsDelta = absDelta;
            worstIdx = i;
            worstDelta = delta;
        }

        if(tolCount > kMaxTol)  /* FAIL: too many small deltas */
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
    free(out_s32);
    free(exp_s32);

    delete_fluid_synth(synth_f);
    delete_fluid_synth(synth_s32);
    delete_fluid_settings(settings);

    return 0;
}
