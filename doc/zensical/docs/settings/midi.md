# MIDI driver settings


## `midi.autoconnect` {#settings_midi_autoconnect}

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `0 (FALSE)` |

If 1 (TRUE), automatically connects FluidSynth to available MIDI input ports. alsa_seq, coremidi and jack are currently the only drivers making use of this.

## `midi.driver` {#settings_midi_driver}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `alsa_seq (Linux),` |

The MIDI system to be used.

## `midi.realtime-prio` {#settings_midi_realtime-prio}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `50` |
| Min | `0` |
| Max | `99` |

Sets the realtime scheduling priority of the MIDI thread (0 disables high priority scheduling). Linux is the only platform which currently makes use of different priority levels. Drivers which use this option: alsa_raw, alsa_seq, oss

## `midi.portname` {#settings_midi_portname}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `(empty string)` |

Used by coremidi and alsa_seq drivers for the portnames registered with the MIDI subsystem.

## `midi.alsa.device` {#settings_midi_alsa_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `default` |

ALSA MIDI hardware device to use for RAW ALSA MIDI driver (not to be confused with the MIDI port). Since fluidsynth 2.3.0 this setting will be populated with available devices when fluidsynth starts up.

## `midi.alsa_seq.device` {#settings_midi_alsa_seq_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `default` |

ALSA sequencer hardware device to use for ALSA sequencer driver (not to be confused with the MIDI port).

## `midi.alsa_seq.id` {#settings_midi_alsa_seq_id}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `pid` |

ID to use when registering ports with the ALSA sequencer driver. If set to "pid" then the ID will be "FLUID Synth (PID)", where PID is the FluidSynth process ID of the audio thread otherwise the provided string will be used in place of PID.

## `midi.coremidi.id` {#settings_midi_coremidi_id}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `pid` |

Client ID to use for CoreMIDI driver. 'pid' will use process ID as port of the client name.

## `midi.jack.server` {#settings_midi_jack_server}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `(empty string)` |

Jack server to connect to for Jack MIDI driver. If an empty string then the default server will be used.

## `midi.jack.id` {#settings_midi_jack_id}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `fluidsynth-midi` |

Client ID to use with the Jack MIDI driver. If jack is also used as audio driver and "midi.jack.server" and "audio.jack.server" are equal, this setting will be overridden by "audio.jack.id", because a client cannot have multiple names.

## `midi.oss.device` {#settings_midi_oss_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `/dev/midi` |

The hardware device to use for OSS MIDI driver (not to be confused with the MIDI port).

## `midi.winmidi.device` {#settings_midi_winmidi_device}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `default` |

The hardware device to use for Windows MIDI driver (not to be confused with the MIDI port). Multiple devices can be specified by a list of devices index separated by a semicolon (e.g "2;0", which is equivalent to one device with 32 MIDI channels). Starting with 2.3.6 all device names are expected to be UTF8 encoded.
