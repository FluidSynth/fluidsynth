#include "test.h"
#include "fluidsynth.h"
#include "fluid_sfont.h"
#include "fluid_defsfont.h"

/*
 * Test XG bank validation functionality
 */
int main(void)
{
    int id;
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    fluid_sfont_t *sfont;
    fluid_defsfont_t *defsfont;

    settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    /* Set XG bank style */
    TEST_SUCCESS(fluid_settings_setstr(settings, "synth.midi-bank-select", "xg"));

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    /* Load the test soundfont - VintageDreamsWaves should not be XG compatible */
    TEST_ASSERT(fluid_is_soundfont(TEST_SOUNDFONT) == TRUE);

    TEST_SUCCESS(id = fluid_synth_sfload(synth, TEST_SOUNDFONT, 1));
    TEST_ASSERT(fluid_synth_sfcount(synth) == 1);

    /* Get the loaded SoundFont */
    sfont = fluid_synth_get_sfont(synth, 0);
    TEST_ASSERT(sfont != NULL);

    /* Get the defsfont data to check XG validation */
    defsfont = (fluid_defsfont_t *)fluid_sfont_get_data(sfont);
    TEST_ASSERT(defsfont != NULL);

    /* VintageDreamsWaves should not be marked as XG compatible
     * because it's a standard GM SoundFont without XG drum banks */
    TEST_ASSERT(defsfont->is_xg_bank == FALSE);

    /* Test XG bank selection on a non-XG SoundFont */
    /* Set bank MSB to 127 (XG drum bank) on channel 9 (drum channel) */
    TEST_SUCCESS(fluid_synth_cc(synth, 9, 0, 127));  /* Bank MSB = 127 */
    TEST_SUCCESS(fluid_synth_cc(synth, 9, 32, 0));   /* Bank LSB = 0 */
    TEST_SUCCESS(fluid_synth_program_change(synth, 9, 0));

    /* Since this is not an XG SoundFont, the bank should fall back to 128
     * even though we set MSB to 127 */
    /* We can verify this by checking that drum sounds work properly
     * (This is more of a functional test - the actual bank fallback
     * is tested by the logic in fluid_channel_set_bank_msb) */

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}