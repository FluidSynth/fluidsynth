// Referent oboe: 40aa2e7d

//#ifndef OBOE_C_H_INCLUDED
//#define OBOE_C_H_INCLUDED

#include <oboe/Oboe.h>

using namespace oboe;

#ifdef __cplusplus
extern "C" {
#endif

#include "oboe-c.h"

// Utility converters

OboeResultWithValueInt32 ToResultWithValueInt32 (oboe::ResultWithValue<int32_t> value)
{
	OboeResultWithValueInt32 ret;
	ret.value = value.value ();
	ret.error = (OboeResult) value.error ();
	return ret;
}

OboeResultWithValueDouble ToResultWithValueDouble (oboe::ResultWithValue<double> value)
{
	OboeResultWithValueDouble ret;
	ret.value = value.value ();
	ret.error = (OboeResult) value.error ();
	return ret;
}


// forward decls.

class AudioStreamCallbackImplementor;
typedef void* oboe_result_with_value_ptr_t;
typedef void* oboe_latency_tuner_ptr_t;
typedef void* oboe_audio_stream_ptr_t;
typedef void* oboe_audio_stream_base_ptr_t;
typedef void* oboe_audio_stream_builder_ptr_t;
typedef void* oboe_audio_stream_callback_ptr_t;


// ResultWithValue (is annoying; maybe I just remove them all)

/*

oboe_result_with_value_ptr_t oboe_result_with_value_of_int32_create (int32_t value, OboeResult result)
{
	return result == OboeResult::OK ? new oboe::ResultWithValue<int32_t> (value) : new oboe::ResultWithValue<int32_t> ((oboe::Result) result);
}

oboe_result_with_value_ptr_t oboe_result_with_value_of_double_create (double value, OboeResult result)
{
	return result == OboeResult::OK ? new oboe::ResultWithValue<double> (value) : new oboe::ResultWithValue<double> ((oboe::Result) result);
}

void oboe_result_with_value_of_int32_free (oboe_result_with_value_ptr_t instance)
{
	delete (oboe::ResultWithValue<int32_t>*) instance;
}

void oboe_result_with_value_of_double_free (oboe_result_with_value_ptr_t instance)
{
	delete (oboe::ResultWithValue<double>*) instance;
}

int32_t oboe_result_with_value_of_int32_value (oboe_result_with_value_ptr_t instance)
{
	return ((oboe::ResultWithValue<int32_t>*) instance)->value();
}

double oboe_result_with_value_of_double_value (oboe_result_with_value_ptr_t instance)
{
	return ((oboe::ResultWithValue<double>*) instance)->value();
}

*/

// LatencyTuner

oboe_latency_tuner_ptr_t oboe_latency_tuner_create (oboe_audio_stream_ptr_t stream)
{
	return new oboe::LatencyTuner ((oboe::AudioStream&) stream);
} 

void oboe_latency_tuner_free (oboe_latency_tuner_ptr_t instance)
{
	delete (oboe::LatencyTuner*) instance;
}

enum OboeResult oboe_latency_tuner_tune (oboe_latency_tuner_ptr_t instance)
{
	return (OboeResult) ((oboe::LatencyTuner*) instance)->tune ();
}

void oboe_latency_tuner_request_reset (oboe_latency_tuner_ptr_t instance)
{
	((oboe::LatencyTuner*) instance)->requestReset ();
}

// AudioStream

OboeResult oboe_audio_stream_open (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->open ();
}

OboeResult oboe_audio_stream_close (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->close ();
}

OboeResult oboe_audio_stream_start (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->start ();
}

OboeResult oboe_audio_stream_pause (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->pause ();
}

OboeResult oboe_audio_stream_flush (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->flush ();
}

OboeResult oboe_audio_stream_stop (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->stop ();
}

OboeResult oboe_audio_stream_request_start (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->requestStart ();
}

OboeResult oboe_audio_stream_request_pause (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->requestPause ();
}

OboeResult oboe_audio_stream_request_flush (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->requestFlush ();
}

OboeResult oboe_audio_stream_request_stop (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->requestStop ();
}

OboeStreamState oboe_audio_stream_get_state (oboe_audio_stream_ptr_t instance)
{
	return (OboeStreamState) ((oboe::AudioStream*) instance)->getState ();
}

OboeResult oboe_audio_stream_wait_for_state_change (oboe_audio_stream_ptr_t instance, OboeStreamState inputState, OboeStreamState* nextState, int64_t timeoutNanoseconds)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->waitForStateChange ((oboe::StreamState) inputState, (oboe::StreamState*) (void*) nextState, timeoutNanoseconds);
}

OboeResultWithValueInt32 oboe_audio_stream_set_buffer_size_in_frames (oboe_audio_stream_ptr_t instance, int32_t requestedFrames)
{
	return ToResultWithValueInt32 (((oboe::AudioStream*) instance)->setBufferSizeInFrames (requestedFrames));
}

OboeResultWithValueInt32 oboe_audio_stream_get_x_run_count (oboe_audio_stream_ptr_t instance)
{
	return ToResultWithValueInt32 (((oboe::AudioStream*) instance)->getXRunCount ());
}

int32_t oboe_audio_stream_get_frames_per_burst (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->getFramesPerBurst ();
}

int32_t oboe_audio_stream_is_playing (oboe_audio_stream_ptr_t instance)
{
	return ((oboe::AudioStream*) instance)->isPlaying ();
}

int32_t oboe_audio_stream_get_bytes_per_frame (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->getBytesPerFrame ();
}

int32_t oboe_audio_stream_get_bytes_per_sample (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->getBytesPerSample ();
}

int64_t oboe_audio_stream_get_frames_written (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->getFramesWritten ();
}

int64_t oboe_audio_stream_get_frames_read (oboe_audio_stream_ptr_t instance)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->getFramesRead ();
}

OboeResultWithValueDouble oboe_audio_stream_calculate_latency_millis (oboe_audio_stream_ptr_t instance)
{
	return ToResultWithValueDouble (((oboe::AudioStream*) instance)->calculateLatencyMillis ());
}

OboeResult oboe_audio_stream_get_timestamp (oboe_audio_stream_ptr_t instance, clockid_t clockId, int64_t *framePosition, int64_t *timeNanoseconds)
{
	return (OboeResult) ((oboe::AudioStream*) instance)->getTimestamp (clockId, framePosition, timeNanoseconds);
}

OboeResultWithValueInt32 oboe_audio_stream_write (oboe_audio_stream_ptr_t instance, const void *buffer, int32_t numFrames, int64_t timeoutNanoseconds)
{
	return ToResultWithValueInt32 (((oboe::AudioStream*) instance)->write (buffer, numFrames, timeoutNanoseconds));
}

OboeResultWithValueInt32 oboe_audio_stream_read (oboe_audio_stream_ptr_t instance, void *buffer, int32_t numFrames, int64_t timeoutNanoseconds)
{
	return ToResultWithValueInt32 (((oboe::AudioStream*) instance)->read (buffer, numFrames, timeoutNanoseconds));
}

OboeAudioApi oboe_audio_stream_get_audio_api (oboe_audio_stream_ptr_t instance)
{
	return (OboeAudioApi) ((oboe::AudioStream*) instance)->getAudioApi ();
}

int32_t oboe_audio_stream_uses_aaudio (oboe_audio_stream_ptr_t instance)
{
	return ((oboe::AudioStream*) instance)->usesAAudio ();
}

// "Do not use this for production. This is only for debugging."
//
// void *oboe_audio_stream_get_underlying_stream (oboe_audio_stream_ptr_t instance)
// {
//	 return ((oboe::AudioStream*) instance)->getUnderlyingStream ();
// }

// AudioStreamBase

int oboe_audio_stream_base_get_channel_count (oboe_audio_stream_base_ptr_t instance)
{
	return ((oboe::AudioStreamBase*) instance)->getChannelCount ();
}

OboeDirection oboe_audio_stream_base_get_direction (oboe_audio_stream_base_ptr_t instance)
{
	return (OboeDirection) ((oboe::AudioStreamBase*) instance)->getDirection ();
}

int32_t oboe_audio_stream_base_get_sample_rate (oboe_audio_stream_base_ptr_t instance)
{
	return ((oboe::AudioStreamBase*) instance)->getSampleRate ();
}

int oboe_audio_stream_base_get_frames_per_callback (oboe_audio_stream_base_ptr_t instance)
{
	return ((oboe::AudioStreamBase*) instance)->getFramesPerCallback ();
}

OboeAudioFormat oboe_audio_stream_base_get_format (oboe_audio_stream_base_ptr_t instance)
{
	return (OboeAudioFormat) ((oboe::AudioStreamBase*) instance)->getFormat ();
}

int32_t oboe_audio_stream_base_get_buffer_size_in_frames (oboe_audio_stream_base_ptr_t instance)
{
	return ((oboe::AudioStreamBase*) instance)->getBufferSizeInFrames ();
}

int32_t oboe_audio_stream_base_get_buffer_capacity_in_frames (oboe_audio_stream_base_ptr_t instance)
{
	return ((oboe::AudioStreamBase*) instance)->getBufferCapacityInFrames ();
}

OboeSharingMode oboe_audio_stream_base_get_sharing_mode (oboe_audio_stream_base_ptr_t instance)
{
	return (OboeSharingMode) ((oboe::AudioStreamBase*) instance)->getSharingMode ();
}

OboePerformanceMode oboe_audio_stream_base_get_performance_mode (oboe_audio_stream_base_ptr_t instance)
{
	return (OboePerformanceMode) ((oboe::AudioStreamBase*) instance)->getPerformanceMode ();
}

int32_t oboe_audio_stream_base_get_device_id (oboe_audio_stream_base_ptr_t instance)
{
	return ((oboe::AudioStreamBase*) instance)->getDeviceId ();
}

oboe_audio_stream_callback_ptr_t oboe_audio_stream_base_get_callback (oboe_audio_stream_base_ptr_t instance)
{
	return ((oboe::AudioStreamBase*) instance)->getCallback ();
}

OboeUsage oboe_audio_stream_base_get_usage (oboe_audio_stream_base_ptr_t instance)
{
	return (OboeUsage) ((oboe::AudioStreamBase*) instance)->getUsage ();
}

OboeContentType oboe_audio_stream_base_get_content_type (oboe_audio_stream_base_ptr_t instance)
{
	return (OboeContentType) ((oboe::AudioStreamBase*) instance)->getContentType ();
}

OboeInputPreset oboe_audio_stream_base_get_input_preset (oboe_audio_stream_base_ptr_t instance)
{
	return (OboeInputPreset) ((oboe::AudioStreamBase*) instance)->getInputPreset ();
}

OboeSessionId oboe_audio_stream_base_get_session_id (oboe_audio_stream_base_ptr_t instance)
{
	return (OboeSessionId) ((oboe::AudioStreamBase*) instance)->getSessionId ();
}

// AudioStreamBuilder


oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_create ()
{
	return new oboe::AudioStreamBuilder ();
}

void oboe_audio_stream_builder_delete (oboe_audio_stream_builder_ptr_t instance)
{
	delete (oboe::AudioStreamBuilder*) instance;
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_channel_count (oboe_audio_stream_builder_ptr_t instance, int channelCount)
{
	return ((oboe::AudioStreamBuilder*) instance)->setChannelCount (channelCount);
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_direction (oboe_audio_stream_builder_ptr_t instance, OboeDirection direction)
{
	return ((oboe::AudioStreamBuilder*) instance)->setDirection ((oboe::Direction) direction);
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_sample_rate (oboe_audio_stream_builder_ptr_t instance, int32_t sampleRate)
{
	return ((oboe::AudioStreamBuilder*) instance)->setSampleRate (sampleRate);
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_frames_per_callback (oboe_audio_stream_builder_ptr_t instance, int framesPerCallback)
{
	return ((oboe::AudioStreamBuilder*) instance)->setFramesPerCallback (framesPerCallback);
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_format (oboe_audio_stream_builder_ptr_t instance, OboeAudioFormat format)
{
	return ((oboe::AudioStreamBuilder*) instance)->setFormat ((oboe::AudioFormat) format);
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_buffer_capacity_in_frames (oboe_audio_stream_builder_ptr_t instance, int bufferCapacityInFrames)
{
	return ((oboe::AudioStreamBuilder*) instance)->setBufferCapacityInFrames (bufferCapacityInFrames);
}

OboeAudioApi oboe_audio_stream_builder_get_audio_api (oboe_audio_stream_builder_ptr_t instance)
{
	return (OboeAudioApi) ((oboe::AudioStreamBuilder*) instance)->getAudioApi ();
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_audio_api (oboe_audio_stream_builder_ptr_t instance, OboeAudioApi audioApi)
{
	return ((oboe::AudioStreamBuilder*) instance)->setAudioApi ((oboe::AudioApi) audioApi);
}

int32_t oboe_audio_stream_builder_is_aaudio_supported (oboe_audio_stream_builder_ptr_t instance)
{
	return ((oboe::AudioStreamBuilder*) instance)->isAAudioSupported ();
}

int32_t oboe_audio_stream_builder_is_aaudio_recommended (oboe_audio_stream_builder_ptr_t instance)
{
	return ((oboe::AudioStreamBuilder*) instance)->isAAudioRecommended ();
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_sharing_mode (oboe_audio_stream_builder_ptr_t instance, OboeSharingMode sharingMode)
{
	return ((oboe::AudioStreamBuilder*) instance)->setSharingMode ((oboe::SharingMode) sharingMode);
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_performance_mode (oboe_audio_stream_builder_ptr_t instance, OboePerformanceMode performanceMode)
{
	return ((oboe::AudioStreamBuilder*) instance)->setPerformanceMode ((oboe::PerformanceMode) performanceMode);
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_usage (oboe_audio_stream_builder_ptr_t instance, OboeUsage usage)
{
	return ((oboe::AudioStreamBuilder*) instance)->setUsage ((oboe::Usage) usage);
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_content_type (oboe_audio_stream_builder_ptr_t instance, OboeContentType contentType)
{
	return ((oboe::AudioStreamBuilder*) instance)->setContentType ((oboe::ContentType) contentType);
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_input_preset (oboe_audio_stream_builder_ptr_t instance, OboeInputPreset inputPreset)
{
	return ((oboe::AudioStreamBuilder*) instance)->setInputPreset ((oboe::InputPreset) inputPreset);
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_session_id (oboe_audio_stream_builder_ptr_t instance, OboeSessionId sessionId)
{
	return ((oboe::AudioStreamBuilder*) instance)->setSessionId ((oboe::SessionId) sessionId);
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_device_id (oboe_audio_stream_builder_ptr_t instance, int32_t deviceId)
{
	return ((oboe::AudioStreamBuilder*) instance)->setDeviceId (deviceId);
}

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_callback (oboe_audio_stream_builder_ptr_t instance, oboe_audio_stream_callback_ptr_t streamCallback)
{
	return ((oboe::AudioStreamBuilder*) instance)->setCallback ((AudioStreamCallback*) streamCallback);
}

OboeResult oboe_audio_stream_builder_open_stream (oboe_audio_stream_builder_ptr_t instance, oboe_audio_stream_ptr_t *stream)
{
	oboe::AudioStream *ptr;
	Result ret = ((oboe::AudioStreamBuilder*) instance)->openStream (&ptr);
	*stream = ptr;
	return (OboeResult) ret;
}

// AudioStreamCallback

class AudioStreamCallbackImplementor : AudioStreamCallback
{
public:
	AudioStreamCallbackImplementor () {};

	on_audio_ready_func on_audio_ready;
	on_error_close_func on_error_before_close;
	on_error_close_func on_error_after_close;
	void *user_data;
	
	oboe::DataCallbackResult onAudioReady (oboe::AudioStream* oboeStream, void* audioData, int32_t numFrames)
	{
		return (oboe::DataCallbackResult) on_audio_ready ((oboe_audio_stream_ptr_t) oboeStream, audioData, numFrames);
	}
	
	void onErrorBeforeClose (oboe::AudioStream* oboeStream, oboe::Result error)
	{
		on_error_before_close ((oboe_audio_stream_ptr_t) oboeStream, (OboeResult) error);
	}
	
	void onErrorAfterClose (oboe::AudioStream* oboeStream, oboe::Result error)
	{
		on_error_after_close ((oboe_audio_stream_ptr_t) oboeStream, (OboeResult) error);
	}
};


oboe_audio_stream_callback_ptr_t oboe_audio_stream_callback_create ()
{
	return new AudioStreamCallbackImplementor ();
}

void oboe_audio_stream_callback_free (oboe_audio_stream_callback_ptr_t instance)
{
	delete (AudioStreamCallbackImplementor*) instance;
}

void oboe_audio_stream_callback_set_on_audio_ready (oboe_audio_stream_callback_ptr_t instance, on_audio_ready_func onAudioReady)
{
	((AudioStreamCallbackImplementor*) instance)->on_audio_ready = onAudioReady;
}

void oboe_audio_stream_callback_set_on_error_before_close (oboe_audio_stream_callback_ptr_t instance, on_error_close_func onErrorBeforeClose)
{
	((AudioStreamCallbackImplementor*) instance)->on_error_before_close = onErrorBeforeClose;
}

void oboe_audio_stream_callback_set_on_error_after_close (oboe_audio_stream_callback_ptr_t instance, on_error_close_func onErrorAfterClose)
{
	((AudioStreamCallbackImplementor*) instance)->on_error_after_close = onErrorAfterClose;
}

void* oboe_audio_stream_callback_get_user_data (oboe_audio_stream_callback_ptr_t instance)
{
	return ((AudioStreamCallbackImplementor*) instance)->user_data;
}

void oboe_audio_stream_callback_set_user_data (oboe_audio_stream_callback_ptr_t instance, void *userData)
{
    ((AudioStreamCallbackImplementor*) instance)->user_data = userData;
}

#ifdef __cplusplus
} // extern "C"
#endif

//#endif // OBOE_C_H_INCLUDED
