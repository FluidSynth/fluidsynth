#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_midi.h"

// render enough samples to go past the release phase of the voices
enum { SAMPLES = 100 * 1024 };

// number of voices that the test soundfont plays for a note
enum { VOICES_PER_NOTE = 2 };

// checks the basic channel group that a MIDI channel belongs to
static void check_group(fluid_synth_t *synth, int chan, int basic_chan, int mode, int val)
{
    int basic_chan_out, mode_out, val_out;

    TEST_SUCCESS(fluid_synth_get_basic_channel(synth, chan, &basic_chan_out, &mode_out, &val_out));
    TEST_ASSERT(basic_chan_out == basic_chan);
    TEST_ASSERT(mode_out == mode);
    TEST_ASSERT(val_out == val);
}

static void check_disabled(fluid_synth_t *synth, int chan)
{
    check_group(synth, chan, FLUID_FAILED, FLUID_FAILED, FLUID_FAILED);
}

static int active_voices(fluid_synth_t *synth)
{
    TEST_SUCCESS(fluid_synth_process(synth, SAMPLES, 0, NULL, 0, NULL));
    return fluid_synth_get_active_voice_count(synth);
}

// A note that plays on a channel which is released from its group by a mono on message keeps on playing
// and ends with its note off, and the channel goes on accepting notes.
static void test_released_channel_keeps_working(fluid_synth_t *synth)
{
    TEST_SUCCESS(fluid_synth_noteon(synth, 1, 72, 127));
    TEST_ASSERT(active_voices(synth) == VOICES_PER_NOTE);

    // channel 0 is the basic channel of all the 16 channels, mono on limits its group to one channel
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_OFF, 1));
    TEST_ASSERT(active_voices(synth) == VOICES_PER_NOTE);

    TEST_SUCCESS(fluid_synth_noteoff(synth, 1, 72));
    TEST_ASSERT(active_voices(synth) == 0);

    TEST_SUCCESS(fluid_synth_noteon(synth, 1, 60, 127));
    TEST_ASSERT(active_voices(synth) == VOICES_PER_NOTE);
    TEST_SUCCESS(fluid_synth_noteoff(synth, 1, 60));
    TEST_ASSERT(active_voices(synth) == 0);
}

// Each released channel is a basic channel of its own, in the mode it had.
static void test_released_channels_are_basic_channels(fluid_synth_t *synth)
{
    int chan;

    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_OFF, 1));

    check_group(synth, 0, 0, FLUID_CHANNEL_MODE_OMNION_MONO, 1);

    for(chan = 1; chan < fluid_synth_count_midi_channels(synth); chan++)
    {
        check_group(synth, chan, chan, FLUID_CHANNEL_MODE_OMNION_POLY, 1);
    }
}

// Channel mode messages are accepted by the released channels, in any order, and only change the channel
// that receives them.
static void test_released_channels_accept_mode_messages(fluid_synth_t *synth)
{
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_OFF, 1));

    TEST_SUCCESS(fluid_synth_cc(synth, 5, POLY_OFF, 1));
    TEST_SUCCESS(fluid_synth_cc(synth, 3, OMNI_OFF, 0));

    check_group(synth, 3, 3, FLUID_CHANNEL_MODE_OMNIOFF_POLY, 1);
    check_group(synth, 5, 5, FLUID_CHANNEL_MODE_OMNION_MONO, 1);

    check_group(synth, 1, 1, FLUID_CHANNEL_MODE_OMNION_POLY, 1);
    check_group(synth, 4, 4, FLUID_CHANNEL_MODE_OMNION_POLY, 1);
    check_group(synth, 6, 6, FLUID_CHANNEL_MODE_OMNION_POLY, 1);
}

// A group that keeps its size does not release any channel: its voice channels stay voice channels.
static void test_group_of_same_size_is_unchanged(fluid_synth_t *synth)
{
    // zero channels means all the channels from the basic channel on
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_OFF, 0));

    check_group(synth, 0, 0, FLUID_CHANNEL_MODE_OMNION_MONO, 16);
    check_group(synth, 1, 0, FLUID_CHANNEL_MODE_OMNION_MONO, 16);
    check_group(synth, 15, 0, FLUID_CHANNEL_MODE_OMNION_MONO, 16);
}

// Only the channels of the group are released, the ones outside of it stay disabled.
static void test_channels_outside_of_the_group_stay_disabled(fluid_synth_t *synth)
{
    int chan;

    // a mono group of four channels, all the other channels are disabled
    TEST_SUCCESS(fluid_synth_reset_basic_channel(synth, -1));
    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 0, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 4));

    // poly on turns it into Omni Off Poly, which is a group of one channel
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_ON, 0));

    check_group(synth, 0, 0, FLUID_CHANNEL_MODE_OMNIOFF_POLY, 1);

    for(chan = 1; chan < 4; chan++)
    {
        check_group(synth, chan, chan, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 1);
    }

    for(chan = 4; chan < fluid_synth_count_midi_channels(synth); chan++)
    {
        check_disabled(synth, chan);
    }
}

int main(void)
{
    fluid_synth_t *synth;
    fluid_settings_t *settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);
    TEST_SUCCESS(fluid_synth_sfload(synth, TEST_SOUNDFONT, 1));

    test_released_channel_keeps_working(synth);
    fluid_synth_system_reset(synth);

    test_released_channels_are_basic_channels(synth);
    fluid_synth_system_reset(synth);

    test_released_channels_accept_mode_messages(synth);
    fluid_synth_system_reset(synth);

    test_group_of_same_size_is_unchanged(synth);
    fluid_synth_system_reset(synth);

    test_channels_outside_of_the_group_stay_disabled(synth);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
