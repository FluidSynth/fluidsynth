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
//#include "fluid_midi_router.h"
#include "fluid_settings.h"

#if ALSA_SUPPORT
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>

#include "config.h"

#ifdef HAVE_LADCCA
#include <ladcca/ladcca.h>
extern cca_client_t * fluid_cca_client;
#endif /* HAVE_LADCCA */



#define FLUID_ALSA_DEFAULT_MIDI_DEVICE  "default"
#define FLUID_ALSA_DEFAULT_SEQ_DEVICE   "default"

#define BUFFER_LENGTH 512


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
  int seq_port;
  struct pollfd *pfd;
  int npfd;
  pthread_t thread;
  int status;
} fluid_alsa_seq_driver_t;

fluid_midi_driver_t* new_fluid_alsa_seq_driver(fluid_settings_t* settings, 
					     handle_midi_event_func_t handler, 
					     void* data);
int delete_fluid_alsa_seq_driver(fluid_midi_driver_t* p);
static void* fluid_alsa_seq_run(void* d);



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
  int count;
  struct pollfd *pfd = NULL;
  char* device = NULL;
  char* id;
  char full_id[64];
  char full_name[64];

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
  dev->seq_port = -1;
  dev->driver.data = data;
  dev->driver.handler = handler;

  /* get the device name. if none is specified, use the default device. */
  fluid_settings_getstr(settings, "midi.alsa_seq.device", &device);
  if (device == NULL) {
    device = "default";
  }

  /* open the sequencer INPUT only, non-blocking */
  if ((err = snd_seq_open(&dev->seq_handle, device, SND_SEQ_OPEN_INPUT,
			  SND_SEQ_NONBLOCK)) < 0) {
    FLUID_LOG(FLUID_ERR, "Error opening ALSA sequencer");
    goto error_recovery;
  }

  /* tell the ladcca server our client id */
#ifdef HAVE_LADCCA
  {
    int enable_ladcca = 0;
    fluid_settings_getint (settings, "ladcca.enable", &enable_ladcca);
    if (enable_ladcca)
      cca_alsa_client_id (fluid_cca_client, snd_seq_client_id (dev->seq_handle));
  }
#endif /* HAVE_LADCCA */

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

  fluid_settings_getstr(settings, "midi.alsa_seq.id", &id);
  
  if (id != NULL) {
    if (FLUID_STRCMP(id, "pid") == 0) {
      snprintf(full_id, 64, "FLUID Synth (%d)", getpid());
      snprintf(full_name, 64, "Synth input port (%d)", getpid());
    } else {
      snprintf(full_id, 64, "FLUID Synth (%s)", id);
      snprintf(full_name, 64, "Synth input port (%s)", id);
    }
  } else {
    snprintf(full_id, 64, "FLUID Synth");
    snprintf(full_name, 64, "Synth input port");
  }
  
  /* set the client name */
  snd_seq_set_client_name (dev->seq_handle, full_id);

  if ((dev->seq_port = snd_seq_create_simple_port (dev->seq_handle,
	full_name,
	SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
	SND_SEQ_PORT_TYPE_APPLICATION)) < 0)
    {
      FLUID_LOG(FLUID_ERR, "Error creating ALSA sequencer port");
      goto error_recovery;
    }
  

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
  if (dev->seq_port >= 0) {
    snd_seq_delete_simple_port (dev->seq_handle, dev->seq_port);
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
	      evt.channel = seq_ev->data.note.channel;
	      evt.param1 = seq_ev->data.note.note;
	      evt.param2 = seq_ev->data.note.velocity;
	      break;
	    case SND_SEQ_EVENT_NOTEOFF:
	      evt.type = NOTE_OFF;
	      evt.channel = seq_ev->data.note.channel;
	      evt.param1 = seq_ev->data.note.note;
	      evt.param2 = seq_ev->data.note.velocity;
	      break;
	    case SND_SEQ_EVENT_KEYPRESS:
	      evt.type = KEY_PRESSURE;
	      evt.channel = seq_ev->data.note.channel;
	      evt.param1 = seq_ev->data.note.note;
	      evt.param2 = seq_ev->data.note.velocity;
	      break;
	    case SND_SEQ_EVENT_CONTROLLER:
	      evt.type = CONTROL_CHANGE;
	      evt.channel = seq_ev->data.control.channel;
	      evt.param1 = seq_ev->data.control.param;
	      evt.param2 = seq_ev->data.control.value;
	      break;
	    case SND_SEQ_EVENT_PITCHBEND:
	      evt.type = PITCH_BEND;
	      evt.channel = seq_ev->data.control.channel;

	      /* ALSA pitch bend is -8192 - 8191, we adjust it here */
	      evt.param1 = seq_ev->data.control.value + 8192;
	      break;
	    case SND_SEQ_EVENT_PGMCHANGE:
	      evt.type = PROGRAM_CHANGE;
	      evt.channel = seq_ev->data.control.channel;
	      evt.param1 = seq_ev->data.control.value;
	      break;
	    case SND_SEQ_EVENT_CHANPRESS:
	      evt.type = CHANNEL_PRESSURE;
	      evt.channel = seq_ev->data.control.channel;
	      evt.param1 = seq_ev->data.control.value;
	      break;
	    default:
	      continue;		/* unhandled event, next loop iteration */
	    }

	  /* send the events to the next link in the chain */
	  (*dev->driver.handler)(dev->driver.data, &evt);
	}
    }

    if ((n < 0) && (n != -EAGAIN)) {
      FLUID_LOG(FLUID_ERR, "Error occured while reading ALSA sequencer events");
      dev->status = FLUID_MIDI_DONE;
    }
  }
  pthread_exit(NULL);
}

#endif /* #if ALSA_SUPPORT */
