
#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include "fluid_midi.h"
#include "fluid_chan.h"

/* Test that GM2 bank selection style uses CC32/LSB as bank for melodic channels */
static void test_gm2_bank_lsb_melodic(fluid_synth_t *synth)
{
    int bank = -1;
    int chan = 0;

    /* Force GM2 mode */
    synth->bank_select = FLUID_BANK_STYLE_GM2;

    /* Reset channel to melodic mode */
    fluid_synth_system_reset(synth);

    /* Set bank MSB to 121 (melodic in GM2) - should switch to melodic, MSB ignored */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_MSB, 121));

    /* Set bank LSB to 3 - should set bank to 3 */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_LSB, 3));

    fluid_channel_get_sfont_bank_prog(synth->channel[chan], NULL, &bank, NULL);
    TEST_ASSERT(bank == 3);
}

/* Test that GM2 bank MSB is ignored for bank number on melodic channels */
static void test_gm2_bank_msb_ignored_melodic(fluid_synth_t *synth)
{
    int bank = -1;
    int chan = 0;

    synth->bank_select = FLUID_BANK_STYLE_GM2;
    fluid_synth_system_reset(synth);

    /* Set bank MSB to some melodic value - should be ignored for bank num */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_MSB, 121));

    /* Set bank LSB to 5 */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_LSB, 5));

    fluid_channel_get_sfont_bank_prog(synth->channel[chan], NULL, &bank, NULL);
    /* Bank should be 5 (from LSB only) */
    TEST_ASSERT(bank == 5);
}

/* Test that GM2 bank MSB=120 switches to drum mode */
static void test_gm2_drum_switch(fluid_synth_t *synth)
{
    int bank = -1;
    int chan = 0;

    synth->bank_select = FLUID_BANK_STYLE_GM2;
    fluid_synth_system_reset(synth);

    /* Set MSB to 120 - should switch to drum mode and set bank to 128 */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_MSB, 120));

    TEST_ASSERT(synth->channel[chan]->channel_type == CHANNEL_TYPE_DRUM);

    fluid_channel_get_sfont_bank_prog(synth->channel[chan], NULL, &bank, NULL);
    TEST_ASSERT(bank == 128);
}

/* Test that GM2 drum channels ignore bank LSB */
static void test_gm2_drum_lsb_ignored(fluid_synth_t *synth)
{
    int bank = -1;
    int chan = 0;

    synth->bank_select = FLUID_BANK_STYLE_GM2;
    fluid_synth_system_reset(synth);

    /* Switch to drum via MSB=120 */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_MSB, 120));

    /* Try setting LSB - should be ignored for drum channels */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_LSB, 10));

    fluid_channel_get_sfont_bank_prog(synth->channel[chan], NULL, &bank, NULL);
    /* Bank should still be 128 (drum) */
    TEST_ASSERT(bank == 128);
}

/* Test that GM2 MSB=121 switches back from drum to melodic */
static void test_gm2_drum_to_melodic(fluid_synth_t *synth)
{
    int chan = 0;

    synth->bank_select = FLUID_BANK_STYLE_GM2;
    fluid_synth_system_reset(synth);

    /* Switch to drum */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_MSB, 120));
    TEST_ASSERT(synth->channel[chan]->channel_type == CHANNEL_TYPE_DRUM);

    /* Switch back to melodic via MSB=121 */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_MSB, 121));
    TEST_ASSERT(synth->channel[chan]->channel_type == CHANNEL_TYPE_MELODIC);
}

/* Test that GM2 System ON SysEx sets GM2 bank style (not GM) */
static void test_gm2_sysex_system_on(fluid_synth_t *synth)
{
    /* GM2 System ON: F0 7E 7F 09 03 F7
       In fluid_synth_sysex, data excludes F0 and F7 */
    char gm2_sysex[] = { MIDI_SYSEX_UNIV_NON_REALTIME, MIDI_SYSEX_DEVICE_ID_ALL, MIDI_SYSEX_GM_ID, MIDI_SYSEX_GM2_ON };

    /* First set to GM mode */
    synth->bank_select = FLUID_BANK_STYLE_GM;

    /* Send GM2 System ON */
    TEST_SUCCESS(fluid_synth_sysex(synth, gm2_sysex, sizeof(gm2_sysex), NULL, NULL, NULL, FALSE));

    /* Bank select should now be GM2, not GM */
    TEST_ASSERT(synth->bank_select == FLUID_BANK_STYLE_GM2);
}

/* Test that GM System ON SysEx still sets GM bank style */
static void test_gm_sysex_system_on(fluid_synth_t *synth)
{
    char gm_sysex[] = { MIDI_SYSEX_UNIV_NON_REALTIME, MIDI_SYSEX_DEVICE_ID_ALL, MIDI_SYSEX_GM_ID, MIDI_SYSEX_GM_ON };

    synth->bank_select = FLUID_BANK_STYLE_GS;

    TEST_SUCCESS(fluid_synth_sysex(synth, gm_sysex, sizeof(gm_sysex), NULL, NULL, NULL, FALSE));

    TEST_ASSERT(synth->bank_select == FLUID_BANK_STYLE_GM);
}

/* Test that after GM System ON, GM2 System ON re-enables bank selection */
static void test_gm_then_gm2_reenables_bank_select(fluid_synth_t *synth)
{
    int bank = -1;
    int chan = 0;
    char gm_sysex[] = { MIDI_SYSEX_UNIV_NON_REALTIME, MIDI_SYSEX_DEVICE_ID_ALL, MIDI_SYSEX_GM_ID, MIDI_SYSEX_GM_ON };
    char gm2_sysex[] = { MIDI_SYSEX_UNIV_NON_REALTIME, MIDI_SYSEX_DEVICE_ID_ALL, MIDI_SYSEX_GM_ID, MIDI_SYSEX_GM2_ON };

    /* Start in GS mode */
    synth->bank_select = FLUID_BANK_STYLE_GS;

    /* GM System ON - disables bank select */
    TEST_SUCCESS(fluid_synth_sysex(synth, gm_sysex, sizeof(gm_sysex), NULL, NULL, NULL, FALSE));
    TEST_ASSERT(synth->bank_select == FLUID_BANK_STYLE_GM);

    /* Verify bank select is ignored in GM mode */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_LSB, 3));
    fluid_channel_get_sfont_bank_prog(synth->channel[chan], NULL, &bank, NULL);
    TEST_ASSERT(bank == 0);

    /* GM2 System ON - should re-enable bank select */
    TEST_SUCCESS(fluid_synth_sysex(synth, gm2_sysex, sizeof(gm2_sysex), NULL, NULL, NULL, FALSE));
    TEST_ASSERT(synth->bank_select == FLUID_BANK_STYLE_GM2);

    /* Verify bank select now works via LSB */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_MSB, 121));
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_LSB, 3));
    fluid_channel_get_sfont_bank_prog(synth->channel[chan], NULL, &bank, NULL);
    TEST_ASSERT(bank == 3);
}

/* Test that synth.midi-bank-select=gm2 setting works */
static void test_gm2_setting(void)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    int bank = -1;

    TEST_ASSERT(settings != NULL);
    TEST_SUCCESS(fluid_settings_setstr(settings, "synth.midi-bank-select", "gm2"));

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);
    TEST_SUCCESS(fluid_synth_sfload(synth, TEST_SOUNDFONT, 1));

    TEST_ASSERT(synth->bank_select == FLUID_BANK_STYLE_GM2);

    /* Verify LSB works as bank number */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, BANK_SELECT_MSB, 121));
    TEST_SUCCESS(fluid_synth_cc(synth, 0, BANK_SELECT_LSB, 7));
    fluid_channel_get_sfont_bank_prog(synth->channel[0], NULL, &bank, NULL);
    TEST_ASSERT(synth->channel[0]->channel_type == CHANNEL_TYPE_MELODIC);
    TEST_ASSERT(bank == 7);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

/* Test that GM2 style only recognizes MSB=120 as drum (not 126, 127 like XG) */
static void test_gm2_only_120_is_drum(fluid_synth_t *synth)
{
    int chan = 0;

    synth->bank_select = FLUID_BANK_STYLE_GM2;
    fluid_synth_system_reset(synth);

    /* MSB 126 should NOT switch to drum in GM2 (unlike XG) */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_MSB, 126));
    TEST_ASSERT(synth->channel[chan]->channel_type == CHANNEL_TYPE_MELODIC);

    /* MSB 127 should NOT switch to drum in GM2 (unlike XG) */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_MSB, 127));
    TEST_ASSERT(synth->channel[chan]->channel_type == CHANNEL_TYPE_MELODIC);

    /* Only MSB 120 switches to drum in GM2 */
    TEST_SUCCESS(fluid_synth_cc(synth, chan, BANK_SELECT_MSB, 120));
    TEST_ASSERT(synth->channel[chan]->channel_type == CHANNEL_TYPE_DRUM);
}

int main(void)
{
    fluid_synth_t *synth;
    fluid_settings_t *settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);
    TEST_SUCCESS(fluid_synth_sfload(synth, TEST_SOUNDFONT, 1));

    test_gm2_bank_lsb_melodic(synth);
    fluid_synth_system_reset(synth);

    test_gm2_bank_msb_ignored_melodic(synth);
    fluid_synth_system_reset(synth);

    test_gm2_drum_switch(synth);
    fluid_synth_system_reset(synth);

    test_gm2_drum_lsb_ignored(synth);
    fluid_synth_system_reset(synth);

    test_gm2_drum_to_melodic(synth);
    fluid_synth_system_reset(synth);

    test_gm2_sysex_system_on(synth);
    fluid_synth_system_reset(synth);

    test_gm_sysex_system_on(synth);
    fluid_synth_system_reset(synth);

    test_gm_then_gm2_reenables_bank_select(synth);
    fluid_synth_system_reset(synth);

    test_gm2_only_120_is_drum(synth);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    /* Test setting-based initialization */
    test_gm2_setting();

    return EXIT_SUCCESS;
}
