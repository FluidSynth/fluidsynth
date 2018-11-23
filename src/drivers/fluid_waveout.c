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

#if WAVEOUT_SUPPORT

#include <mmsystem.h>

#define NOBITMAP
#include <mmreg.h>

/* Number of buffers in the chain */
#define NB_SOUND_BUFFERS    4

/* Milliseconds of a single sound buffer */
#define MS_BUFFER_LENGTH    20

typedef struct
{
    fluid_audio_driver_t driver;

    fluid_synth_t *synth;
    fluid_audio_callback_t write_ptr;

    HWAVEOUT hWaveOut;
    WAVEHDR  waveHeader[NB_SOUND_BUFFERS];

    int sample_size;
    int num_frames;

    volatile BOOL bQuit;

    int    nQuit;
    HANDLE hQuit;

} fluid_waveout_audio_driver_t;


static void CALLBACK
waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    if (uMsg == WOM_DONE)
    {
        fluid_waveout_audio_driver_t *dev = (fluid_waveout_audio_driver_t *)dwInstance;
        WAVEHDR                      *pWave = (WAVEHDR *)dwParam1;

        if (dev->bQuit)
        {
            waveOutUnprepareHeader(hwo, pWave, sizeof(WAVEHDR));

            if (--dev->nQuit <= 0)
            {
                SetEvent(dev->hQuit);
            }
            return;
        }

        dev->write_ptr(dev->synth, dev->num_frames, pWave->lpData, 0, 2, pWave->lpData, 1, 2);

        waveOutWrite(hwo, pWave, sizeof(WAVEHDR));
    }
}

void fluid_waveout_audio_driver_settings(fluid_settings_t *settings)
{
    UINT n, nDevs = waveOutGetNumDevs();
#ifdef _UNICODE
    char dev_name[MAXPNAMELEN];
#endif

    fluid_settings_register_str(settings, "audio.waveout.device", "default", 0);
    fluid_settings_add_option(settings, "audio.waveout.device", "default");

    for (n = 0; n < nDevs; n++)
    {
        WAVEOUTCAPS caps;
        MMRESULT    res;

        res = waveOutGetDevCaps(n, &caps, sizeof(caps));
        if (res == MMSYSERR_NOERROR)
        {
#ifdef _UNICODE
            WideCharToMultiByte(CP_UTF8, 0, caps.szPname, -1, dev_name, MAXPNAMELEN, 0, 0);
            FLUID_LOG(FLUID_DBG, "Testing audio device: %s", dev_name);
            fluid_settings_add_option(settings, "audio.waveout.device", dev_name);
#else
            FLUID_LOG(FLUID_DBG, "Testing audio device: %s", caps.szPname);
            fluid_settings_add_option(settings, "audio.waveout.device", caps.szPname);
#endif
        }
    }
}


/*
 * new_fluid_waveout_audio_driver
 */
fluid_audio_driver_t *
new_fluid_waveout_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    fluid_waveout_audio_driver_t *dev = NULL;
    fluid_audio_callback_t write_ptr;
    double sample_rate;
    int periods, period_size, frequency, sample_size;
    LPSTR ptrBuffer;
    int lenBuffer;
    int device;
    int i;
    WAVEFORMATEX wfx;
    char *dev_name;
    MMRESULT errCode;

    /* Retrieve the settings */
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);

    /* Clear the format buffer */
    ZeroMemory(&wfx, sizeof(WAVEFORMATEX));

    /* check the format */
    if(fluid_settings_str_equal(settings, "audio.sample-format", "float"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 32 bit sample format");

        sample_size = sizeof(float);
        write_ptr   = fluid_synth_write_float;

        wfx.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    }
    else if(fluid_settings_str_equal(settings, "audio.sample-format", "16bits"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 16 bit sample format");

        sample_size = sizeof(short);
        write_ptr   = fluid_synth_write_s16;

        wfx.wFormatTag = WAVE_FORMAT_PCM;
    }
    else
    {
        FLUID_LOG(FLUID_ERR, "Unhandled sample format");
        return NULL;
    }

    /* Set frequency to integer */
    frequency = (int)sample_rate;

    /* Compile the format buffer */
    wfx.nChannels       = 2;
    wfx.wBitsPerSample  = sample_size * 8;
    wfx.nSamplesPerSec  = frequency;
    wfx.nBlockAlign     = sample_size * wfx.nChannels;
    wfx.nAvgBytesPerSec = frequency * wfx.nBlockAlign;

    /* Calculate the length of a single buffer */
    lenBuffer = (MS_BUFFER_LENGTH * wfx.nAvgBytesPerSec + 999) / 1000;

    /* Round to 8-bytes size */
    lenBuffer = (lenBuffer + 7) & ~7;

    /* create and clear the driver data */
    dev = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                sizeof(fluid_waveout_audio_driver_t) + lenBuffer * NB_SOUND_BUFFERS);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    /* Save copy of synth */
    dev->synth = synth;

    /* Save copy of other variables */
    dev->write_ptr = write_ptr;
    dev->sample_size = sample_size;

    /* Calculate the number of frames in a block */
    dev->num_frames = lenBuffer / wfx.nBlockAlign;

    /* Set default device to use */
    device = WAVE_MAPPER;

    /* get the selected device name. if none is specified, use default device. */
    if(fluid_settings_dupstr(settings, "audio.waveout.device", &dev_name) == FLUID_OK
            && dev_name && strlen(dev_name) > 0)
    {
        UINT nDevs = waveOutGetNumDevs();
        UINT n;
#ifdef _UNICODE
        WCHAR lpwDevName[MAXPNAMELEN];

        MultiByteToWideChar(CP_UTF8, 0, dev_name, -1, lpwDevName, MAXPNAMELEN);
#endif

        for (n = 0; n < nDevs; n++)
        {
            WAVEOUTCAPS caps;
            MMRESULT    res;

            res = waveOutGetDevCaps(n, &caps, sizeof(caps));
            if (res == MMSYSERR_NOERROR)
            {
#ifdef _UNICODE
                if(wcsicmp(lpwDevName, caps.szPname) == 0)
#else
                if(FLUID_STRCASECMP(dev_name, caps.szPname) == 0)
#endif
                {
                    FLUID_LOG(FLUID_DBG, "Selected audio device GUID: %s", dev_name);
                    device = n;
                    break;
                }
            }
        }
    }

    if(dev_name)
    {
        FLUID_FREE(dev_name);
    }

    errCode = waveOutOpen(&dev->hWaveOut,
                          device,
                          &wfx,
                          (DWORD_PTR)waveOutProc,
                          (DWORD_PTR)dev,
                          CALLBACK_FUNCTION);

    if (errCode != MMSYSERR_NOERROR)
    {
        HeapFree(GetProcessHeap(), 0, dev);
        return NULL;
    }

    /* Create handle for QUIT event */
    dev->hQuit = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (dev->hQuit == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create event handle");

        delete_fluid_dsound_audio_driver(&dev->driver);
        return NULL;
    }

    /* Get pointer to sound buffer memory */
    ptrBuffer = (LPSTR)(dev+1);

    /* Setup the sample buffers */
    for (i = 0; i < NB_SOUND_BUFFERS; i++)
    {
        /* Clear the sample buffer */
        memset(ptrBuffer, 0, lenBuffer);

        /* Clear descriptor buffer */
        memset(dev->waveHeader+i, 0, sizeof(WAVEHDR));

        /* Compile descriptor buffer */
        dev->waveHeader[i].lpData         = ptrBuffer;
        dev->waveHeader[i].dwBufferLength = lenBuffer;

        waveOutPrepareHeader(dev->hWaveOut, &dev->waveHeader[i], sizeof(WAVEHDR));

        ptrBuffer += lenBuffer;
    }

    /* Play the sample buffers */
    for (i = 0; i < NB_SOUND_BUFFERS; i++)
    {
        waveOutWrite(dev->hWaveOut, &dev->waveHeader[i], sizeof(WAVEHDR));
    }

    return (fluid_audio_driver_t *) dev;
}


void delete_fluid_waveout_audio_driver(fluid_audio_driver_t *d)
{
    int i;

    fluid_waveout_audio_driver_t *dev = (fluid_waveout_audio_driver_t *) d;
    fluid_return_if_fail(dev != NULL);

    /* release all the allocated resources */
    if (dev->hWaveOut != NULL)
    {
        if (dev->hQuit != NULL)
        {
            /* Wait sample buffers to be flushed */
            dev->nQuit = NB_SOUND_BUFFERS;
            dev->bQuit = TRUE;

            WaitForSingleObject(dev->hQuit, INFINITE);
            CloseHandle(dev->hQuit);
        }

        waveOutClose(dev->hWaveOut);
    }
    HeapFree(GetProcessHeap(), 0, dev);
}

#endif /* WAVEOUT_SUPPORT */
