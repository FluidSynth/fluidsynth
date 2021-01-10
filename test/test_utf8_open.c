
#include "test.h"
#include "fluidsynth.h"
#include "utils/fluid_sys.h"


// this tests utf-8 file handling by loading the test .sf2 file
// manually and through the soundfont-related APIs
int main(void)
{
    int id;
    fluid_settings_t *settings;
    fluid_synth_t *synth;

	FILE *sfont_file;
    sfont_file = FLUID_FOPEN(TEST_SOUNDFONT_UTF8, "rb");
    TEST_ASSERT(sfont_file != NULL);
    TEST_ASSERT(FLUID_FCLOSE(sfont_file) == 0);

    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);

    TEST_ASSERT(settings != NULL);
    TEST_ASSERT(synth != NULL);

    // no sfont loaded
    TEST_ASSERT(fluid_synth_sfcount(synth) == 0);

    TEST_ASSERT(fluid_is_soundfont(TEST_SOUNDFONT_UTF8) == TRUE);

    // load a sfont to synth
    TEST_SUCCESS(id = fluid_synth_sfload(synth, TEST_SOUNDFONT_UTF8, 1));

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
