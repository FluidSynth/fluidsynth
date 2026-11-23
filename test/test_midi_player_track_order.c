#include "test.h"
#include "fluidsynth.h"

/* Format 1: note-on at tick 0, sustain at tick 1, note-off at tick 2.
 * At 48 kHz all three events fit in one 64-sample block. */
static unsigned char midi[] = {
    'M','T','h','d', 0,0,0,6, 0,1, 0,2, 3,0xC0,
    'M','T','r','k', 0,0,0,13,
    0,0x90,60,100, 2,0x80,60,64, 0x87,0x68,0xFF,0x2F,0,
    'M','T','r','k', 0,0,0,9,
    1,0xB0,64,127, 0x87,0x68,0xFF,0x2F,0
};

/* Equivalent single-track format 0 file. */
static const unsigned char midi_format0[] = {
    'M','T','h','d', 0,0,0,6, 0,0, 0,1, 3,0xC0,
    'M','T','r','k', 0,0,0,16,
    0,0x90,60,100, 1,0xB0,64,127, 1,0x80,60,64, 0,0xFF,0x2F,0
};

struct events { int types[8]; int count; };

static int record_event(void *data, fluid_midi_event_t *event)
{
    struct events *events = data;
    if(events->count < 8 && (fluid_midi_event_get_type(event) != 0xB0 || fluid_midi_event_get_control(event) == 64))
    {
        events->types[events->count++] = fluid_midi_event_get_type(event);
    }
    return FLUID_OK;
}

int main(void)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    fluid_player_t *player;
    struct events events = {{0}, 0};
    int i;
    TEST_ASSERT(settings != NULL);
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.sample-rate", 48000.0));
    TEST_SUCCESS(fluid_settings_setstr(settings, "player.timing-source", "sample"));
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);
    player = new_fluid_player(synth);
    TEST_ASSERT(player != NULL);
    TEST_SUCCESS(fluid_player_set_playback_callback(player, record_event, &events));
    TEST_SUCCESS(fluid_player_add_mem(player, midi, sizeof(midi)));
    TEST_SUCCESS(fluid_player_play(player));
    for(i = 0; i < 8 && events.count < 3; i++)
    {
        TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    }
    TEST_ASSERT(events.count == 3);
    TEST_ASSERT(events.types[0] == 0x90);
    TEST_ASSERT(events.types[1] == 0xB0);
    TEST_ASSERT(events.types[2] == 0x80);

    /* A backward seek replays controllers at the target tick, skips earlier
     * note events, and then plays note-off when its tick is reached. */
    events.count = 0;
    TEST_SUCCESS(fluid_player_seek(player, 1));
    for(i = 0; i < 8 && events.count < 2; i++)
    {
        TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    }
    TEST_ASSERT(events.count == 2);
    TEST_ASSERT(events.types[0] == 0xB0);
    TEST_ASSERT(events.types[1] == 0x80);
    delete_fluid_player(player);

    /* With events at the same tick, the earlier track wins the tie. */
    midi[26] = 1; /* track 0 note-off moves from tick 2 to tick 1 */
    events.count = 0;
    player = new_fluid_player(synth);
    TEST_ASSERT(player != NULL);
    TEST_SUCCESS(fluid_player_set_playback_callback(player, record_event, &events));
    TEST_SUCCESS(fluid_player_add_mem(player, midi, sizeof(midi)));
    TEST_SUCCESS(fluid_player_play(player));
    for(i = 0; i < 8 && events.count < 3; i++)
    {
        TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    }
    TEST_ASSERT(events.count == 3);
    TEST_ASSERT(events.types[0] == 0x90);
    TEST_ASSERT(events.types[1] == 0x80);
    TEST_ASSERT(events.types[2] == 0xB0);
    delete_fluid_player(player);

    /* A format 0 file with the same ticks has the same event order. */
    events.count = 0;
    player = new_fluid_player(synth);
    TEST_ASSERT(player != NULL);
    TEST_SUCCESS(fluid_player_set_playback_callback(player, record_event, &events));
    TEST_SUCCESS(fluid_player_add_mem(player, midi_format0, sizeof(midi_format0)));
    TEST_SUCCESS(fluid_player_play(player));
    for(i = 0; i < 8 && events.count < 3; i++)
    {
        TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    }
    TEST_ASSERT(events.count == 3);
    TEST_ASSERT(events.types[0] == 0x90);
    TEST_ASSERT(events.types[1] == 0xB0);
    TEST_ASSERT(events.types[2] == 0x80);
    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    return EXIT_SUCCESS;
}
