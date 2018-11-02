/* Referent oboe: c5c3cc1 */

enum OboeStreamState
{
	STREAM_STATE_UNINITIALIZED,
	STREAM_STATE_UNKNOWN,
	STREAM_STATE_OPEN,
	STREAM_STATE_STARTING,
	STREAM_STATE_STARTED,
	STREAM_STATE_PAUSING,
	STREAM_STATE_PAUSED,
	STREAM_STATE_FLUSHING,
	STREAM_STATE_FLUSHED,
	STREAM_STATE_STOPPING,
	STREAM_STATE_STOPPED,
	STREAM_STATE_CLOSING,
	STREAM_STATE_CLOSED,
	STREAM_STATE_DISCONNECTED,
};

enum OboeDirection 
{
	DIRECTION_OUTPUT,
	DIRECTION_INPUT,
};

enum OboeAudioFormat 
{
	AUDIO_FORMAT_INVALID,
	AUDIO_FORMAT_UNSPECIFIED,
	AUDIO_FORMAT_I16,
	AUDIO_FORMAT_FLOAT,
};

enum OboeDataCallbackResult 
{
	CALLBACK_RESULT_CONTINUE,
	CALLBACK_RESULT_STOP,
};

enum OboeResult
{
	RESULT_OK,
	ERROR_BASE,
	ERROR_DISCONNECTED,
	ERROR_ILLEGAL_ARGUMENT,
	ERROR_INTERNAL,
	ERROR_INVALID_STATE,
	ERROR_INVALID_HANDLE,
	ERROR_UNIMPLEMENTED,
	ERROR_UNAVAILABLE,
	ERROR_NO_FREE_HANDLES,
	ERROR_NO_MEMORY,
	ERROR_NULL,
	ERROR_TIMEOUT,
	ERROR_WOULD_BLOCK,
	ERROR_INVALID_FORMAT,
	ERROR_OUTOFRANGE,
	ERROR_NO_SERVICE,
	ERROR_INVALID_RATE,
};

enum OboeSharingMode 
{
	SHARING_MODE_EXCLUSIVE,
	SHARING_MODE_SHARED,
};

enum OboePerformanceMode 
{
	PERFORMANCE_MODE_NONE,
	PERFORMANCE_MODE_POWER_SAVING,
	PERFORMANCE_MODE_LOW_LATENCY,
};

enum OboeAudioApi 
{
	AUDIO_API_UNSPECIFIED,
	AUDIO_API_OPENSLES,
	AUDIO_API_AAUDIO,
};

enum OboeUsage 
{
	USAGE_MEDIA,
	USAGE_VOICE_COMMUNICATION,
	USAGE_VOICE_COMMUNICATION_SIGNALLING,
	USAGE_ALARM,
	USAGE_NOTIFICATION,
	USAGE_NOTIFICATION_RINGTONE,
	USAGE_NOTIFICATION_EVENT,
	USAGE_ASSISTANCE_ACCESSIBILITY,
	USAGE_ASSISTANCE_NAVIGATION_GUIDANCE,
	USAGE_ASSISTANCE_SONIFICATION,
	USAGE_GAME,
	USAGE_ASSISTANT,
};

enum OboeContentType 
{
	CONTENT_TYPE_SPEECH,
	CONTENT_TYPE_MUSIC,
	CONTENT_TYPE_MOVIE,
	CONTENT_TYPE_SONIFICATION,
};

enum OboeInputPreset 
{
	INPUT_PRESET_GENERIC,
	INPUT_PRESET_CAMCORDER,
	INPUT_PRESET_VOICERECOGNITION,
	INPUT_PRESET_VOICECOMMUNICATION,
	INPUT_PRESET_UNPROCESSED,
};

enum OboeSessionId 
{
	SESSION_ID_NONE,
	SESSION_ID_ALLOCATE,
};

typedef struct OboeResultWithValueInt32_t
{
	enum OboeResult error;
	int32_t value;
} OboeResultWithValueInt32;

typedef struct OboeResultWithValueDouble_t
{
	enum OboeResult error;
	double value;
} OboeResultWithValueDouble;

/* forward decls. */

typedef void* oboe_result_with_value_ptr_t;
typedef void* oboe_latency_tuner_ptr_t;
typedef void* oboe_audio_stream_ptr_t;
typedef void* oboe_audio_stream_base_ptr_t;
typedef void* oboe_audio_stream_builder_ptr_t;
typedef void* oboe_audio_stream_callback_ptr_t;


/* ResultWithValue (is annoying; maybe I just remove them all) */

/*

oboe_result_with_value_ptr_t oboe_result_with_value_of_int32_create (int32_t value, OboeResult result);

oboe_result_with_value_ptr_t oboe_result_with_value_of_double_create (double value, OboeResult result);

void oboe_result_with_value_of_int32_free (oboe_result_with_value_ptr_t instance);

void oboe_result_with_value_of_double_free (oboe_result_with_value_ptr_t instance);

int32_t oboe_result_with_value_of_int32_value (oboe_result_with_value_ptr_t instance);

double oboe_result_with_value_of_double_value (oboe_result_with_value_ptr_t instance);

*/

/* LatencyTuner */

oboe_latency_tuner_ptr_t oboe_latency_tuner_create (oboe_audio_stream_ptr_t stream);

void oboe_latency_tuner_free (oboe_latency_tuner_ptr_t instance);

enum OboeResult oboe_latency_tuner_tune (oboe_latency_tuner_ptr_t instance);

void oboe_latency_tuner_request_reset (oboe_latency_tuner_ptr_t instance);

/* AudioStream */

enum OboeResult oboe_audio_stream_open (oboe_audio_stream_ptr_t instance);

enum OboeResult oboe_audio_stream_close (oboe_audio_stream_ptr_t instance);

enum OboeResult oboe_audio_stream_start (oboe_audio_stream_ptr_t instance);

enum OboeResult oboe_audio_stream_pause (oboe_audio_stream_ptr_t instance);

enum OboeResult oboe_audio_stream_flush (oboe_audio_stream_ptr_t instance);

enum OboeResult oboe_audio_stream_stop (oboe_audio_stream_ptr_t instance);

enum OboeResult oboe_audio_stream_request_start (oboe_audio_stream_ptr_t instance);

enum OboeResult oboe_audio_stream_request_pause (oboe_audio_stream_ptr_t instance);

enum OboeResult oboe_audio_stream_request_flush (oboe_audio_stream_ptr_t instance);

enum OboeResult oboe_audio_stream_request_stop (oboe_audio_stream_ptr_t instance);

enum OboeStreamState oboe_audio_stream_get_state (oboe_audio_stream_ptr_t instance);

enum OboeResult oboe_audio_stream_wait_for_state_change (oboe_audio_stream_ptr_t instance, enum OboeStreamState inputState, enum OboeStreamState* nextState, int64_t timeoutNanoseconds);

OboeResultWithValueInt32 oboe_audio_stream_set_buffer_size_in_frames (oboe_audio_stream_ptr_t instance, int32_t requestedFrames);

OboeResultWithValueInt32 oboe_audio_stream_get_x_run_count (oboe_audio_stream_ptr_t instance);

int32_t oboe_audio_stream_get_frames_per_burst (oboe_audio_stream_ptr_t instance);

int32_t oboe_audio_stream_is_playing (oboe_audio_stream_ptr_t instance);

int32_t oboe_audio_stream_get_bytes_per_frame (oboe_audio_stream_ptr_t instance);

int32_t oboe_audio_stream_get_bytes_per_sample (oboe_audio_stream_ptr_t instance);

int64_t oboe_audio_stream_get_frames_written (oboe_audio_stream_ptr_t instance);

int64_t oboe_audio_stream_get_frames_read (oboe_audio_stream_ptr_t instance);

OboeResultWithValueDouble oboe_audio_stream_calculate_latency_millis (oboe_audio_stream_ptr_t instance);

enum OboeResult oboe_audio_stream_get_timestamp (oboe_audio_stream_ptr_t instance, clockid_t clockId, int64_t *framePosition, int64_t *timeNanoseconds);

OboeResultWithValueInt32 oboe_audio_stream_write (oboe_audio_stream_ptr_t instance, const void *buffer, int32_t numFrames, int64_t timeoutNanoseconds);

OboeResultWithValueInt32 oboe_audio_stream_read (oboe_audio_stream_ptr_t instance, void *buffer, int32_t numFrames, int64_t timeoutNanoseconds);

enum OboeAudioApi oboe_audio_stream_get_audio_api (oboe_audio_stream_ptr_t instance);

int32_t oboe_audio_stream_uses_aaudio (oboe_audio_stream_ptr_t instance);

/* AudioStreamBase */

int oboe_audio_stream_base_get_channel_count (oboe_audio_stream_base_ptr_t instance);

enum OboeDirection oboe_audio_stream_base_get_direction (oboe_audio_stream_base_ptr_t instance);

int32_t oboe_audio_stream_base_get_sample_rate (oboe_audio_stream_base_ptr_t instance);

int oboe_audio_stream_base_get_frames_per_callback (oboe_audio_stream_base_ptr_t instance);

enum OboeAudioFormat oboe_audio_stream_base_get_format (oboe_audio_stream_base_ptr_t instance);

int32_t oboe_audio_stream_base_get_buffer_size_in_frames (oboe_audio_stream_base_ptr_t instance);

int32_t oboe_audio_stream_base_get_buffer_capacity_in_frames (oboe_audio_stream_base_ptr_t instance);

enum OboeSharingMode oboe_audio_stream_base_get_sharing_mode (oboe_audio_stream_base_ptr_t instance);

enum OboePerformanceMode oboe_audio_stream_base_get_performance_mode (oboe_audio_stream_base_ptr_t instance);

int32_t oboe_audio_stream_base_get_device_id (oboe_audio_stream_base_ptr_t instance);

oboe_audio_stream_callback_ptr_t oboe_audio_stream_base_get_callback (oboe_audio_stream_base_ptr_t instance);

enum OboeUsage oboe_audio_stream_base_get_usage (oboe_audio_stream_base_ptr_t instance);

enum OboeContentType oboe_audio_stream_base_get_content_type (oboe_audio_stream_base_ptr_t instance);

enum OboeInputPreset oboe_audio_stream_base_get_input_preset (oboe_audio_stream_base_ptr_t instance);

enum OboeSessionId oboe_audio_stream_base_get_session_id (oboe_audio_stream_base_ptr_t instance);

/* AudioStreamBuilder */

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_create ();

void oboe_audio_stream_builder_delete (oboe_audio_stream_builder_ptr_t instance);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_channel_count (oboe_audio_stream_builder_ptr_t instance, int channelCount);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_direction (oboe_audio_stream_builder_ptr_t instance, enum OboeDirection direction);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_sample_rate (oboe_audio_stream_builder_ptr_t instance, int32_t sampleRate);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_frames_per_callback (oboe_audio_stream_builder_ptr_t instance, int framesPerCallback);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_format (oboe_audio_stream_builder_ptr_t instance, enum OboeAudioFormat format);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_buffer_capacity_in_frames (oboe_audio_stream_builder_ptr_t instance, int bufferCapacityInFrames);

enum OboeAudioApi oboe_audio_stream_builder_get_audio_api (oboe_audio_stream_builder_ptr_t instance);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_audio_api (oboe_audio_stream_builder_ptr_t instance, enum OboeAudioApi audioApi);

int32_t oboe_audio_stream_builder_is_aaudio_supported (oboe_audio_stream_builder_ptr_t instance);

int32_t oboe_audio_stream_builder_is_aaudio_recommended (oboe_audio_stream_builder_ptr_t instance);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_sharing_mode (oboe_audio_stream_builder_ptr_t instance, enum OboeSharingMode sharingMode);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_performance_mode (oboe_audio_stream_builder_ptr_t instance, enum OboePerformanceMode performanceMode);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_usage (oboe_audio_stream_builder_ptr_t instance, enum OboeUsage usage);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_content_type (oboe_audio_stream_builder_ptr_t instance, enum OboeContentType contentType);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_input_preset (oboe_audio_stream_builder_ptr_t instance, enum OboeInputPreset inputPreset);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_session_id (oboe_audio_stream_builder_ptr_t instance, enum OboeSessionId sessionId);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_device_id (oboe_audio_stream_builder_ptr_t instance, int32_t deviceId);

oboe_audio_stream_builder_ptr_t oboe_audio_stream_builder_set_callback (oboe_audio_stream_builder_ptr_t instance, oboe_audio_stream_callback_ptr_t streamCallback);

enum OboeResult oboe_audio_stream_builder_open_stream (oboe_audio_stream_builder_ptr_t instance, oboe_audio_stream_ptr_t *stream);

/* AudioStreamCallback */

typedef enum OboeDataCallbackResult (*on_audio_ready_func) (oboe_audio_stream_ptr_t oboeStream, void *audioData, int32_t numFrames);
typedef void (*on_error_close_func) (oboe_audio_stream_ptr_t oboeStream, enum OboeResult error);

oboe_audio_stream_callback_ptr_t oboe_audio_stream_callback_create ();

void oboe_audio_stream_callback_free (oboe_audio_stream_callback_ptr_t instance);

void oboe_audio_stream_callback_set_on_audio_ready (oboe_audio_stream_callback_ptr_t instance, on_audio_ready_func onAudioReady);

void oboe_audio_stream_callback_set_on_error_before_close (oboe_audio_stream_callback_ptr_t instance, on_error_close_func onErrorBeforeClose);

void oboe_audio_stream_callback_set_on_error_after_close (oboe_audio_stream_callback_ptr_t instance, on_error_close_func onErrorAfterClose);

void* oboe_audio_stream_callback_get_user_data (oboe_audio_stream_callback_ptr_t instance);

void oboe_audio_stream_callback_set_user_data (oboe_audio_stream_callback_ptr_t instance, void *userData);
