
#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include <stdint.h>
#include "fluid_voice.h"
#include "fluid_chan.h"
#include "fluid_synth.h"

/* fluid_voice_calculate_pitch is not declared in the header, forward declare it here */
fluid_real_t fluid_voice_calculate_pitch(fluid_voice_t *voice, int key);

/* Test for https://github.com/FluidSynth/fluidsynth/issues/1773
 *
 * The sample fine tune (pitchadj) should NOT influence the pitch calculated
 * by fluid_voice_calculate_pitch(). Between fluidsynth 1.1.0 and 2.5.x, when GEN_SCALETUNE is set to
 * a value other than 100, pitchadj incorrectly affects the result.
 *
 * root_pitch is set to: origpitch * 100 - pitchadj  (i.e. root_pitch_cents - pitchadj)
 *
 * With the current (buggy) code:
 *   pitch = SCALETUNE * (key - root_pitch/100) + root_pitch
 *         = SCALETUNE * (key - (root_pitch_cents - a)/100) + (root_pitch_cents - a)
 *
 * When SCALETUNE == 100, pitchadj cancels out: pitch = key*100
 * When SCALETUNE != 100, pitchadj does NOT cancel out, which is incorrect.
 *
 * The correct formula should exclude pitchadj from the pitch calculation:
 *   pitch = SCALETUNE * (key - root_pitch_cents/100) + root_pitch_cents
 *
 * pitchadj is already accounted for separately in rvoice.dsp.root_pitch_hz.
 */

int main(void)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth = new_fluid_synth(settings);
    fluid_voice_t *voice;
    fluid_sample_t sample;
    int origpitch;
    int key;
    fluid_real_t root_pitch_cents;
    fluid_real_t pitch_no_adj_100, pitch_with_adj_100;
    fluid_real_t pitch_no_adj_50, pitch_with_adj_50;

    /* Zero-initialize to ensure clean state */
    FLUID_MEMSET(&sample, 0, sizeof(sample));

    /* Set up channel with no tuning (the common case) */
    voice = synth->voice[0];
    FLUID_MEMSET(voice->gen, 0, sizeof(voice->gen));
    FLUID_MEMSET(voice->mod, 0, sizeof(voice->mod));
    voice->channel = synth->channel[0];
    voice->sample = &sample;

    /* Use middle C (MIDI key 60) as the root key, and test with key 64 (E4) */
    sample.origpitch = origpitch = 60;       /* sample root key in MIDI note number */
    sample.pitchadj = 220;        /* sample fine tune in cents, i.e. must be correct by 20 cents sharp */
    key = 60;             /* the MIDI key to calculate pitch for */

    /* root_pitch_cents is the root pitch in cents without fine tune */
    root_pitch_cents = origpitch * 100.0f;

    voice->gen[GEN_KEYNUM].val = key;
    voice->gen[GEN_OVERRIDEROOTKEY].val = -1;
    /*
     * Test 1: With default SCALETUNE=100, pitchadj does not affect pitch (it cancels out)
     */
    voice->gen[GEN_SCALETUNE].val = 100;

    /* Force calculate pitch without pitchadj */
    voice->root_pitch = root_pitch_cents;
    pitch_no_adj_100 = fluid_voice_calculate_pitch(voice, key);

    /* Calculate pitch */
    fluid_voice_update_param(voice, GEN_OVERRIDEROOTKEY); /* This sets voice.root_key = root_pitch_cents - pitchadj in faulty fluidsynth */
    pitch_with_adj_100 = fluid_voice_calculate_pitch(voice, key);

    /* With SCALETUNE=100, both should give the same result: key*100 = 6400 */
    FLUID_LOG(FLUID_INFO,
              "SCALETUNE=100: pitch_no_adj=%.4f, pitch_with_adj=%.4f (expected both %.4f)",
              pitch_no_adj_100, pitch_with_adj_100, (fluid_real_t)(key * 100));
    TEST_ASSERT(pitch_no_adj_100 == pitch_with_adj_100);
    TEST_ASSERT(pitch_no_adj_100 == (key * 100.f));

    /*
     * Test 2: With non-default SCALETUNE=50, pitchadj should still NOT affect pitch.
     *         This test demonstrates the bug: the assertion will FAIL with current code.
     */
    voice->gen[GEN_SCALETUNE].val = 50;

    /* Force calculate pitch without pitchadj */
    voice->root_pitch = root_pitch_cents;
    pitch_no_adj_50 = fluid_voice_calculate_pitch(voice, key);
    /* Calculate pitch */
    fluid_voice_update_param(voice, GEN_OVERRIDEROOTKEY); /* This sets voice.root_key = root_pitch_cents - pitchadj in faulty fluidsynth */
    pitch_with_adj_50 = fluid_voice_calculate_pitch(voice, key);

    FLUID_LOG(FLUID_INFO,
              "SCALETUNE=50: pitch_no_adj=%.4f, pitch_with_adj=%.4f (should both be equal, diff %.4f)",
              pitch_no_adj_50, pitch_with_adj_50, pitch_with_adj_50 - pitch_no_adj_50);

    /* This assertion demonstrates the bug: with SCALETUNE != 100,
     * pitchadj incorrectly influences the calculated pitch.
     * The difference is: pitchadj * (SCALETUNE/100 - 1) = 20 * (50/100 - 1) = -10 */
    TEST_ASSERT(pitch_no_adj_50 == pitch_with_adj_50);
    TEST_ASSERT(pitch_no_adj_50 == 6000);

    /*
     * Test 3: With non-default SCALETUNE=50, pitchadj should still NOT affect pitch.
     *         This test demonstrates the bug: the assertion will FAIL with current code.
     */
    voice->gen[GEN_SCALETUNE].val = 50;
    voice->gen[GEN_KEYNUM].val = key = origpitch / 2;

    /* Force calculate pitch without pitchadj */
    voice->root_key = root_pitch_cents;
    pitch_no_adj_50 = fluid_voice_calculate_pitch(voice, key);
    /* Calculate pitch */
    fluid_voice_update_param(voice, GEN_OVERRIDEROOTKEY); /* This sets voice.root_key = root_pitch_cents - pitchadj in faulty fluidsynth */
    pitch_with_adj_50 = fluid_voice_calculate_pitch(voice, key);

    FLUID_LOG(FLUID_INFO,
              "SCALETUNE=50: pitch_no_adj=%.4f, pitch_with_adj=%.4f (should both be equal, diff %.4f)",
              pitch_no_adj_50, pitch_with_adj_50, pitch_with_adj_50 - pitch_no_adj_50);

    TEST_ASSERT(pitch_no_adj_50 == pitch_with_adj_50);
    TEST_ASSERT(pitch_no_adj_50 == 4500);

    /*
     * Test 4
     */
    voice->gen[GEN_SCALETUNE].val = 200;
    voice->root_key = root_pitch_cents;
    pitch_no_adj_50 = fluid_voice_calculate_pitch(voice, key);
    fluid_voice_update_param(voice, GEN_OVERRIDEROOTKEY);
    pitch_with_adj_50 = fluid_voice_calculate_pitch(voice, key);

    FLUID_LOG(FLUID_INFO,
              "SCALETUNE=50: pitch_no_adj=%.4f, pitch_with_adj=%.4f (should both be equal, diff %.4f)",
              pitch_no_adj_50, pitch_with_adj_50, pitch_with_adj_50 - pitch_no_adj_50);
    TEST_ASSERT(pitch_no_adj_50 == pitch_with_adj_50);
    TEST_ASSERT(pitch_no_adj_50 == 0);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    return EXIT_SUCCESS;
}
