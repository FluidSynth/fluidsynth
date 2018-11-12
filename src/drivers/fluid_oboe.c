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

/* fluid_oboe.c
 *
 * Audio driver for Android Oboe.
 *
 */

#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_settings.h"

#if OBOE_SUPPORT

#include <sys/time.h>
#include <oboe-c.h>

#define NUM_CHANNELS 2

/** fluid_oboe_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct {
  fluid_audio_driver_t driver;
  fluid_synth_t *synth;
  int32_t cont;
  fluid_audio_func_t callback;
  oboe_audio_stream_callback_ptr_t oboe_callback;
  oboe_audio_stream_ptr_t stream;
} fluid_oboe_audio_driver_t;


fluid_audio_driver_t* new_fluid_oboe_audio_driver(fluid_settings_t* settings,
						   fluid_synth_t* synth);
fluid_audio_driver_t* new_fluid_oboe_audio_driver2(fluid_settings_t* settings,
						    fluid_audio_func_t func, void* data);
void delete_fluid_oboe_audio_driver(fluid_audio_driver_t* p);
void fluid_oboe_audio_driver_settings(fluid_settings_t* settings);
static fluid_thread_return_t fluid_oboe_audio_run(void* d);
static fluid_thread_return_t fluid_oboe_audio_run2(void* d);
enum OboeDataCallbackResult on_audio_ready(oboe_audio_stream_callback_ptr_t callback, oboe_audio_stream_ptr_t stream, void *audioData, int32_t numFrames);

void fluid_oboe_audio_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "audio.oboe.sharing-mode", "Shared", 0);
  fluid_settings_register_str(settings, "audio.oboe.sharing-mode", "Exclusive", 1);
  fluid_settings_add_option(settings, "audio.oboe.sharing-mode", "Shared");
  fluid_settings_register_str(settings, "audio.oboe.performance-mode", "None", 0);
  fluid_settings_register_str(settings, "audio.oboe.performance-mode", "PowerSaving", 1);
  fluid_settings_register_str(settings, "audio.oboe.performance-mode", "LowLatency", 2);
  fluid_settings_add_option(settings, "audio.oboe.performance-mode", "None");
}


/*
 * new_fluid_oboe_audio_driver
 */
fluid_audio_driver_t*
new_fluid_oboe_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth)
{
  return new_fluid_oboe_audio_driver2 (settings,
                                        (fluid_audio_func_t) fluid_synth_process,
                                        (void*) synth);
}

/*
 * new_fluid_oboe_audio_driver2
 */
fluid_audio_driver_t*
new_fluid_oboe_audio_driver2(fluid_settings_t* settings, fluid_audio_func_t func, void* data)
{
  int32_t result;
  fluid_oboe_audio_driver_t* dev;
  oboe_audio_stream_builder_ptr_t builder;
  oboe_audio_stream_ptr_t stream;
  
  int period_frames;
  double sample_rate;
  int is_sample_format_float;
  int device_id;
  int sharing_mode; // 0: Shared, 1: Exclusive
  int performance_mode; // 0: None, 1: PowerSaving, 2: LowLatency

  fluid_synth_t* synth = (fluid_synth_t*) data;

  dev = FLUID_NEW(fluid_oboe_audio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  FLUID_MEMSET(dev, 0, sizeof(fluid_oboe_audio_driver_t));
  
  dev->synth = synth;
  dev->callback = func;
  dev->oboe_callback = oboe_audio_stream_callback_create ();
  oboe_audio_stream_callback_set_on_audio_ready (dev->oboe_callback, &on_audio_ready);
  oboe_audio_stream_callback_set_user_data (dev->oboe_callback, dev);

  fluid_settings_getint(settings, "audio.period-size", &period_frames);
  fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
  is_sample_format_float = fluid_settings_str_equal (settings, "audio.sample-format", "float");
  fluid_settings_getint(settings, "audio.oboe.device-id", &device_id);
  sharing_mode = 
    fluid_settings_str_equal (settings, "audio.oboe.sharing-mode", "Exclusive") ? 1 : 0;
  performance_mode =
    fluid_settings_str_equal (settings, "audio.oboe.performance-mode", "PowerSaving") ? 1 :
    fluid_settings_str_equal (settings, "audio.oboe.performance-mode", "LowLatency") ? 2 : 0;

  oboe_audio_stream_builder_set_device_id (builder, device_id);
  oboe_audio_stream_builder_set_direction (builder, DIRECTION_OUTPUT);
  oboe_audio_stream_builder_set_channel_count (builder, 2);
  oboe_audio_stream_builder_set_sample_rate (builder, sample_rate);
  oboe_audio_stream_builder_set_frames_per_callback (builder, period_frames);
  oboe_audio_stream_builder_set_format (builder,
      is_sample_format_float ? AUDIO_FORMAT_FLOAT : AUDIO_FORMAT_I16);
  oboe_audio_stream_builder_set_sharing_mode (builder,
      sharing_mode == 1 ? SHARING_MODE_EXCLUSIVE : SHARING_MODE_SHARED);
  oboe_audio_stream_builder_set_performance_mode (builder,
      performance_mode == 1 ? PERFORMANCE_MODE_POWER_SAVING :
      performance_mode == 2 ? PERFORMANCE_MODE_LOW_LATENCY : PERFORMANCE_MODE_NONE);
  oboe_audio_stream_builder_set_usage (builder, USAGE_MEDIA);
  oboe_audio_stream_builder_set_content_type (builder, CONTENT_TYPE_MUSIC);
  oboe_audio_stream_builder_set_callback (builder, dev->oboe_callback);

  result = oboe_audio_stream_builder_open_stream (builder, &stream);
  dev->stream = stream;
  if (result != RESULT_OK)
    goto error_recovery;

  dev->cont = 1;

  FLUID_LOG(FLUID_INFO, "Using Oboe driver");

  oboe_audio_stream_start (stream);
  
  return (fluid_audio_driver_t*) dev;

 error_recovery:
  delete_fluid_oboe_audio_driver((fluid_audio_driver_t*) dev);
  return NULL;
}

void delete_fluid_oboe_audio_driver(fluid_audio_driver_t* p)
{
  fluid_oboe_audio_driver_t* dev = (fluid_oboe_audio_driver_t*) p;

  if (dev == NULL) {
    return;
  }

  dev->cont = 0;
  
  oboe_audio_stream_stop (dev->stream);
  
  oboe_audio_stream_close (dev->stream);
  
  oboe_audio_stream_callback_free (dev->oboe_callback);
  
  FLUID_FREE(dev);
}

enum OboeDataCallbackResult on_audio_ready(oboe_audio_stream_callback_ptr_t callback, oboe_audio_stream_ptr_t stream, void *audioData, int32_t numFrames)
{
  float *callback_buffers[2];
  fluid_oboe_audio_driver_t *dev;
    
  dev = (fluid_oboe_audio_driver_t*) oboe_audio_stream_callback_get_user_data (callback);
  
  if (!dev->cont)
    return CALLBACK_RESULT_STOP;
  
  if (dev->callback)
  {
    callback_buffers [0] = (float*) audioData;
    callback_buffers [1] = (float*) audioData;
    (*dev->callback)(dev->synth, numFrames, 0, NULL, 2, callback_buffers);
  }
  else
  {
    if (oboe_audio_stream_base_get_format (stream) == AUDIO_FORMAT_FLOAT)
    {
      fluid_synth_write_float(dev->synth, numFrames, (float*) audioData, 0, 2, (float*) audioData, 1, 2);
    }
    else
    {
	  fluid_synth_write_s16(dev->synth, numFrames, (short*) audioData, 0, 2, (short*) audioData, 1, 2);
	}
  }
  return CALLBACK_RESULT_CONTINUE;
}

#endif
