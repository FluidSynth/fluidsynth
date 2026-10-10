
#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include "fluid_chan.h"
#include "fluid_tuning.h"

void verify_equal_temperament(const fluid_synth_t *synth)
{
    int i;
    /* Verify that the default tuning is equal-tempered (pitch[key] == key * 100 cents) */
    for(i = 0; i < 128; i++)
    {
        double expected_pitch = i * 100.0;
        double actual_pitch = fluid_tuning_get_pitch(fluid_channel_get_tuning(synth->channel[0]), i);
        TEST_ASSERT(actual_pitch == expected_pitch);
    }
}

/* Tests that channels default to tuning bank 0, prog 0 so that MTS SysEx
 * messages that modify tuning 0/0 are automatically applied to all channels.
 * See https://github.com/FluidSynth/fluidsynth/issues/102
 */
int main(void)
{
    fluid_synth_t *synth;
    fluid_settings_t *settings = new_fluid_settings();
    int i;

    TEST_ASSERT(settings != NULL);

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    /* All channels should have tuning bank 0, prog 0 assigned by default */
    for(i = 0; i < fluid_synth_count_midi_channels(synth); i++)
    {
        const fluid_channel_t *chan = synth->channel[i];
        TEST_ASSERT(fluid_channel_has_tuning(chan));
        TEST_ASSERT(fluid_tuning_get_bank(fluid_channel_get_tuning(chan)) == 0);
        TEST_ASSERT(fluid_tuning_get_prog(fluid_channel_get_tuning(chan)) == 0);
    }

    verify_equal_temperament(synth);

    /* Modify the tuning of bank 0, prog 0 - change note 60 (middle C) to 6000 cents */
    int key = 60;
    double new_pitch = 6.0;
    TEST_SUCCESS(fluid_synth_tune_notes(synth, 0, 0, 1, &key, &new_pitch, FALSE));

    /* All channels should now reflect the modified tuning */
    for(i = 0; i < fluid_synth_count_midi_channels(synth); i++)
    {
        const fluid_channel_t *chan = synth->channel[i];
        TEST_ASSERT(fluid_channel_has_tuning(chan));
        TEST_ASSERT(fluid_tuning_get_pitch(fluid_channel_get_tuning(chan), 60) == new_pitch);
    }

    /* After a system reset, channels should still have tuning 0/0 */
    TEST_SUCCESS(fluid_synth_system_reset(synth));
    for(i = 0; i < fluid_synth_count_midi_channels(synth); i++)
    {
        const fluid_channel_t *chan = synth->channel[i];
        TEST_ASSERT(fluid_channel_has_tuning(chan));
        TEST_ASSERT(fluid_tuning_get_bank(fluid_channel_get_tuning(chan)) == 0);
        TEST_ASSERT(fluid_tuning_get_prog(fluid_channel_get_tuning(chan)) == 0);
    }

    /* But the tuning change from before should be gone */
    verify_equal_temperament(synth);

    /* Test that explicitly deactivating tuning on a channel works */
    TEST_SUCCESS(fluid_synth_deactivate_tuning(synth, 2, FALSE));
    TEST_ASSERT(!fluid_channel_has_tuning(synth->channel[2]));

    /* After another system reset, the channel should again have tuning 0/0 */
    TEST_SUCCESS(fluid_synth_system_reset(synth));
    TEST_ASSERT(fluid_channel_has_tuning(synth->channel[2]));
    /* All channels should have tuning bank 0, prog 0 assigned by default */
    for(i = 0; i < fluid_synth_count_midi_channels(synth); i++)
    {
        const fluid_channel_t *chan = synth->channel[i];
        TEST_ASSERT(fluid_channel_has_tuning(chan));
        TEST_ASSERT(fluid_tuning_get_bank(fluid_channel_get_tuning(chan)) == 0);
        TEST_ASSERT(fluid_tuning_get_prog(fluid_channel_get_tuning(chan)) == 0);
    }

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
