#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "utils/fluidsynth_priv.h"

int main(void)
{
    char *s;
    int id;
    fluid_sfont_t *sfont;
    fluid_preset_t *preset;
    fluid_preset_t *prev_preset;
    int count = 0;

    /* setup */
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth = new_fluid_synth(settings);
    fluid_settings_dupstr(settings, "synth.default-soundfont", &s);
    id = fluid_synth_sfload(synth, s, 1);
    sfont = fluid_synth_get_sfont_by_id(synth, id);

    /* code under test */
    fluid_sfont_iteration_start(sfont);

    while ((preset = fluid_sfont_iteration_next(sfont)) != NULL) {
        count++;

        /* make sure we actually got a different preset */
        TEST_ASSERT(preset != prev_preset);
        prev_preset = preset;
    }

    TEST_ASSERT(count > 0);

    /* teardown */
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
