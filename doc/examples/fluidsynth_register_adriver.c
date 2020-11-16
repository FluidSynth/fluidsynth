/*
 * This is a simple C99 program that demonstrates the usage of fluid_audio_driver_register()
 *
 * There are 3 calls to fluid_audio_driver_register(), i.e. 3 iterations:
 * First the alsa driver is registered and created, followed by the jack and portaudio driver.
 *
 * The usual usecase would be to call fluid_audio_driver_register() only once providing the audio drivers needed during fluidsynth usage.
 * If necessary however fluid_audio_driver_register() can be called multiple times as demonstrated here.
 * Therefore the user must make sure to delete all fluid-instances of any kind before making the call to fluid_audio_driver_register().
 * Else the behaviour is undefined and the application is likely to crash.
 */

#include <stdio.h>
#include <fluidsynth.h>

int main()
{
    const char *DRV[] = { "alsa", "jack", "portaudio" };
    const char *adrivers[2];

    /* three iterations, first register only alsa, then only jack, and last portaudio
     * ...just to demonstrate how and under which conditions fluid_audio_driver_register()
     * can be called
     */
    for(int i = 0; i < sizeof(DRV) / sizeof(DRV[0]); i++)
    {
        adrivers[0] = DRV[i];
        /* register any other driver you need
         *
         * adrivers[X] = "whatever";
         */
        adrivers[1] = NULL; /* NULL terminate the array */

        /* register those audio drivers. Note that at this time no fluidsynth objects are alive! */
        int res = fluid_audio_driver_register(adrivers);

        if(res != FLUID_OK)
        {
            puts("adriver reg err");
            return -1;
        }

        fluid_settings_t *settings = new_fluid_settings();
        res = fluid_settings_setstr(settings, "audio.driver", DRV[i]);

        /* As of fluidsynth 2, settings API has been refactored to return FLUID_OK|FAILED
         * rather than returning TRUE or FALSE
         */
#if FLUIDSYNTH_VERSION_MAJOR >= 2
        if(res != FLUID_OK)
#else
        if(res == 0)
#endif
        {
            puts("audio.driver set err");
            return -1;
        }

        fluid_synth_t *synth = new_fluid_synth(settings);
        fluid_audio_driver_t *ad = new_fluid_audio_driver(settings, synth);

        /*
         * ~~~ Do your daily business here ~~~
         */

        delete_fluid_audio_driver(ad);
        delete_fluid_synth(synth);
        delete_fluid_settings(settings);

        /* everything cleaned up, fluid_audio_driver_register() can be called again if needed */
    }

    return 0;
}
