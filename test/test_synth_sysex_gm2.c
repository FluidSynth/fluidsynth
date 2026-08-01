#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include "fluid_midi.h"
#include "fluid_chan.h"

static void
assert_channel_bank(fluid_synth_t *synth, int channel, int expected_bank,
                    enum fluid_midi_channel_type expected_type)
{
    int bank;

    fluid_channel_get_sfont_bank_prog(synth->channel[channel], NULL, &bank, NULL);
    TEST_ASSERT(bank == expected_bank);
    TEST_ASSERT(synth->channel[channel]->channel_type == expected_type);
}

int main(void)
{
    const char gm_on[] =
    {
        MIDI_SYSEX_UNIV_NON_REALTIME,
        MIDI_SYSEX_DEVICE_ID_ALL,
        MIDI_SYSEX_GM_ID,
        MIDI_SYSEX_GM_ON
    };
    const char gm2_on[] =
    {
        MIDI_SYSEX_UNIV_NON_REALTIME,
        MIDI_SYSEX_DEVICE_ID_ALL,
        MIDI_SYSEX_GM_ID,
        MIDI_SYSEX_GM2_ON
    };
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    int handled = FALSE;

    TEST_ASSERT(settings != NULL);
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    TEST_SUCCESS(fluid_synth_sysex(synth, gm_on, sizeof(gm_on),
                                   NULL, NULL, &handled, FALSE));
    TEST_ASSERT(handled == TRUE);

    TEST_SUCCESS(fluid_synth_cc(synth, 0, BANK_SELECT_MSB, 121));
    TEST_SUCCESS(fluid_synth_cc(synth, 0, BANK_SELECT_LSB, 2));
    assert_channel_bank(synth, 0, 0, CHANNEL_TYPE_MELODIC);

    handled = FALSE;
    TEST_SUCCESS(fluid_synth_sysex(synth, gm2_on, sizeof(gm2_on),
                                   NULL, NULL, &handled, FALSE));
    TEST_ASSERT(handled == TRUE);

    TEST_SUCCESS(fluid_synth_cc(synth, 0, BANK_SELECT_MSB, 121));
    TEST_SUCCESS(fluid_synth_cc(synth, 0, BANK_SELECT_LSB, 2));
    assert_channel_bank(synth, 0, 2, CHANNEL_TYPE_MELODIC);

    TEST_SUCCESS(fluid_synth_cc(synth, 0, BANK_SELECT_MSB, 120));
    TEST_SUCCESS(fluid_synth_cc(synth, 0, BANK_SELECT_LSB, 7));
    assert_channel_bank(synth, 0, 128, CHANNEL_TYPE_DRUM);

    TEST_SUCCESS(fluid_synth_cc(synth, 0, BANK_SELECT_MSB, 121));
    TEST_SUCCESS(fluid_synth_cc(synth, 0, BANK_SELECT_LSB, 1));
    assert_channel_bank(synth, 0, 1, CHANNEL_TYPE_MELODIC);

    delete_fluid_synth(synth);

    TEST_SUCCESS(fluid_settings_setstr(settings, "synth.midi-bank-select", "gm2"));
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    TEST_SUCCESS(fluid_synth_cc(synth, 0, BANK_SELECT_MSB, 121));
    TEST_SUCCESS(fluid_synth_cc(synth, 0, BANK_SELECT_LSB, 3));
    assert_channel_bank(synth, 0, 3, CHANNEL_TYPE_MELODIC);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
