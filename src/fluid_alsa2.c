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


typedef struct {
  fluid_audio_driver_t driver;
  snd_pcm_t *handle;
  fluid_audio_func_t callback;
  void* data;
  pthread_t thread;
  int cont;
  int period_size;
  float* buffers[2];
} fluid_alsa_audio_driver_t;

fluid_audio_driver_t* new_fluid_alsa_audio_driver(fluid_settings_t* settings, 
						fluid_synth_t* synth);

fluid_audio_driver_t* new_fluid_alsa_audio_driver2(fluid_settings_t* settings, 
						 fluid_audio_func_t func, 
						 void* data)

void fluid_alsa_audio_driver_settings(fluid_settings_t* settings);
int delete_fluid_alsa_audio_driver(fluid_audio_driver_t* p);
static void* fluid_alsa_audio_run(void* d);

static int fluid_alsa_audio_set_hwparams(snd_pcm_t *handle,
					snd_pcm_hw_params_t *params,
					snd_pcm_access_t access);

static int fluid_alsa_audio_set_swparams(snd_pcm_t *handle, 
					snd_pcm_sw_params_t *swparams);

static int fluid_alsa_xrun_recovery(fluid_alsa_audio_driver_t *dev, int err)


void
fluid_alsa_audio_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "audio.alsa.device", "default", 0, NULL, NULL);
}

fluid_audio_driver_t* 
new_fluid_alsa_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth)
{
  return new_fluid_alsa_audio_driver2(settings, 
				     (fluid_audio_func_t) fluid_synth_process, 
				     (void*) synth);
}


fluid_audio_driver_t* 
new_fluid_alsa_audio_driver2(fluid_settings_t* settings, fluid_audio_func_t func, void* data)
{
  fluid_alsa_audio_driver_t* dev = NULL;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  char* device;
  int i, periods;
  double sample_rate;

  dev = FLUID_NEW(fluid_alsa_audio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;    
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_alsa_audio_driver_t));

  dev->callback = func;
  dev->data = data;
  dev->cont = 1;

  fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
  fluid_settings_getint(settings, "audio.periods", &periods);
  fluid_settings_getint(settings, "audio.period-size", &dev->period_size);

  if (!fluid_settings_getstr(settings, "audio.alsa.device", &devname)) {
    device = "plughw:0,0";
  }

  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);

  if ((err = snd_pcm_open(&dev->handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    FLUID_LOG(FLUID_ERR, "Playback open error: %s", snd_strerror(err));
    goto error_recovery;
  }
  if ((err = set_hwparams(dev->handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    FLUID_LOG(FLUID_ERR, "Setting of hwparams failed: %s", snd_strerror(err));
    goto error_recovery;
  }
  if ((err = set_swparams(dev->handle, swparams)) < 0) {
    FLUID_LOG(FLUID_ERR, "Setting of swparams failed: %s", snd_strerror(err));
    goto error_recovery;
  }

  for (i = 0; i < 2; i++) {
    dev->buffers[i] = FLUID_ARRAY(float, dev->period_size);
    if (dev->buffers[i] == NULL) {
      FLUID_LOG(FLUID_ERR, "Out of memory");
      goto error_recovery;
    }
  }

 error_recovery:
  
  delete_fluid_alsa_audio_driver((fluid_audio_driver_t*) dev);
  return NULL; 
}



static int set_hwparams(snd_pcm_t *handle,
			snd_pcm_hw_params_t *params,
			snd_pcm_access_t access,
			int channels,
			int sample_rate,
			snd_pcm_format_t format)
{
  int err, dir;

  /* choose all parameters */
  err = snd_pcm_hw_params_any(handle, params);
  if (err < 0) {
    printf("Broken configuration for playback: no configurations available: %s", 
	   snd_strerror(err));
    return err;
  }

  /* set the interleaved read/write format */
  err = snd_pcm_hw_params_set_access(handle, params, access);
  if (err < 0) {
    printf("Access type not available for playback: %s", snd_strerror(err));
    return err;
  }

  /* set the sample format */
  err = snd_pcm_hw_params_set_format(handle, params, format);
  if (err < 0) {
    printf("Sample format not available for playback: %s", snd_strerror(err));
    return err;
  }

  /* set the count of channels */
  err = snd_pcm_hw_params_set_channels(handle, params, channels);
  if (err < 0) {
    printf("Channels count (%i) not available for playbacks: %s", 
	   channels, snd_strerror(err));
    return err;
  }

  /* set the stream rate */
  err = snd_pcm_hw_params_set_rate_near(handle, params, rate, 0);
  if (err < 0) {
    printf("Rate %iHz not available for playback: %s", 
	   rate, snd_strerror(err));
    return err;
  }
  if (err != rate) {
    printf("Rate doesn't match (requested %iHz, get %iHz)", rate, err);
    return -EINVAL;
  }

  /* set the buffer time */
  err = snd_pcm_hw_params_set_buffer_time_near(handle, params, buffer_time, &dir);
  if (err < 0) {
    printf("Unable to set buffer time %i for playback: %s", 
	   buffer_time, snd_strerror(err));
    return err;
  }
  buffer_size = snd_pcm_hw_params_get_buffer_size(params);

  /* set the period time */
  err = snd_pcm_hw_params_set_period_time_near(handle, params, period_time, &dir);
  if (err < 0) {
    printf("Unable to set period time %i for playback: %s", 
	   period_time, snd_strerror(err));
    return err;
  }
  period_size = snd_pcm_hw_params_get_period_size(params, &dir);

  /* write the parameters to device */
  err = snd_pcm_hw_params(handle, params);
  if (err < 0) {
    printf("Unable to set hw params for playback: %s", snd_strerror(err));
    return err;
  }
  return 0;
}

static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
  int err;

  /* get the current swparams */
  err = snd_pcm_sw_params_current(handle, swparams);
  if (err < 0) {
    printf("Unable to determine current swparams for playback: %s", 
	   snd_strerror(err));
    return err;
  }

  /* start the transfer when the buffer is full */
  err = snd_pcm_sw_params_set_start_threshold(handle, swparams, buffer_size);
  if (err < 0) {
    printf("Unable to set start threshold mode for playback: %s", 
	   snd_strerror(err));
    return err;
  }

  /* allow the transfer when at least period_size samples can be processed */
  err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
  if (err < 0) {
    printf("Unable to set avail min for playback: %s", snd_strerror(err));
    return err;
  }

  /* align all transfers to 1 sample */
  err = snd_pcm_sw_params_set_xfer_align(handle, swparams, 1);
  if (err < 0) {
    printf("Unable to set transfer align for playback: %s", snd_strerror(err));
    return err;
  }

  /* write the parameters to the playback device */
  err = snd_pcm_sw_params(handle, swparams);
  if (err < 0) {
    printf("Unable to set sw params for playback: %s", snd_strerror(err));
    return err;
  }
  return 0;
}


static void* fluid_alsa_audio_run(void* d)
{
  fluid_alsa_audio_driver_t* dev = (fluid_alsa_audio_driver_t*) d;
  double phase = 0;
  const snd_pcm_channel_area_t *my_areas;
  snd_pcm_uframes_t offset, frames, size;
  snd_pcm_sframes_t avail, commitres;
  snd_pcm_state_t state;
  int err, first = 1;
  
  while (dev->cont) {

    state = snd_pcm_state(dev->handle);
    if (state == SND_PCM_STATE_XRUN) {
      err = fluid_alsa_xrun_recovery(dev->handle, -EPIPE);
      if (err < 0) {
	printf("XRUN recovery failed: %s", snd_strerror(err));
	return err;
      }
      first = 1;
    } else if (state == SND_PCM_STATE_SUSPENDED) {
      err = fluid_alsa_xrun_recovery(dev->handle, -ESTRPIPE);
      if (err < 0) {
	printf("SUSPEND recovery failed: %s", snd_strerror(err));
	return err;
      }
    }
    avail = snd_pcm_avail_update(dev->handle);
    if (avail < 0) {
      err = fluid_alsa_xrun_recovery(dev->handle, avail);
      if (err < 0) {
	printf("avail update failed: %s", snd_strerror(err));
	return err;
      }
      first = 1;
      continue;
    }
    if (avail < period_size) {
      if (first) {
	first = 0;
	err = snd_pcm_start(dev->handle);
	if (err < 0) {
	  printf("Start error: %s", snd_strerror(err));
	  exit(EXIT_FAILURE);
	}
      } else {
	err = snd_pcm_wait(dev->handle, -1);
	if (err < 0) {
	  if ((err = fluid_alsa_xrun_recovery(dev->handle, err)) < 0) {
	    printf("snd_pcm_wait error: %s", snd_strerror(err));
	    exit(EXIT_FAILURE);
	  }
	  first = 1;
	}
      }
      continue;
    }

    size = dev->period_size;

    while (size > 0) {

      frames = size;
      err = snd_pcm_mmap_begin(dev->handle, &my_areas, &offset, &frames);
      if (err < 0) {
	if ((err = fluid_alsa_xrun_recovery(dev->handle, err)) < 0) {
	  printf("MMAP begin avail error: %s", snd_strerror(err));
	  exit(EXIT_FAILURE);
	}
	first = 1;
      }

      generate_sine(my_areas, offset, frames, &phase);
      commitres = snd_pcm_mmap_commit(dev->handle, offset, frames);

      if (commitres < 0 || commitres != frames) {
	err = fluid_alsa_xrun_recovery(dev->handle, commitres >= 0 ? -EPIPE : commitres);
	if (err  < 0) {
	  printf("MMAP commit error: %s", snd_strerror(err));
	  exit(EXIT_FAILURE);
	}
	first = 1;
      }
      size -= frames;
    }
  }
}


/* fluid_alsa_xrun_recovery
 *
 * Underrun and suspend recovery
 */
static int fluid_alsa_xrun_recovery(fluid_alsa_audio_driver_t *dev, int err)
{
  if (err == -EPIPE) {	/* under-run */
    err = snd_pcm_prepare(dev->handle);
    if (err < 0) {
      printf("Can't recovery from underrun, prepare failed: %s", snd_strerror(err));
    }
    return 0;

  } else if (err == -ESTRPIPE) {
    while ((err = snd_pcm_resume(dev->handle)) == -EAGAIN) {
      sleep(1);	/* wait until the suspend flag is released */
    }
    if (err < 0) {
      err = snd_pcm_prepare(dev->handle);
      if (err < 0) {
	printf("Can't recovery from suspend, prepare failed: %s", snd_strerror(err));
      }
    }
    return 0;
  }
  return err;
}
