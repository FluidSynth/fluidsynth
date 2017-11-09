/**
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
    const char* DRV[] = { "alsa", "jack", "portaudio" };
    const char* adrivers[2];
    
    for(int i=0; i<sizeof(DRV)/sizeof(DRV[0]); i++)
    {
        adrivers[0] = DRV[i];
        adrivers[1] = NULL;

        int res = fluid_audio_driver_register(adrivers);
        if(res != FLUID_OK)
        {
            puts("adriver reg err");
            return -1;
        }

        fluid_settings_t* settings = new_fluid_settings();
        res = fluid_settings_setstr(settings, "audio.driver", DRV[i]);
        if(res != FLUID_OK)
        {
            puts("audio.driver set err");
            return -1;
        }

        fluid_synth_t* synth = new_fluid_synth(settings);
        fluid_audio_driver_t* ad = new_fluid_audio_driver(settings, synth);

        delete_fluid_audio_driver(ad);
        delete_fluid_synth(synth);
        delete_fluid_settings(settings);
    }
    return 0;
}
