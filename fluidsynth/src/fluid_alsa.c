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

/* fluid_alsa.c
 *
 * Driver for the Advanced Linux Sound Architecture
 *
 */

#include "fluid_synth.h"
#include "fluid_midi.h"
#include "fluid_adriver.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#if ALSA_SUPPORT

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>

#include "config.h"

#include "fluid_lash.h"

#define FLUID_ALSA_DEFAULT_MIDI_DEVICE  "default"
#define FLUID_ALSA_DEFAULT_SEQ_DEVICE   "default"

#define BUFFER_LENGTH 512

/* SCHED_FIFO priorities for ALSA threads (see pthread_attr_setschedparam) */
#define ALSA_PCM_SCHED_PRIORITY 90
#define ALSA_RAWMIDI_SCHED_PRIORITY 90
#define ALSA_SEQ_SCHED_PRIORITY 90

/** fluid_alsa_audio_driver_t
 * 
 * This structure should not be accessed directly. Use audio port
 * functions instead.  
 */
typedef struct {
  fluid_audio_driver_t driver;
  snd_pcm_t* pcm;
  fluid_audio_func_t callback;
  void* data;
  int buffer_size;
  pthread_t thread;
  int cont;
} fluid_alsa_audio_driver_t;


fluid_audio_driver_t* new_fluid_alsa_audio_driver(fluid_settings_t* settings,
						  fluid_synth_t* synth);
fluid_audio_driver_t* new_fluid_alsa_audio_driver2(fluid_settings_t* settings,
						   fluid_audio_func_t func, void* data);

int delete_fluid_alsa_audio_driver(fluid_audio_driver_t* p);
void fluid_alsa_audio_driver_settings(fluid_settings_t* settings);
static void* fluid_alsa_audio_run_float(void* d);
static void* fluid_alsa_audio_run_s16(void* d);


struct fluid_alsa_formats_t {
  char* name;
  snd_pcm_format_t format;
  snd_pcm_access_t access;
  void* (*run)(void* d);
};

struct fluid_alsa_formats_t fluid_alsa_formats[] = {
  { "s16, rw, interleaved", 
    SND_PCM_FORMAT_S16, 
    SND_PCM_ACCESS_RW_INTERLEAVED, 
    fluid_alsa_audio_run_s16 },
  { "float, rw, non interleaved", 
    SND_PCM_FORMAT_FLOAT, 
    SND_PCM_ACCESS_RW_NONINTERLEAVED, 
    fluid_alsa_audio_run_float },
  { NULL, 0, 0, NULL }
};



/*
 * fluid_alsa_rawmidi_driver_t
 *
 */
typedef struct {
  fluid_midi_driver_t driver;
  snd_rawmidi_t *rawmidi_in;
  struct pollfd *pfd;
  int npfd;
  pthread_t thread;
  int status;
  unsigned char buffer[BUFFER_LENGTH];
  fluid_midi_parser_t* parser;
} fluid_alsa_rawmidi_driver_t;


fluid_midi_driver_t* new_fluid_alsa_rawmidi_driver(fluid_settings_t* settings, 
						   handle_midi_event_func_t handler, 
						   void* event_handler_data);

int delete_fluid_alsa_rawmidi_driver(fluid_midi_driver_t* p);
static void* fluid_alsa_midi_run(void* d);


/*
 * fluid_alsa_seq_driver_t
 *
 */
typedef struct {
  fluid_midi_driver_t driver;
  snd_seq_t *seq_handle;
  struct pollfd *pfd;
  int npfd;
  pthread_t thread;
  int status;
  int port_count;  
} fluid_alsa_seq_driver_t;

fluid_midi_driver_t* new_fluid_alsa_seq_driver(fluid_settings_t* settings, 
					     handle_midi_event_func_t handler, 
					     void* data);
int delete_fluid_alsa_seq_driver(fluid_midi_driver_t* p);
static void* fluid_alsa_seq_run(void* d);

/**************************************************************
 *
 *        Alsa audio driver
 *
 */

void fluid_alsa_audio_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "audio.alsa.device", "default", 0, NULL, NULL);
}


fluid_audio_driver_t* 
new_fluid_alsa_audio_driver(fluid_settings_t* settings,
			    fluid_synth_t* synth)
{
  return new_fluid_alsa_audio_driver2(settings, 
				      (fluid_audio_func_t) fluid_synth_process, 
				      synth);
}

fluid_audio_driver_t* 
new_fluid_alsa_audio_driver2(fluid_settings_t* settings,
			     fluid_audio_func_t func, void* data)
{
  fluid_alsa_audio_driver_t* dev;
  double sample_rate;
  int periods, period_size;
  char* device;
  pthread_attr_t attr;
  int sched = SCHED_FIFO;
  struct sched_param priority;
  int i, err, dir = 0;
  snd_pcm_hw_params_t* hwparams;
  snd_pcm_sw_params_t* swparams = NULL;
  snd_pcm_uframes_t uframes;
  unsigned int tmp;

  dev = FLUID_NEW(fluid_alsa_audio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;    
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_alsa_audio_driver_t));

  fluid_settings_getint(settings, "audio.periods", &periods);
  fluid_settings_getint(settings, "audio.period-size", &period_size);
  fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
  fluid_settings_getstr(settings, "audio.alsa.device", &device);

  dev->data = data;
  dev->callback = func;
  dev->cont = 1;
  dev->buffer_size = period_size;
  uframes = period_size;

  /* Open the PCM device */
  if ((err = snd_pcm_open(&dev->pcm, device, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) != 0) {
    if (err == -EBUSY) {
      FLUID_LOG(FLUID_ERR, "The \"%s\" audio device is used by another application", device);
      goto error_recovery;
    } else {
      FLUID_LOG(FLUID_ERR, "Failed to open the \"%s\" audio device", device);
      goto error_recovery;
    }
  }

  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);

  /* Set hardware parameters. We continue trying access methods and
     sample formats until we have one that works. For example, if
     memory mapped access fails we try regular IO methods. (not
     finished, yet). */

  for (i = 0; fluid_alsa_formats[i].name != NULL; i++) {

    snd_pcm_hw_params_any(dev->pcm, hwparams);

    if (snd_pcm_hw_params_set_access(dev->pcm, hwparams, fluid_alsa_formats[i].access) != 0) {
      continue;
    }

    if (snd_pcm_hw_params_set_format(dev->pcm, hwparams, fluid_alsa_formats[i].format) != 0) {
      continue;
    }

    if ((err = snd_pcm_hw_params_set_channels(dev->pcm, hwparams, 2)) != 0) {
      FLUID_LOG(FLUID_ERR, "Failed to set the channels: %s",
		snd_strerror (err));
      goto error_recovery;    
    }

    tmp = (unsigned int) sample_rate;
    if ((err = snd_pcm_hw_params_set_rate_near(dev->pcm, hwparams, &tmp, &dir)) != 0) {
      FLUID_LOG(FLUID_ERR, "Failed to set the sample rate: %s",
		snd_strerror (err));
      goto error_recovery;    
    }
    if (dir != 0) {
      /* There's currently no way to change the sampling rate of the
	 synthesizer after it's been created. */
      FLUID_LOG(FLUID_WARN, "The sample rate is set to %d, "
		"the synthesizer may be out of tune", tmp);
    }

    if (snd_pcm_hw_params_set_period_size_near(dev->pcm, hwparams, &uframes, &dir) != 0) {
      FLUID_LOG(FLUID_ERR, "Failed to set the period size");
      goto error_recovery;          
    }
    if (uframes != (unsigned long) period_size) {
      FLUID_LOG(FLUID_WARN, "Requested a period size of %d, got %d instead", 
		period_size, (int) uframes);
      dev->buffer_size = (int) uframes;
    }

    tmp = periods;
    if (snd_pcm_hw_params_set_periods_near(dev->pcm, hwparams, &tmp, &dir) != 0) {
      FLUID_LOG(FLUID_ERR, "Failed to set the number of periods");
      goto error_recovery;          
    }
    if (tmp != (unsigned int) periods) {
      FLUID_LOG(FLUID_WARN, "Requested %d periods, got %d instead", 
		periods, (int) tmp);
    }
    
    if (snd_pcm_hw_params(dev->pcm, hwparams) != 0) {
      FLUID_LOG(FLUID_WARN, "Audio device hardware configuration failed");
      continue;
    }

    break;
  }

  if (fluid_alsa_formats[i].name == NULL) {
    FLUID_LOG(FLUID_ERR, "Failed to find a workable audio format");
    goto error_recovery;    
  }

  FLUID_LOG(FLUID_INFO, "ALSA driver: Using format %s", fluid_alsa_formats[i].name);

  /* Set the software params */
  snd_pcm_sw_params_current(dev->pcm, swparams);
	
  if (snd_pcm_sw_params_set_start_threshold(dev->pcm, swparams, period_size) != 0) {
    FLUID_LOG(FLUID_ERR, "Cannot turn off start threshold.");
  }
  
  if (snd_pcm_sw_params_set_stop_threshold(dev->pcm, swparams, ~0u) != 0) {
    FLUID_LOG(FLUID_ERR, "Cannot turn off stop threshold.");
  }
  
  if (snd_pcm_sw_params_set_silence_threshold(dev->pcm, swparams, 0) != 0) {
    FLUID_LOG(FLUID_ERR, "Cannot set 0 silence threshold.");
  }
  
  if (snd_pcm_sw_params_set_silence_size(dev->pcm, swparams, 0) != 0) {
    FLUID_LOG(FLUID_ERR, "Cannot set 0 silence size.");
  }
  
  if (snd_pcm_sw_params_set_avail_min(dev->pcm, swparams, period_size / 2) != 0) {
    FLUID_LOG(FLUID_ERR, "Software setup for minimum available frames failed.");
  }
  
  if (snd_pcm_sw_params(dev->pcm, swparams) != 0) {
    FLUID_LOG(FLUID_ERR, "Software setup failed.");
  }


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
    priority.sched_priority = (sched == SCHED_FIFO) ? ALSA_PCM_SCHED_PRIORITY : 0;
    pthread_attr_setschedparam(&attr, &priority);

    err = pthread_create(&dev->thread, &attr, fluid_alsa_formats[i].run, (void*) dev);
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
  delete_fluid_alsa_audio_driver((fluid_audio_driver_t*) dev);
  return NULL;
}

int delete_fluid_alsa_audio_driver(fluid_audio_driver_t* p)
{
  fluid_alsa_audio_driver_t* dev = (fluid_alsa_audio_driver_t*) p;
  
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

  if (dev->pcm) {
    snd_pcm_state_t state = snd_pcm_state(dev->pcm);
    if ((state == SND_PCM_STATE_RUNNING)
	|| (state == SND_PCM_STATE_XRUN)
	|| (state == SND_PCM_STATE_SUSPENDED)
	|| (state == SND_PCM_STATE_PAUSED)) {
      snd_pcm_drop(dev->pcm);
    }
  }

  FLUID_FREE(dev);

  return FLUID_OK;
}

static void* fluid_alsa_audio_run_float(void* d)
{
  fluid_alsa_audio_driver_t* dev = (fluid_alsa_audio_driver_t*) d;
  float* left;
  float* right;
  float* handle[2];
  int n, buffer_size, offset;

  buffer_size = dev->buffer_size;

  left = FLUID_ARRAY(float, buffer_size);
  right = FLUID_ARRAY(float, buffer_size);

  if ((left == NULL) || (right == NULL)) {
    FLUID_LOG(FLUID_ERR, "Out of memory.");
    return NULL;
  }

  handle[0] = left;
  handle[1] = right;

  if (snd_pcm_nonblock(dev->pcm, 0) != 0) { /* double negation */
    FLUID_LOG(FLUID_ERR, "Failed to set the audio device to blocking mode");
    goto error_recovery;    
  }

  if (snd_pcm_prepare(dev->pcm) != 0) {
    FLUID_LOG(FLUID_ERR, "Failed to prepare the audio device");
    goto error_recovery;    
  }

  while (dev->cont) {

    handle[0] = left;
    handle[1] = right;

    (*dev->callback)(dev->data, buffer_size, 0, NULL, 2, handle);
    
    offset = 0;

    while (offset < buffer_size) {

      handle[0] = left + offset;
      handle[1] = right + offset;

      n = snd_pcm_writen(dev->pcm, (void**) handle, buffer_size - offset);
    
      if (n == -EAGAIN) {
	snd_pcm_wait(dev->pcm, 1);

      } else if ((n == -EPIPE) || (n == -EBADFD)) {
	if (snd_pcm_prepare(dev->pcm) != 0) {
	  FLUID_LOG(FLUID_ERR, "Failed to prepare the audio device");
	  goto error_recovery;    
	}

      } else if (n == -ESTRPIPE) {
	if ((snd_pcm_resume(dev->pcm) != 0) 
	    && (snd_pcm_prepare(dev->pcm) != 0)) {
	  FLUID_LOG(FLUID_ERR, "Failed to resume the audio device");
	  goto error_recovery;    
	}

      } else if (n < 0) {
	  FLUID_LOG(FLUID_ERR, "The audio device error: %s", snd_strerror(n));
	  goto error_recovery;    	

      } else {
	offset += n;
      }
    }  
  }
  
 error_recovery:
  
  FLUID_FREE(left);
  FLUID_FREE(right);

  return NULL;
}

static void* fluid_alsa_audio_run_s16(void* d)
{
  fluid_alsa_audio_driver_t* dev = (fluid_alsa_audio_driver_t*) d;
  float* left;
  float* right;
  short* buf;
  float* handle[2];
  int i, k, n, buffer_size, offset;
  float s;

  buffer_size = dev->buffer_size;

  left = FLUID_ARRAY(float, buffer_size);
  right = FLUID_ARRAY(float, buffer_size);
  buf = FLUID_ARRAY(short, 2 * buffer_size);

  if ((left == NULL) || (right == NULL) || (buf == NULL)) {
    FLUID_LOG(FLUID_ERR, "Out of memory.");
    return NULL;
  }

  handle[0] = left;
  handle[1] = right;

  if (snd_pcm_nonblock(dev->pcm, 0) != 0) { /* double negation */
    FLUID_LOG(FLUID_ERR, "Failed to set the audio device to blocking mode");
    goto error_recovery;
  }

  if (snd_pcm_prepare(dev->pcm) != 0) {
    FLUID_LOG(FLUID_ERR, "Failed to prepare the audio device");
    goto error_recovery;    
  }

  while (dev->cont) {

    handle[0] = left;
    handle[1] = right;

    (*dev->callback)(dev->data, buffer_size, 0, NULL, 2, handle);

    for (i = 0, k = 0; i < buffer_size; i++, k += 2) {
      s = 32768.0f * left[i];
      fluid_clip(s, -32768.0f, 32767.0f);
      buf[k] = (short) s;
    }

    for (i = 0, k = 1; i < buffer_size; i++, k += 2) {
      s = 32768.0f * right[i];
      fluid_clip(s, -32768.0f, 32767.0f);
      buf[k] = (short) s;
    }
    
    offset = 0;

    while (offset < buffer_size) {

      n = snd_pcm_writei(dev->pcm, (void*) (buf + 2 * offset), buffer_size - offset);

      if (n == -EAGAIN) {
	snd_pcm_wait(dev->pcm, 1);

      } else if ((n == -EPIPE) || (n == -EBADFD)) {
	if (snd_pcm_prepare(dev->pcm) != 0) {
	  FLUID_LOG(FLUID_ERR, "Failed to prepare the audio device");
	  goto error_recovery;    
	}

      } else if (n == -ESTRPIPE) {
	if ((snd_pcm_resume(dev->pcm) != 0) 
	    && (snd_pcm_prepare(dev->pcm) != 0)) {
	  FLUID_LOG(FLUID_ERR, "Failed to resume the audio device");
	  goto error_recovery;    
	}

      } else if (n < 0) {
	  FLUID_LOG(FLUID_ERR, "The audio device error: %s", snd_strerror(n));
	  goto error_recovery;    	

      } else {
	offset += n;
      }
    }  

  }
  
 error_recovery:
  
  FLUID_FREE(left);
  FLUID_FREE(right);
  FLUID_FREE(buf);

  return NULL;
}


/**************************************************************
 *
 *        Alsa MIDI driver
 *
 */


void fluid_alsa_rawmidi_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "midi.alsa.device", "default", 0, NULL, NULL);
}

/*
 * new_fluid_alsa_rawmidi_driver
 */
fluid_midi_driver_t* 
new_fluid_alsa_rawmidi_driver(fluid_settings_t* settings, 
			     handle_midi_event_func_t handler, 
			     void* data)
{
  int i, err;
  fluid_alsa_rawmidi_driver_t* dev;
  pthread_attr_t attr;
  int sched = SCHED_FIFO;
  struct sched_param priority;
  int count;
  struct pollfd *pfd = NULL;
  char* device = NULL;

  /* not much use doing anything */
  if (handler == NULL) {
    FLUID_LOG(FLUID_ERR, "Invalid argument");
    return NULL;
  }

  /* allocate the device */
  dev = FLUID_NEW(fluid_alsa_rawmidi_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_alsa_rawmidi_driver_t));

  dev->driver.handler = handler;
  dev->driver.data = data;

  /* allocate one event to store the input data */
  dev->parser = new_fluid_midi_parser();
  if (dev->parser == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    goto error_recovery;
  }

  /* get the device name. if none is specified, use the default device. */
  fluid_settings_getstr(settings, "midi.alsa.device", &device);
  if (device == NULL) {
    device = "default";
  }

  /* open the hardware device. only use midi in. */
  if ((err = snd_rawmidi_open(&dev->rawmidi_in, NULL, device, SND_RAWMIDI_NONBLOCK)) < 0) {
    FLUID_LOG(FLUID_ERR, "Error opening ALSA raw MIDI port");
    goto error_recovery;
  }

  /* get # of MIDI file descriptors */
  count = snd_rawmidi_poll_descriptors_count(dev->rawmidi_in);
  if (count > 0) {		/* make sure there are some */
    pfd = FLUID_MALLOC(sizeof (struct pollfd) * count);
    dev->pfd = FLUID_MALLOC(sizeof (struct pollfd) * count);
    /* grab file descriptor POLL info structures */
    count = snd_rawmidi_poll_descriptors(dev->rawmidi_in, pfd, count);
  }

  /* copy the input FDs */
  for (i = 0; i < count; i++) {		/* loop over file descriptors */
    if (pfd[i].events & POLLIN) { /* use only the input FDs */
      dev->pfd[dev->npfd].fd = pfd[i].fd;
      dev->pfd[dev->npfd].events = POLLIN; 
      dev->pfd[dev->npfd].revents = 0; 
      dev->npfd++;
    }
  }
  FLUID_FREE(pfd);

  dev->status = FLUID_MIDI_READY;

  /* create the midi thread */
  if (pthread_attr_init(&attr)) {
    FLUID_LOG(FLUID_ERR, "Couldn't initialize midi thread attributes");
    goto error_recovery;
  }

  /* Was: "use fifo scheduling. if it fails, use default scheduling." */
  /* Now normal scheduling is used by default for the MIDI thread. The reason is,
   * that fluidsynth works better with low latencies under heavy load, if only the 
   * audio thread is prioritized.
   * With MIDI at ordinary priority, that could result in individual notes being played
   * a bit late. On the other hand, if the audio thread is delayed, an audible dropout
   * is the result.
   * To reproduce this: Edirol UA-1 USB-MIDI interface, four buffers
   * with 45 samples each (roughly 4 ms latency), ravewave soundfont. -MN
   */ 

  /* Not so sure anymore. We're losing MIDI data, if we can't keep up with
   * the speed it is generated. */
/*  FLUID_LOG(FLUID_WARN, "Note: High-priority scheduling for the MIDI thread was intentionally disabled.");
    sched=SCHED_OTHER;*/

  while (1) {
    err = pthread_attr_setschedpolicy(&attr, sched);
    if (err) {
      FLUID_LOG(FLUID_WARN, "Couldn't set high priority scheduling for the MIDI input");
      if (sched == SCHED_FIFO) {
	sched = SCHED_OTHER;
	continue;
      } else {
	FLUID_LOG(FLUID_ERR, "Couldn't set scheduling policy.");
	goto error_recovery;
      }
    }

    /* SCHED_FIFO will not be active without setting the priority */
    priority.sched_priority = (sched == SCHED_FIFO) ? ALSA_RAWMIDI_SCHED_PRIORITY : 0;
    pthread_attr_setschedparam (&attr, &priority);

    err = pthread_create(&dev->thread, &attr, fluid_alsa_midi_run, (void*) dev);
    if (err) {
      FLUID_LOG(FLUID_WARN, "Couldn't set high priority scheduling for the MIDI input");
      if (sched == SCHED_FIFO) {
	sched = SCHED_OTHER;
	continue;
      } else {
	FLUID_LOG(FLUID_PANIC, "Couldn't create the midi thread.");
	goto error_recovery;
      }
    }
    break;
  }  
  return (fluid_midi_driver_t*) dev;
  
 error_recovery:
  delete_fluid_alsa_rawmidi_driver((fluid_midi_driver_t*) dev);
  return NULL;
  
}

/*
 * delete_fluid_alsa_rawmidi_driver
 */
int 
delete_fluid_alsa_rawmidi_driver(fluid_midi_driver_t* p)
{
  fluid_alsa_rawmidi_driver_t* dev;

  dev = (fluid_alsa_rawmidi_driver_t*) p;
  if (dev == NULL) {
    return FLUID_OK;
  }

  dev->status = FLUID_MIDI_DONE;

  /* cancel the thread and wait for it before cleaning up */
  if (dev->thread) {
    if (pthread_cancel(dev->thread)) {
      FLUID_LOG(FLUID_ERR, "Failed to cancel the midi thread");
      return FLUID_FAILED;
    }
    if (pthread_join(dev->thread, NULL)) {
      FLUID_LOG(FLUID_ERR, "Failed to join the midi thread");
      return FLUID_FAILED;
    }
  }
  if (dev->rawmidi_in) {
    snd_rawmidi_close(dev->rawmidi_in);
  }
  if (dev->parser != NULL) {
    delete_fluid_midi_parser(dev->parser);
  }
  FLUID_FREE(dev);
  return FLUID_OK;
}

/*
 * fluid_alsa_midi_run
 */
void* 
fluid_alsa_midi_run(void* d)
{
  int n, i;
  fluid_midi_event_t* evt;
  fluid_alsa_rawmidi_driver_t* dev = (fluid_alsa_rawmidi_driver_t*) d;

  /* make sure the other threads can cancel this thread any time */
  if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)) {
    FLUID_LOG(FLUID_ERR, "Failed to set the cancel state of the midi thread");
    pthread_exit(NULL);
  }
  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL)) {
    FLUID_LOG(FLUID_ERR, "Failed to set the cancel state of the midi thread");
    pthread_exit(NULL);
  }

  /* go into a loop until someone tells us to stop */
  dev->status = FLUID_MIDI_LISTENING;
  while (dev->status == FLUID_MIDI_LISTENING) {

    /* is there something to read? */
    n = poll(dev->pfd, dev->npfd, 100); /* use a 100 milliseconds timeout */
    if (n < 0) {
      perror("poll");
    } else if (n > 0) {

      /* read new data */
      n = snd_rawmidi_read(dev->rawmidi_in, dev->buffer, BUFFER_LENGTH);
      if ((n < 0) && (n != -EAGAIN)) {
	FLUID_LOG(FLUID_ERR, "Failed to read the midi input");
	dev->status = FLUID_MIDI_DONE;
      }

      /* let the parser convert the data into events */
      for (i = 0; i < n; i++) {
	evt = fluid_midi_parser_parse(dev->parser, dev->buffer[i]);
	if (evt != NULL) {
	  (*dev->driver.handler)(dev->driver.data, evt);
	}
      }
    };
  }
  pthread_exit(NULL);
}

/**************************************************************
 *
 *        Alsa sequencer
 *
 */


void fluid_alsa_seq_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "midi.alsa_seq.device", "default", 0, NULL, NULL);
  fluid_settings_register_str(settings, "midi.alsa_seq.id", "pid", 0, NULL, NULL);
}


static char* fluid_alsa_seq_full_id(char* id, char* buf, int len)
{
  if (id != NULL) {
    if (FLUID_STRCMP(id, "pid") == 0) {
      snprintf(buf, len, "FLUID Synth (%d)", getpid());
    } else {
      snprintf(buf, len, "FLUID Synth (%s)", id);
    }
  } else {
    snprintf(buf, len, "FLUID Synth");
  }

  return buf;
}

static char* fluid_alsa_seq_full_name(char* id, int port, char* buf, int len)
{  
  if (id != NULL) {
    if (FLUID_STRCMP(id, "pid") == 0) {
      snprintf(buf, len, "Synth input port (%d:%d)", getpid(), port);
    } else {
      snprintf(buf, len, "Synth input port (%s:%d)", id, port);
    }
  } else {
    snprintf(buf, len, "Synth input port");
  }
  return buf;
}


/*
 * new_fluid_alsa_seq_driver
 */
fluid_midi_driver_t* 
new_fluid_alsa_seq_driver(fluid_settings_t* settings, 
			 handle_midi_event_func_t handler, void* data)
{
  int i, err;
  fluid_alsa_seq_driver_t* dev;
  pthread_attr_t attr;
  int sched = SCHED_FIFO;
  struct sched_param priority;
  int count;
  struct pollfd *pfd = NULL;
  char* device = NULL;
  char* id;
  char full_id[64];
  char full_name[64];
  char id_pid[16];
  snd_seq_port_info_t *port_info = NULL;
  int midi_channels;

  /* not much use doing anything */
  if (handler == NULL) {
    FLUID_LOG(FLUID_ERR, "Invalid argument");
    return NULL;
  }

  /* allocate the device */
  dev = FLUID_NEW(fluid_alsa_seq_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_alsa_seq_driver_t));
  dev->driver.data = data;
  dev->driver.handler = handler;


  /* get the device name. if none is specified, use the default device. */
  fluid_settings_getstr(settings, "midi.alsa_seq.device", &device);
  if (device == NULL) {
    device = "default";
  }

  fluid_settings_getstr(settings, "midi.alsa_seq.id", &id);
  if (id == NULL) {
    sprintf(id_pid, "%d", getpid());
    id = id_pid;
  }

  /* open the sequencer INPUT only, non-blocking */
  err = snd_seq_open(&dev->seq_handle, device, SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK);
  if (err < 0) {
    FLUID_LOG(FLUID_ERR, "Error opening ALSA sequencer");
    goto error_recovery;
  }

  /* get # of MIDI file descriptors */
  count = snd_seq_poll_descriptors_count(dev->seq_handle, POLLIN);
  if (count > 0) {		/* make sure there are some */
    pfd = FLUID_MALLOC(sizeof (struct pollfd) * count);
    dev->pfd = FLUID_MALLOC(sizeof (struct pollfd) * count);
    /* grab file descriptor POLL info structures */
    count = snd_seq_poll_descriptors(dev->seq_handle, pfd, count, POLLIN);
  }

  /* copy the input FDs */
  for (i = 0; i < count; i++) {		/* loop over file descriptors */
    if (pfd[i].events & POLLIN) { /* use only the input FDs */
      dev->pfd[dev->npfd].fd = pfd[i].fd;
      dev->pfd[dev->npfd].events = POLLIN; 
      dev->pfd[dev->npfd].revents = 0; 
      dev->npfd++;
    }
  }
  FLUID_FREE(pfd);
  
  /* set the client name */
  snd_seq_set_client_name(dev->seq_handle, fluid_alsa_seq_full_id(id, full_id, 64));


  /* create the ports */
  snd_seq_port_info_alloca(&port_info);
  FLUID_MEMSET(port_info, 0, snd_seq_port_info_sizeof());

  fluid_settings_getint(settings, "synth.midi-channels", &midi_channels);
  dev->port_count = midi_channels / 16;

  snd_seq_port_info_set_capability(port_info, 
				   SND_SEQ_PORT_CAP_WRITE | 
				   SND_SEQ_PORT_CAP_SUBS_WRITE);
  snd_seq_port_info_set_type(port_info, 
			     SND_SEQ_PORT_TYPE_APPLICATION);
  snd_seq_port_info_set_midi_channels(port_info, 16);
  snd_seq_port_info_set_port_specified(port_info, 1);

  for (i = 0; i < dev->port_count; i++) {

    snd_seq_port_info_set_name(port_info, fluid_alsa_seq_full_name(id, i, full_name, 64));
    snd_seq_port_info_set_port(port_info, i);

    err = snd_seq_create_port(dev->seq_handle, port_info);
    if (err  < 0) {
      FLUID_LOG(FLUID_ERR, "Error creating ALSA sequencer port");
      goto error_recovery;
    }
  }

  /* tell the lash server our client id */
#ifdef LASH_ENABLED
  {
    int enable_lash = 0;
    fluid_settings_getint (settings, "lash.enable", &enable_lash);
    if (enable_lash)
      fluid_lash_alsa_client_id (fluid_lash_client, snd_seq_client_id (dev->seq_handle));
  }
#endif /* LASH_ENABLED */

  dev->status = FLUID_MIDI_READY;

  /* create the midi thread */
  if (pthread_attr_init(&attr)) {
    FLUID_LOG(FLUID_ERR, "Couldn't initialize midi thread attributes");
    goto error_recovery;
  }
  /* use fifo scheduling. if it fails, use default scheduling. */
  while (1) {
    err = pthread_attr_setschedpolicy(&attr, sched);
    if (err) {
      FLUID_LOG(FLUID_WARN, "Couldn't set high priority scheduling for the MIDI input");
      if (sched == SCHED_FIFO) {
	sched = SCHED_OTHER;
	continue;
      } else {
	FLUID_LOG(FLUID_ERR, "Couldn't set scheduling policy.");
	goto error_recovery;
      }
    }

    /* SCHED_FIFO will not be active without setting the priority */
    priority.sched_priority = (sched == SCHED_FIFO) ? ALSA_SEQ_SCHED_PRIORITY : 0;
    pthread_attr_setschedparam (&attr, &priority);

    err = pthread_create(&dev->thread, &attr, fluid_alsa_seq_run, (void*) dev);
    if (err) {
      FLUID_LOG(FLUID_WARN, "Couldn't set high priority scheduling for the MIDI input");
      if (sched == SCHED_FIFO) {
	sched = SCHED_OTHER;
	continue;
      } else {
	FLUID_LOG(FLUID_PANIC, "Couldn't create the midi thread.");
	goto error_recovery;
      }
    }
    break;
  }  
  return (fluid_midi_driver_t*) dev;

 error_recovery:
  delete_fluid_alsa_seq_driver((fluid_midi_driver_t*) dev);
  return NULL;
}

/*
 * delete_fluid_alsa_seq_driver
 */
int 
delete_fluid_alsa_seq_driver(fluid_midi_driver_t* p)
{
  fluid_alsa_seq_driver_t* dev;

  dev = (fluid_alsa_seq_driver_t*) p;
  if (dev == NULL) {
    return FLUID_OK;
  }

  dev->status = FLUID_MIDI_DONE;

  /* cancel the thread and wait for it before cleaning up */
  if (dev->thread) {
    if (pthread_cancel(dev->thread)) {
      FLUID_LOG(FLUID_ERR, "Failed to cancel the midi thread");
      return FLUID_FAILED;
    }
    if (pthread_join(dev->thread, NULL)) {
      FLUID_LOG(FLUID_ERR, "Failed to join the midi thread");
      return FLUID_FAILED;
    }
  }
  if (dev->seq_handle) {
    snd_seq_close(dev->seq_handle);
  }
  FLUID_FREE(dev);
  return FLUID_OK;
}

/*
 * fluid_alsa_seq_run
 */
void* 
fluid_alsa_seq_run(void* d)
{
  int n, i;
  snd_seq_event_t *seq_ev;
  fluid_midi_event_t evt;
  fluid_alsa_seq_driver_t* dev = (fluid_alsa_seq_driver_t*) d;
  int channel;

  /* make sure the other threads can cancel this thread any time */
  if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)) {
    FLUID_LOG(FLUID_ERR, "Failed to set the cancel state of the midi thread");
    pthread_exit(NULL);
  }
  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL)) {
    FLUID_LOG(FLUID_ERR, "Failed to set the cancel state of the midi thread");
    pthread_exit(NULL);
  }

  /* go into a loop until someone tells us to stop */
  dev->status = FLUID_MIDI_LISTENING;
  while (dev->status == FLUID_MIDI_LISTENING) {

    /* is there something to read? */
    n = poll(dev->pfd, dev->npfd, 1); /* use a 1 milliseconds timeout */
    if (n < 0) {
      perror("poll");
    } else if (n > 0) {

      /* read new events */
      while ((n = snd_seq_event_input(dev->seq_handle, &seq_ev)) >= 0)
	{
	  switch (seq_ev->type)
	    {
	    case SND_SEQ_EVENT_NOTEON:
	      evt.type = NOTE_ON;
	      evt.channel = seq_ev->dest.port * 16 + seq_ev->data.note.channel;
	      evt.param1 = seq_ev->data.note.note;
	      evt.param2 = seq_ev->data.note.velocity;
	      break;
	    case SND_SEQ_EVENT_NOTEOFF:
	      evt.type = NOTE_OFF;
	      evt.channel = seq_ev->dest.port * 16 + seq_ev->data.note.channel;
	      evt.param1 = seq_ev->data.note.note;
	      evt.param2 = seq_ev->data.note.velocity;
	      break;
	    case SND_SEQ_EVENT_KEYPRESS:
	      evt.type = KEY_PRESSURE;
	      evt.channel = seq_ev->dest.port * 16 + seq_ev->data.note.channel;
	      evt.param1 = seq_ev->data.note.note;
	      evt.param2 = seq_ev->data.note.velocity;
	      break;
	    case SND_SEQ_EVENT_CONTROLLER:
	      evt.type = CONTROL_CHANGE;
	      evt.channel = seq_ev->dest.port * 16 + seq_ev->data.control.channel;
	      evt.param1 = seq_ev->data.control.param;
	      evt.param2 = seq_ev->data.control.value;
	      break;
	    case SND_SEQ_EVENT_PITCHBEND:
	      evt.type = PITCH_BEND;
	      evt.channel = seq_ev->dest.port * 16 + seq_ev->data.control.channel;

	      /* ALSA pitch bend is -8192 - 8191, we adjust it here */
	      evt.param1 = seq_ev->data.control.value + 8192;
	      break;
	    case SND_SEQ_EVENT_PGMCHANGE:
	      evt.type = PROGRAM_CHANGE;
	      evt.channel = seq_ev->dest.port * 16 + seq_ev->data.control.channel;
	      evt.param1 = seq_ev->data.control.value;
	      break;
	    case SND_SEQ_EVENT_CHANPRESS:
	      evt.type = CHANNEL_PRESSURE;
	      evt.channel = seq_ev->dest.port * 16 + seq_ev->data.control.channel;
	      evt.param1 = seq_ev->data.control.value;
	      break;
	    default:
	      continue;		/* unhandled event, next loop iteration */
	    }

	  /* send the events to the next link in the chain */
	  (*dev->driver.handler)(dev->driver.data, &evt);
	}
    }

    if (n < 0)		/* Negative value indicates an error */
    {
      if (n == -EPERM)		/* interrupted system call? */
	;
      else if (n != -ENOSPC)	/* input event buffer overrun? */
	FLUID_LOG(FLUID_WARN, "ALSA sequencer buffer overrun, lost events");
      else
      {
	FLUID_LOG(FLUID_ERR, "Error occured while reading ALSA sequencer events (code=%d)", n);
	dev->status = FLUID_MIDI_DONE;
      }
    }
  }
  pthread_exit(NULL);
}

#endif /* #if ALSA_SUPPORT */
