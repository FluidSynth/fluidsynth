#include "test.h"
#include "fluidsynth.h"
#include "midi/fluid_midi.h"
#include "utils/fluid_sys.h"

/*
 * test_midi_track_order
 *
 * Regression test for https://github.com/FluidSynth/fluidsynth/issues/1866
 *
 * When events on different tracks of a format 1 MIDI file land in the same
 * audio block, the player must send them in global tick order, not track
 * order. This test builds a small format 1 MIDI file in memory with
 * interleaved events across two tracks, plays it through the player with a
 * recording callback, and verifies the events arrive in tick order.
 *
 * Track 0: note_on at tick 0, note_off at tick 2
 * Track 1: CC(64,127) at tick 1, CC(64,0) at tick 2
 *
 * Expected playback order (tick order, ties broken by track index):
 *   1. note_on   (tick 0, track 0)
 *   2. CC=127    (tick 1, track 1)
 *   3. note_off  (tick 2, track 0)
 *   4. CC=0      (tick 2, track 1)
 *
 * Without the fix, note_off (tick 2, track 0) would be sent before
 * CC=127 (tick 1, track 1), because track 0 is fully processed first.
 */

typedef struct {
    unsigned char type;
    unsigned char channel;
    unsigned int  param1;
    unsigned int  param2;
    unsigned int  tick;
} recorded_event_t;

#define MAX_RECORDED_EVENTS 64

typedef struct {
    recorded_event_t events[MAX_RECORDED_EVENTS];
    int count;
} event_recorder_t;

static int
record_callback(void *data, fluid_midi_event_t *event)
{
    event_recorder_t *rec = (event_recorder_t *) data;

    if(rec->count >= MAX_RECORDED_EVENTS)
    {
        return FLUID_FAILED;
    }

    rec->events[rec->count].type = event->type;
    rec->events[rec->count].channel = event->channel;
    rec->events[rec->count].param1 = event->param1;
    rec->events[rec->count].param2 = event->param2;
    /* The player does not expose the absolute tick to the callback, so we
     * infer it from the order of events and our knowledge of the test file. */
    rec->events[rec->count].tick = rec->count; /* placeholder, order is what matters */
    rec->count++;

    return FLUID_OK;
}

/*
 * Minimal format 1 MIDI file, 2 tracks, 960 ticks/quarter note.
 *
 * Track 0:
 *   tick 0:  note_on   ch0 key60 vel100
 *   tick 2:  note_off  ch0 key60 vel64
 *   EOT
 *
 * Track 1:
 *   tick 1:  CC  ch0 ctrl64 val127
 *   tick 2:  CC  ch0 ctrl64 val0
 *   EOT
 */
static const unsigned char midi_data[] =
{
    /* MThd chunk */
    0x4D, 0x54, 0x68, 0x64,            /* "MThd" */
    0x00, 0x00, 0x00, 0x06,            /* header length = 6 */
    0x00, 0x01,                        /* format = 1 */
    0x00, 0x02,                        /* ntracks = 2 */
    0x03, 0xC0,                        /* division = 960 */

    /* Track 0 MTrk chunk */
    0x4D, 0x54, 0x72, 0x6B,            /* "MTrk" */
    0x00, 0x00, 0x00, 0x0C,            /* track length = 12 */
    0x00,                              /* delta-time = 0 */
    0x90, 0x3C, 0x64,                  /* note_on  ch0 key=60 vel=100  (tick 0) */
    0x02,                              /* delta-time = 2 */
    0x80, 0x3C, 0x40,                  /* note_off ch0 key=60 vel=64   (tick 2) */
    0x00,                              /* delta-time = 0 */
    0xFF, 0x2F, 0x00,                  /* meta EOT (end of track) */

    /* Track 1 MTrk chunk */
    0x4D, 0x54, 0x72, 0x6B,            /* "MTrk" */
    0x00, 0x00, 0x00, 0x0C,            /* track length = 12 */
    0x01,                              /* delta-time = 1 */
    0xB0, 0x40, 0x7F,                  /* CC     ch0 ctrl=64 val=127 (tick 1) */
    0x01,                              /* delta-time = 1 */
    0xB0, 0x40, 0x00,                  /* CC     ch0 ctrl=64 val=0     (tick 2) */
    0x00,                              /* delta-time = 0 */
    0xFF, 0x2F, 0x00,                  /* meta EOT (end of track) */
};

int
main(void)
{
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    fluid_player_t *player;
    event_recorder_t rec = { { { 0 } }, 0 };
    int i;

    settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    player = new_fluid_player(synth);
    TEST_ASSERT(player != NULL);

    fluid_player_set_playback_callback(player, record_callback, &rec);

    /* Load the MIDI file from memory */
    TEST_SUCCESS(fluid_player_add_mem(player, midi_data, sizeof(midi_data)));

    /* Start playback */
    TEST_SUCCESS(fluid_player_play(player));

    /* Process events: keep calling the player callback until done.
     * The callback is driven by a timer in real usage; here we simulate
     * by calling it with increasing msec values so the tick position
     * advances past all events. With tempo=500000 and division=960,
     * deltatime ≈ 0.52 ms/tick, so msec values 0, 1, 2, ... advance the
     * tick position enough to reach ticks 0, 1, and 2. */
    for(i = 0; i < 10 && fluid_player_get_status(player) == FLUID_PLAYER_PLAYING; i++)
    {
        fluid_player_callback(player, (unsigned int) i);
    }

    /* Verify we recorded the expected events in tick order */
    TEST_ASSERT(rec.count == 4);

    /* Event 1: note_on at tick 0 (track 0) */
    TEST_ASSERT(rec.events[0].type == NOTE_ON);
    TEST_ASSERT(rec.events[0].channel == 0);
    TEST_ASSERT(rec.events[0].param1 == 60);
    TEST_ASSERT(rec.events[0].param2 == 100);

    /* Event 2: CC=127 at tick 1 (track 1) */
    TEST_ASSERT(rec.events[1].type == CONTROL_CHANGE);
    TEST_ASSERT(rec.events[1].channel == 0);
    TEST_ASSERT(rec.events[1].param1 == 64);
    TEST_ASSERT(rec.events[1].param2 == 127);

    /* Event 3: note_off at tick 2 (track 0) */
    TEST_ASSERT(rec.events[2].type == NOTE_OFF);
    TEST_ASSERT(rec.events[2].channel == 0);
    TEST_ASSERT(rec.events[2].param1 == 60);
    TEST_ASSERT(rec.events[2].param2 == 64);

    /* Event 4: CC=0 at tick 2 (track 1) */
    TEST_ASSERT(rec.events[3].type == CONTROL_CHANGE);
    TEST_ASSERT(rec.events[3].channel == 0);
    TEST_ASSERT(rec.events[3].param1 == 64);
    TEST_ASSERT(rec.events[3].param2 == 0);

    /* teardown */
    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
