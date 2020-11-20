/*

  An example of how to use FluidSynth.

  To compile it on Linux:
  $ gcc -o example example.c `pkg-config fluidsynth --libs`

  To compile it on Windows:
    ...


  Author: Peter Hanappe.
  This code is in the public domain. Use it as you like.

*/

#include <fluidsynth.h>

#if defined(WIN32)
#include <windows.h>
#define sleep(_t) Sleep(_t * 1000)
#else
#include <stdlib.h>
#include <unistd.h>
#endif

int main(int argc, char **argv)
{
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    fluid_audio_driver_t *adriver;
    int sfont_id;
    int i, key;

    /* Create the settings. */
    settings = new_fluid_settings();

    /* Change the settings if necessary*/

    /* Create the synthesizer. */
    synth = new_fluid_synth(settings);

    /* Create the audio driver. The synthesizer starts playing as soon
       as the driver is created. */
    adriver = new_fluid_audio_driver(settings, synth);

    /* Load a SoundFont and reset presets (so that new instruments
     * get used from the SoundFont) */
    sfont_id = fluid_synth_sfload(synth, "example.sf2", 1);

    if(sfont_id == FLUID_FAILED)
    {
        puts("Loading the SoundFont failed!");
        goto err;
    }

    /* Initialize the random number generator */
    srand(getpid());

    for(i = 0; i < 12; i++)
    {

        /* Generate a random key */
        key = 60 + (int)(12.0f * rand() / (float) RAND_MAX);

        /* Play a note */
        fluid_synth_noteon(synth, 0, key, 80);

        /* Sleep for 1 second */
        sleep(1);

        /* Stop the note */
        fluid_synth_noteoff(synth, 0, key);
    }

err:
    /* Clean up */
    delete_fluid_audio_driver(adriver);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return 0;
}
