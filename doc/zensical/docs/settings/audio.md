# Audio driver settings


## `audio.driver` {#settings_audio_driver}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `alsa (Linux),` |

The audio system to be used.
				Some audio drivers support only a subset of audio sample formats.
				See

## `audio.periods` {#settings_audio_periods}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `8 (Windows, MacOS9),` |
| Min | `2` |
| Max | `64` |

The number of the audio buffers used by the driver. This number of buffers, multiplied by the buffer size (see setting audio.period-size), determines the maximum latency of the audio driver.

## `audio.period-size` {#settings_audio_period-size}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `512 (Windows),` |
| Min | `64` |
| Max | `8192` |

This is the number of audio samples most audio drivers will request from the synth at one time. In other words, it's the amount of samples the synth is allowed to render in one go when no state changes (events) are about to happen. Because of that, specifying too big numbers here may cause MIDI events to be poorly quantized (=untimed) when a MIDI driver or the synth's API directly is used, as fluidsynth cannot determine when those events are to arrive. This issue does not matter, when using the MIDI player or the MIDI sequencer, because in this case, fluidsynth does know when events will be received.

## `audio.realtime-prio` {#settings_audio_realtime-prio}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `60` |
| Min | `0` |
| Max | `99` |

Sets the realtime scheduling priority of the audio synthesis thread. This includes the synthesis threads created by the synth (in case synth.cpu-cores was greater 1). A value of 0 disables high priority scheduling. Linux is the only platform which currently makes use of different priority levels as specified by this setting. On other operating systems the thread priority is set to maximum. Drivers which use this option: alsa, oss and pulseaudio

## `audio.sample-format` {#settings_audio_sample-format}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `16bits` |

Sample format requested for transferring floating point audio samples
				from the synthesizer to the audio hardware. The format actually used
				depends on the selected audio driver and output device. Some drivers
				may ignore this setting or fail to initialize when an unsupported
				format is requested.

## `audio.alsa.device` {#settings_audio_alsa_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `default` |

Selects the ALSA audio device to use.

## `audio.coreaudio.device` {#settings_audio_coreaudio_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `default` |

Selects the CoreAudio device to use.

## `audio.coreaudio.channel-map` {#settings_audio_coreaudio_channel-map}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `(empty string)` |

This setting is a comma-separated integer list that maps fluidsynth mono-channels
                to CoreAudio device output channels. Each position in the list represents the output channel
                of the CoreAudio device.
                The value of each position indicates the zero-based index of the fluidsynth
                output mono-channel to route there (i.e. the buffer index used for fluid_synth_process()).
                Additionally, the special value of -1 will turn off an output.

## `audio.dart.device` {#settings_audio_dart_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `default` |

Selects the Dart (OS/2 driver) device to use.

## `audio.dsound.device` {#settings_audio_dsound_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `default` |

Selects the DirectSound (Windows) device to use. Starting with 2.3.6 all device names are expected to be UTF8 encoded.

## `audio.file.endian` {#settings_audio_file_endian}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `'auto' if libsndfile support is built in,` |

Defines the byte order when using the 'file' driver or file renderer to store audio to a file. 'auto' uses the default for the given file type, 'cpu' uses the CPU byte order, 'big' uses big endian byte order and 'little' uses little endian byte order.

## `audio.file.format` {#settings_audio_file_format}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `s16` |

Defines the audio format when rendering audio to a file. Limited to 's16' if no libsndfile support.

## `audio.file.name` {#settings_audio_file_name}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `'fluidsynth.wav' if libsndfile support is built in,` |

Specifies the file name to store the audio to, when rendering audio to a file.

## `audio.file.type` {#settings_audio_file_type}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `'auto' if libsndfile support is built in,` |

Sets the file type of the file which the audio will be stored to. 'auto' attempts to determine the file type from the audio.file.name file extension and falls back to 'wav' if the extension doesn't match any types. Limited to 'raw' if compiled without libsndfile support. Actual options will vary depending on libsndfile library.

## `audio.jack.autoconnect` {#settings_audio_jack_autoconnect}

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `0 (FALSE)` |

If 1 (TRUE), then FluidSynth output is automatically connected to jack system audio output.

## `audio.jack.id` {#settings_audio_jack_id}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `fluidsynth` |

Unique identifier used when creating Jack client connection.

## `audio.jack.multi` {#settings_audio_jack_multi}

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `0 (FALSE)` |

If 1 (TRUE), then multi-channel Jack output will be enabled if synth.audio-channels is greater than 1.

## `audio.jack.server` {#settings_audio_jack_server}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `(empty string)` |

Jack server to connect to. Defaults to an empty string, which uses default Jack server.

## `audio.oboe.id` {#settings_audio_oboe_id}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `0` |
| Min | `0` |
| Max | `2147483647` |

Request an audio device identified device using an ID as pointed out by Oboe's documentation.

## `audio.oboe.sample-rate-conversion-quality` {#settings_audio_oboe_sample-rate-conversion-quality}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `None` |

Sets the sample-rate conversion quality as pointed out by Oboe's documentation.

## `audio.oboe.sharing-mode` {#settings_audio_oboe_sharing-mode}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `Shared` |

Sets the sharing mode as pointed out by Oboe's documentation.

## `audio.oboe.performance-mode` {#settings_audio_oboe_performance-mode}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `None` |

Sets the performance mode as pointed out by Oboe's documentation.

## `audio.oboe.error-recovery-mode` {#settings_audio_oboe_error-recovery-mode}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `Reconnect` |

Sets the error recovery mode when audio device error such as earphone disconnection occurred. It reconnects by default (same as OpenSLES behavior), but can be stopped if Stop is specified.

## `audio.oss.device` {#settings_audio_oss_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `/dev/dsp` |

Device to use for OSS audio output.

## `audio.pipewire.media-category` {#settings_audio_pipewire_media-category}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `Playback` |

The media category to use. This value will be passed to

## `audio.pipewire.media-role` {#settings_audio_pipewire_media-role}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `Music` |

The media role to use. This value will be passed to

## `audio.pipewire.media-type` {#settings_audio_pipewire_media-type}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `Audio` |

The media type to use. This value will be passed to

## `audio.portaudio.device` {#settings_audio_portaudio_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `PortAudio Default` |

Device to use for PortAudio driver output. Note that 'PortAudio Default' is a special value which outputs to the default PortAudio device. The format of the device name is: "::" e.g. "11:Windows DirectSound:SB PCI"

## `audio.pulseaudio.adjust-latency` {#settings_audio_pulseaudio_adjust-latency}

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `1 (TRUE)` |

If TRUE initializes the maximum length of the audio buffer to the highest supported value and increases the latency dynamically if PulseAudio suggests so. Else uses a buffer with length of "audio.period-size".

## `audio.pulseaudio.device` {#settings_audio_pulseaudio_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `default` |

Device to use for PulseAudio driver output.

## `audio.pulseaudio.media-role` {#settings_audio_pulseaudio_media-role}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `music` |

PulseAudio media role information.

## `audio.pulseaudio.server` {#settings_audio_pulseaudio_server}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `default` |

Server to use for PulseAudio driver output.

## `audio.sdl3.device` {#settings_audio_sdl3_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `default` |

Device to use for SDL3 driver output.

## `audio.wasapi.device` {#settings_audio_wasapi_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `default` |

Device to use for WASAPI driver output. Starting with 2.3.6 all device names are expected to be UTF8 encoded.

## `audio.wasapi.exclusive-mode` {#settings_audio_wasapi_exclusive-mode}

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `0 (FALSE)` |

By default, WASAPI will operate in shared mode. Set it to 1 (TRUE) to use WASAPI in exclusive mode.
				In this mode, you'll benefit from direct soundcard access, which has extremely
				low latency. However, in exclusive mode the requested audio configuration must be supported exactly
				by the output device.

## `audio.waveout.device` {#settings_audio_waveout_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `default` |

Device to use for WaveOut driver output. Starting with 2.3.6 all device names are expected to be UTF8 encoded.
