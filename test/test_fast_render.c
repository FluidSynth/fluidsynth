
#include "test.h"
#include "fluidsynth.h"
#include "utils/fluid_sys.h"


// this tests fast file rendering and libsndfile output
int main(void)
{
    int id;
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    fluid_player_t *player;
    fluid_file_renderer_t *renderer;

    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);

    TEST_ASSERT(settings != NULL);
    TEST_ASSERT(synth != NULL);

    // no sfont loaded
    TEST_ASSERT(fluid_synth_sfcount(synth) == 0);

    TEST_ASSERT(fluid_is_soundfont(TEST_SOUNDFONT_UTF8_1) == TRUE);
    TEST_SUCCESS(id = fluid_synth_sfload(synth, TEST_SOUNDFONT_UTF8_1, 1));

    fluid_settings_setstr(settings, "audio.file.name", TEST_WAV_UTF8);

    player = new_fluid_player(synth);
    TEST_ASSERT(player != NULL);

    TEST_ASSERT(fluid_is_midifile(TEST_MIDI_UTF8) == TRUE);
    TEST_SUCCESS(fluid_player_add(player, TEST_MIDI_UTF8));

    renderer = new_fluid_file_renderer(synth);
    TEST_ASSERT(renderer != NULL);
    
    TEST_SUCCESS(fluid_player_play(player));

    while (fluid_player_get_status(player) == FLUID_PLAYER_PLAYING)
    {
        if (fluid_file_renderer_process_block(renderer) != FLUID_OK)
        {
            break;
        }
    }

    delete_fluid_file_renderer(renderer);
    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    
#if 0    
    FILE *file;
    file = FLUID_FOPEN(TEST_WAV_UTF8, "rb");
    TEST_ASSERT(file != NULL);
    TEST_ASSERT(FLUID_FCLOSE(file) == 0);
#endif
    
    return EXIT_SUCCESS;
}
