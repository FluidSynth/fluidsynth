/*
 * FluidSynth - A Software Synthesizer
 *
 * Lighytweight s24 render identity test (float-oracle).
 *
 * Verifies that fluid_synth_write_s24() matches a local reference conversion
 * computed from fluid_synth_write_float() using the same s32 scale + round + clip + mask
 * semantics as the s24 renderer. No golden EXPECTED buffer yet.
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
#if defined(__MINGW32__)
#define S24_DELTA_TOLERANCE 256 /* 1 LSB in 24-bit container for x87 precision drift */
#else
#define S24_DELTA_TOLERANCE 0
#endif

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

    /* Convert float oracle -> expected s24. */
    float_to_s24_ref(out_f, exp_s24, 2 * len);

    /* Render s24. Interleaved stereo. */
    TEST_SUCCESS(fluid_synth_write_s24(synth_s24, len, out_s24, 0, 2, out_s24, 1, 2));

    /* Compare. */
    for (i = 0; i < 2 * len; ++i)
    {
        if (out_s24[i] != exp_s24[i])
        {
            const int64_t delta = (int64_t)out_s24[i] - (int64_t)exp_s24[i];
            const int64_t abs_delta = llabs(delta);
            if (abs_delta <= S24_DELTA_TOLERANCE)
            {
                continue;
            }
            fprintf(stderr,
                    "s24 mismatch @%d (interleaved index): exp=%d got=%d delta=%lld\n",
                    i,
                    (int)exp_s24[i],
                    (int)out_s24[i],
                    (long long)delta);
            TEST_ASSERT(0);
        }

        if ((((uint32_t)out_s24[i]) & 0xFFu) != 0u)
        {
            fprintf(stderr, "s24 low-byte nonzero @%d: got=0x%08X\n", i, (unsigned)((uint32_t)out_s24[i]));
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
