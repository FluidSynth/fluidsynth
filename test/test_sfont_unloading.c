
#include "test.h"
#include "fluidsynth.h"
#include "synth/fluid_synth.h"
#include "utils/fluid_sys.h"

// we make this a macro, so the line information of the TEST_ASSERT macro remains useful
#define WAIT_AND_FREE \
do {\
    list = synth->fonts_to_be_unloaded;\
    synth->fonts_to_be_unloaded = NULL;\
    delete_fluid_synth(synth);\
    \
    for(; list; list = fluid_list_next(list))\
    {\
        fluid_timer_t* timer = fluid_list_get(list);\
        FLUID_LOG(FLUID_INFO, "%s(): Start waiting for soundfont %d to unload", __func__, id);\
        if(fluid_timer_is_running(timer))\
        {\
            /* timer still running, wait a bit*/\
            fluid_msleep(50 * fluid_timer_get_interval(timer));\
            TEST_ASSERT(!fluid_timer_is_running(timer));\
        }\
        delete_fluid_timer(timer);\
        FLUID_LOG(FLUID_INFO, "%s(): End waiting for soundfont %d to unload", __func__, id);\
    }\
    delete_fluid_list(list);\
} while(0);

static void test_without_rendering(fluid_settings_t* settings)
{
    int id;
    fluid_list_t *list;
    fluid_synth_t *synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    TEST_ASSERT(fluid_is_soundfont(TEST_SOUNDFONT) == TRUE);

    // load a sfont to synth
    TEST_SUCCESS(id = fluid_synth_sfload(synth, TEST_SOUNDFONT, 1));
    // one sfont loaded
    TEST_ASSERT(fluid_synth_sfcount(synth) == 1);
    
    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 60, 127));
    
    TEST_SUCCESS(fluid_synth_sfunload(synth, id, 1));
    
    TEST_SUCCESS(fluid_synth_noteoff(synth, 0, 60));
    
    // there must be one font scheduled for lazy unloading
    TEST_ASSERT(synth->fonts_to_be_unloaded != NULL);
    
    WAIT_AND_FREE;
}

// this should work fine after applying JJCs fix a4ac56502fec5f0c20a60187d965c94ba1dc81c2
static void test_after_polyphony_exceeded(fluid_settings_t* settings)
{
    int id;
    fluid_list_t *list;
    fluid_synth_t *synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    TEST_ASSERT(fluid_is_soundfont(TEST_SOUNDFONT) == TRUE);

    // load a sfont to synth
    TEST_SUCCESS(id = fluid_synth_sfload(synth, TEST_SOUNDFONT, 1));
    // one sfont loaded
    TEST_ASSERT(fluid_synth_sfcount(synth) == 1);
    
    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 60, 127));
    FLUID_LOG(FLUID_INFO, "test_after_polyphony_exceeded(): note on C4, voice count=%d",
                           fluid_synth_get_active_voice_count(synth));
    
    // need to render a bit to make synth->ticks_since_start advance, to make the previous voice "killable"
    TEST_SUCCESS(fluid_synth_process(synth, 2048, 0, NULL, 0, NULL));
    
    // polyphony exceeded - killing the killable voice from above
    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 61, 127));
    
    // need to render again, to make the synth thread assign rvoice->dsp.sample, so that sample_unref() later really unrefs
    TEST_SUCCESS(fluid_synth_process(synth, 2048, 0, NULL, 0, NULL));
    FLUID_LOG(FLUID_INFO, "test_after_polyphony_exceeded(): note on C#4, voice count=%d",
                           fluid_synth_get_active_voice_count(synth));

    FLUID_LOG(FLUID_INFO, "test_after_polyphony_exceeded(): unload sounfont");
    TEST_SUCCESS(fluid_synth_sfunload(synth, id, 1));
    
    TEST_SUCCESS(fluid_synth_noteoff(synth, 0, 61));
    
    // need to render yet again, to make the synth thread release the rvoice so it can be reclaimed by
    // fluid_synth_check_finished_voices()
    // need to render may more samples this time, so the voice makes it pass the release phase...
    TEST_SUCCESS(fluid_synth_process(synth, 204800, 0, NULL, 0, NULL));
    
    // make any API call to execute fluid_synth_check_finished_voices()
    FLUID_LOG(FLUID_INFO, "test_after_polyphony_exceeded(): note off C#4, voice count=%d",
                           fluid_synth_get_active_voice_count(synth));
    
    // there must be one font scheduled for lazy unloading
    TEST_ASSERT(synth->fonts_to_be_unloaded != NULL);
    
    WAIT_AND_FREE;
}

static void test_default_polyphony(fluid_settings_t* settings)
{
    enum { BUFSIZE = 128 };
    fluid_voice_t* buf[BUFSIZE];
    
    int id;
    fluid_list_t *list;
    fluid_synth_t *synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    TEST_ASSERT(fluid_is_soundfont(TEST_SOUNDFONT) == TRUE);

    // load a sfont to synth
    TEST_SUCCESS(id = fluid_synth_sfload(synth, TEST_SOUNDFONT, 1));
    // one sfont loaded
    TEST_ASSERT(fluid_synth_sfcount(synth) == 1);
    
    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 60, 127));
    
    TEST_SUCCESS(fluid_synth_process(synth, fluid_synth_get_internal_bufsize(synth), 0, NULL, 0, NULL));
    
    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 61, 127));
    
    fluid_synth_get_voicelist(synth, buf, BUFSIZE, -1);
    
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) == 4);
    
    // make the synth thread assign rvoice->dsp.sample
    TEST_SUCCESS(fluid_synth_process(synth, 2 * fluid_synth_get_internal_bufsize(synth), 0, NULL, 0, NULL));
    
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) == 4);

    TEST_ASSERT(synth->fonts_to_be_unloaded == NULL);
    
    TEST_SUCCESS(fluid_synth_sfunload(synth, id, 1));
    
    // now, there must be one font scheduled for lazy unloading
    TEST_ASSERT(synth->fonts_to_be_unloaded != NULL);
    TEST_ASSERT(fluid_timer_is_running(fluid_list_get(synth->fonts_to_be_unloaded)));
    
    // noteoff the second note and render something
    TEST_SUCCESS(fluid_synth_noteoff(synth, 0, 61));
    TEST_SUCCESS(fluid_synth_process(synth, fluid_synth_get_internal_bufsize(synth), 0, NULL, 0, NULL));
    
    // still 4 because key 61 is playing in release phase now
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) == 4);
    // must be still running
    TEST_ASSERT(synth->fonts_to_be_unloaded != NULL);
    TEST_ASSERT(fluid_timer_is_running(fluid_list_get(synth->fonts_to_be_unloaded)));
    
    // noteoff the first note and render something
    TEST_SUCCESS(fluid_synth_noteoff(synth, 0, 60));
    TEST_SUCCESS(fluid_synth_process(synth, fluid_synth_get_internal_bufsize(synth), 0, NULL, 0, NULL));
    
    // still 4 because keys 60 + 61 are playing in release phase now
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) == 4);
    // must be still running
    TEST_ASSERT(synth->fonts_to_be_unloaded != NULL);
    TEST_ASSERT(fluid_timer_is_running(fluid_list_get(synth->fonts_to_be_unloaded)));
    
    
    // render enough, to make the synth thread release the rvoice so it can be reclaimed by
    // fluid_synth_check_finished_voices()
    TEST_SUCCESS(fluid_synth_process(synth, 2048000, 0, NULL, 0, NULL));
    
    // this API call should reclaim the rvoices and call fluid_voice_stop()
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) == 0);

    TEST_ASSERT(synth->fonts_to_be_unloaded != NULL);
    // We want to see that the timer thread unloads the soundfont before we call delete_fluid_synth().
    // Wait to give the timer thread a chance to unload and finish.
    fluid_msleep(10 * fluid_timer_get_interval(fluid_list_get(synth->fonts_to_be_unloaded)));
    TEST_ASSERT(!fluid_timer_is_running(fluid_list_get(synth->fonts_to_be_unloaded)));
    
    WAIT_AND_FREE;
}

// this tests the soundfont loading API of the synth.
// might be expanded to test the soundfont loader as well...
int main(void)
{
    fluid_settings_t *settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);
    
    FLUID_LOG(FLUID_INFO, "Begin test_default_polyphony()");
    test_default_polyphony(settings);
    FLUID_LOG(FLUID_INFO, "End test_default_polyphony()\n");
    
    fluid_settings_setint(settings, "synth.polyphony", 2);
    
    FLUID_LOG(FLUID_INFO, "Begin test_after_polyphony_exceeded()");
    test_after_polyphony_exceeded(settings);
    FLUID_LOG(FLUID_INFO, "End test_after_polyphony_exceeded()\n");

    FLUID_LOG(FLUID_INFO, "Begin test_without_rendering()");
    test_without_rendering(settings);
    FLUID_LOG(FLUID_INFO, "End test_without_rendering()");
    
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
