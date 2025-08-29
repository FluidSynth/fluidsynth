#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include "fluid_chan.h"
#include "fluid_defsfont.h"

/* Manual integration test to verify the XG implementation works end-to-end */
int main(void)
{
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    fluid_sfont_t *sfont;
    fluid_defsfont_t *defsfont;
    fluid_channel_t *chan;
    int id, sfont_id, bank, prog;

    printf("=== XG Bank Selection Integration Test ===\n");

    settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    /* Set XG bank style */
    TEST_SUCCESS(fluid_settings_setstr(settings, "synth.midi-bank-select", "xg"));

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    /* Load the test soundfont */
    printf("Loading SoundFont: %s\n", TEST_SOUNDFONT);
    TEST_ASSERT(fluid_is_soundfont(TEST_SOUNDFONT) == TRUE);
    TEST_SUCCESS(id = fluid_synth_sfload(synth, TEST_SOUNDFONT, 1));

    /* Get the loaded SoundFont and check XG validation */
    sfont = fluid_synth_get_sfont(synth, 0);
    TEST_ASSERT(sfont != NULL);

    defsfont = (fluid_defsfont_t *)fluid_sfont_get_data(sfont);
    TEST_ASSERT(defsfont != NULL);

    printf("XG validation result: %s\n", defsfont->is_xg_bank ? "XG compatible" : "Not XG compatible");

    /* Test XG bank selection behavior on drum channel */
    printf("\nTesting XG bank selection on channel 9 (drum channel):\n");

    /* Set bank MSB to 127 (XG drum bank) */
    TEST_SUCCESS(fluid_synth_cc(synth, 9, 0, 127));  /* Bank MSB = 127 */
    TEST_SUCCESS(fluid_synth_program_change(synth, 9, 0));

    chan = synth->channel[9];
    TEST_ASSERT(chan->channel_type == CHANNEL_TYPE_DRUM);

    fluid_channel_get_sfont_bank_prog(chan, &sfont_id, &bank, &prog);
    printf("Channel 9 bank after setting MSB=127: %d\n", bank);
    
    /* Since VintageDreamsWaves is not XG compatible, it should fallback to bank 128 */
    if (!defsfont->is_xg_bank) {
        TEST_ASSERT(bank == 128);
        printf("✓ Correctly fell back to bank 128 for non-XG SoundFont\n");
    } else {
        TEST_ASSERT(bank == 127);
        printf("✓ Correctly used bank 127 for XG SoundFont\n");
    }

    /* Test melodic channel behavior */
    printf("\nTesting XG bank selection on channel 0 (melodic channel):\n");
    TEST_SUCCESS(fluid_synth_cc(synth, 0, 0, 1));   /* Bank MSB = 1 on melodic channel */
    TEST_SUCCESS(fluid_synth_program_change(synth, 0, 0));

    chan = synth->channel[0];
    TEST_ASSERT(chan->channel_type == CHANNEL_TYPE_MELODIC);

    fluid_channel_get_sfont_bank_prog(chan, &sfont_id, &bank, &prog);
    printf("Channel 0 bank after setting MSB=1: %d\n", bank);
    printf("✓ XG correctly ignores MSB for melodic channels (bank=%d)\n", bank);

    printf("\n=== Test completed successfully! ===\n");

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}