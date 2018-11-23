/* sndio backend for FluidSynth - A Software Synthesizer
 *
 * Copyright (c) 2008 Jacob Meuser <jakemsr@sdf.lonestar.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/* fluid_sndio.c
 *
 * Driver for the sndio audio access library
 */

#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_midi.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#if SNDIO_SUPPORT

#include <sndio.h>

#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>


/** fluid_sndio_audio_driver_t
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
  struct sio_hdl *hdl;
  struct sio_par par;
  int buffer_size;
  int buffer_byte_size;
  fluid_audio_func_t callback;
  void* data;
  float* buffers[2];
} fluid_sndio_audio_driver_t;

typedef struct {
  fluid_midi_driver_t driver;
  struct mio_hdl *hdl;
  pthread_t thread;
  int status;
  fluid_midi_parser_t *parser;
} fluid_sndio_midi_driver_t;

int delete_fluid_sndio_audio_driver(fluid_audio_driver_t* p);

/* local utilities */
static void* fluid_sndio_audio_run(void* d);
static void* fluid_sndio_audio_run2(void* d);


void
fluid_sndio_audio_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "audio.sndio.device", "default", 0);
}

/*
 * new_fluid_sndio_audio_driver
 */
fluid_audio_driver_t*
new_fluid_sndio_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth)
{
  fluid_sndio_audio_driver_t* dev = NULL;
  double sample_rate;
  int periods, period_size;
  char* devname;
  pthread_attr_t attr;
  int err;

  dev = FLUID_NEW(fluid_sndio_audio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_sndio_audio_driver_t));

  fluid_settings_getint(settings, "audio.periods", &periods);
  fluid_settings_getint(settings, "audio.period-size", &period_size);
  fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);

  dev->hdl = NULL;
  dev->synth = synth;
  dev->callback = NULL;
  dev->data = NULL;
  dev->cont = 1;

  if (fluid_settings_dupstr(settings, "audio.sndio.device", &devname) != FLUID_OK || !devname) {
    devname = NULL;
  }

  dev->hdl = sio_open(devname, SIO_PLAY, 0);
  if (dev->hdl == NULL) {
    FLUID_LOG(FLUID_ERR, "sndio could not be opened for writing");
    goto error_recovery;
  }

  sio_initpar(&dev->par);

  if (fluid_settings_str_equal(settings, "audio.sample-format", "16bits")) {
    dev->par.bits = 16;
    dev->par.le = SIO_LE_NATIVE;
    dev->read = fluid_synth_write_s16;
  } else {
    FLUID_LOG(FLUID_ERR, "Unknown sample format");
    goto error_recovery;
  }

  dev->par.appbufsz = period_size * periods;
  dev->par.round = period_size;

  dev->par.pchan = 2;
  dev->par.rate = sample_rate;

  if (!sio_setpar(dev->hdl, &dev->par)) {
    FLUID_LOG(FLUID_ERR, "Couldn't set sndio audio parameters");
    goto error_recovery;
  }

  if (!sio_getpar(dev->hdl, &dev->par)) {
    FLUID_LOG(FLUID_ERR, "Couldn't get sndio audio parameters");
    goto error_recovery;
  } else if (dev->par.pchan != 2 || dev->par.rate != sample_rate ||
      dev->par.bits != 16) {
    FLUID_LOG(FLUID_ERR, "Couldn't set sndio audio parameters as desired");
    goto error_recovery;
  }

  dev->buffer_size = dev->par.round;
  dev->buffer_byte_size = dev->par.round * dev->par.bps * dev->par.pchan;

  dev->buffer = FLUID_MALLOC(dev->buffer_byte_size);
  if (dev->buffer == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    goto error_recovery;
  }

  if (!sio_start(dev->hdl)) {
    FLUID_LOG(FLUID_ERR, "Couldn't start sndio");
    goto error_recovery;
  }

  if (pthread_attr_init(&attr)) {
    FLUID_LOG(FLUID_ERR, "Couldn't initialize audio thread attributes");
    goto error_recovery;
  }

  err = pthread_create(&dev->thread, &attr, fluid_sndio_audio_run, (void*) dev);
  if (err) {
    FLUID_LOG(FLUID_ERR, "Couldn't create audio thread");
    goto error_recovery;
  }

  return (fluid_audio_driver_t*) dev;

error_recovery:
  delete_fluid_sndio_audio_driver((fluid_audio_driver_t*) dev);
  return NULL;
}

fluid_audio_driver_t*
new_fluid_sndio_audio_driver2(fluid_settings_t* settings, fluid_audio_func_t func, void* data)
{
  fluid_sndio_audio_driver_t* dev = NULL;
  double sample_rate;
  int periods, period_size;
  char* devname;
  pthread_attr_t attr;
  int err;

  dev = FLUID_NEW(fluid_sndio_audio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_sndio_audio_driver_t));

  fluid_settings_getint(settings, "audio.periods", &periods);
  fluid_settings_getint(settings, "audio.period-size", &period_size);
  fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);

  dev->hdl = NULL;
  dev->synth = NULL;
  dev->read = NULL;
  dev->callback = func;
  dev->data = data;
  dev->cont = 1;

  if (fluid_settings_dupstr(settings, "audio.sndio.device", &devname) != FLUID_OK || !devname) {
    devname = NULL;
  }

  dev->hdl = sio_open(devname, SIO_PLAY, 0);
  if (dev->hdl == NULL) {
    FLUID_LOG(FLUID_ERR, "sndio could not be opened for writing");
    goto error_recovery;
  }

  sio_initpar(&dev->par);

  dev->par.appbufsz = period_size * periods;
  dev->par.round = period_size;

  dev->par.bits = 16;
  dev->par.le = SIO_LE_NATIVE;
  dev->par.pchan = 2;
  dev->par.rate = sample_rate;

  if (!sio_setpar(dev->hdl, &dev->par)){
    FLUID_LOG(FLUID_ERR, "Can't configure sndio parameters");
    goto error_recovery;
  }

  if (!sio_getpar(dev->hdl, &dev->par)) {
    FLUID_LOG(FLUID_ERR, "Couldn't get sndio audio parameters");
    goto error_recovery;
  } else if (dev->par.pchan != 2 || dev->par.rate != sample_rate ||
      dev->par.bits != 16) {
    FLUID_LOG(FLUID_ERR, "Couldn't set sndio audio parameters as desired");
    goto error_recovery;
  }

  dev->buffer_size = dev->par.round;
  dev->buffer_byte_size = dev->par.round * dev->par.bps * dev->par.pchan;

  /* allocate the buffers. FIXME!!! don't use interleaved samples */
  dev->buffer = FLUID_MALLOC(dev->buffer_byte_size);
  if (dev->buffer == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    goto error_recovery;
  }
  dev->buffers[0] = FLUID_ARRAY(float, dev->buffer_size);
  dev->buffers[1] = FLUID_ARRAY(float, dev->buffer_size);
  if ((dev->buffer == NULL) || (dev->buffers[0] == NULL) || (dev->buffers[1] == NULL)) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    goto error_recovery;
  }

  if (!sio_start(dev->hdl)) {
    FLUID_LOG(FLUID_ERR, "Couldn't start sndio");
    goto error_recovery;
  }

  if (pthread_attr_init(&attr)) {
    FLUID_LOG(FLUID_ERR, "Couldn't initialize audio thread attributes");
    goto error_recovery;
  }

  err = pthread_create(&dev->thread, &attr, fluid_sndio_audio_run2, (void*) dev);
  if (err) {
    FLUID_LOG(FLUID_ERR, "Couldn't create audio2 thread");
    goto error_recovery;
  }

  return (fluid_audio_driver_t*) dev;

error_recovery:
  delete_fluid_sndio_audio_driver((fluid_audio_driver_t*) dev);
  return NULL;
}

/*
 * delete_fluid_sndio_audio_driver
 */
int
delete_fluid_sndio_audio_driver(fluid_audio_driver_t* p)
{
  fluid_sndio_audio_driver_t* dev = (fluid_sndio_audio_driver_t*) p;

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
  if (dev->hdl) {
    sio_close(dev->hdl);
  }
  if (dev->buffer != NULL) {
    FLUID_FREE(dev->buffer);
  }
  FLUID_FREE(dev);
  return FLUID_OK;
}

/*
 * fluid_sndio_audio_run
 */
void*
fluid_sndio_audio_run(void* d)
{
  fluid_sndio_audio_driver_t* dev = (fluid_sndio_audio_driver_t*) d;
  fluid_synth_t* synth = dev->synth;
  void* buffer = dev->buffer;
  int len = dev->buffer_size;

  /* it's as simple as that: */
  while (dev->cont)
  {
    dev->read (synth, len, buffer, 0, 2, buffer, 1, 2);
    sio_write (dev->hdl, buffer, dev->buffer_byte_size);
  }

  FLUID_LOG(FLUID_DBG, "Audio thread finished");

  pthread_exit(NULL);

  return 0; /* not reached */
}


/*
 * fluid_sndio_audio_run
 */
void*
fluid_sndio_audio_run2(void* d)
{
  fluid_sndio_audio_driver_t* dev = (fluid_sndio_audio_driver_t*) d;
  short* buffer = (short*) dev->buffer;
  float* left = dev->buffers[0];
  float* right = dev->buffers[1];
  int buffer_size = dev->buffer_size;
  int dither_index = 0;

  FLUID_LOG(FLUID_DBG, "Audio thread running");

  /* it's as simple as that: */
  while (dev->cont)
  {
    (*dev->callback)(dev->data, buffer_size, 0, NULL, 2, dev->buffers);

    fluid_synth_dither_s16 (&dither_index, buffer_size, left, right,
			    buffer, 0, 2, buffer, 1, 2);

    sio_write (dev->hdl, buffer, dev->buffer_byte_size);
  }

  FLUID_LOG(FLUID_DBG, "Audio thread finished");

  pthread_exit(NULL);

  return 0; /* not reached */
}

void fluid_sndio_midi_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "midi.sndio.device", "default", 0);
}

void
delete_fluid_sndio_midi_driver(fluid_midi_driver_t *addr)
{
  int err;
  fluid_sndio_midi_driver_t *dev = (fluid_sndio_midi_driver_t *)addr;

  if (dev == NULL) {
    return;
  }
  dev->status = FLUID_MIDI_DONE;

  /* cancel the thread and wait for it before cleaning up */
  if (dev->thread) {
    err = pthread_cancel(dev->thread);
    if (err) {
      FLUID_LOG(FLUID_ERR, "Failed to cancel the midi thread");
      return;
    }
    if (pthread_join(dev->thread, NULL)) {
      FLUID_LOG(FLUID_ERR, "Failed to join the midi thread");
      return;
    }
  }
  if (dev->hdl != NULL) {
    mio_close(dev->hdl);
  }
  if (dev->parser != NULL) {
    delete_fluid_midi_parser(dev->parser);
  }
  FLUID_FREE(dev);
}

void *
fluid_sndio_midi_run(void *addr)
{
  int n, i;
  fluid_midi_event_t* evt;
  fluid_sndio_midi_driver_t *dev = (fluid_sndio_midi_driver_t *)addr;
#define MIDI_BUFLEN (3125 / 10)
  unsigned char buffer[MIDI_BUFLEN];

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
    n = mio_read(dev->hdl, buffer, MIDI_BUFLEN);
    if (n == 0 && mio_eof(dev->hdl)) {
      FLUID_LOG(FLUID_ERR, "Failed to read the midi input");
      dev->status = FLUID_MIDI_DONE;
    }

    /* let the parser convert the data into events */
    for (i = 0; i < n; i++) {
      evt = fluid_midi_parser_parse(dev->parser, buffer[i]);
      if (evt != NULL) {
	/* send the event to the next link in the chain */
	(*dev->driver.handler)(dev->driver.data, evt);
      }
    }
  }
  pthread_exit(NULL);
}

int
fluid_sndio_midi_driver_status(fluid_midi_driver_t *addr)
{
  fluid_sndio_midi_driver_t *dev = (fluid_sndio_midi_driver_t *)addr;
  return dev->status;
}


fluid_midi_driver_t *
new_fluid_sndio_midi_driver(fluid_settings_t *settings,
			       handle_midi_event_func_t handler, void *data)
{
  int err;
  fluid_sndio_midi_driver_t *dev;
  char *device;

  /* not much use doing anything */
  if (handler == NULL) {
    FLUID_LOG(FLUID_ERR, "Invalid argument");
    return NULL;
  }

  /* allocate the device */
  dev = FLUID_NEW(fluid_sndio_midi_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_sndio_midi_driver_t));
  dev->hdl = NULL;

  dev->driver.handler = handler;
  dev->driver.data = data;

  /* allocate one event to store the input data */
  dev->parser = new_fluid_midi_parser();
  if (dev->parser == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    goto error_recovery;
  }

  /* get the device name. if none is specified, use the default device. */
  if (fluid_settings_dupstr(settings, "midi.sndio.device", &device) != FLUID_OK || !device) {
	device = NULL;
  }

  /* open the default hardware device. only use midi in. */
  dev->hdl = mio_open(device, MIO_IN, 0);
  if (dev->hdl == NULL) {
    FLUID_LOG(FLUID_ERR, "Couldn't open sndio midi device");
    goto error_recovery;
  }

  dev->status = FLUID_MIDI_READY;

  err = pthread_create(&dev->thread, NULL, fluid_sndio_midi_run, (void *)dev);
  if (err) {
    FLUID_LOG(FLUID_PANIC, "Couldn't create the midi thread.");
    goto error_recovery;
  }
  return (fluid_midi_driver_t *) dev;

 error_recovery:
  delete_fluid_sndio_midi_driver((fluid_midi_driver_t *)dev);
  return NULL;
}

#endif /*#if SNDIO_SUPPORT */
