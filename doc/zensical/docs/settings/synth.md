# Synthesizer settings


## `synth.audio-channels` {#settings_synth_audio-channels}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `1` |
| Min | `1` |
| Max | `128` |

By default, the synthesizer outputs a single stereo signal. Using this option, the synthesizer can output multi-channel audio. Sets the number of stereo channel pairs. So 1 is actually 2 channels (a stereo pair).

## `synth.audio-groups` {#settings_synth_audio-groups}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `1` |
| Min | `1` |
| Max | `128` |

The output audio channel associated with a MIDI channel is wrapped around using the number of synth.audio-groups as modulo divider. This is typically the number of output channels on the sound card, as long as the LADSPA Fx unit is not used. In case of LADSPA unit, think of it as subgroups on a mixer.

## `synth.chorus.active` {#settings_synth_chorus_active}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `1 (TRUE)` |

When set to 1 (TRUE) the chorus effects module is activated. Otherwise, no chorus will be added to the output signal. Note that the amount of signal sent to the chorus module depends on the "chorus send" generator defined in the SoundFont.

## `synth.chorus.depth` {#settings_synth_chorus_depth}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `4.25 (since version 2.4.0),` |
| Min | `0.0` |
| Max | `256.0` |

Specifies the modulation depth of the chorus.

## `synth.chorus.level` {#settings_synth_chorus_level}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `0.6 (since version 2.4.0),` |
| Min | `0.0` |
| Max | `10.0` |

Specifies the output amplitude of the chorus signal.

## `synth.chorus.nr` {#settings_synth_chorus_nr}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `3` |
| Min | `0` |
| Max | `99` |

Sets the voice count of the chorus.

## `synth.chorus.speed` {#settings_synth_chorus_speed}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `0.2 (since version 2.4.0),` |
| Min | `0.1` |
| Max | `5.0` |

Sets the modulation speed in Hz.

## `synth.cpu-cores` {#settings_synth_cpu-cores}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `1` |
| Min | `1` |
| Max | `256` |

Sets the number of synthesis CPU cores. If set to a value greater than 1, additional synthesis threads will be created to do the actual rendering work that is then returned synchronously by the render function. This has the affect of utilizing more of the total CPU for voices or decreasing render times when synthesizing audio.
                So for example, if you set cpu-cores to 4, fluidsynth will attempt to split the synthesis work it needs to do between the client's calling thread and three additional (internal) worker threads. As soon as all threads have done their work, their results are collected and the resulting buffer is returned to the caller.

## `synth.default-soundfont` {#settings_synth_default-soundfont}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `C:\soundfonts\default.sf2 (Windows),` |

The default soundfont file to use by the fluidsynth executable. The default value can be overridden during compilation time by setting the DEFAULT_SOUNDFONT cmake variable.

## `synth.device-id` {#settings_synth_device-id}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `0` |
| Min | `0` |
| Max | `127` |

Device identifier used for SYSEX commands, such as MIDI Tuning Standard commands. Fluidsynth will only process those SYSEX commands destined for this ID (except when this setting is set to 127, which causes fluidsynth to process all SYSEX commands, regardless of the device ID). Broadcast commands (with ID=127) will always be processed. It has been observed that setting this ID to 16 provides best compatibility when playing MIDI files which contain SYSEX commands that you want to have honored.

## `synth.dynamic-sample-loading` {#settings_synth_dynamic-sample-loading}

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `0 (FALSE)` |

When set to 1 (TRUE), samples are loaded to and unloaded from memory whenever presets are being selected or unselected for a MIDI channel (PROGRAM_CHANGE and PROGRAM_SELECT events are typically responsible for this). This involves memory allocation, which is not realtime safe! So only enable this in non-realtime scenarios! E.g. when rendering to a WAVE file using the fast-file-renderer.

## `synth.effects-channels` {#settings_synth_effects-channels}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `2` |
| Min | `2` |
| Max | `2` |

Specifies the number of effects per effects group. Currently this value can not be changed so there are always two effects per group available (reverb and chorus).

## `synth.effects-groups` {#settings_synth_effects-groups}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `1` |
| Min | `1` |
| Max | `128` |

Specifies the number of effects groups. By default, the sound of all voices is rendered by one reverb and one chorus effect respectively (even for multi-channel rendering). This setting gives the user control which effects of a voice to render to which independent audio channels. E.g. setting synth.effects-groups == synth.midi-channels allows to render the effects of each MIDI channel to separate audio buffers. If synth.effects-groups is smaller than the number of MIDI channels, it will wrap around. Note that any value >1 will significantly increase CPU usage.

## `synth.gain` {#settings_synth_gain}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `0.2` |
| Min | `0.0` |
| Max | `10.0` |

The gain is applied to the final or master output of the synthesizer, but before it will be processed by the limiter (if enabled). It is set to a low value by default to avoid the saturation of the output when many notes are played.

## `synth.ladspa.active` {#settings_synth_ladspa_active}

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `0 (FALSE)` |

When set to 1 (TRUE) the LADSPA subsystem will be enabled. This subsystem allows to load and interconnect LADSPA plug-ins. The output of the synthesizer is processed by the LADSPA subsystem. Note that the synthesizer has to be compiled with LADSPA support. More information about the LADSPA subsystem can be found in doc/ladspa.md or on the FluidSynth website.

## `synth.limiter.active` {#settings_synth_limiter_active}

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `0 (FALSE)` |

When set to 1 (TRUE) a look-ahead limiter is added at the end of fluidsynth's audio processing chain. This limiter helps to prevent clipping and distortion by limiting the gain of the output signal. Note that fluidsynth has to be compiled with limiter support, otherwise any of the limiter related settings will return FLUID_FAILED when attempting to set or get them.

## `synth.limiter.attack` {#settings_synth_limiter_attack}

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `5` |
| Min | `1` |
| Max | `250` |

When the limiter "sees" an incoming gain-peak, the attack specifies the look-ahead time in milliseconds, i.e. how much time before that peak the output gain will start to be reduced. Higher values allow the limiter to react more smoothly, but also increase latency because the generated audio must be delayed accordingly.

## `synth.limiter.hold` {#settings_synth_limiter_hold}

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `15` |
| Min | `0` |
| Max | `250` |

Specifies the time in milliseconds how long the previously reduced ("damped") output gain is maintained (after the gain-peak has passed), before the limiter begins to return to the normal gain level.

## `synth.limiter.release` {#settings_synth_limiter_release}

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `40` |
| Min | `0` |
| Max | `250` |

Specifies the time in milliseconds how quickly the output gain returns towards the normal level after the gain-peak has passed and the hold phase has completed.

## `synth.limiter.output-limit` {#settings_synth_limiter_output-limit}

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `0.99999` |
| Min | `0.063 (i.e. 240dB of attenuation)` |
| Max | `1 (i.e. 0dB of attenuation)` |

Specifies the target output limit of the limiter in linear range and therefore the final output level of fluidsynth's synthesized audio. The audio input for the limiter is amplified by the

## `synth.limiter.smoothing-stages` {#settings_synth_limiter_smoothing-stages}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `1` |
| Min | `1` |
| Max | `3` |

Controls how strongly the limiter smooths sudden gain changes. A value of 1 uses a single smoothing stage and gives the fastest response. Values of 2 or 3 apply additional smoothing to the gain envelope, which can reduce distortion and pumping artifacts at the cost of a slightly slower reaction and longer release.

## `synth.limiter.link-channels` {#settings_synth_limiter_link-channels}

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `0.5` |
| Min | `0` |
| Max | `1` |

Specifies how closely the limiter gains of all output channels (i.e. the left and right channels of a stereo output) are linked together. A value of 1.0 applies the same gain to all channels, based on the loudest channel. A value of 0.0 lets each channel be limited independently. Intermediate values interpolate between these behaviours.

## `synth.lock-memory` {#settings_synth_lock-memory}

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `1 (TRUE)` |

Page-lock memory that contains audio sample data, if true.

## `synth.midi-channels` {#settings_synth_midi-channels}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `16` |
| Min | `16` |
| Max | `256` |

This setting defines the number of MIDI channels of the synthesizer. The MIDI standard defines 16 channels, so MIDI hardware is limited to this number. Internally FluidSynth can use more channels which can be mapped to different MIDI sources.

## `synth.midi-bank-select` {#settings_synth_midi-bank-select}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `gs` |

This setting defines how the synthesizer interprets Bank Select messages.

## `synth.min-note-length` {#settings_synth_min-note-length}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `10` |
| Min | `0` |
| Max | `65535` |

Sets the minimum note duration in milliseconds. This ensures that really short duration note events, such as percussion notes, have a better chance of sounding as intended. Set to 0 to disable this feature.

## `synth.note-cut` {#settings_synth_note-cut}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `0` |
| Min | `0` |
| Max | `2` |

This setting specifies the behavior for releasing voices, if the same note is hit twice on the same channel. Early synthesizers like the Roland SC-55 and Microsoft Wavetable GS MIDI synthesizer (MSGS) are terminating notes abruptly that have already received a noteOff after receiving a noteOn for the same key. This behavior was presumably implemented to save polyphony in these systems. This setting was introduced in fluidsynth 2.4.3 and can be enabled to mimic this behavior, to esp. play back old tunes like Doom E1M1 more accurately. Please note that using a SoundFont which makes proper use of exclusive classes for esp. percussion instruments will yield a similar or better result. Also, this approach is generally preferable because it's portable among SF2 compliant synths and can be applied more fine-grained among instruments. This setting supports the following values:

## `synth.overflow.age` {#settings_synth_overflow_age}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `1000.0` |
| Min | `-10000.0` |
| Max | `10000.0` |

This score is divided by the number of seconds this voice has been
                active and is added to the overflow priority. It is usually a positive
                value and gives voices which have just been started a higher priority,
                making them less likely to be killed in an overflow situation.

## `synth.overflow.important` {#settings_synth_overflow_important}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `5000.0` |
| Min | `-50000.0` |
| Max | `50000.0` |

This score is added to voices on channels marked with the
                synth.overflow.important-channels setting.

## `synth.overflow.important-channels` {#settings_synth_overflow_important-channels}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `(empty string)` |

This setting is a comma-separated list of MIDI channel numbers that should
                be treated as "important" by the overflow calculation, adding the score
                set by synth.overflow.important to each voice on those channels. It can
                be used to make voices on particular MIDI channels
                less likely (synth.overflow.important > 0) or more likely
                (synth.overflow.important < 0) to be killed in an overflow situation. Channel
                numbers are 1-based, so the first MIDI channel is number 1.

## `synth.overflow.percussion` {#settings_synth_overflow_percussion}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `4000.0` |
| Min | `-10000.0` |
| Max | `10000.0` |

Sets the overflow priority score added to voices on a percussion
                channel. This is usually a positive score, to give percussion voices
                a higher priority and less chance of being killed in an overflow
                situation.

## `synth.overflow.released` {#settings_synth_overflow_released}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `-2000.0` |
| Min | `-10000.0` |
| Max | `10000.0` |

Sets the overflow priority score added to voices that have already
                received a note-off event. This is usually a negative score, to give released
                voices a lower priority so that they are killed first in an overflow
                situation.

## `synth.overflow.sustained` {#settings_synth_overflow_sustained}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `-1000.0` |
| Min | `-10000.0` |
| Max | `10000.0` |

Sets the overflow priority score added to voices that are currently
                sustained. With the default value, sustained voices are considered less
                important and are more likely to be killed in an overflow situation.

## `synth.overflow.volume` {#settings_synth_overflow_volume}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `500.0` |
| Min | `-10000.0` |
| Max | `10000.0` |

Sets the overflow priority score added to voices based on their current
                volume. The voice volume is normalized to a value between 0 and 1 and
                multiplied with this setting. So voices with maximum volume get added
                the full score, voices with only half that volume get added half of this
                score.

## `synth.polyphony` {#settings_synth_polyphony}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `256` |
| Min | `1` |
| Max | `65535` |

The polyphony defines how many voices can be played in parallel. A note event produces one or more voices. Its good to set this to a value which the system can handle and will thus limit FluidSynth's CPU usage. When FluidSynth runs out of voices it will begin terminating lower priority voices for new note events.

## `synth.portamento-time` {#settings_synth_portamento-time}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `auto` |

This setting was introduced in 2.5.0 to specify how to handle portamento time CC MSB and LSB.

## `synth.reverb.active` {#settings_synth_reverb_active}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `1 (TRUE)` |

When set to 1 (TRUE) the reverb effects module is activated. Otherwise, no reverb will be added to the output signal. Note that the amount of signal sent to the reverb module depends on the "reverb send" generator defined in the SoundFont.

## `synth.reverb.damp` {#settings_synth_reverb_damp}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `0.3 (since version 2.4.0),` |
| Min | `0.0` |
| Max | `1.0` |

Sets the amount of reverb damping.

## `synth.reverb.level` {#settings_synth_reverb_level}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `0.7 (since version 2.4.0),` |
| Min | `0.0` |
| Max | `1.0` |

Sets the reverb output amplitude.

## `synth.reverb.room-size` {#settings_synth_reverb_room-size}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `0.5 (since version 2.4.0),` |
| Min | `0.0` |
| Max | `1.0` |

Sets the room size (i.e. amount of wet) reverb.

## `synth.reverb.width` {#settings_synth_reverb_width}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `0.8 (since version 2.4.0),` |
| Min | `0.0` |
| Max | `100.0` |

Sets the stereo spread of the reverb signal. A value of 0 indicates no stereo-separation causing the reverb to sound like a monophonic signal. A value of 1 indicates maximum separation between the uncorrelated left and right channels (note that reverb is still a monophonic effect). This subrange \[0;1\] is recommended for general usage. Values bigger than 1 increase (or exaggerate) the perception of the uncorrelated left and right signals. Otherwise, this setting should be considered as dimensionless quantity, with its maximum value existing for historical reasons. Please note that under some circumstances, values bigger than 1 may induce a feedback into the signal which can be perceived as unpleasant.

## `synth.sample-rate` {#settings_synth_sample-rate}

| Property | Value |
|----------|-------|
| Type | `num` |
| Default | `44100.0` |
| Min | `8000.0` |
| Max | `96000.0` |

The sample rate of the audio generated by the synthesizer. For optimal performance,
				make sure this value equals the native output rate of the audio driver (in case you
				are using any of FluidSynth's audio drivers). Some drivers, such as Oboe, will
				interpolate sample rates, whereas others, such as JACK, will override this setting
				if a mismatch with the native output rate is detected.

## `synth.threadsafe-api` {#settings_synth_threadsafe-api}

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `1 (TRUE)` |

Controls whether the synth's public API is protected by a mutex or not. Default is on, turn it off for slightly better performance if you know you're only accessing the synth from one thread only, this could be the case in many embedded use cases for example. Note that libfluidsynth can use many threads by itself (shell is one, midi driver is one, midi player is one etc) so you should usually leave it on.

## `synth.verbose` {#settings_synth_verbose}

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `0 (FALSE)` |

When set to 1 (TRUE) the synthesizer will print out information about the received MIDI events to the stdout. This can be helpful for debugging. This setting cannot be changed after the synthesizer has started.
