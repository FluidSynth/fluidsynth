
#include "test.h"
#include "fluidsynth.h"

/* C90: all variable declarations at top of blocks */

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
    int i;
    float left[1024], right[1024];
    float *bufs[2];
    bufs[0] = left;
    bufs[1] = right;

    for(i = 0; i < frames; i += 1024)
    {
        int count = frames - i;
        if(count > 1024)
        {
            count = 1024;
        }
        fluid_synth_process(synth, count, 2, bufs, 0, NULL);
    }
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
    render_frames(synth, 512);

    /* Now set the callback on all playing voices.
     * Since we used noteon, we look for voices on channel 0, key 60. */
    {
        int i;
        fluid_voice_t *voices[256];
        int voice_count = 0;

        /* Use fluid_synth_get_voicelist to find the active voice */
        fluid_synth_get_voicelist(synth, voices, 256, -1);

        for(i = 0; i < 256; i++)
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
                voice_count++;
            }
        }
        TEST_ASSERT(voice_count > 0);
    }

    /* Send noteoff */
    TEST_SUCCESS(fluid_synth_noteoff(synth, 0, 60));

    /* Render some frames to process the noteoff */
    render_frames(synth, 512);

    /* The noteoff callback should have been invoked at least once */
    TEST_ASSERT(noteoff_count > 0);
    TEST_ASSERT(noteoff_data_received == &marker);

    /* Render a large number of frames to let the voice finish its release phase.
     * Use all_sounds_off to force immediate release if needed. */
    render_frames(synth, 44100 * 2);
    fluid_synth_all_sounds_off(synth, 0);
    render_frames(synth, 44100 * 2);

    /* The finished callback should have been invoked */
    TEST_ASSERT(finished_count > 0);
    TEST_ASSERT(finished_data_received == &marker);

    /* === Test 2: Removing callback with NULL === */
    noteoff_count = 0;
    finished_count = 0;

    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 64, 100));
    render_frames(synth, 512);

    {
        fluid_voice_t *voices[256];
        int i;
        int voice_count = 0;

        fluid_synth_get_voicelist(synth, voices, 256, -1);

        for(i = 0; i < 256; i++)
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
                voice_count++;
            }
        }
        TEST_ASSERT(voice_count > 0);
    }

    TEST_SUCCESS(fluid_synth_noteoff(synth, 0, 64));
    render_frames(synth, 512);

    /* Callback was removed, so counts should remain 0 */
    TEST_ASSERT(noteoff_count == 0);

    fluid_synth_all_sounds_off(synth, 0);
    render_frames(synth, 44100 * 2);
    TEST_ASSERT(finished_count == 0);

    /* cleanup */
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
