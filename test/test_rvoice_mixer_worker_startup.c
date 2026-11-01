#include "test.h"
#include "fluid_sys.h"
#include <setjmp.h>
#include <string.h>

enum { FRAMES = 8 * FLUID_BUFSIZE };
static fluid_thread_func_t worker;
static void *worker_data;
static jmp_buf worker_wait;

/* Save the worker entry point; render() invokes it after the first audio block. */
static fluid_thread_t *test_new_fluid_thread(const char *name, fluid_thread_func_t func,
                                           void *data, int prio, int detach)
{
    TEST_ASSERT(worker == NULL);
    worker = func;
    worker_data = data;
    return data;
}

static int test_fluid_thread_join(fluid_thread_t *thread)
{
    return FLUID_OK;
}

static void test_delete_fluid_thread(fluid_thread_t *thread)
{
}

/* Unlock the wait mutex and return control to render() without blocking. */
static void test_fluid_cond_wait(fluid_cond_t cond, fluid_cond_mutex_t *mutex)
{
    fluid_cond_mutex_unlock(mutex);
    longjmp(worker_wait, 1);
}

static void render(int cores, float *left, float *right)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    int sfid;

    worker = NULL;
    TEST_ASSERT(settings != NULL);
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.cpu-cores", cores));
    TEST_SUCCESS(fluid_settings_setint(settings, "audio.realtime-prio", 0));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.sample-rate", 48000));
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.reverb.active", 0));
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.chorus.active", 0));
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);
    sfid = fluid_synth_sfload(synth, TEST_SOUNDFONT, 1);
    TEST_ASSERT(sfid >= 0);
    TEST_SUCCESS(fluid_synth_program_select(synth, 0, sfid, 0, 0));
    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 60, 100));
    TEST_SUCCESS(fluid_synth_write_float(synth, FLUID_BUFSIZE, left, 0, 1, right, 0, 1));
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) > 0);
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) < 8);

    if(cores > 1)
    {
        TEST_ASSERT(worker != NULL);
        if(setjmp(worker_wait) == 0)
        {
            worker(worker_data);
            TEST_ASSERT(0); /* The worker must wait for its first job. */
        }
    }

    TEST_SUCCESS(fluid_synth_write_float(synth, FRAMES - FLUID_BUFSIZE,
                                        left, FLUID_BUFSIZE, 1, right, FLUID_BUFSIZE, 1));
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

int main(void)
{
    float reference_left[FRAMES], reference_right[FRAMES];
    float actual_left[FRAMES], actual_right[FRAMES];

    render(1, reference_left, reference_right);
    render(2, actual_left, actual_right);
    TEST_ASSERT(memcmp(reference_left, actual_left, sizeof(reference_left)) == 0);
    TEST_ASSERT(memcmp(reference_right, actual_right, sizeof(reference_right)) == 0);
    return 0;
}

/* Compile this test's mixer with deferred thread creation and controlled waits. */
#define new_fluid_thread test_new_fluid_thread
#define delete_fluid_thread test_delete_fluid_thread
#define fluid_thread_join test_fluid_thread_join
#define fluid_cond_wait test_fluid_cond_wait
#include "../src/rvoice/fluid_rvoice_mixer.c"
