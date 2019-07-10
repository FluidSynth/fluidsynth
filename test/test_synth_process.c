
#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include <string.h>

// static const int CHANNELS=16;
enum { SAMPLES=1024 };

int render_one_mock(fluid_synth_t *synth, int blocks)
{
    static int smpl;
    
    fluid_real_t *left_in, *fx_left_in;
    fluid_real_t *right_in, *fx_right_in;

    int i, j;

    int nfxchan = fluid_synth_count_effects_channels(synth),
        nfxunits = fluid_synth_count_effects_groups(synth),
        naudchan = fluid_synth_count_audio_channels(synth);

    fluid_rvoice_mixer_get_bufs(synth->eventhandler->mixer, &left_in, &right_in);
    fluid_rvoice_mixer_get_fx_bufs(synth->eventhandler->mixer, &fx_left_in, &fx_right_in);

    for(i = 0; i < naudchan; i++)
    {
        for(j = 0; j < blocks * FLUID_BUFSIZE; j++)
        {
            int idx = i * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE + j;

            right_in[idx] = left_in[idx] = (float)smpl++;
        }
    }

    return blocks;
}

int render_and_check(fluid_synth_t* synth, int number_of_samples, int offset)
{
    int i;
    float left[SAMPLES], right[SAMPLES];
    float *dry[1 * 2];
    dry[0] = left;
    dry[1] = right;
    memset(left, 0, sizeof(left));
    memset(right, 0, sizeof(right));
    
    TEST_SUCCESS(fluid_synth_process_LOCAL(synth, number_of_samples, 0, NULL, 2, dry, render_one_mock));
    
    for(i=0; i<number_of_samples; i++)
    {
        TEST_ASSERT(left[i]==i+offset);
        TEST_ASSERT(right[i]==i+offset);
    }
    
    return i+offset;
}

// this test should make sure that sample rate changed are handled correctly
int main(void)
{
    int off=0;
    fluid_synth_t *synth;
    fluid_settings_t *settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

//     TEST_SUCCESS(fluid_settings_setint(settings, "synth.audio-channels", CHANNELS));
//     TEST_SUCCESS(fluid_settings_setint(settings, "synth.audio-groups", CHANNELS));

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    off = render_and_check(synth, 100, off);
    off = render_and_check(synth, 200, off);
    off = render_and_check(synth, 300, off);
    off = render_and_check(synth, 1000, off);
    off = render_and_check(synth, 900, off);
    off = render_and_check(synth, 800, off);
    

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
