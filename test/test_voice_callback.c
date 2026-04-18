
#include "test.h"
#include "fluidsynth.h"
#include "fluid_midi.h"

enum { Polyphony = 64 };

static int noteoff_count = 0;
static int finished_count = 0;
static void *noteoff_data_received = NULL;
static void *finished_data_received = NULL;

static void voice_callback(const fluid_voice_t *voice, enum fluid_voice_callback_reason reason, void *data)
{
    if(reason == FLUID_VOICE_CALLBACK_NOTEOFF)
    {
        noteoff_count++;
        noteoff_data_received = data;
    }
    else if(reason == FLUID_VOICE_CALLBACK_FINISHED)
    {
        finished_count++;
        finished_data_received = data;
    }
}

/* Render some audio to advance the synth state */
static void render_frames(fluid_synth_t *synth, int frames)
{
    fluid_synth_process(synth, frames, 0, NULL, 0, NULL);
}

/* Reset callback counters and received-data pointers */
static void reset_callback_state(void)
{
    noteoff_count = 0;
    finished_count = 0;
    noteoff_data_received = NULL;
    finished_data_received = NULL;
}

/*
 * Find all playing voices on the given channel and key, apply a callback
 * (may be NULL to clear it) with the given user data, and configure instant
 * release so tests finish quickly.  Returns the number of voices configured.
 */
static int setup_voices(fluid_synth_t *synth, int chan, int key,
                        fluid_voice_callback_t callback, void *callback_data)
{
    fluid_voice_t *voices[Polyphony];
    int i;
    int voice_count = 0;

    fluid_synth_get_voicelist(synth, voices, Polyphony, -1);

    for(i = 0; i < Polyphony; i++)
    {
        if(voices[i] == NULL)
        {
            break;
        }
        if(fluid_voice_get_key(voices[i]) == key &&
           fluid_voice_get_channel(voices[i]) == chan &&
           fluid_voice_is_playing(voices[i]))
        {
            fluid_voice_set_callback(voices[i], callback, callback_data);
            fluid_voice_gen_set(voices[i], GEN_VOLENVRELEASE, -32768);
            fluid_voice_update_param(voices[i], GEN_VOLENVRELEASE);
            voice_count++;
        }
    }

    return voice_count;
}

int main(void)
{
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    int sfont_id;
    int marker = 42;

    settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    /* Disable reverb and chorus for faster/cleaner test */
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.reverb.active", 0));
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.chorus.active", 0));

    TEST_SUCCESS(fluid_settings_setint(settings, "synth.polyphony", Polyphony));

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    sfont_id = fluid_synth_sfload(synth, TEST_SOUNDFONT, 1);
    TEST_ASSERT(sfont_id != FLUID_FAILED);

    /* === Test 1: Callback receives noteoff and finished events === */
    reset_callback_state();

    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 60, 100));
    render_frames(synth, fluid_synth_get_internal_bufsize(synth));

    TEST_ASSERT(setup_voices(synth, 0, 60, voice_callback, &marker) > 0);

    TEST_SUCCESS(fluid_synth_noteoff(synth, 0, 60));

    TEST_ASSERT(noteoff_count > 0);
    TEST_ASSERT(noteoff_data_received == &marker);

    render_frames(synth, 44100 * 2);

    TEST_ASSERT(finished_count > 0);
    TEST_ASSERT(finished_data_received == &marker);

    /* === Test 2: Removing callback with NULL === */
    reset_callback_state();

    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 64, 100));
    render_frames(synth, fluid_synth_get_internal_bufsize(synth));

    /* Set callback then immediately remove it */
    TEST_ASSERT(setup_voices(synth, 0, 64, voice_callback, &marker) > 0);
    TEST_ASSERT(setup_voices(synth, 0, 64, NULL, NULL) > 0);

    TEST_SUCCESS(fluid_synth_noteoff(synth, 0, 64));
    render_frames(synth, fluid_synth_get_internal_bufsize(synth));
    TEST_SUCCESS(fluid_synth_all_sounds_off(synth, 0));
    render_frames(synth, 44100 * 2);

    /* Callback was removed, so counts should remain 0 */
    TEST_ASSERT(noteoff_count == 0);
    TEST_ASSERT(finished_count == 0);

    /* === Test 3: Sustain pedal defers the noteoff callback === */
    reset_callback_state();

    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 60, 100));
    render_frames(synth, fluid_synth_get_internal_bufsize(synth));

    TEST_ASSERT(setup_voices(synth, 0, 60, voice_callback, &marker) > 0);

    fluid_synth_cc(synth, 0, SUSTAIN_SWITCH, 127);

    TEST_SUCCESS(fluid_synth_noteoff(synth, 0, 60));

    /* No callback yet — sustain is holding the voice */
    TEST_ASSERT(noteoff_count == 0);

    render_frames(synth, fluid_synth_get_internal_bufsize(synth));

    /* Still no callback */
    TEST_ASSERT(noteoff_count == 0);
    TEST_ASSERT(noteoff_data_received == NULL);

    /* Releasing sustain triggers the deferred noteoff */
    fluid_synth_cc(synth, 0, SUSTAIN_SWITCH, 0);
    TEST_ASSERT(noteoff_count > 0);
    TEST_ASSERT(noteoff_data_received == &marker);

    render_frames(synth, 44100 * 2);

    TEST_ASSERT(finished_count > 0);
    TEST_ASSERT(finished_data_received == &marker);

    /* cleanup */
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
