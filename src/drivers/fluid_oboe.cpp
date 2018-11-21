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

/*
 * It is annoying, but
 * 
 * - we cannot include oboe/Oboe.h outside #if OBOE_SUPPORT,
 * - but OBOE_SUPPORT is defined only within fluid_synth.h,
 * - we cannot include oboe/Oboe.h within C scope,
 * - but we cannot include fluid_*.h outside C scope.
 * 
 * Therefore there are two divided extern decls.
 */

extern "C" {

#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_settings.h"

} // extern "C"

#if OBOE_SUPPORT

#include <sys/time.h>
#include <oboe/Oboe.h>

extern "C" {

using namespace oboe;

#define NUM_CHANNELS 2

DataCallbackResult on_audio_ready(AudioStreamCallback *callback, AudioStream *stream, void *audioData, int32_t numFrames);

class OboeAudioStreamCallback : public AudioStreamCallback
{
public:

  OboeAudioStreamCallback (void *userData)
    : user_data (userData)
  {
  }

  void *user_data;

  DataCallbackResult onAudioReady (AudioStream *oboeStream, void *audioData, int32_t numFrames)
  {
    return on_audio_ready (this, oboeStream, audioData, numFrames);
  }
};

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
  OboeAudioStreamCallback *oboe_callback;
  AudioStream *stream;
} fluid_oboe_audio_driver_t;

static fluid_thread_return_t fluid_oboe_audio_run(void* d);
static fluid_thread_return_t fluid_oboe_audio_run2(void* d);

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
  Result result;
  fluid_oboe_audio_driver_t* dev;
  AudioStreamBuilder builder_obj;
  AudioStreamBuilder *builder = &builder_obj;
  AudioStream *stream;
  
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
  dev->oboe_callback = new OboeAudioStreamCallback (dev);

  fluid_settings_getint(settings, "audio.period-size", &period_frames);
  fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
  is_sample_format_float = fluid_settings_str_equal (settings, "audio.sample-format", "float");
  fluid_settings_getint(settings, "audio.oboe.device-id", &device_id);
  sharing_mode = 
    fluid_settings_str_equal (settings, "audio.oboe.sharing-mode", "Exclusive") ? 1 : 0;
  performance_mode =
    fluid_settings_str_equal (settings, "audio.oboe.performance-mode", "PowerSaving") ? 1 :
    fluid_settings_str_equal (settings, "audio.oboe.performance-mode", "LowLatency") ? 2 : 0;

  builder->setDeviceId (device_id)
	->setDirection (Direction::Output)
	->setChannelCount (NUM_CHANNELS)
	->setSampleRate (sample_rate)
	->setFramesPerCallback (period_frames)
	->setFormat (is_sample_format_float ? AudioFormat::Float : AudioFormat::I16)
	->setSharingMode (sharing_mode == 1 ? SharingMode::Exclusive : SharingMode::Shared)
	->setPerformanceMode (
	  performance_mode == 1 ? PerformanceMode::PowerSaving :
      performance_mode == 2 ? PerformanceMode::LowLatency : PerformanceMode::None)
    ->setUsage (Usage::Media)
    ->setContentType (ContentType::Music)
    ->setCallback (dev->oboe_callback);

  result = builder->openStream (&stream);
  dev->stream = stream;
  if (result != Result::OK)
    goto error_recovery;

  dev->cont = 1;

  FLUID_LOG(FLUID_INFO, "Using Oboe driver");

  stream->start ();
  
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
  
  dev->stream->stop ();
  
  dev->stream->close ();
  
  delete dev->oboe_callback;
  
  FLUID_FREE(dev);
}

DataCallbackResult on_audio_ready(AudioStreamCallback *callback, AudioStream *stream, void *audioData, int32_t numFrames)
{
  float *callback_buffers[2];
  fluid_oboe_audio_driver_t *dev;
  OboeAudioStreamCallback *oboe_callback;
  
  oboe_callback = (OboeAudioStreamCallback*) callback;
  dev = (fluid_oboe_audio_driver_t*) oboe_callback->user_data;
  
  if (!dev->cont)
    return DataCallbackResult::Stop;
  
  if (dev->callback)
  {
    callback_buffers [0] = (float*) audioData;
    callback_buffers [1] = (float*) audioData;
    (*dev->callback)(dev->synth, numFrames, 0, NULL, 2, callback_buffers);
  }
  else
  {
    if (stream->getFormat () == AudioFormat::Float)
    {
      fluid_synth_write_float(dev->synth, numFrames, (float*) audioData, 0, 2, (float*) audioData, 1, 2);
    }
    else
    {
	  fluid_synth_write_s16(dev->synth, numFrames, (short*) audioData, 0, 2, (short*) audioData, 1, 2);
	}
  }
  return DataCallbackResult::Continue;
}

} // extern "C"

#endif // OBOE_SUPPORT

