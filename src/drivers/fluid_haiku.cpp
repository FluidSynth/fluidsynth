/* Haiku MediaKit Driver for FluidSynth
 *
 * Copyright (C) 2019  Gerasim Troeglazov
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


#include <MediaKit.h>
#include <SupportKit.h>

extern "C" {
#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_settings.h"
}

typedef struct
{
    fluid_audio_driver_t driver;

    fluid_synth_t *synth;
    fluid_audio_callback_t write_ptr;

    int frame_size;
	
	BSoundPlayer *m_player;

} fluid_haiku_audio_driver_t;


static void playerProc(void *cookie, void *buffer, size_t len, const media_raw_audio_format &format)
{
    fluid_haiku_audio_driver_t *dev = (fluid_haiku_audio_driver_t *)cookie;
    len /= dev->frame_size;
    dev->write_ptr(dev->synth, len, buffer, 0, 2, buffer, 1, 2);
}

void fluid_haiku_audio_driver_settings(fluid_settings_t *settings)
{
}

fluid_audio_driver_t *
new_fluid_haiku_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    fluid_haiku_audio_driver_t *dev = NULL;
    fluid_audio_callback_t write_ptr;
    
    double sample_rate;
    int period_size;
    int periods;
    int sample_size;

    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);    

    if(fluid_settings_str_equal(settings, "audio.sample-format", "float"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 32 bit sample format");
        sample_size = sizeof(float);
        write_ptr   = fluid_synth_write_float;
    }
   else if(fluid_settings_str_equal(settings, "audio.sample-format", "16bits"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 16 bit sample format");
        sample_size = sizeof(short);
        write_ptr   = fluid_synth_write_s16;
    } else {
        FLUID_LOG(FLUID_ERR, "Unhandled sample format");
        return NULL;
    }

	dev = FLUID_NEW(fluid_haiku_audio_driver_t);
	if(dev == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	FLUID_MEMSET(dev, 0, sizeof(fluid_haiku_audio_driver_t));

    dev->synth = synth;
    dev->frame_size = 2 * sample_size;
	dev->write_ptr = write_ptr;
	
	media_raw_audio_format mediaKitFormat = {
		(float)sample_rate,
		(uint32)2,
       sample_size == sizeof(float) ? media_raw_audio_format::B_AUDIO_FLOAT : media_raw_audio_format::B_AUDIO_SHORT,
		B_MEDIA_LITTLE_ENDIAN,
		(uint32)periods * period_size * dev->frame_size
	};
	
	dev->m_player = new BSoundPlayer(&mediaKitFormat, "FluidSynth", playerProc, NULL, (void*)dev);

	if(dev->m_player->InitCheck() != B_OK) {
		delete dev->m_player;
		dev->m_player = NULL;
	    delete_fluid_haiku_audio_driver(&dev->driver);
    	return NULL;
	}
	
	dev->m_player->SetHasData(true);
	dev->m_player->Start();

	return (fluid_audio_driver_t *) dev;
}


void delete_fluid_haiku_audio_driver(fluid_audio_driver_t *d)
{
    fluid_haiku_audio_driver_t *dev = (fluid_haiku_audio_driver_t *) d;
	if (dev->m_player != NULL) {
		dev->m_player->SetHasData(false);
		dev->m_player->Stop();
		delete dev->m_player;
		dev->m_player = NULL;
	}
	FLUID_FREE(dev);
}
