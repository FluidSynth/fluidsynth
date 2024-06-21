/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 * Copyright (C) 2024  KO Myung-Hun
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

/* fluid_kai.c
 *
 * Driver for OS/2 KAI
 *
 */

#include "fluid_adriver.h"
#include "fluid_settings.h"
#include "fluid_sys.h"

#if KAI_SUPPORT

#define INCL_DOS
#include <os2.h>

#include <kai.h>

#define NUM_MIX_BUFS    2

/** fluid_kai_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct {
    fluid_audio_driver_t driver;
    fluid_synth_t *synth;
    int frame_size;
    HKAI hkai;                                  /* KAI handle              */
} fluid_kai_audio_driver_t;

static APIRET APIENTRY
fluid_kai_callback(PVOID pCBData, PVOID pBuffer, ULONG ulSize);

/**************************************************************
 *
 *        KAI audio driver
 *
 */

void fluid_kai_audio_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "audio.kai.device", "default", 0);
}

fluid_audio_driver_t *
new_fluid_kai_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    fluid_kai_audio_driver_t *dev;
    KAISPEC wanted, obtained;
    double sample_rate;
    int periods, period_size;
    ULONG rc;

    dev = FLUID_NEW(fluid_kai_audio_driver_t);
    if (dev == NULL) {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_kai_audio_driver_t));

    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);

    /* check the format */
    if (!fluid_settings_str_equal(settings, "audio.sample-format", "16bits")) {
        FLUID_LOG(FLUID_ERR, "Unhandled sample format");
        goto error_recovery;
    }

    dev->synth = synth;
    dev->frame_size = 2/* channels */ * sizeof(short)/* 16bits sample */;

    /* Initialize KAI */
    rc = kaiInit(KAIM_AUTO);
    if (rc != KAIE_NO_ERROR) {
        FLUID_LOG(FLUID_ERR, "Cannot initialize KAI, rc = %lu", rc);
        goto error_recovery;
    }

    /* Open KAI */
    wanted.usDeviceIndex    = 0;        /* default device */
    wanted.ulType           = KAIT_PLAY;
    wanted.ulBitsPerSample  = BPS_16;   /* only 16bits audio */
    wanted.ulSamplingRate   = sample_rate;
    wanted.ulDataFormat     = MCI_WAVE_FORMAT_PCM;
    wanted.ulChannels       = 2;        /* only 2 channels */
    wanted.ulNumBuffers     = NUM_MIX_BUFS;
    wanted.ulBufferSize     = periods * period_size * dev->frame_size;
    wanted.fShareable       = TRUE;
    wanted.pfnCallBack      = fluid_kai_callback;
    wanted.pCallBackData    = dev;

    rc = kaiOpen(&wanted, &obtained, &dev->hkai);
    if (rc != KAIE_NO_ERROR) {
        FLUID_LOG(FLUID_ERR, "Cannot open KAI, rc = %lu", rc);
        goto error_recovery;
    }

    /* Start to play */
    kaiPlay(dev->hkai);

    return (fluid_audio_driver_t *) dev;

error_recovery:

    delete_fluid_kai_audio_driver((fluid_audio_driver_t *) dev);
    return NULL;
}

void delete_fluid_kai_audio_driver(fluid_audio_driver_t *p)
{
    fluid_kai_audio_driver_t *dev = (fluid_kai_audio_driver_t *) p;

    fluid_return_if_fail(dev != NULL);

    /* Stop playing */
    kaiStop(dev->hkai);

    /* Close KAI */
    kaiClose(dev->hkai);

    /* Terminate KAI */
    kaiDone();

    FLUID_FREE(dev);
}

static APIRET APIENTRY
fluid_kai_callback(PVOID pCBData, PVOID pBuffer, ULONG ulSize)
{
    fluid_kai_audio_driver_t *dev = (fluid_kai_audio_driver_t *) pCBData;

    FLUID_MEMSET(pBuffer, 0, ulSize);
    fluid_synth_write_s16(dev->synth, ulSize / dev->frame_size,
                          pBuffer, 0, 2, pBuffer, 1, 2 );

    return ulSize;
}

#endif /* #if KAI_SUPPORT */
