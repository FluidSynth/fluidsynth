/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
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

/* fluid_portaudio.c
 *
 * Drivers for the PortAudio API : www.portaudio.com
 * Implementation files for PortAudio on each platform have to be added
 *
 * Stephane Letz  (letz@grame.fr)  Grame
 * 12/20/01 Adapdation for new audio drivers
 */

#include "fluid_synth.h"
#include "fluid_sys.h"

#if PORTAUDIO_SUPPORT

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "portaudio.h"


/** fluid_portaudio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct {
	fluid_synth_t* synth;
  	fluid_audio_callback_t read;
 	PortAudioStream * stream;
} fluid_portaudio_driver_t;

int delete_fluid_portaudio_driver(fluid_audio_driver_t* p);

/* PortAudio callback
 * fluid_portaudio_run
 */
static int fluid_portaudio_run( void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             PaTimestamp outTime, void *userData )
{
  fluid_portaudio_driver_t* dev = (fluid_portaudio_driver_t*) userData;
  /* it's as simple as that: */
  dev->read(dev->synth, framesPerBuffer, outputBuffer, 0, 2, outputBuffer, 1, 2);
  return 0;
}

/*
 * new_fluid_portaudio_driver
 */

fluid_audio_driver_t*
new_fluid_portaudio_driver(char* devname, int format, int chan, int sample_rate,
			  int bufsize, int queuesize, fluid_synth_t* synth)
{
  fluid_portaudio_driver_t* dev = NULL;
  PaError err;
  PaSampleFormat portaudio_format;

  dev = FLUID_NEW(fluid_portaudio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_portaudio_driver_t));

  dev->synth = synth;

  switch (format) {
	  case FLUID_S16_FORMAT:
	    portaudio_format = paInt16;
	    dev->read = fluid_synth_write_s16;
	    break;

	  case FLUID_FLOAT_FORMAT:
	    portaudio_format = paFloat32;
	    dev->read = fluid_synth_write_float;
	    break;
  }

  /* PortAudio section */

  err = Pa_Initialize();
  if( err != paNoError ) goto error_recovery;

  err = Pa_OpenStream(
				&dev->stream,
				paNoDevice,		/* default input device */
				0,         		/* no input */
				portaudio_format,
				NULL,
				Pa_GetDefaultOutputDeviceID() ,	 				/* default output device */
				2,       			/* stereo output */
				portaudio_format,
				NULL,
				sample_rate,
				bufsize,        /* frames per buffer */
				0,              /* number of buffers, if zero then use default minimum */
				paClipOff,      /* we won't output out of range samples so don't bother clipping them */
				fluid_portaudio_run,
				dev );

  if( err != paNoError ) goto error_recovery;
  err = Pa_StartStream( dev->stream );
  if( err != paNoError ) goto error_recovery;

  return (fluid_audio_driver_t*) dev;

error_recovery:
  delete_fluid_portaudio_driver((fluid_audio_driver_t*) dev);
  return NULL;
}

/*
 * delete_fluid_portaudio_driver
 */
int delete_fluid_portaudio_driver(fluid_audio_driver_t* p)
{
  fluid_portaudio_driver_t* dev;

  dev = (fluid_portaudio_driver_t*) p;
  if (dev == NULL) {
    return FLUID_OK;
  }

  /* PortAudio section */
  if(dev->stream) Pa_CloseStream(dev->stream);
  Pa_Terminate();

  FLUID_FREE(dev);
  return FLUID_OK;
}

#endif /*#if PORTAUDIO_SUPPORT */
