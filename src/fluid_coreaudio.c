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

/* fluid_coreaudio.c
 *
 * Driver for the Apple's CoreAudio on MacOS X
 *
 */

#include "fluid_synth.h"
#include "fluid_midi.h"
#include "fluid_adriver.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#include "config.h"

#if COREAUDIO_SUPPORT
#include <CoreAudio/AudioHardware.h>
#include <CoreAudio/CoreAudioTypes.h>

/*
 * fluid_core_audio_driver_t
 *
 */
typedef struct {
  fluid_audio_driver_t driver;
  AudioDeviceID id;
  AudioStreamBasicDescription format;
  fluid_audio_func_t callback;
  void* data;
  unsigned int buffer_size;
  float* buffers[2];
  double phase;
} fluid_core_audio_driver_t;

fluid_audio_driver_t* new_fluid_core_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth);

fluid_audio_driver_t* new_fluid_core_audio_driver2(fluid_settings_t* settings, 
						      fluid_audio_func_t func, 
						      void* data);

OSStatus fluid_core_audio_callback(AudioDeviceID dev, 
				   const AudioTimeStamp* now,
				   const AudioBufferList* in, 
				   const AudioTimeStamp* intime,
				   AudioBufferList* out, 
				   const AudioTimeStamp* outtime,
				   void* data);

int delete_fluid_core_audio_driver(fluid_audio_driver_t* p);


/**************************************************************
 *
 *        CoreAudio audio driver
 *
 */

void
fluid_core_audio_driver_settings(fluid_settings_t* settings)
{
/*   fluid_settings_register_str(settings, "audio.coreaudio.device", "default", 0, NULL, NULL); */
}

/*
 * new_fluid_core_audio_driver
 */
fluid_audio_driver_t* 
new_fluid_core_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth)
{
  return new_fluid_core_audio_driver2(settings, 
					  (fluid_audio_func_t) fluid_synth_process, 
					  (void*) synth);
}

/*
 * new_fluid_core_audio_driver2
 */
fluid_audio_driver_t* 
new_fluid_core_audio_driver2(fluid_settings_t* settings, fluid_audio_func_t func, void* data)
{
  fluid_core_audio_driver_t* dev = NULL;
  UInt32 size;
  OSStatus status;

  dev = FLUID_NEW(fluid_core_audio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;    
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_core_audio_driver_t));

  dev->callback = func;
  dev->data = data;

  size = sizeof(AudioDeviceID);
  status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,  &size, (void*) &dev->id);
  if (status != noErr) {
    FLUID_LOG(FLUID_ERR, "Failed to get the default audio device");
    goto error_recovery;
  }

  size = sizeof(UInt32);
  status = AudioDeviceGetProperty(dev->id, 0, false, 
				  kAudioDevicePropertyBufferSize, 
				  &size, &dev->buffer_size);
  if (status != noErr) {
    FLUID_LOG(FLUID_ERR, "Failed to get the default buffer size");
    goto error_recovery;
  }

  size = sizeof(AudioStreamBasicDescription);
  status = AudioDeviceGetProperty(dev->id, 0, false, 
				  kAudioDevicePropertyStreamFormat, 
				  &size, &dev->format);
  if (status != noErr) {
    FLUID_LOG(FLUID_ERR, "Failed to get the default audio format");
    goto error_recovery;
  }

  FLUID_LOG(FLUID_DBG, "sampleRate %g", dev->format.mSampleRate);
  FLUID_LOG(FLUID_DBG, "mFormatFlags %08X", dev->format.mFormatFlags);
  FLUID_LOG(FLUID_DBG, "mBytesPerPacket %d", dev->format.mBytesPerPacket);
  FLUID_LOG(FLUID_DBG, "mFramesPerPacket %d", dev->format.mFramesPerPacket);
  FLUID_LOG(FLUID_DBG, "mChannelsPerFrame %d", dev->format.mChannelsPerFrame);
  FLUID_LOG(FLUID_DBG, "mBytesPerFrame %d", dev->format.mBytesPerFrame);
  FLUID_LOG(FLUID_DBG, "mBitsPerChannel %d", dev->format.mBitsPerChannel);

  if (dev->format.mFormatID != kAudioFormatLinearPCM) {
    FLUID_LOG(FLUID_ERR, "The default audio format is not PCM");
    goto error_recovery;
  }
  if (!(dev->format.mFormatFlags & kLinearPCMFormatFlagIsFloat)) {
    FLUID_LOG(FLUID_ERR, "The default audio format is not float");
    goto error_recovery;
  }

  dev->buffers[0] = FLUID_ARRAY(float, dev->buffer_size);
  dev->buffers[1] = FLUID_ARRAY(float, dev->buffer_size);

  status = AudioDeviceAddIOProc(dev->id, fluid_core_audio_callback, (void*) dev);
  if (status != noErr) {
    FLUID_LOG(FLUID_ERR, "The default audio format is not PCM");
    goto error_recovery;
  }

  status = AudioDeviceStart(dev->id, fluid_core_audio_callback);			
  if (status != noErr) {
    FLUID_LOG(FLUID_ERR, "The default audio format is not PCM");
    goto error_recovery;
  }

  return (fluid_audio_driver_t*) dev;

error_recovery:

  delete_fluid_core_audio_driver((fluid_audio_driver_t*) dev);
  return NULL;  
}

/*
 * delete_fluid_core_audio_driver
 */
int 
delete_fluid_core_audio_driver(fluid_audio_driver_t* p) 
{
  fluid_core_audio_driver_t* dev = (fluid_core_audio_driver_t*) p;  

  if (dev == NULL) {
    return FLUID_OK;
  }

  if (AudioDeviceStop(dev->id, fluid_core_audio_callback) == noErr) {
    AudioDeviceRemoveIOProc(dev->id, fluid_core_audio_callback);	
  }

  if (dev->buffers[0]) {
    FLUID_FREE(dev->buffers[0]);
  }
  if (dev->buffers[1]) {
    FLUID_FREE(dev->buffers[1]);
  }

  FLUID_FREE(dev);

  return FLUID_OK;
}

OSStatus 
fluid_core_audio_callback(AudioDeviceID id, 
			  const AudioTimeStamp* now,
			  const AudioBufferList* in, 
			  const AudioTimeStamp* intime,
			  AudioBufferList* out, 
			  const AudioTimeStamp* outtime,
			  void* data)
{
  int i, k;
  fluid_core_audio_driver_t* dev = (fluid_core_audio_driver_t*) data;
/*   int len = out->mBuffers[0].mDataByteSize / dev->format.mBytesPerFrame; */
  int len = dev->buffer_size / dev->format.mBytesPerFrame;
  float* buffer = out->mBuffers[0].mData;
  float* left = dev->buffers[0];
  float* right = dev->buffers[1];

#if 0
  double incr = 220.0 * 2. * 3.14159265359 / dev->format.mSampleRate;

  for (i = 0, k = 0; i < len; i++) {
    float s = 0.2 * sin(dev->phase);
    dev->phase += incr;
    buffer[k++] = s;
    buffer[k++] = s;
  }

#else

#if 0
  (*dev->callback)(dev->data, len, 0, NULL, 2, dev->buffers);
#else
  fluid_synth_write_float((fluid_synth_t*) dev->data, len, dev->buffers[0], 0, 1, dev->buffers[1], 0, 1);
#endif
  
  for (i = 0, k = 0; i < len; i++) {
    buffer[k++] = left[i];
    buffer[k++] = right[i];
  }
#endif

  return noErr;
}


#endif /* #if COREAUDIO_SUPPORT */
