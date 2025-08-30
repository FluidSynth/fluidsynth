#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include "fluid_chan.h"

/*
 * Test XG bank selection behavior with different bank MSB values
 */
int main(void)
{
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    fluid_channel_t *chan;
    int id;
    int sfont, bank, prog;

    settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    /* Set XG bank style */
    TEST_SUCCESS(fluid_settings_setstr(settings, "synth.midi-bank-select", "xg"));

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    /* Load the test soundfont */
    TEST_ASSERT(fluid_is_soundfont(TEST_SOUNDFONT) == TRUE);
    TEST_SUCCESS(id = fluid_synth_sfload(synth, TEST_SOUNDFONT, 1));

    /* Get channel 9 (standard drum channel) */
    chan = synth->channel[9];
    TEST_ASSERT(chan != NULL);

    /* Test 1: Set bank MSB to 127 (XG drum bank) - should fallback to 128 
     * since VintageDreamsWaves is not an XG SoundFont */
    TEST_SUCCESS(fluid_synth_cc(synth, 9, 0, 127));  /* Bank MSB = 127 */
    TEST_SUCCESS(fluid_synth_program_change(synth, 9, 0));

    /* Check that channel type is set to drum */
    TEST_ASSERT(chan->channel_type == CHANNEL_TYPE_DRUM);

    /* Get the bank program info - should fallback to bank 128 */
    fluid_channel_get_sfont_bank_prog(chan, &sfont, &bank, &prog);
    TEST_ASSERT(bank == 128);  /* Should fallback to standard drum bank */

    /* Test 2: Set bank MSB to 120 (another XG drum bank) */
    TEST_SUCCESS(fluid_synth_cc(synth, 9, 0, 120));  /* Bank MSB = 120 */
    TEST_SUCCESS(fluid_synth_program_change(synth, 9, 1));

    /* Should still fallback to bank 128 for non-XG SoundFont */
    fluid_channel_get_sfont_bank_prog(chan, &sfont, &bank, &prog);
    TEST_ASSERT(bank == 128);

    /* Test 3: Set bank MSB to something non-XG (like 1) on a melodic channel */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, 0, 1));    /* Bank MSB = 1 on channel 0 */
    TEST_SUCCESS(fluid_synth_program_change(synth, 0, 0));

    chan = synth->channel[0];
    TEST_ASSERT(chan->channel_type == CHANNEL_TYPE_MELODIC);

    /* For melodic channels in XG mode, bank MSB should be ignored */
    fluid_channel_get_sfont_bank_prog(chan, &sfont, &bank, &prog);
    /* Bank should remain 0 since XG ignores MSB for melodic channels */
    TEST_ASSERT(bank == 0);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}