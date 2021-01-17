/* FluidSynth Simple - An example of using fluidsynth
 *
 * This code is in the public domain.
 *
 * To compile:
 *   gcc -g -O -o fluidsynth_simple fluidsynth_simple.c -lfluidsynth
 *
 * To run
 *   fluidsynth_simple soundfont
 *
 * [Peter Hanappe]
 */


#include <stdio.h>
#include <fluidsynth.h>

int main(int argc, char **argv)
{
    fluid_settings_t *settings;
    fluid_synth_t *synth = NULL;
    fluid_audio_driver_t *adriver = NULL;
    int err = 0;

    if(argc != 2)
    {
        fprintf(stderr, "Usage: fluidsynth_simple [soundfont]\n");
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

    /* Create the audio driver. As soon as the audio driver is
     * created, the synthesizer can be played. */
    adriver = new_fluid_audio_driver(settings, synth);

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
