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
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

/* fluid_opensles.c
 *
 * Audio driver for OpenSLES.
 *
 */

#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_settings.h"

#if OPENSLES_SUPPORT

#include <sys/time.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define NUM_CHANNELS 2

/** fluid_opensles_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct {
  fluid_audio_driver_t driver;
  SLObjectItf engine;
  SLObjectItf output_mix_object;
  SLObjectItf audio_player;
  SLPlayItf audio_player_interface;
  SLAndroidSimpleBufferQueueItf player_buffer_queue_interface;
  
  void* synth;
  int period_frames;

  int is_sample_format_float;
  int use_callback_mode;

  /* used only by callback mode */
  short* sles_buffer_short;
  float* sles_buffer_float;
  fluid_audio_func_t callback;
  /* used only by non-callback mode */
  fluid_thread_t *thread;

  int cont;
  long next_expected_enqueue_time;
  
  double sample_rate;
} fluid_opensles_audio_driver_t;


fluid_audio_driver_t* new_fluid_opensles_audio_driver(fluid_settings_t* settings,
						   fluid_synth_t* synth);
fluid_audio_driver_t* new_fluid_opensles_audio_driver2(fluid_settings_t* settings,
						    fluid_audio_func_t func, void* data);
void delete_fluid_opensles_audio_driver(fluid_audio_driver_t* p);
void fluid_opensles_audio_driver_settings(fluid_settings_t* settings);
static fluid_thread_return_t fluid_opensles_audio_run(void* d);
static void opensles_callback(SLAndroidSimpleBufferQueueItf caller, void *pContext);
void adjust_latency(fluid_opensles_audio_driver_t* dev);
void process_fluid_buffer(fluid_opensles_audio_driver_t* dev);

void fluid_opensles_audio_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_int(settings, "audio.opensles.use-callback-mode", 1, 0, 1,
                              FLUID_HINT_TOGGLED);
}


/*
 * new_fluid_opensles_audio_driver
 */
fluid_audio_driver_t*
new_fluid_opensles_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth)
{
  return new_fluid_opensles_audio_driver2 (settings,
                                        (fluid_audio_func_t) fluid_synth_process,
                                        (void*) synth);
}

/*
 * new_fluid_opensles_audio_driver2
 */
fluid_audio_driver_t*
new_fluid_opensles_audio_driver2(fluid_settings_t* settings, fluid_audio_func_t func, void* data)
{
  SLresult result;
  fluid_opensles_audio_driver_t* dev;
  double sample_rate;
  int period_size;
  int realtime_prio = 0;
  int is_sample_format_float;
  int use_callback_mode = 0;
  SLEngineItf engine_interface;

  fluid_synth_t* synth = (fluid_synth_t*) data;

  dev = FLUID_NEW(fluid_opensles_audio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  FLUID_MEMSET(dev, 0, sizeof(fluid_opensles_audio_driver_t));

  fluid_settings_getint(settings, "audio.period-size", &period_size);
  fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
  fluid_settings_getint(settings, "audio.realtime-prio", &realtime_prio);
  is_sample_format_float = fluid_settings_str_equal (settings, "audio.sample-format", "float");
  fluid_settings_getint(settings, "audio.opensles.use-callback-mode", &use_callback_mode);

  dev->synth = synth;
  dev->use_callback_mode = use_callback_mode;
  dev->is_sample_format_float = is_sample_format_float;
  dev->period_frames = period_size;
  dev->sample_rate = sample_rate;
  dev->cont = 1;

  result = slCreateEngine (&(dev->engine), 0, NULL, 0, NULL, NULL);

  if (!dev->engine)
  {
    FLUID_LOG(FLUID_ERR, "Failed to create OpenSLES connection");
    goto error_recovery;
  }
  result = (*dev->engine)->Realize (dev->engine, SL_BOOLEAN_FALSE);
  if (result != 0) goto error_recovery;
  
  result = (*dev->engine)->GetInterface (dev->engine, SL_IID_ENGINE, &engine_interface);
  if (result != 0) goto error_recovery;

  result = (*engine_interface)->CreateOutputMix (engine_interface, &dev->output_mix_object, 0, 0, 0);
  if (result != 0) goto error_recovery;
  
  result = (*dev->output_mix_object)->Realize (dev->output_mix_object, SL_BOOLEAN_FALSE);
  if (result != 0) goto error_recovery;

  SLDataLocator_AndroidSimpleBufferQueue loc_buffer_queue = {
    SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
    2 /* number of buffers */
    };
  SLDataFormat_PCM format_pcm = {
    SL_DATAFORMAT_PCM,
    NUM_CHANNELS,
    ((SLuint32) sample_rate) * 1000,
    SL_PCMSAMPLEFORMAT_FIXED_16,
    SL_PCMSAMPLEFORMAT_FIXED_16,
    SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
    SL_BYTEORDER_LITTLEENDIAN
    };
  SLDataSource audio_src = {
    &loc_buffer_queue,
    &format_pcm
    };

  SLDataLocator_OutputMix loc_outmix = {
    SL_DATALOCATOR_OUTPUTMIX,
    dev->output_mix_object
    };
  SLDataSink audio_sink = {&loc_outmix, NULL};

  const SLInterfaceID ids1[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
  const SLboolean req1[] = {SL_BOOLEAN_TRUE};
  result = (*engine_interface)->CreateAudioPlayer (engine_interface,
    &(dev->audio_player), &audio_src, &audio_sink, 1, ids1, req1);
  if (result != 0) goto error_recovery;

  result = (*dev->audio_player)->Realize (dev->audio_player,SL_BOOLEAN_FALSE);
  if (result != 0) goto error_recovery;

  result = (*dev->audio_player)->GetInterface (dev->audio_player, 
    SL_IID_PLAY, &(dev->audio_player_interface));
  if (result != 0) goto error_recovery;

  result = (*dev->audio_player)->GetInterface(dev->audio_player,
    SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &(dev->player_buffer_queue_interface));
  if (result != 0) goto error_recovery;

  if (dev->is_sample_format_float)
    dev->sles_buffer_float = FLUID_ARRAY(float, dev->period_frames * NUM_CHANNELS);
  else
    dev->sles_buffer_short = FLUID_ARRAY(short, dev->period_frames * NUM_CHANNELS);
  if (dev->sles_buffer_float == NULL && dev->sles_buffer_short == NULL)
  {
    FLUID_LOG(FLUID_ERR, "Out of memory.");
    goto error_recovery;
  }

  if (dev->use_callback_mode) {

    result = (*dev->player_buffer_queue_interface)->RegisterCallback(dev->player_buffer_queue_interface, opensles_callback, dev);
    if (result != 0) goto error_recovery;

    if (dev->is_sample_format_float)
      (*dev->player_buffer_queue_interface)->Enqueue(dev->player_buffer_queue_interface, dev->sles_buffer_float, dev->period_frames * NUM_CHANNELS * sizeof(float));
    else
      (*dev->player_buffer_queue_interface)->Enqueue(dev->player_buffer_queue_interface, dev->sles_buffer_short, dev->period_frames * NUM_CHANNELS * sizeof(short));

    (*dev->audio_player_interface)->SetCallbackEventsMask(dev->audio_player_interface, SL_PLAYEVENT_HEADATEND);
    result = (*dev->audio_player_interface)->SetPlayState(dev->audio_player_interface, SL_PLAYSTATE_PLAYING);
    if (result != 0) goto error_recovery;

  } else { /* non-callback mode */

    result = (*dev->audio_player_interface)->SetPlayState(dev->audio_player_interface, SL_PLAYSTATE_PLAYING);
    if (result != 0) goto error_recovery;

    /* Create the audio thread */
    dev->thread = new_fluid_thread ("opensles-audio", fluid_opensles_audio_run,
                                    dev, realtime_prio, FALSE);
    if (!dev->thread)
      goto error_recovery;
  }

  FLUID_LOG(FLUID_INFO, "Using OpenSLES driver.");

  return (fluid_audio_driver_t*) dev;

  error_recovery:
  
  delete_fluid_opensles_audio_driver((fluid_audio_driver_t*) dev);
  return NULL;
}

void delete_fluid_opensles_audio_driver(fluid_audio_driver_t* p)
{
  fluid_opensles_audio_driver_t* dev = (fluid_opensles_audio_driver_t*) p;

  if (dev == NULL) {
    return;
  }

  dev->cont = 0;

  if (!dev->use_callback_mode)
  {
    if (dev->thread)
      fluid_thread_join (dev->thread);
  }

  if (dev->audio_player)
    (*dev->audio_player)->Destroy (dev->audio_player);
  if (dev->output_mix_object)
    (*dev->output_mix_object)->Destroy (dev->output_mix_object);
  if (dev->engine)
    (*dev->engine)->Destroy (dev->engine);
  
  if (dev->is_sample_format_float)
  {
    if (dev->sles_buffer_float)
      FLUID_FREE(dev->sles_buffer_float);
  }
  else
  {
    if (dev->sles_buffer_short)
      FLUID_FREE(dev->sles_buffer_short);
  }
 
  FLUID_FREE(dev);
}

/* FIXME: this causes crash on x86 etc. It should be revised anyways. */
void adjust_latency(fluid_opensles_audio_driver_t* dev)
{
  struct timespec ts;
  long current_time, wait_in_theory, time_delta;

  wait_in_theory = 1000000 * dev->period_frames / dev->sample_rate;

  /* compute delta time and update 'next expected enqueue' time */
  clock_gettime(CLOCK_REALTIME, &ts);
  current_time = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
  time_delta = dev->next_expected_enqueue_time == 0 ? 0 : dev->next_expected_enqueue_time - current_time;
  if (time_delta == 0)
    dev->next_expected_enqueue_time += current_time + wait_in_theory;
  else
    dev->next_expected_enqueue_time += wait_in_theory;
  /* take some sleep only if it's running ahead */
  if (time_delta > 0)
    usleep (time_delta);
}

void opensles_callback(SLAndroidSimpleBufferQueueItf caller, void *pContext)
{
  fluid_opensles_audio_driver_t* dev = (fluid_opensles_audio_driver_t*) pContext;
  int err;
  SLresult result;

  process_fluid_buffer(dev);

  if (dev->is_sample_format_float)
    result = (*caller)->Enqueue (
    dev->player_buffer_queue_interface, dev->sles_buffer_float, dev->period_frames * sizeof (float) * NUM_CHANNELS);
  else
    result = (*caller)->Enqueue (
    dev->player_buffer_queue_interface, dev->sles_buffer_short, dev->period_frames * sizeof (short) * NUM_CHANNELS);
  
  if (result != 0) {
    err = result;
    /* Do not simply break at just one single insufficient buffer. Go on. */
  }
}

static fluid_thread_return_t
fluid_opensles_audio_run(void* d)
{
  fluid_opensles_audio_driver_t* dev = (fluid_opensles_audio_driver_t*) d;
  short *out_short = dev->sles_buffer_short;
  float *out_float = dev->sles_buffer_float;
  int period_frames;
  int err;
  SLresult result;

  period_frames = dev->period_frames;

  while (dev->cont)
  {
    adjust_latency (dev);

    process_fluid_buffer(dev);

    if (dev->is_sample_format_float)
      result = (*dev->player_buffer_queue_interface)->Enqueue (
      dev->player_buffer_queue_interface, out_float, period_frames * sizeof (float) * NUM_CHANNELS);
    else
      result = (*dev->player_buffer_queue_interface)->Enqueue (
      dev->player_buffer_queue_interface, out_short, period_frames * sizeof (short) * NUM_CHANNELS);
    if (result != 0) {
      err = result;
      /* Do not simply break at just one single insufficient buffer. Go on. */
    }
  }	/* while (dev->cont) */

  return NULL;
}

void process_fluid_buffer(fluid_opensles_audio_driver_t* dev)
{
  short *out_short = dev->sles_buffer_short;
  float *out_float = dev->sles_buffer_float;
  int period_frames = dev->period_frames;
  float *float_callback_buffers[2];
  short *short_callback_buffers[2];
  
  if (dev->is_sample_format_float)
  {
    if (dev->callback)
    {
      float_callback_buffers [0] = out_float;
      float_callback_buffers [1] = out_float;

      (*dev->callback)(dev->synth, period_frames, 0, NULL, 2, float_callback_buffers);
    }
    else
      fluid_synth_write_float(dev->synth, period_frames, out_float, 0, 2, out_float, 1, 2);
  }
  else
  {
    if (dev->callback)
    {
      short_callback_buffers [0] = out_short;
      short_callback_buffers [1] = out_short;

      (*dev->callback)(dev->synth, period_frames, 0, NULL, 2, (float**) short_callback_buffers);
	}
    else
      fluid_synth_write_s16(dev->synth, period_frames, out_short, 0, 2, out_short, 1, 2);
  }
}

#endif
