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

/* fluid_pulse.c
 *
 * Audio driver for PulseAudio.
 *
 */

#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_settings.h"

#include <pthread.h>
#include "config.h"

#include <pulse/simple.h>
#include <pulse/error.h>

/* SCHED_FIFO priorities for threads (see pthread_attr_setschedparam) */
#define PULSE_PCM_SCHED_PRIORITY 90

/** fluid_pulse_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct {
  fluid_audio_driver_t driver;
  pa_simple *pa_handle;
  fluid_audio_func_t callback;
  void* data;
  int buffer_size;
  pthread_t thread;
  int cont;
} fluid_pulse_audio_driver_t;


fluid_audio_driver_t* new_fluid_pulse_audio_driver(fluid_settings_t* settings,
						   fluid_synth_t* synth);
fluid_audio_driver_t* new_fluid_pulse_audio_driver2(fluid_settings_t* settings,
						    fluid_audio_func_t func, void* data);
int delete_fluid_pulse_audio_driver(fluid_audio_driver_t* p);
void fluid_pulse_audio_driver_settings(fluid_settings_t* settings);
static void* fluid_pulse_audio_run(void* d);
static void* fluid_pulse_audio_run2(void* d);


void fluid_pulse_audio_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "audio.pulseaudio.server", "default", 0, NULL, NULL);
  fluid_settings_register_str(settings, "audio.pulseaudio.device", "default", 0, NULL, NULL);
}


fluid_audio_driver_t*
new_fluid_pulse_audio_driver(fluid_settings_t* settings,
			    fluid_synth_t* synth)
{
  return new_fluid_pulse_audio_driver2(settings, NULL, synth);
}

fluid_audio_driver_t*
new_fluid_pulse_audio_driver2(fluid_settings_t* settings,
			     fluid_audio_func_t func, void* data)
{
  fluid_pulse_audio_driver_t* dev;
  pa_sample_spec samplespec;
  pa_buffer_attr bufattr;
  double sample_rate;
  int period_size, period_bytes;
  char *server;
  char *device;
  pthread_attr_t attr;
  int sched = SCHED_FIFO;
  struct sched_param priority;
  int i, err, dir = 0;

  dev = FLUID_NEW(fluid_pulse_audio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  FLUID_MEMSET(dev, 0, sizeof(fluid_pulse_audio_driver_t));

//  fluid_settings_getint(settings, "audio.periods", &periods);
  fluid_settings_getint(settings, "audio.period-size", &period_size);
  fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
  fluid_settings_getstr(settings, "audio.pulseaudio.server", &server);
  fluid_settings_getstr(settings, "audio.pulseaudio.device", &device);

  if (strcmp (server, "default") == 0) server = NULL;
  if (strcmp (device, "default") == 0) device = NULL;

  dev->data = data;
  dev->callback = func;
  dev->cont = 1;
  dev->buffer_size = period_size;

  samplespec.format = PA_SAMPLE_FLOAT32NE;
  samplespec.channels = 2;
  samplespec.rate = sample_rate;

  period_bytes = period_size * sizeof (float) * 2;
  bufattr.maxlength = period_bytes;
  bufattr.tlength = period_bytes;
  bufattr.minreq = -1;
  bufattr.prebuf = -1;    /* Just initialize to same value as tlength */
  bufattr.fragsize = -1;  /* Not used */

  dev->pa_handle = pa_simple_new (server, "FluidSynth", PA_STREAM_PLAYBACK,
				  device, "Synth output", &samplespec,
				  NULL, /* pa_channel_map */
				  &bufattr,
				  &err);

  if (!dev->pa_handle)
  {
    FLUID_LOG(FLUID_ERR, "Failed to create PulseAudio connection");
    goto error_recovery;
  }

  FLUID_LOG(FLUID_INFO, "Using PulseAudio driver");

  /* Create the audio thread */

  if (pthread_attr_init(&attr)) {
    FLUID_LOG(FLUID_ERR, "Couldn't initialize audio thread attributes");
    goto error_recovery;
  }

  /* The pthread_create man page explains that
     pthread_attr_setschedpolicy returns an error if the user is not
     permitted the set SCHED_FIFO. It seems however that no error is
     returned but pthread_create fails instead. That's why I try to
     create the thread twice in a while loop. */
  while (1) {
    err = pthread_attr_setschedpolicy(&attr, sched);
    if (err) {
      FLUID_LOG(FLUID_WARN, "Couldn't set high priority scheduling for the audio output");
      if (sched == SCHED_FIFO) {
	sched = SCHED_OTHER;
	continue;
      } else {
	FLUID_LOG(FLUID_ERR, "Couldn't set scheduling policy.");
	goto error_recovery;
      }
    }

    /* SCHED_FIFO will not be active without setting the priority */
    priority.sched_priority = (sched == SCHED_FIFO) ? PULSE_PCM_SCHED_PRIORITY : 0;
    pthread_attr_setschedparam(&attr, &priority);

    err = pthread_create(&dev->thread, &attr,
			 func ? fluid_pulse_audio_run2 : fluid_pulse_audio_run, (void*) dev);
    if (err) {
      FLUID_LOG(FLUID_WARN, "Couldn't set high priority scheduling for the audio output");
      if (sched == SCHED_FIFO) {
	sched = SCHED_OTHER;
	continue;
      } else {
	FLUID_LOG(FLUID_PANIC, "Couldn't create the audio thread.");
	goto error_recovery;
      }
    }
    break;
  }

  return (fluid_audio_driver_t*) dev;

 error_recovery:
  delete_fluid_pulse_audio_driver((fluid_audio_driver_t*) dev);
  return NULL;
}

int delete_fluid_pulse_audio_driver(fluid_audio_driver_t* p)
{
  fluid_pulse_audio_driver_t* dev = (fluid_pulse_audio_driver_t*) p;

  if (dev == NULL) {
    return FLUID_OK;
  }

  dev->cont = 0;

  if (dev->thread) {
    if (pthread_join(dev->thread, NULL)) {
      FLUID_LOG(FLUID_ERR, "Failed to join the audio thread");
      return FLUID_FAILED;
    }
  }

  if (dev->pa_handle)
    pa_simple_free(dev->pa_handle);

  FLUID_FREE(dev);

  return FLUID_OK;
}

/* Thread without audio callback, more efficient */
static void* fluid_pulse_audio_run(void* d)
{
  fluid_pulse_audio_driver_t* dev = (fluid_pulse_audio_driver_t*) d;
  fluid_synth_t *synth = (fluid_synth_t *)(dev->data);
  float *buf;
  int buffer_size;
  int err;

  buffer_size = dev->buffer_size;

  buf = FLUID_ARRAY(float, buffer_size * 2);

  if (buf == NULL)
  {
    FLUID_LOG(FLUID_ERR, "Out of memory.");
    return NULL;
  }

  while (dev->cont)
  {
    fluid_synth_write_float(dev->data, buffer_size, buf, 0, 2, buf, 1, 2);

    if (pa_simple_write (dev->pa_handle, buf,
			 buffer_size * sizeof (float) * 2, &err) < 0)
    {
      FLUID_LOG(FLUID_ERR, "Error writing to PulseAudio connection.");
      break;
    }
  }	/* while (dev->cont) */

  FLUID_FREE(buf);

  return NULL;
}

static void* fluid_pulse_audio_run2(void* d)
{
  fluid_pulse_audio_driver_t* dev = (fluid_pulse_audio_driver_t*) d;
  fluid_synth_t *synth = (fluid_synth_t *)(dev->data);
  float *left, *right, *buf;
  float* handle[2];
  int buffer_size;
  int err;
  int i;

  buffer_size = dev->buffer_size;

  left = FLUID_ARRAY(float, buffer_size);
  right = FLUID_ARRAY(float, buffer_size);
  buf = FLUID_ARRAY(float, buffer_size * 2);

  if (left == NULL || right == NULL || buf == NULL)
  {
    FLUID_LOG(FLUID_ERR, "Out of memory.");
    return NULL;
  }

  handle[0] = left;
  handle[1] = right;

  while (dev->cont)
  {
    (*dev->callback)(synth, buffer_size, 0, NULL, 2, handle);

    /* Interleave the floating point data */
    for (i = 0; i < buffer_size; i++)
    {
      buf[i * 2] = left[i];
      buf[i * 2 + 1] = right[i];
    }

    if (pa_simple_write (dev->pa_handle, buf,
			 buffer_size * sizeof (float) * 2, &err) < 0)
    {
      FLUID_LOG(FLUID_ERR, "Error writing to PulseAudio connection.");
      break;
    }
  }	/* while (dev->cont) */

  FLUID_FREE(left);
  FLUID_FREE(right);
  FLUID_FREE(buf);

  return NULL;
}
