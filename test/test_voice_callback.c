
#include "test.h"
#include "fluidsynth.h"
#include "fluid_midi.h"

enum { Polyphony = 64 };

static int noteoff_count = 0;
static int finished_count = 0;
static void *noteoff_data_received = NULL;
static void *finished_data_received = NULL;
static unsigned int noteoff_voice_id = 0;
static unsigned int finished_voice_id = 0;

static void voice_callback(fluid_voice_t *voice, enum fluid_voice_callback_reason reason, void *data)
{
    if(reason == FLUID_VOICE_CALLBACK_NOTEOFF)
    {
        noteoff_count++;
        noteoff_data_received = data;
        noteoff_voice_id = fluid_voice_get_id(voice);
    }
    else if(reason == FLUID_VOICE_CALLBACK_FINISHED)
    {
        finished_count++;
        finished_data_received = data;
        finished_voice_id = fluid_voice_get_id(voice);
    }
}

/* Render some audio to advance the synth state */
static void render_frames(fluid_synth_t *synth, int frames)
{
    fluid_synth_process(synth, frames, 0, NULL, 0, NULL);
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
    noteoff_count = 0;
    finished_count = 0;
    noteoff_data_received = NULL;
    finished_data_received = NULL;

    /* Play a note using the public MIDI API (noteon) */
    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 60, 100));

    /* Render a small amount to let the voice start */
    render_frames(synth, fluid_synth_get_internal_bufsize(synth));

    /* Now set the callback on all playing voices.
     * Since we used noteon, we look for voices on channel 0, key 60. */
    {
        int i;
        fluid_voice_t *voices[Polyphony];
        int voice_count = 0;

        /* Use fluid_synth_get_voicelist to find the active voice */
        fluid_synth_get_voicelist(synth, voices, Polyphony, -1);

        for(i = 0; i < Polyphony; i++)
        {
            if(voices[i] == NULL)
            {
                break;
            }
            if(fluid_voice_get_key(voices[i]) == 60 &&
               fluid_voice_get_channel(voices[i]) == 0 &&
               fluid_voice_is_playing(voices[i]))
            {
                fluid_voice_set_callback(voices[i], voice_callback, &marker);
                // Set the voice to instant release
                fluid_voice_gen_set(voices[i], GEN_VOLENVRELEASE, -32768);
                fluid_voice_update_param(voices[i], GEN_VOLENVRELEASE);
                voice_count++;
            }
        }
        TEST_ASSERT(voice_count > 0);
    }

    /* Send noteoff */
    TEST_SUCCESS(fluid_synth_noteoff(synth, 0, 60));

    /* The noteoff callback should have been invoked at least once */
    TEST_ASSERT(noteoff_count > 0);
    TEST_ASSERT(noteoff_data_received == &marker);

    /* Render a large number of frames to let the voice finish its release phase.
     * Use all_sounds_off to force immediate release if needed. */
    render_frames(synth, 44100 * 2);

    /* The finished callback should have been invoked */
    TEST_ASSERT(finished_count > 0);
    TEST_ASSERT(finished_data_received == &marker);

    /* === Test 2: Removing callback with NULL === */
    noteoff_count = 0;
    finished_count = 0;

    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 64, 100));
    render_frames(synth, fluid_synth_get_internal_bufsize(synth));

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
            if(fluid_voice_get_key(voices[i]) == 64 &&
               fluid_voice_get_channel(voices[i]) == 0 &&
               fluid_voice_is_playing(voices[i]))
            {
                /* Set callback then remove it */
                fluid_voice_set_callback(voices[i], voice_callback, &marker);
                fluid_voice_set_callback(voices[i], NULL, NULL);
                // Set the voice to instant release
                fluid_voice_gen_set(voices[i], GEN_VOLENVRELEASE, -32768);
                fluid_voice_update_param(voices[i], GEN_VOLENVRELEASE);
                voice_count++;
            }
        }
        TEST_ASSERT(voice_count > 0);
    }

    TEST_SUCCESS(fluid_synth_noteoff(synth, 0, 64));
    render_frames(synth, fluid_synth_get_internal_bufsize(synth));
    TEST_SUCCESS(fluid_synth_all_sounds_off(synth, 0));
    render_frames(synth, 44100 * 2);

    /* Callback was removed, so counts should remain 0 */
    TEST_ASSERT(noteoff_count == 0);
    TEST_ASSERT(finished_count == 0);

    /* === Test 3: Same as test 1, but activate sustain to defer noteOff callback === */
    noteoff_count = 0;
    finished_count = 0;
    noteoff_data_received = NULL;
    finished_data_received = NULL;

    /* Play a note using the public MIDI API (noteon) */
    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 60, 100));

    /* Render a small amount to let the voice start */
    render_frames(synth, fluid_synth_get_internal_bufsize(synth));

    {
        int i;
        fluid_voice_t *voices[Polyphony];
        int voice_count = 0;

        /* Use fluid_synth_get_voicelist to find the active voice */
        fluid_synth_get_voicelist(synth, voices, Polyphony, -1);

        for(i = 0; i < Polyphony; i++)
        {
            if(voices[i] == NULL)
            {
                break;
            }
            if(fluid_voice_get_key(voices[i]) == 60 &&
               fluid_voice_get_channel(voices[i]) == 0 &&
               fluid_voice_is_playing(voices[i]))
            {
                fluid_voice_set_callback(voices[i], voice_callback, &marker);
                // Set the voice to instant release
                fluid_voice_gen_set(voices[i], GEN_VOLENVRELEASE, -32768);
                fluid_voice_update_param(voices[i], GEN_VOLENVRELEASE);
                voice_count++;
            }
        }
        TEST_ASSERT(voice_count > 0);
    }

    fluid_synth_cc(synth, 0, SUSTAIN_SWITCH, 127);

    /* Send noteoff */
    TEST_SUCCESS(fluid_synth_noteoff(synth, 0, 60));

    // no callback yet because sustain is on
    TEST_ASSERT(noteoff_count == 0);

    // render some frames
    render_frames(synth, fluid_synth_get_internal_bufsize(synth));

    // still no callback
    TEST_ASSERT(noteoff_count == 0);
    TEST_ASSERT(noteoff_data_received == NULL);

    fluid_synth_cc(synth, 0, SUSTAIN_SWITCH, 0);
    TEST_ASSERT(noteoff_count > 0);
    TEST_ASSERT(noteoff_data_received == &marker);

    /* Render a large number of frames to let the voice finish its release phase.
     * Use all_sounds_off to force immediate release if needed. */
    render_frames(synth, 44100 * 2);

    /* The finished callback should have been invoked */
    TEST_ASSERT(finished_count > 0);
    TEST_ASSERT(finished_data_received == &marker);

    /* cleanup */
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
