/* FluidSynth FX - An example of using effects with fluidsynth
 *
 * This code is in the public domain.
 *
 * To compile:
 *   gcc -g -O -o fluidsynth_fx fluidsynth_fx.c -lfluidsynth
 *
 * To run
 *   fluidsynth_fx soundfont gain
 *
 * [Peter Hanappe]
 */


#include <stdio.h>
#include <stdlib.h>
#include <fluidsynth.h>


/* The structure with the effects data. This example simply applies a
 * linear gain the to synthesizer output. */
struct fx_data_t
{
    fluid_synth_t *synth;
    float gain;
} fx_data_t;

/* This function implements the callback function of the audio driver
 * (see new_fluid_audio_driver2 below). The data argument is a pointer
 * to your private data structure. 'len' is the number of audio frames
 * in the buffers. 'nfx' and 'nout' are the number of input and output
 * audio buffers. 'fx' and 'out' are arrays of float buffers containing
 * the audio. The audio driver fills zero-initializes those buffers.
 * You are responsible for filling up those buffers, as the result will
 * be sent to the sound card. This is usually done by asking the synth
 * to fill those buffers appropriately using fluid_synth_process()
 *
 * NOTE: The API was designed to be generic. Audio driver may fill the
 * buffers with audio input from the soundcard, rather than zeros.
 */
int fx_function(void *data, int len,
                int nfx, float **fx,
                int nout, float **out)
{
    struct fx_data_t *fx_data = (struct fx_data_t *) data;
    int i, k;

    if(fx == 0)
    {
        /* Note that some audio drivers may not provide buffers for effects like
         * reverb and chorus. In this case it's your decision what to do. If you
         * had called fluid_synth_process() like in the else branch below, no
         * effects would have been rendered. Instead, you may mix the effects
         * directly into the out buffers. */
        if(fluid_synth_process(fx_data->synth, len, nout, out, nout, out) != FLUID_OK)
        {
            /* Some error occurred. Very unlikely to happen, though. */
            return FLUID_FAILED;
        }
    }
    else
    {
        /* Call the synthesizer to fill the output buffers with its
         * audio output. */
        if(fluid_synth_process(fx_data->synth, len, nfx, fx, nout, out) != FLUID_OK)
        {
            /* Some error occurred. Very unlikely to happen, though. */
            return FLUID_FAILED;
        }
    }

    /* Apply your effects here. In this example, the gain is
     * applied to all the dry-audio output buffers. */
    for(i = 0; i < nout; i++)
    {
        float *out_i = out[i];

        for(k = 0; k < len; k++)
        {
            out_i[k] *= fx_data->gain;
        }
    }

    /* Apply the same effect to all available effect buffer. */
    for(i = 0; i < nfx; i++)
    {
        float *fx_i = fx[i];

        for(k = 0; k < len; k++)
        {
            fx_i[k] *= fx_data->gain;
        }
    }

    return FLUID_OK;
}


int main(int argc, char **argv)
{
    fluid_settings_t *settings;
    fluid_synth_t *synth = NULL;
    fluid_audio_driver_t *adriver = NULL;
    int err = 0;
    struct fx_data_t fx_data;

    if(argc != 3)
    {
        fprintf(stderr, "Usage: fluidsynth_simple [soundfont] [gain]\n");
        return 1;
    }

    /* Create the settings object. This example uses the default
     * values for the settings. */
    settings = new_fluid_settings();

    if(settings == NULL)
    {
        fprintf(stderr, "Failed to create the settings\n");
        err = 2;
        goto cleanup;
    }

    /* Create the synthesizer */
    synth = new_fluid_synth(settings);

    if(synth == NULL)
    {
        fprintf(stderr, "Failed to create the synthesizer\n");
        err = 3;
        goto cleanup;
    }

    /* Load the soundfont */
    if(fluid_synth_sfload(synth, argv[1], 1) == -1)
    {
        fprintf(stderr, "Failed to load the SoundFont\n");
        err = 4;
        goto cleanup;
    }

    /* Fill in the data of the effects unit */
    fx_data.synth = synth;
    fx_data.gain = atof(argv[2]);

    /* Create the audio driver. As soon as the audio driver is
     * created, the synthesizer can be played. */
    adriver = new_fluid_audio_driver2(settings, fx_function, (void *) &fx_data);

    if(adriver == NULL)
    {
        fprintf(stderr, "Failed to create the audio driver\n");
        err = 5;
        goto cleanup;
    }

    /* Play a note */
    fluid_synth_noteon(synth, 0, 60, 100);

    printf("Press \"Enter\" to stop: ");
    fgetc(stdin);
    printf("done\n");


cleanup:

    if(adriver)
    {
        delete_fluid_audio_driver(adriver);
    }

    if(synth)
    {
        delete_fluid_synth(synth);
    }

    if(settings)
    {
        delete_fluid_settings(settings);
    }

    return err;
}
