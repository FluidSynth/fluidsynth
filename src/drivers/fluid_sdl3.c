/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 * Copyright (C) 2018  Carlo Bramini
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_settings.h"

#if SDL3_SUPPORT

#include <SDL3/SDL.h>

typedef struct AudioDeviceInfo
{
    SDL_AudioDeviceID devid;
    char *name;
} AudioDeviceInfo;

typedef struct AudioDeviceList
{
    AudioDeviceInfo *devices;
    int num_devices;
} AudioDeviceList;

typedef struct
{
    fluid_audio_driver_t driver;

    fluid_synth_t *synth;
    fluid_audio_callback_t write_ptr;

    SDL_AudioDeviceID devid;
    SDL_AudioStream *stream;
    AudioDeviceList AudioSDL3PlaybackDevices;
    AudioDeviceList AudioSDL3RecordingDevices;

    int frame_size;
    int period_size;
} fluid_sdl3_audio_driver_t;

const char *SDLC_GetAudioDeviceName(int idx, int iscapture, fluid_sdl3_audio_driver_t *dev)
{
    AudioDeviceList *list;
    SDL_Mutex *AudioDeviceLock = SDL_CreateMutex();
    const char *retval = NULL;

    if (!SDL_GetCurrentAudioDriver()) {
        SDL_SetError("Audio subsystem is not initialized");
        return NULL;
    }

    SDL_LockMutex(AudioDeviceLock);
    list = iscapture ? &(dev->AudioSDL3RecordingDevices) : &(dev->AudioSDL3PlaybackDevices);
    if ((idx < 0) || (idx >= list->num_devices)) {
        SDL_InvalidParamError("index");
    } else {
        retval = list->devices[idx].name;
    }
    SDL_UnlockMutex(AudioDeviceLock);
    SDL_free(AudioDeviceLock);

    return retval;
}

static int GetNumAudioDevices(int iscapture, fluid_sdl3_audio_driver_t *dev)
{
    AudioDeviceList newlist;
    AudioDeviceList *list = NULL;
    SDL_AudioDeviceID *devices = NULL;
    int num_devices = 0;
    int i = 0;

    if (dev != NULL)
    {
        list = iscapture ? &(dev->AudioSDL3RecordingDevices) : &(dev->AudioSDL3PlaybackDevices);
    }

    /* SDL_GetNumAudioDevices triggers a device redetect in sdl3, so we'll just build our list from here. */
    devices = iscapture ? SDL_GetAudioRecordingDevices(&num_devices) : SDL_GetAudioPlaybackDevices(&num_devices);
    if (!devices) {
        return list->num_devices;  /* just return the existing one for now. Oh well. */
    }

    SDL_zero(newlist);
    if (num_devices > 0) {
        newlist.num_devices = num_devices;
        newlist.devices = (AudioDeviceInfo *) SDL_malloc(sizeof (AudioDeviceInfo) * num_devices);
        if (!newlist.devices) {
            SDL_free(devices);
            return list->num_devices;  /* just return the existing one for now. Oh well. */
        }

        for (i = 0; i < num_devices; i++) {
            const char *newname = SDL_GetAudioDeviceName(devices[i]);
            char *fullname = NULL;
            if (newname == NULL) {
                /* ugh, whatever, just make up a name. */
                newname = "Unidentified device";
            }

            /* Device names must be unique in sdl3, as that's how we open them.
               sdl3 took serious pains to try to add numbers to the end of duplicate device names ("SoundBlaster Pro" and then "SoundBlaster Pro (2)"),
               but here we're just putting the actual SDL3 instance id at the end of everything. Good enough. I hope. */
            if (!newname || (SDL_asprintf(&fullname, "%s (id=%u)", newname, (unsigned int) devices[i]) < 0)) {
                /* we're in real trouble now.  :/  */
                int j;
                for (j = 0; j < i; j++) {
                    SDL_free(newlist.devices[i].name);
                }
                SDL_free(fullname);
                SDL_free(devices);
                return list->num_devices;  /* just return the existing one for now. Oh well. */
            }

            newlist.devices[i].devid = devices[i];
            newlist.devices[i].name = fullname;
        }
    }

    if (list != NULL)
    {
        for (i = 0; i < list->num_devices; i++) {
            SDL_free(list->devices[i].name);
        }
        SDL_free(list->devices);
    }

    SDL_free(devices);

    if (list != NULL)
    {
        SDL_memcpy(list, &newlist, sizeof (AudioDeviceList));
    }

    return num_devices;
}

static void
SDLAudioCallback(void *data, SDL_AudioStream *stream, int add_len, int len)
{
    fluid_sdl3_audio_driver_t *dev = (fluid_sdl3_audio_driver_t *)data;
    unsigned int *buffer = SDL_malloc(dev->period_size * dev->frame_size);
    int buf_len = 0;

    if (buffer == NULL)
    {
        FLUID_LOG(FLUID_WARN, "Audio callback buffer allocation has failed");
        return;
    }

    while (add_len > 0)
    {
        buf_len = SDL_min(add_len, dev->period_size * dev->frame_size);
        dev->write_ptr(dev->synth, buf_len / dev->frame_size, buffer, 0, 2, buffer, 1, 2);
        SDL_PutAudioStreamData(stream, buffer, buf_len);
        add_len -= buf_len;  /* subtract what we've just fed the stream. */
    }

    if (buffer != NULL)
    {
        SDL_free(buffer);
    }
}

void fluid_sdl3_audio_driver_settings(fluid_settings_t *settings)
{
    int n = 0, j = 0, nDevs = 0;
    SDL_Mutex *AudioDeviceLock = SDL_CreateMutex();
    AudioDeviceList *list = NULL;
    SDL_AudioDeviceID *devices = NULL;

    fluid_settings_register_str(settings, "audio.sdl3.device", "default", 0);
    fluid_settings_add_option(settings, "audio.sdl3.device", "default");

    if(!SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        FLUID_LOG(FLUID_WARN, "SDL3 subsystem not initialized, SDL3 audio driver won't be usable");
        return;
    }

    if(!SDL_WasInit(SDL_INIT_AUDIO))
    {
        FLUID_LOG(FLUID_WARN, "SDL3 not initialized, SDL3 audio driver won't be usable");
        return;
    }

    list= SDL_malloc(sizeof(AudioDeviceList));
    devices = SDL_GetAudioPlaybackDevices(&nDevs);

    if (list == NULL)
    {
        return;
    }

    if (nDevs > 0) {
        list->num_devices = nDevs;
        list->devices = (AudioDeviceInfo *)SDL_malloc(sizeof (AudioDeviceInfo) * nDevs);
        if (!list->devices) {
            SDL_free(devices);
        }
        
        for (n = 0; n < nDevs; n++) {
            const char *newname = SDL_GetAudioDeviceName(devices[n]);
            char *fullname = NULL;
            if (newname == NULL) {
                /* ugh, whatever, just make up a name. */
                newname = "Unidentified device";
            }
            
            /* Device names must be unique in sdl3, as that's how we open them.
             sdl3 took serious pains to try to add numbers to the end of duplicate device names ("SoundBlaster Pro" and then "SoundBlaster Pro (2)"),
             but here we're just putting the actual SDL3 instance id at the end of everything. Good enough. I hope. */
            if (!newname || (SDL_asprintf(&fullname, "%s (id=%u)", newname, (unsigned int) devices[n]) < 0)) {
                /* we're in real trouble now.  :/  */
                for (j = 0; j < n; j++) {
                    SDL_free(list->devices[n].name);
                }

                SDL_free(fullname);
                SDL_free(devices);
            }
            
            list->devices[n].devid = devices[n];
            list->devices[n].name = fullname;
        }
    }

    for (n = 0; n < nDevs; n++)
    {
        const char *dev_name = NULL;
        
        if (!SDL_GetCurrentAudioDriver()) {
            SDL_SetError("Audio subsystem is not initialized");
            return;
        }

        SDL_LockMutex(AudioDeviceLock);

        if ((n < 0) || (n >= list->num_devices)) {
            SDL_InvalidParamError("index");
        } else {
            dev_name = list->devices[n].name;
        }
        SDL_UnlockMutex(AudioDeviceLock);

        if(dev_name != NULL)
        {
            FLUID_LOG(FLUID_DBG, "sdl3 driver testing audio device: %s", dev_name);
            fluid_settings_add_option(settings, "audio.sdl3.device", dev_name);
        }
    }

    SDL_free(AudioDeviceLock);
}


/*
 * new_fluid_sdl3_audio_driver
 */
fluid_audio_driver_t *
new_fluid_sdl3_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    fluid_sdl3_audio_driver_t *dev = NULL;
    fluid_audio_callback_t write_ptr;
    double sample_rate;
    int period_size, sample_size;
    SDL_AudioSpec aspec;
    char *device;
    const char *dev_name;

    /* Check if SDL library has been started */
    if(!SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        FLUID_LOG(FLUID_WARN, "SDL3 subsystem not initialized, SDL3 audio driver won't be usable");
        return NULL;
    }

    if(!SDL_WasInit(SDL_INIT_AUDIO))
    {
        FLUID_LOG(FLUID_ERR, "Failed to create SDL3 audio driver, because the audio subsystem of SDL3 is not initialized.");
        return NULL;
    }

    /* Retrieve the settings */
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.period-size", &period_size);

    /* Lower values do not seem to give good results */
    if(period_size < 1024)
    {
        period_size = 1024;
    }
    else
    {
        /* According to documentation, it MUST be a power of two */
        if((period_size & (period_size - 1)) != 0)
        {
            FLUID_LOG(FLUID_ERR, "\"audio.period-size\" must be a power of 2 for sdl3");
            return NULL;
        }
    }
    /* Clear the format buffer */
    FLUID_MEMSET(&aspec, 0, sizeof(aspec));

    /* Setup mixing frequency */
    aspec.freq = (int)sample_rate;

    /* Check the format */
    if(fluid_settings_str_equal(settings, "audio.sample-format", "float"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 32 bit sample format");

        sample_size = sizeof(float);
        write_ptr   = fluid_synth_write_float;

        aspec.format = SDL_AUDIO_F32;
    }
    else if(fluid_settings_str_equal(settings, "audio.sample-format", "16bits"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 16 bit sample format");

        sample_size = sizeof(short);
        write_ptr   = fluid_synth_write_s16;

        aspec.format = SDL_AUDIO_S16;
    }
    else
    {
        FLUID_LOG(FLUID_ERR, "Unhandled sample format");
        return NULL;
    }

    /* Compile the format buffer */
    aspec.channels   = 2;
    /*aspec.samples    = aspec.channels * ((period_size + 7) & ~7);
    aspec.callback   = (SDL_AudioCallback)SDLAudioCallback;*/

    /* Set default device to use */
    device   = NULL;
    dev_name = NULL;

    /* create and clear the driver data */
    dev = FLUID_NEW(fluid_sdl3_audio_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_sdl3_audio_driver_t));

    /* get the selected device name. if none is specified, use default device. */
    if(fluid_settings_dupstr(settings, "audio.sdl3.device", &device) == FLUID_OK
            && device != NULL && device[0] != '\0')
    {
        int n, nDevs = GetNumAudioDevices(0, dev);

        for(n = 0; n < nDevs; n++)
        {
            dev_name = SDLC_GetAudioDeviceName(n, 0, dev);

            if(FLUID_STRCASECMP(dev_name, device) == 0)
            {
                FLUID_LOG(FLUID_DBG, "Selected audio device GUID: %s", dev_name);
                break;
            }
        }

        if(n >= nDevs)
        {
            FLUID_LOG(FLUID_DBG, "Audio device %s, using \"default\"", device);
            dev_name = NULL;
        }
    }

    if(device != NULL)
    {
        FLUID_FREE(device);
    }

    do
    {
        /* Save copy of synth */
        dev->synth = synth;

        /* Save copy of other variables */
        dev->write_ptr = write_ptr;
        dev->frame_size = sample_size * aspec.channels;
        dev->period_size = period_size;

        /* Open audio device */
        dev->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &aspec, SDLAudioCallback, dev);
        dev->devid = SDL_GetAudioStreamDevice(dev->stream);

        if(!dev->stream)
        {
            FLUID_LOG(FLUID_ERR, "Failed to open audio stream");
            break;
        }

        /* Start to play */
        SDL_ResumeAudioStreamDevice(dev->stream);

        return (fluid_audio_driver_t *) dev;
    } while(0);

    delete_fluid_sdl3_audio_driver(&dev->driver);
    return NULL;
}


void delete_fluid_sdl3_audio_driver(fluid_audio_driver_t *d)
{
    fluid_sdl3_audio_driver_t *dev = (fluid_sdl3_audio_driver_t *) d;

    if(dev != NULL)
    {
        if(dev->devid)
        {
            /* Stop audio and close */
            SDL_PauseAudioDevice(dev->devid);
            SDL_CloseAudioDevice(dev->devid);
        }

        FLUID_FREE(dev);

        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

}

#endif /* SDL3_SUPPORT */
