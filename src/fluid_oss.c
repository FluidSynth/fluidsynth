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


/* fluid_oss.c
 *
 * Drivers for the Open (?) Sound System
 */

#include "fluid_synth.h"
#include "fluid_midi.h"
#include "fluid_adriver.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#if OSS_SUPPORT

#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>

#define BUFFER_LENGTH 512

/* SCHED_FIFO priorities for OSS threads (see pthread_attr_setschedparam) */
#define OSS_PCM_SCHED_PRIORITY 90
#define OSS_MIDI_SCHED_PRIORITY 90

/** fluid_oss_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct {
  fluid_audio_driver_t driver;
  fluid_synth_t* synth;
  fluid_audio_callback_t read;
  void* buffer;
  pthread_t thread;
  int cont;
  int dspfd;
  int buffer_size;
  int buffer_byte_size;
  int bigendian;
  int formats;
  int format;
  int caps;
  fluid_audio_func_t callback;
  void* data;
  float* buffers[2];
} fluid_oss_audio_driver_t;

int delete_fluid_oss_audio_driver(fluid_audio_driver_t* p);

/* local utilities */
static int fluid_oss_get_caps(fluid_oss_audio_driver_t* dev);
static int fluid_oss_set_queue_size(fluid_oss_audio_driver_t* dev, int ss, int ch, int qs, int bs);
static int fluid_oss_get_sample_formats(fluid_oss_audio_driver_t* dev);
static void* fluid_oss_audio_run(void* d);
static void* fluid_oss_audio_run2(void* d);


typedef struct {
  fluid_midi_driver_t driver;
  int fd;
  pthread_t thread;
  int status;
  unsigned char buffer[BUFFER_LENGTH];
  fluid_midi_parser_t* parser;
} fluid_oss_midi_driver_t;

fluid_midi_driver_t*
new_fluid_oss_midi_driver(fluid_settings_t* settings,
			 handle_midi_event_func_t handler, void* data);
int delete_fluid_oss_midi_driver(fluid_midi_driver_t* p);
int fluid_oss_midi_driver_status(fluid_midi_driver_t* p);
static void* fluid_oss_midi_run(void* d);


void
fluid_oss_audio_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "audio.oss.device", "/dev/dsp", 0, NULL, NULL);
}

/*
 * new_fluid_oss_audio_driver
 */
fluid_audio_driver_t*
new_fluid_oss_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth)
{
  fluid_oss_audio_driver_t* dev = NULL;
  int channels, sr, sample_size = 0, oss_format;
  struct stat devstat;
  int queuesize;
  double sample_rate;
  int periods, period_size;
  char* devname;
  int format;
  pthread_attr_t attr;
  int err;
  int sched = SCHED_FIFO;
  struct sched_param priority;

  dev = FLUID_NEW(fluid_oss_audio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_oss_audio_driver_t));

  fluid_settings_getint(settings, "audio.periods", &periods);
  fluid_settings_getint(settings, "audio.period-size", &period_size);
  fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);

  dev->dspfd = -1;
  dev->synth = synth;
  dev->callback = NULL;
  dev->data = NULL;
  dev->cont = 1;
  dev->buffer_size = (int) period_size;
  queuesize = (int) (periods * period_size);

  if (fluid_settings_str_equal(settings, "audio.sample-format", "16bits")) {
    sample_size = 16;
    oss_format = AFMT_S16_LE;
    dev->read = fluid_synth_write_s16;
    dev->buffer_byte_size = dev->buffer_size * 4;

  } else if (fluid_settings_str_equal(settings, "audio.sample-format", "float")) {
    sample_size = 32;
    oss_format = -1;
    dev->read = fluid_synth_write_float;
    dev->buffer_byte_size = dev->buffer_size * 8;

  } else {
    FLUID_LOG(FLUID_ERR, "Unknown sample format");
    goto error_recovery;
  }

  dev->buffer = FLUID_MALLOC(dev->buffer_byte_size);
  if (dev->buffer == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    goto error_recovery;
  }

  if (!fluid_settings_getstr(settings, "audio.oss.device", &devname)) {
    devname = "/dev/dsp";
  }

  if (stat(devname, &devstat) == -1) {
    FLUID_LOG(FLUID_ERR, "Device <%s> does not exists", devname);
    goto error_recovery;
  }
  if ((devstat.st_mode & S_IFCHR) != S_IFCHR) {
    FLUID_LOG(FLUID_ERR, "Device <%s> is not a device file", devname);
    goto error_recovery;
  }

  dev->dspfd = open(devname, O_WRONLY, 0);

  if (dev->dspfd == -1) {
    FLUID_LOG(FLUID_ERR, "Device <%s> could not be opened for writing: %s",
	     devname, strerror(errno));
    goto error_recovery;
  }

  if (fluid_oss_set_queue_size(dev, sample_size, 2, queuesize, period_size) < 0) {
    FLUID_LOG(FLUID_ERR, "Can't set device buffer size");
    goto error_recovery;
  }

  format = oss_format;
  if (ioctl(dev->dspfd, SNDCTL_DSP_SETFMT, &oss_format) < 0) {
    FLUID_LOG(FLUID_ERR, "Can't set the sample format");
    goto error_recovery;
  }
  if (oss_format != format) {
    FLUID_LOG(FLUID_ERR, "Can't set the sample format");
    goto error_recovery;
  }

  channels = 2;
  if (ioctl(dev->dspfd, SOUND_PCM_WRITE_CHANNELS, &channels) < 0){
    FLUID_LOG(FLUID_ERR, "Can't set the number of channels");
    goto error_recovery;
  }
  if (channels != 2) {
    FLUID_LOG(FLUID_ERR, "Can't set the number of channels");
    goto error_recovery;
  }

  sr = sample_rate;
  if (ioctl(dev->dspfd, SNDCTL_DSP_SPEED, &sr) < 0){
    FLUID_LOG(FLUID_ERR, "Can't set the sample rate");
    goto error_recovery;
  }
  if ((sr < 0.95 * sample_rate) ||
      (sr > 1.05 * sample_rate)) {
    FLUID_LOG(FLUID_ERR, "Can't set the sample rate");
    goto error_recovery;
  }

  if (pthread_attr_init(&attr)) {
    FLUID_LOG(FLUID_ERR, "Couldn't initialize audio thread attributes");
    goto error_recovery;
  }

  /* the pthread_create man page explains that
     pthread_attr_setschedpolicy returns an error if the user is not
     permitted the set SCHED_FIFO. it seems however that no error is
     returned but pthread_create fails instead. that's why i try to
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
    priority.sched_priority = (sched == SCHED_FIFO) ? OSS_PCM_SCHED_PRIORITY : 0;
    pthread_attr_setschedparam (&attr, &priority);

    err = pthread_create(&dev->thread, &attr, fluid_oss_audio_run, (void*) dev);
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
  delete_fluid_oss_audio_driver((fluid_audio_driver_t*) dev);
  return NULL;
}

fluid_audio_driver_t*
new_fluid_oss_audio_driver2(fluid_settings_t* settings, fluid_audio_func_t func, void* data)
{
  fluid_oss_audio_driver_t* dev = NULL;
  int channels, sr;
  struct stat devstat;
  int queuesize;
  double sample_rate;
  int periods, period_size;
  char* devname;
  int format;
  pthread_attr_t attr;
  int err;
  int sched = SCHED_FIFO;
  struct sched_param priority;

  dev = FLUID_NEW(fluid_oss_audio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_oss_audio_driver_t));

  fluid_settings_getint(settings, "audio.periods", &periods);
  fluid_settings_getint(settings, "audio.period-size", &period_size);
  fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);

  dev->dspfd = -1;
  dev->synth = NULL;
  dev->read = NULL;
  dev->callback = func;
  dev->data = data;
  dev->cont = 1;
  dev->buffer_size = (int) period_size;
  queuesize = (int) (periods * period_size);
  dev->buffer_byte_size = dev->buffer_size * 2 * 2; /* 2 channels * 16 bits audio */


  if (!fluid_settings_getstr(settings, "audio.oss.device", &devname)) {
    devname = "/dev/dsp";
  }
  if (stat(devname, &devstat) == -1) {
    FLUID_LOG(FLUID_ERR, "Device <%s> does not exists", devname);
    goto error_recovery;
  }
  if ((devstat.st_mode & S_IFCHR) != S_IFCHR) {
    FLUID_LOG(FLUID_ERR, "Device <%s> is not a device file", devname);
    goto error_recovery;
  }

  dev->dspfd = open(devname, O_WRONLY, 0);
  if (dev->dspfd == -1) {
    FLUID_LOG(FLUID_ERR, "Device <%s> could not be opened for writing: %s",
	     devname, strerror(errno));
    goto error_recovery;
  }


  if (fluid_oss_set_queue_size(dev, 16, 2, queuesize, period_size) < 0) {
    FLUID_LOG(FLUID_ERR, "Can't set device buffer size");
    goto error_recovery;
  }

  format = AFMT_S16_LE;
  if (ioctl(dev->dspfd, SNDCTL_DSP_SETFMT, &format) < 0) {
    FLUID_LOG(FLUID_ERR, "Can't set the sample format");
    goto error_recovery;
  }
  if (format != AFMT_S16_LE) {
    FLUID_LOG(FLUID_ERR, "Can't set the sample format");
    goto error_recovery;
  }

  channels = 2;
  if (ioctl(dev->dspfd, SOUND_PCM_WRITE_CHANNELS, &channels) < 0){
    FLUID_LOG(FLUID_ERR, "Can't set the number of channels");
    goto error_recovery;
  }
  if (channels != 2) {
    FLUID_LOG(FLUID_ERR, "Can't set the number of channels");
    goto error_recovery;
  }

  sr = sample_rate;
  if (ioctl(dev->dspfd, SNDCTL_DSP_SPEED, &sr) < 0){
    FLUID_LOG(FLUID_ERR, "Can't set the sample rate");
    goto error_recovery;
  }
  if ((sr < 0.95 * sample_rate) ||
      (sr > 1.05 * sample_rate)) {
    FLUID_LOG(FLUID_ERR, "Can't set the sample rate");
    goto error_recovery;
  }

  /* allocate the buffers. FIXME!!! don't use interleaved samples */
  dev->buffer = FLUID_MALLOC(dev->buffer_byte_size);
  dev->buffers[0] = FLUID_ARRAY(float, dev->buffer_size);
  dev->buffers[1] = FLUID_ARRAY(float, dev->buffer_size);
  if ((dev->buffer == NULL) || (dev->buffers[0] == NULL) || (dev->buffers[1] == NULL)) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    goto error_recovery;
  }

  if (pthread_attr_init(&attr)) {
    FLUID_LOG(FLUID_ERR, "Couldn't initialize audio thread attributes");
    goto error_recovery;
  }

  /* the pthread_create man page explains that
     pthread_attr_setschedpolicy returns an error if the user is not
     permitted the set SCHED_FIFO. it seems however that no error is
     returned but pthread_create fails instead. that's why i try to
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
    priority.sched_priority = (sched == SCHED_FIFO) ? OSS_PCM_SCHED_PRIORITY : 0;
    pthread_attr_setschedparam (&attr, &priority);

    err = pthread_create(&dev->thread, &attr, fluid_oss_audio_run2, (void*) dev);
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
  delete_fluid_oss_audio_driver((fluid_audio_driver_t*) dev);
  return NULL;
}

/*
 * delete_fluid_oss_audio_driver
 */
int
delete_fluid_oss_audio_driver(fluid_audio_driver_t* p)
{
  fluid_oss_audio_driver_t* dev = (fluid_oss_audio_driver_t*) p;

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
  if (dev->dspfd >= 0) {
    close(dev->dspfd);
  }
  if (dev->buffer != NULL) {
    FLUID_FREE(dev->buffer);
  }
  FLUID_FREE(dev);
  return FLUID_OK;
}

/*
 * fluid_oss_get_sample_formats
 */
int
fluid_oss_get_sample_formats(fluid_oss_audio_driver_t* dev)
{
  int mask;
  unsigned short U16 = 1;
  unsigned char* U8 = (unsigned char*) &U16;

  dev->formats = 0;
  dev->bigendian = 0;
  if (ioctl(dev->dspfd, SNDCTL_DSP_GETFMTS, &mask) == -1) {
    return -1;
  }
  dev->formats = mask;
  if (U8[1] == 1) {
    FLUID_LOG(FLUID_DBG, "Machine is big endian.");
    dev->bigendian = 1;
  }
  if (U8[0] == 1) {
    FLUID_LOG(FLUID_DBG, "Machine is little endian.");
    dev->bigendian = 0;
  }
  FLUID_LOG(FLUID_DBG, "The sound device supports the following audio formats:");
  if (mask & AFMT_U8)        { FLUID_LOG(FLUID_DBG, "  U8"); }
  if (mask & AFMT_S8)        { FLUID_LOG(FLUID_DBG, "  S8"); }
  if (mask & AFMT_U16_LE)    { FLUID_LOG(FLUID_DBG, "  U16LE"); }
  if (mask & AFMT_U16_BE)    { FLUID_LOG(FLUID_DBG, "  U16BE"); }
  if (mask & AFMT_S16_LE)    { FLUID_LOG(FLUID_DBG, "  S16LE"); }
  if (mask & AFMT_S16_BE)    { FLUID_LOG(FLUID_DBG, "  S16BE"); }
  if (mask & AFMT_MU_LAW)    { FLUID_LOG(FLUID_DBG, "  mu-law"); }
  if (mask & AFMT_A_LAW)     { FLUID_LOG(FLUID_DBG, "  a-law"); }
  if (mask & AFMT_IMA_ADPCM) { FLUID_LOG(FLUID_DBG, "  ima-adpcm"); }
  if (mask & AFMT_MPEG)      { FLUID_LOG(FLUID_DBG, "  mpeg"); }
  return 0;
}

/**
 *  fluid_oss_get_caps
 *
 *  Get the audio capacities of the sound card.
 */
int
fluid_oss_get_caps(fluid_oss_audio_driver_t* dev)
{
  int caps;
  dev->caps = 0;
  if (ioctl(dev->dspfd, SNDCTL_DSP_GETCAPS, &caps) < 0) {
    return -1;
  }
  dev->caps = caps;
  FLUID_LOG(FLUID_DBG, "The sound device has the following capabilities:");
  if (caps & DSP_CAP_DUPLEX)   {
    FLUID_LOG(FLUID_DBG, "  Duplex:    simultaneous playing and recording possible") ;
  } else {
    FLUID_LOG(FLUID_DBG, "  Duplex:    simultaneous playing and recording not possible");
  }
  if (caps & DSP_CAP_REALTIME) {
    FLUID_LOG(FLUID_DBG, "  Real-time: precise reporting of output pointer possible");
  } else {
    FLUID_LOG(FLUID_DBG, "  Real-time: precise reporting of output pointer not possible");
  }
  if (caps & DSP_CAP_BATCH) {
    FLUID_LOG(FLUID_DBG, "  Batch:     local storage for recording and/or playback");
  } else {
    FLUID_LOG(FLUID_DBG, "  Batch:     no local storage for recording and/or playback");
  }
  if (caps & DSP_CAP_TRIGGER) {
    FLUID_LOG(FLUID_DBG, "  Trigger:   triggering of recording/playback possible");
  } else {
    FLUID_LOG(FLUID_DBG, "  Trigger:   triggering of recording/playback not possible");
  }
  if (caps & DSP_CAP_MMAP) {
    FLUID_LOG(FLUID_DBG, "  Mmap:      direct access to the hardware level buffer possible");
  } else {
    FLUID_LOG(FLUID_DBG, "  Mmap:      direct access to the hardware level buffer not possible");
  }
  return 0;
}

/**
 *  fluid_oss_set_queue_size
 *
 *  Set the internal buffersize of the output device.
 *
 *  @param ss Sample size in bits
 *  @param ch Number of channels
 *  @param qs The queue size in frames
 *  @param bs The synthesis buffer size in frames
 */
int
fluid_oss_set_queue_size(fluid_oss_audio_driver_t* dev, int ss, int ch, int qs, int bs)
{
  unsigned int fragmentSize;
  unsigned int fragSizePower;
  unsigned int fragments;
  unsigned int fragmentsPower;

  fragmentSize = (unsigned int) (bs * ch * ss / 8);

  fragSizePower = 0;
  while (0 < fragmentSize) {
    fragmentSize = (fragmentSize >> 1);
    fragSizePower++;
  }
  fragSizePower--;

  fragments = (unsigned int) (qs / bs);
  if (fragments < 2) {
    fragments = 2;
  }

  /* make sure fragments is a power of 2 */
  fragmentsPower = 0;
  while (0 < fragments) {
    fragments = (fragments >> 1);
    fragmentsPower++;
  }
  fragmentsPower--;

  fragments = (1 << fragmentsPower);
  fragments = (fragments << 16) + fragSizePower;

  return ioctl(dev->dspfd, SNDCTL_DSP_SETFRAGMENT, &fragments);
}

/*
 * fluid_oss_audio_run
 */
void*
fluid_oss_audio_run(void* d)
{
  fluid_oss_audio_driver_t* dev = (fluid_oss_audio_driver_t*) d;
  fluid_synth_t* synth = dev->synth;
  void* buffer = dev->buffer;
  int len = dev->buffer_size;

  /* it's as simple as that: */
  while (dev->cont)
  {
    dev->read (synth, len, buffer, 0, 2, buffer, 1, 2);
    write (dev->dspfd, buffer, dev->buffer_byte_size);
  }

  FLUID_LOG(FLUID_DBG, "Audio thread finished");

  pthread_exit(NULL);

  return 0; /* not reached */
}


/*
 * fluid_oss_audio_run
 */
void*
fluid_oss_audio_run2(void* d)
{
  fluid_oss_audio_driver_t* dev = (fluid_oss_audio_driver_t*) d;
  short* buffer = (short*) dev->buffer;
  float* left = dev->buffers[0];
  float* right = dev->buffers[1];
  int buffer_size = dev->buffer_size;
  int len = dev->buffer_size;
  int dither_index = 0;
  int i, k;

  FLUID_LOG(FLUID_DBG, "Audio thread running");

  /* it's as simple as that: */
  while (dev->cont)
  {
    (*dev->callback)(dev->data, buffer_size, 0, NULL, 2, dev->buffers);

    fluid_synth_dither_s16 (&dither_index, buffer_size, left, right,
			    buffer, 0, 2, buffer, 1, 2);

    write (dev->dspfd, buffer, dev->buffer_byte_size);
  }

  FLUID_LOG(FLUID_DBG, "Audio thread finished");

  pthread_exit(NULL);

  return 0; /* not reached */
}


void fluid_oss_midi_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "midi.oss.device", "/dev/midi", 0, NULL, NULL);
}

/*
 * new_fluid_oss_midi_driver
 */
fluid_midi_driver_t*
new_fluid_oss_midi_driver(fluid_settings_t* settings,
			 handle_midi_event_func_t handler, void* data)
{
  int err;
  fluid_oss_midi_driver_t* dev;
  pthread_attr_t attr;
  int sched = SCHED_FIFO;
  struct sched_param priority;
  char* device;

  /* not much use doing anything */
  if (handler == NULL) {
    FLUID_LOG(FLUID_ERR, "Invalid argument");
    return NULL;
  }

  /* allocate the device */
  dev = FLUID_NEW(fluid_oss_midi_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_oss_midi_driver_t));
  dev->fd = -1;

  dev->driver.handler = handler;
  dev->driver.data = data;

  /* allocate one event to store the input data */
  dev->parser = new_fluid_midi_parser();
  if (dev->parser == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    goto error_recovery;
  }

  /* get the device name. if none is specified, use the default device. */
  fluid_settings_getstr(settings, "midi.oss.device", &device);
  if (device == NULL) {
    device = "/dev/midi";
  }

  /* open the default hardware device. only use midi in. */
  dev->fd = open(device, O_RDONLY, 0);
  if (dev->fd < 0) {
    perror(device);
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
	FLUID_LOG(FLUID_ERR, "Couldn't set scheduling policy");
	goto error_recovery;
      }
    }

    /* SCHED_FIFO will not be active without setting the priority */
    priority.sched_priority = (sched == SCHED_FIFO) ? OSS_MIDI_SCHED_PRIORITY : 0;
    pthread_attr_setschedparam (&attr, &priority);

    err = pthread_create(&dev->thread, &attr, fluid_oss_midi_run, (void*) dev);
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
  delete_fluid_oss_midi_driver((fluid_midi_driver_t*) dev);
  return NULL;
}

/*
 * delete_fluid_oss_midi_driver
 */
int
delete_fluid_oss_midi_driver(fluid_midi_driver_t* p)
{
  int err;
  fluid_oss_midi_driver_t* dev;

  dev = (fluid_oss_midi_driver_t*) p;
  if (dev == NULL) {
    return FLUID_OK;
  }

  dev->status = FLUID_MIDI_DONE;

  /* cancel the thread and wait for it before cleaning up */
  if (dev->thread) {
    err = pthread_cancel(dev->thread);
    if (err) {
      FLUID_LOG(FLUID_ERR, "Failed to cancel the midi thread");
      return FLUID_FAILED;
    }
    if (pthread_join(dev->thread, NULL)) {
      FLUID_LOG(FLUID_ERR, "Failed to join the midi thread");
      return FLUID_FAILED;
    }
  }
  if (dev->fd >= 0) {
    close(dev->fd);
  }
  if (dev->parser != NULL) {
    delete_fluid_midi_parser(dev->parser);
  }
  FLUID_FREE(dev);
  return FLUID_OK;
}

/*
 * fluid_oss_midi_run
 */
void*
fluid_oss_midi_run(void* d)
{
  int n, i;
  fluid_midi_event_t* evt;
  fluid_oss_midi_driver_t* dev = (fluid_oss_midi_driver_t*) d;

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

    /* read new data */
    n = read(dev->fd, dev->buffer, BUFFER_LENGTH);
    if (n < 0) {
      perror("read");
      FLUID_LOG(FLUID_ERR, "Failed to read the midi input");
      dev->status = FLUID_MIDI_DONE;
    }

    /* let the parser convert the data into events */
    for (i = 0; i < n; i++) {
      evt = fluid_midi_parser_parse(dev->parser, dev->buffer[i]);
      if (evt != NULL) {
	/* send the event to the next link in the chain */
	(*dev->driver.handler)(dev->driver.data, evt);
      }
    }

  }
  pthread_exit(NULL);
}

int
fluid_oss_midi_driver_status(fluid_midi_driver_t* p)
{
  fluid_oss_midi_driver_t* dev = (fluid_oss_midi_driver_t*) p;
  return dev->status;
}

#endif /*#if OSS_SUPPORT */
