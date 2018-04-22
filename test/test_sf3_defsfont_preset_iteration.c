#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "sfloader/fluid_defsfont.h"
#include "utils/fluidsynth_priv.h"
#include "utils/fluid_list.h"

int main(void)
{
    int id;
    fluid_sfont_t *sfont;
    fluid_list_t *list;
    fluid_preset_t *preset;
    fluid_preset_t *prev_preset = NULL;
    fluid_defsfont_t *defsfont;
    fluid_sample_t *sample;
    fluid_sample_t *prev_sample = NULL;
    int count = 0;

    /* setup */
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth = new_fluid_synth(settings);
    id = fluid_synth_sfload(synth, TEST_SOUNDFONT_SF3, 1);
    sfont = fluid_synth_get_sfont_by_id(synth, id);
    defsfont = fluid_sfont_get_data(sfont);

    /* Make sure we have the right number of presets */
    fluid_sfont_iteration_start(sfont);
    while ((preset = fluid_sfont_iteration_next(sfont)) != NULL) {
        count++;

        /* make sure we actually got a different preset */
        TEST_ASSERT(preset != prev_preset);
        prev_preset = preset;
    }
    /* VintageDreams has 136 presets */
    TEST_ASSERT(count == 136);

    /* Make sure we have the right number of samples */
    count = 0;
    for (list = defsfont->sample; list; list = fluid_list_next(list))
    {
        sample = fluid_list_get(list);
        if (sample->data != NULL)
        {
            count++;
        }

        /* Make sure we actually got a different sample */
        TEST_ASSERT(sample != prev_sample);
        prev_sample = sample;
    }
    /* VintageDreams has 123 valid samples (one is a ROM sample and ignored) */
    TEST_ASSERT(count == 123);

    /* teardown */
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
