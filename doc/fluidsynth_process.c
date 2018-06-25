/*
 * This is a C99 program that outlines different usage examples for fluid_synth_process()
 */

#include <stdio.h>
#include <string.h>
#include <fluidsynth.h>

int main()
{
    // any arbitrary number of audio samples to render during on call of fluid_synth_process()
    enum { SAMPLES = 512 };

    // ...creation of synth omitted...

    // USECASE1: render all dry audio channels + reverb and chorus to one stereo channel
    {
        // planar sample buffers that received synthesized (monophonic) audio
        float left[SAMPLES], right[SAMPLES];

        // array of buffers used to setup channel mapping
        float *dry[1 * 2], *fx[1 * 2];

        // first make sure to zero out the sample buffers
        memset(left, 0, sizeof(left));
        memset(right, 0, sizeof(right));

        // setup channel mapping for a single stereo channel to which to render all dry audio to
        dry[0] = left;
        dry[1] = right;

        // Setup channel mapping for a single stereo channel to which to render effects to.
        // Just using the same sample buffers as for dry audio is fine here, as it will cause the effects to be mixed with dry output.
        // Note: reverb and chorus together make up two stereo channels. Setting up only one stereo channel is sufficient
        // as the channels warp around (i.e. chorus will be mixed with reverb channel).
        fx[0] = left;
        fx[1] = right;

        int err = fluid_synth_process(synth, SAMPLES, 2, fx, 2, dry);

        if(err == FLUID_FAILED)
        {
            puts(„oops“);
        }


        // USECASE2: only render dry audio and discard effects
        // same as above, but call fluid_synth_process() like:
        int err = fluid_synth_process(synth, SAMPLES, 0, NULL, 2, dry);

        if(err == FLUID_FAILED)
        {
            puts(„oops“);
        }
    }


    // USECASE3: render audio and discard all samples
    {
        int err = fluid_synth_process(synth, SAMPLES, 0, NULL, 0, NULL);

        if(err == FLUID_FAILED)
        {
            puts(„oops“);
        }
    }


    // USECASE4: multi-channel rendering, i.e. render all audio and effects channels to dedicated audio buffers
    // ofc it‘s not a good idea to allocate all the arrays on the stack
    {
        // lookup number of audio and effect (stereo-)channels of the synth
        // see „synth.audio-channels“ and „synth.effects-channels“ settings respectively
        int n_aud_chan = fluid_synth_count_audio_channels(synth);
        int n_fx_chan = fluid_synth_count_effects_channels(synth);

        // allocate one single sample buffer
        float samp_buf[SAMPLES * (n_aud_chan + n_fx_chan) * 2];

        // array of buffers used to setup channel mapping
        float *dry[n_aud_chan * 2], *fx[n_fx_chan * 2];

        // setup buffers to mix dry stereo audio to
        // buffers are alternating left and right for each n_aud_chan, i.e.:
        // dry[0] = first audio channel left
        // dry[1] = first audio channel right
        // dry[2] = second audio channel left
        // ...
        // dry[i*2 + 0] = i‘th audio channel left
        // dry[i*2 + 1] = i‘th audio channel right
        for(int i = 0; i < n_aud_chan * 2; i++)
        {
            dry[i] = &samp_buf[i * SAMPLES];
        }

        // setup buffers to mix effects stereo audio to
        // similar channel layout as above, but currently special as there are only 2 hardcoded effects channels:
        // fx[0] = global reverb channel left
        // fx[1] = global reverb channel right
        // fx[2] = global chorus channel left
        // fx[3] = global chorus channel right
        for(int i = 0; i < n_fx_chan * 2; i++)
        {
            fx[i] = &samp_buf[n_aud_chan * 2 * SAMPLES + i * SAMPLES];
        }

        // dont forget to zero sample buffer(s) before each rendering
        memset(samp_buf, 0, sizeof(samp_buf));

        int err = fluid_synth_process(synth, SAMPLES, n_fx_chan * 2, fx, n_aud_chan * 2, dry);

        if(err == FLUID_FAILED)
        {
            puts(„oops“);
        }
    }

    return 0;
}
