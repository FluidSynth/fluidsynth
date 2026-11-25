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

/* Track 0 changes sustain at tick 2 and is then quiet until tick 100.
 * Track 1 changes sustain at tick 0 and plays a note at tick 10. */
static const unsigned char midi_seek[] = {
    'M','T','h','d', 0,0,0,6, 0,1, 0,2, 3,0xC0,
    'M','T','r','k', 0,0,0,13,
    2,0xB0,64,127, 98,0x90,60,100, 0x87,0x04,0xFF,0x2F,0,
    'M','T','r','k', 0,0,0,13,
    0,0xB0,64,0, 10,0x90,61,100, 0x87,0x5E,0xFF,0x2F,0
};

/* Several simultaneous events, including two on the same track and tick. */
static const unsigned char midi_multiple_tracks[] = {
    'M','T','h','d', 0,0,0,6, 0,1, 0,3, 3,0xC0,
    'M','T','r','k', 0,0,0,17,
    1,0xB0,64,10, 0,0xB0,64,11, 2,0xB0,64,30, 0x87,0x65,0xFF,0x2F,0,
    'M','T','r','k', 0,0,0,17,
    0,0xB0,64,20, 1,0xB0,64,21, 2,0xB0,64,31, 0x87,0x65,0xFF,0x2F,0,
    'M','T','r','k', 0,0,0,17,
    1,0xB0,64,40, 1,0xB0,64,41, 1,0xB0,64,42, 0x87,0x65,0xFF,0x2F,0
};

/* One track ends immediately; the other ends after a controller event. */
static const unsigned char midi_staggered_end[] = {
    'M','T','h','d', 0,0,0,6, 0,1, 0,2, 3,0xC0,
    'M','T','r','k', 0,0,0,4,
    0,0xFF,0x2F,0,
    'M','T','r','k', 0,0,0,8,
    2,0xB0,64,42, 0,0xFF,0x2F,0
};

struct events { int types[32]; int values[32]; int count; };

static int record_event(void *data, fluid_midi_event_t *event)
{
    struct events *events = data;
    if(events->count < 32 && (fluid_midi_event_get_type(event) != 0xB0 || fluid_midi_event_get_control(event) == 64))
    {
        events->types[events->count] = fluid_midi_event_get_type(event);
        events->values[events->count] = fluid_midi_event_get_value(event);
        events->count++;
    }
    return FLUID_OK;
}

static fluid_player_t *new_seek_player(fluid_synth_t *synth, struct events *events)
{
    fluid_player_t *player = new_fluid_player(synth);
    TEST_ASSERT(player != NULL);
    TEST_SUCCESS(fluid_player_set_playback_callback(player, record_event, events));
    TEST_SUCCESS(fluid_player_add_mem(player, midi_seek, sizeof(midi_seek)));
    TEST_SUCCESS(fluid_player_play(player));
    return player;
}

static void test_backward_seek(fluid_synth_t *synth)
{
    struct events events = {0};
    fluid_player_t *player = new_seek_player(synth, &events);
    int i;

    for(i = 0; i < 32 && fluid_player_get_current_tick(player) < 20; i++)
    {
        TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    }
    TEST_ASSERT(fluid_player_get_current_tick(player) >= 20);
    events.count = 0;
    TEST_SUCCESS(fluid_player_seek(player, 5));
    for(i = 0; i < 8 && events.count < 2; i++)
    {
        TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    }
    TEST_ASSERT(events.count == 2);
    TEST_ASSERT(events.types[0] == 0xB0 && events.values[0] == 0);
    TEST_ASSERT(events.types[1] == 0xB0 && events.values[1] == 127);
    delete_fluid_player(player);
}

static void test_pause_resume(fluid_synth_t *synth)
{
    struct events events = {0};
    fluid_player_t *player = new_seek_player(synth, &events);
    int i;

    /* Load the song, then seek forward to the note at tick 10. Stopping at
     * that exact tick causes one track to need a rewind and the other not. */
    TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    TEST_ASSERT(fluid_player_get_current_tick(player) < 10);
    TEST_SUCCESS(fluid_player_seek(player, 10));
    TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    TEST_ASSERT(fluid_player_get_current_tick(player) == 10);
    TEST_SUCCESS(fluid_player_stop(player));
    events.count = 0;
    TEST_SUCCESS(fluid_player_play(player));
    for(i = 0; i < 8 && events.count < 3; i++)
    {
        TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    }
    TEST_ASSERT(events.count == 3);
    TEST_ASSERT(events.types[0] == 0xB0 && events.values[0] == 0);
    TEST_ASSERT(events.types[1] == 0xB0 && events.values[1] == 127);
    TEST_ASSERT(events.types[2] == 0x90);
    delete_fluid_player(player);
}

static void test_forward_seek(fluid_synth_t *synth)
{
    struct events events = {0};
    fluid_player_t *player = new_seek_player(synth, &events);

    TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    TEST_ASSERT(fluid_player_get_current_tick(player) < 5);
    events.count = 0;
    TEST_SUCCESS(fluid_player_seek(player, 5));
    TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    TEST_ASSERT(events.count == 1);
    TEST_ASSERT(events.types[0] == 0xB0 && events.values[0] == 127);
    delete_fluid_player(player);
}

static void test_simultaneous_track_order(fluid_synth_t *synth)
{
    static const int expected[] = {20, 10, 11, 21, 40, 41, 30, 31, 42};
    struct events events = {0};
    fluid_player_t *player = new_fluid_player(synth);
    int i, j;

    TEST_ASSERT(player != NULL);
    TEST_SUCCESS(fluid_player_set_playback_callback(player, record_event, &events));
    TEST_SUCCESS(fluid_player_add_mem(player, midi_multiple_tracks, sizeof(midi_multiple_tracks)));
    TEST_SUCCESS(fluid_player_play(player));
    for(j = 0; j < 2; j++)
    {
        if(j == 1)
        {
            events.count = 0;
            TEST_SUCCESS(fluid_player_seek(player, 3));
        }
        for(i = 0; i < 8 && events.count < 9; i++)
        {
            TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
        }
        TEST_ASSERT(events.count == 9);
        for(i = 0; i < 9; i++)
        {
            TEST_ASSERT(events.types[i] == 0xB0 && events.values[i] == expected[i]);
        }
    }
    delete_fluid_player(player);
}

static void test_playlist_transition(fluid_synth_t *synth)
{
    struct events events = {0};
    fluid_player_t *player = new_fluid_player(synth);
    int i;

    TEST_ASSERT(player != NULL);
    TEST_SUCCESS(fluid_player_set_playback_callback(player, record_event, &events));
    TEST_SUCCESS(fluid_player_add_mem(player, midi_staggered_end, sizeof(midi_staggered_end)));
    TEST_SUCCESS(fluid_player_add_mem(player, midi_format0, sizeof(midi_format0)));
    TEST_SUCCESS(fluid_player_play(player));
    TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    TEST_ASSERT(events.count == 0); /* The first track's EOT did not end the song. */
    for(i = 0; i < 8 && events.count < 1; i++)
    {
        TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    }
    /* The second song starts in the callback that consumes the first EOT. */
    TEST_ASSERT(events.count == 2);
    TEST_ASSERT(events.types[0] == 0xB0 && events.values[0] == 42);
    TEST_ASSERT(events.types[1] == 0x90);
    delete_fluid_player(player);
}

int main(void)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    fluid_player_t *player;
    struct events events = {0};
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

    test_pause_resume(synth);
    test_backward_seek(synth);
    test_forward_seek(synth);
    test_simultaneous_track_order(synth);
    test_playlist_transition(synth);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    return EXIT_SUCCESS;
}
