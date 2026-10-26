#include "test.h"
#include "midi/fluid_midi.h"

/* Reject a zero Set Tempo value and accept a valid one through file loading. */
int main(void)
{
    unsigned char midi[] = {
        'M','T','h','d', 0,0,0,6, 0,0, 0,1, 0,96,
        'M','T','r','k', 0,0,0,11,
        0,0xFF,0x51,3, 0,0,0, 96,0xFF,0x2F,0
    };
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    int valid;

    TEST_ASSERT(settings != NULL);
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);
    for(valid = 0; valid <= 1; valid++)
    {
        fluid_player_t *player = new_fluid_player(synth);
        TEST_ASSERT(player != NULL);
        TEST_SUCCESS(fluid_player_add_mem(player, midi, sizeof(midi)));
        TEST_SUCCESS(fluid_player_play(player));
        TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
        TEST_ASSERT(player->ntracks == valid);
        delete_fluid_player(player);

        /* 500000 microseconds per quarter note. */
        midi[26] = 0x07;
        midi[27] = 0xA1;
        midi[28] = 0x20;
    }
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    return EXIT_SUCCESS;
}
