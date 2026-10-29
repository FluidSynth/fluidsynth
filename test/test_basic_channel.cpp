#include "test.h"
#include "fluidsynth.h"
#include "fluid_midi.h"

#include <array>

constexpr int NumChannels = 16;
constexpr int SampleFrames = 256;

static void check_basic(fluid_synth_t *synth, int chan, int exp_basic, int exp_mode, int exp_val)
{
    int basic = -2, mode = -2, val = -2;

    TEST_SUCCESS(fluid_synth_get_basic_channel(synth, chan, &basic, &mode, &val));
    TEST_ASSERT(basic == exp_basic);
    TEST_ASSERT(mode == exp_mode);
    TEST_ASSERT(val == exp_val);
}

static void check_enabled(fluid_synth_t *synth, int chan, int enabled)
{
    /* pitch bend is rejected on disabled channels only */
    int res = fluid_synth_pitch_bend(synth, chan, 8192);
    TEST_ASSERT((res == FLUID_OK) == (enabled != 0));
}

static fluid_voice_t *start_voice(fluid_synth_t *synth, fluid_sample_t *sample, int chan)
{
    fluid_voice_t *voice = fluid_synth_alloc_voice(synth, sample, chan, 60, 100);
    TEST_ASSERT(voice != nullptr);
    fluid_synth_start_voice(synth, voice);
    TEST_ASSERT(fluid_voice_is_on(voice));
    return voice;
}

static void test_defaults(fluid_synth_t *synth)
{
    for(int chan = 0; chan < NumChannels; chan++)
    {
        check_basic(synth, chan, 0, FLUID_CHANNEL_MODE_OMNION_POLY, NumChannels);
        check_enabled(synth, chan, true);
    }
}

static void test_invalid_arguments(fluid_synth_t *synth)
{
    int basic, mode, val;

    TEST_ASSERT(fluid_synth_get_basic_channel(nullptr, 0, &basic, &mode, &val) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_get_basic_channel(synth, -1, &basic, &mode, &val) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_get_basic_channel(synth, NumChannels, &basic, &mode, &val) == FLUID_FAILED);

    TEST_ASSERT(fluid_synth_set_basic_channel(nullptr, 0, FLUID_CHANNEL_MODE_OMNION_POLY, 1) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_basic_channel(synth, NumChannels, FLUID_CHANNEL_MODE_OMNION_POLY, 1) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_basic_channel(synth, 0, -1, 1) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_basic_channel(synth, 0, FLUID_CHANNEL_MODE_LAST, 1) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_basic_channel(synth, 0, FLUID_CHANNEL_MODE_OMNION_POLY, -1) == FLUID_FAILED);

    TEST_ASSERT(fluid_synth_reset_basic_channel(synth, NumChannels) == FLUID_FAILED);

    /* nothing may have changed */
    test_defaults(synth);
}

static void test_reset(fluid_synth_t *synth)
{
    /* channel 5 is enabled but not a basic channel */
    TEST_ASSERT(fluid_synth_reset_basic_channel(synth, 5) == FLUID_FAILED);
    test_defaults(synth);

    TEST_SUCCESS(fluid_synth_reset_basic_channel(synth, 0));

    for(int chan = 0; chan < NumChannels; chan++)
    {
        check_basic(synth, chan, FLUID_FAILED, FLUID_FAILED, FLUID_FAILED);
        check_enabled(synth, chan, false);
    }

    /* resetting all channels is always possible */
    TEST_SUCCESS(fluid_synth_reset_basic_channel(synth, -1));
}

static void test_set_groups(fluid_synth_t *synth)
{
    int chan;

    TEST_SUCCESS(fluid_synth_reset_basic_channel(synth, -1));

    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 0, FLUID_CHANNEL_MODE_OMNION_POLY, 4));
    /* val == 0 spans up to the end of the channels */
    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 4, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 0));

    for(chan = 0; chan < 4; chan++)
    {
        check_basic(synth, chan, 0, FLUID_CHANNEL_MODE_OMNION_POLY, 4);
    }

    for(chan = 4; chan < NumChannels; chan++)
    {
        check_basic(synth, chan, 4, FLUID_CHANNEL_MODE_OMNIOFF_MONO, NumChannels - 4);
        check_enabled(synth, chan, true);
    }

    /* an existing basic channel cannot be modified, and groups must not overlap */
    TEST_ASSERT(fluid_synth_set_basic_channel(synth, 0, FLUID_CHANNEL_MODE_OMNION_MONO, 4) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_basic_channel(synth, 2, FLUID_CHANNEL_MODE_OMNION_MONO, 2) == FLUID_FAILED);
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNION_POLY, 4);
    check_basic(synth, 2, 0, FLUID_CHANNEL_MODE_OMNION_POLY, 4);
}

static void test_group_limits(fluid_synth_t *synth)
{
    TEST_SUCCESS(fluid_synth_reset_basic_channel(synth, -1));

    /* group exceeds the channel count */
    TEST_ASSERT(fluid_synth_set_basic_channel(synth, 14, FLUID_CHANNEL_MODE_OMNION_POLY, 4) == FLUID_FAILED);
    check_enabled(synth, 14, false);

    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 8, FLUID_CHANNEL_MODE_OMNION_POLY, 2));

    /* explicit size overlapping the next basic channel */
    TEST_ASSERT(fluid_synth_set_basic_channel(synth, 6, FLUID_CHANNEL_MODE_OMNION_POLY, 4) == FLUID_FAILED);
    check_enabled(synth, 6, false);

    /* val == 0 gets clipped at the next basic channel */
    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 6, FLUID_CHANNEL_MODE_OMNION_MONO, 0));
    check_basic(synth, 6, 6, FLUID_CHANNEL_MODE_OMNION_MONO, 2);
    check_basic(synth, 7, 6, FLUID_CHANNEL_MODE_OMNION_MONO, 2);
    check_basic(synth, 8, 8, FLUID_CHANNEL_MODE_OMNION_POLY, 2);

    /* omni off / poly is always a single channel */
    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 12, FLUID_CHANNEL_MODE_OMNIOFF_POLY, 3));
    check_basic(synth, 12, 12, FLUID_CHANNEL_MODE_OMNIOFF_POLY, 1);
    check_enabled(synth, 12, true);
    check_enabled(synth, 13, false);
}

static void test_mode_cc(fluid_synth_t *synth)
{
    int chan;

    TEST_SUCCESS(fluid_synth_system_reset(synth));
    test_defaults(synth);

    /* omni on ignores M: mono on spans all channels (issue 1859) */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_OFF, 1));
    for(chan = 0; chan < NumChannels; chan++)
    {
        check_basic(synth, chan, 0, FLUID_CHANNEL_MODE_OMNION_MONO, NumChannels);
        check_enabled(synth, chan, true);
    }

    /* omni off then mono on, M = 4 */
    TEST_SUCCESS(fluid_synth_system_reset(synth));
    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_OFF, 0));
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNIOFF_POLY, 1);
    check_enabled(synth, 1, false);
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_OFF, 4));
    for(chan = 0; chan < 4; chan++)
    {
        check_basic(synth, chan, 0, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 4);
        check_enabled(synth, chan, true);
    }
    for(chan = 4; chan < NumChannels; chan++)
    {
        check_enabled(synth, chan, false);
    }

    /* mono on, M = 4, then omni off: M is remembered (issue 1859) */
    TEST_SUCCESS(fluid_synth_system_reset(synth));
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_OFF, 4));
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNION_MONO, NumChannels);
    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_OFF, 0));
    for(chan = 0; chan < 4; chan++)
    {
        check_basic(synth, chan, 0, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 4);
        check_enabled(synth, chan, true);
    }
    for(chan = 4; chan < NumChannels; chan++)
    {
        check_enabled(synth, chan, false);
    }

    /* omni off + poly on collapses the group to one channel */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_ON, 0));
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNIOFF_POLY, 1);
    check_enabled(synth, 1, false);

    /* poly on, then omni off */
    TEST_SUCCESS(fluid_synth_system_reset(synth));
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_ON, 0));
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNION_POLY, NumChannels);
    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_OFF, 0));
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNIOFF_POLY, 1);
    check_enabled(synth, 1, false);

    /* omni on, then mono on with M = 4 */
    TEST_SUCCESS(fluid_synth_system_reset(synth));
    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_ON, 0));
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_OFF, 4));
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNION_MONO, NumChannels);
    check_basic(synth, NumChannels - 1, 0, FLUID_CHANNEL_MODE_OMNION_MONO, NumChannels);

    /* mono on with M = 4, then omni on */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_ON, 0));
    check_basic(synth, NumChannels - 1, 0, FLUID_CHANNEL_MODE_OMNION_MONO, NumChannels);

    /* omni on, then poly on */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_ON, 0));
    check_basic(synth, NumChannels - 1, 0, FLUID_CHANNEL_MODE_OMNION_POLY, NumChannels);

    /* poly on, then omni on */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_ON, 0));
    check_basic(synth, NumChannels - 1, 0, FLUID_CHANNEL_MODE_OMNION_POLY, NumChannels);

    /* the group grows back after it has been shrunk by omni off */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_OFF, 0));
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNIOFF_POLY, 1);
    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_ON, 0));
    check_basic(synth, NumChannels - 1, 0, FLUID_CHANNEL_MODE_OMNION_POLY, NumChannels);

    /* M = 0 means all channels up to the end */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_OFF, 0));
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_OFF, 0));
    check_basic(synth, NumChannels - 1, 0, FLUID_CHANNEL_MODE_OMNIOFF_MONO, NumChannels);

    /* mode CCs are rejected on channels that are not basic channels */
    TEST_ASSERT(fluid_synth_cc(synth, 1, POLY_OFF, 1) == FLUID_FAILED);
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNIOFF_MONO, NumChannels);

    /* group larger than the channel count is rejected, M stays untouched */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_OFF, 2));
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 2);
    TEST_ASSERT(fluid_synth_cc(synth, 0, POLY_OFF, 17) == FLUID_FAILED);
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 2);
    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_ON, 0));
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNION_MONO, NumChannels);
    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_OFF, 0));
    check_basic(synth, 0, 0, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 2);

    /* an omni off / mono group set up via the API keeps its size across omni on / off */
    TEST_SUCCESS(fluid_synth_reset_basic_channel(synth, -1));
    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 2, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 3));
    TEST_SUCCESS(fluid_synth_cc(synth, 2, OMNI_ON, 0));
    check_basic(synth, NumChannels - 1, 2, FLUID_CHANNEL_MODE_OMNION_MONO, NumChannels - 2);
    TEST_SUCCESS(fluid_synth_cc(synth, 2, OMNI_OFF, 0));
    check_basic(synth, 2, 2, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 3);
    check_enabled(synth, 5, false);
}

static void test_global_cc(fluid_synth_t *synth)
{
    int val;

    TEST_SUCCESS(fluid_synth_reset_basic_channel(synth, -1));
    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 1, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 3));

    /* Test the "GLOBAL CONTROLLERS" behavior mentioned in the MIDI spec:
     * a CC on a disabled channel is forwarded to all channels of the omni off / mono group whose
     * basic channel is the next higher channel. Here channel 0 forwards to the group starting at 1. */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, 7, 50));
    for(int chan = 1; chan <= 3; chan++)
    {
        TEST_SUCCESS(fluid_synth_get_cc(synth, chan, 7, &val));
        TEST_ASSERT(val == 50);
    }

    /* channel 5 is disabled, but channel 6 is not a basic channel, so nothing is forwarded */
    TEST_ASSERT(fluid_synth_cc(synth, 5, 7, 20) == FLUID_FAILED);

    /* only omni off / mono groups receive forwarded CCs */
    TEST_SUCCESS(fluid_synth_reset_basic_channel(synth, -1));
    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 1, FLUID_CHANNEL_MODE_OMNION_POLY, 3));
    TEST_ASSERT(fluid_synth_cc(synth, 0, 7, 50) == FLUID_FAILED);
}

static void test_notes_off_on_mode_change(fluid_synth_t *synth, fluid_sample_t *sample)
{
    const fluid_voice_t *v0, *v1, *v4;

    TEST_SUCCESS(fluid_synth_reset_basic_channel(synth, -1));
    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 0, FLUID_CHANNEL_MODE_OMNION_POLY, 4));
    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 4, FLUID_CHANNEL_MODE_OMNION_POLY, 4));

    v0 = start_voice(synth, sample, 0);
    v1 = start_voice(synth, sample, 1);
    v4 = start_voice(synth, sample, 4);

    /* spec: switching modes turns off all notes; only the group being changed is affected */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, POLY_OFF, 4));
    TEST_ASSERT(!fluid_voice_is_on(v0));
    TEST_ASSERT(!fluid_voice_is_on(v1));
    TEST_ASSERT(fluid_voice_is_on(v4));

    /* cleanup, so the next scenario starts without playing voices */
    TEST_SUCCESS(fluid_synth_all_sounds_off(synth, -1));
    TEST_SUCCESS(fluid_synth_reset_basic_channel(synth, -1));
    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 0, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 4));

    v0 = start_voice(synth, sample, 0);
    v1 = start_voice(synth, sample, 1);

    /* spec: in omni off / mono, All Notes Off only cancels notes on the channel it was received on */
    TEST_SUCCESS(fluid_synth_cc(synth, 0, 123, 0));
    TEST_ASSERT(!fluid_voice_is_on(v0));
    TEST_ASSERT(fluid_voice_is_on(v1));
}

static void test_shrink_group_releases_notes(fluid_synth_t *synth, fluid_sample_t *sample)
{
    /* the note off arrives on a channel that is no longer enabled, so the mode change must end the note */
    TEST_SUCCESS(fluid_synth_system_reset(synth));
    const fluid_voice_t *v = start_voice(synth, sample, 1);
    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_OFF, 0));
    check_enabled(synth, 1, false);
    TEST_ASSERT(!fluid_voice_is_on(v));
    TEST_ASSERT(!fluid_voice_is_sustained(v));
    TEST_ASSERT(fluid_synth_noteoff(synth, 1, 60) == FLUID_FAILED);
}

static void test_shrink_group_releases_sustained_notes(fluid_synth_t *synth, fluid_sample_t *sample)
{
    const fluid_voice_t *held, *playing;

    TEST_SUCCESS(fluid_synth_system_reset(synth));
    TEST_SUCCESS(fluid_synth_cc(synth, 1, 64, 127));

    /* held: note off already received, kept alive by the pedal; playing: key still down */
    held = start_voice(synth, sample, 1);
    playing = start_voice(synth, sample, 2);
    TEST_ASSERT(fluid_voice_is_on(held));
    TEST_SUCCESS(fluid_synth_noteoff(synth, 1, 60));
    TEST_ASSERT(!fluid_voice_is_on(held) && fluid_voice_is_playing(held) && fluid_voice_is_sustained(held));
    TEST_ASSERT(fluid_voice_is_on(playing));

    TEST_SUCCESS(fluid_synth_cc(synth, 0, OMNI_OFF, 0));
    check_enabled(synth, 1, false);
    check_enabled(synth, 2, false);

    /* the pedal release can no longer reach the dropped channels */
    TEST_ASSERT(fluid_synth_cc(synth, 1, 64, 0) == FLUID_FAILED);

    TEST_ASSERT(!fluid_voice_is_on(held));
    TEST_ASSERT(!fluid_voice_is_on(playing));
}

static void test_system_reset(fluid_synth_t *synth)
{
    TEST_SUCCESS(fluid_synth_reset_basic_channel(synth, -1));
    TEST_SUCCESS(fluid_synth_set_basic_channel(synth, 2, FLUID_CHANNEL_MODE_OMNIOFF_MONO, 4));

    TEST_SUCCESS(fluid_synth_system_reset(synth));
    test_defaults(synth);
}

int main(void)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    fluid_sample_t *sample = new_fluid_sample();
    std::array<short, SampleFrames> data = {0};

    TEST_ASSERT(settings != nullptr);
    TEST_ASSERT(sample != nullptr);
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.midi-channels", NumChannels));
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != nullptr);

    TEST_SUCCESS(fluid_sample_set_sound_data(sample, data.data(), nullptr, SampleFrames, 44100, 1));
    TEST_SUCCESS(fluid_sample_set_pitch(sample, 60, 0));

    test_defaults(synth);
    test_invalid_arguments(synth);
    test_reset(synth);
    test_set_groups(synth);
    test_group_limits(synth);
    test_mode_cc(synth);
    test_global_cc(synth);
    test_notes_off_on_mode_change(synth, sample);
    test_shrink_group_releases_notes(synth, sample);
    test_shrink_group_releases_sustained_notes(synth, sample);
    test_system_reset(synth);

    delete_fluid_synth(synth);
    delete_fluid_sample(sample);
    delete_fluid_settings(settings);
    return EXIT_SUCCESS;
}
