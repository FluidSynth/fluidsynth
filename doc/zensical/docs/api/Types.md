# Types

Type declarations.

## Types

### `fluid_settings_t` {#fluid_settings_t}

```c
typedef typedef struct _fluid_hashtable_t fluid_settings_t;
```

Configuration settings instance

### `fluid_synth_t` {#fluid_synth_t}

```c
typedef typedef struct _fluid_synth_t fluid_synth_t;
```

Synthesizer instance

### `fluid_voice_t` {#fluid_voice_t}

```c
typedef typedef struct _fluid_voice_t fluid_voice_t;
```

Synthesis voice instance

### `fluid_sfloader_t` {#fluid_sfloader_t}

```c
typedef typedef struct _fluid_sfloader_t fluid_sfloader_t;
```

SoundFont loader plugin

### `fluid_sfont_t` {#fluid_sfont_t}

```c
typedef typedef struct _fluid_sfont_t fluid_sfont_t;
```

SoundFont

### `fluid_preset_t` {#fluid_preset_t}

```c
typedef typedef struct _fluid_preset_t fluid_preset_t;
```

SoundFont preset

### `fluid_sample_t` {#fluid_sample_t}

```c
typedef typedef struct _fluid_sample_t fluid_sample_t;
```

SoundFont sample

### `fluid_mod_t` {#fluid_mod_t}

```c
typedef typedef struct _fluid_mod_t fluid_mod_t;
```

SoundFont modulator

### `fluid_audio_driver_t` {#fluid_audio_driver_t}

```c
typedef typedef struct _fluid_audio_driver_t fluid_audio_driver_t;
```

Audio driver instance

### `fluid_file_renderer_t` {#fluid_file_renderer_t}

```c
typedef typedef struct _fluid_file_renderer_t fluid_file_renderer_t;
```

Audio file renderer instance

### `fluid_player_t` {#fluid_player_t}

```c
typedef typedef struct _fluid_player_t fluid_player_t;
```

MIDI player instance

### `fluid_midi_event_t` {#fluid_midi_event_t}

```c
typedef typedef struct _fluid_midi_event_t fluid_midi_event_t;
```

MIDI event

### `fluid_midi_driver_t` {#fluid_midi_driver_t}

```c
typedef typedef struct _fluid_midi_driver_t fluid_midi_driver_t;
```

MIDI driver instance

### `fluid_midi_router_t` {#fluid_midi_router_t}

```c
typedef typedef struct _fluid_midi_router_t fluid_midi_router_t;
```

MIDI router instance

### `fluid_midi_router_rule_t` {#fluid_midi_router_rule_t}

```c
typedef typedef struct _fluid_midi_router_rule_t fluid_midi_router_rule_t;
```

MIDI router rule

### `fluid_cmd_hash_t` {#fluid_cmd_hash_t}

```c
typedef typedef struct _fluid_hashtable_t fluid_cmd_hash_t;
```

Command handler hash table

### `fluid_shell_t` {#fluid_shell_t}

```c
typedef typedef struct _fluid_shell_t fluid_shell_t;
```

Command shell

### `fluid_server_t` {#fluid_server_t}

```c
typedef typedef struct _fluid_server_t fluid_server_t;
```

TCP/IP shell server instance

### `fluid_event_t` {#fluid_event_t}

```c
typedef typedef struct _fluid_event_t fluid_event_t;
```

Sequencer event

### `fluid_sequencer_t` {#fluid_sequencer_t}

```c
typedef typedef struct _fluid_sequencer_t fluid_sequencer_t;
```

Sequencer instance

### `fluid_cmd_handler_t` {#fluid_cmd_handler_t}

```c
typedef typedef struct _fluid_cmd_handler_t fluid_cmd_handler_t;
```

Shell Command Handler

### `fluid_ladspa_fx_t` {#fluid_ladspa_fx_t}

```c
typedef typedef struct _fluid_ladspa_fx_t fluid_ladspa_fx_t;
```

LADSPA effects instance

### `fluid_file_callbacks_t` {#fluid_file_callbacks_t}

```c
typedef typedef struct _fluid_file_callbacks_t fluid_file_callbacks_t;
```

Callback struct to perform custom file loading of soundfonts

### `fluid_istream_t` {#fluid_istream_t}

```c
typedef typedef int fluid_istream_t;
```

Input stream descriptor

### `fluid_ostream_t` {#fluid_ostream_t}

```c
typedef typedef int fluid_ostream_t;
```

Output stream descriptor

### `fluid_seq_id_t` {#fluid_seq_id_t}

```c
typedef typedef short fluid_seq_id_t;
```

Unique client IDs used by the sequencer and [`fluid_event_t`](Types.md#fluid_event_t), obtained by [`fluid_sequencer_register_client()`](sequencer.md#fluid_sequencer_register_client) and [`fluid_sequencer_register_fluidsynth()`](sequencer.md#fluid_sequencer_register_fluidsynth)

### `fluid_long_long_t` {#fluid_long_long_t}

```c
typedef typedef long long fluid_long_long_t;
```

A typedef for C99's type long long, which is at least 64-bit wide, as guaranteed by the C99. `__int64` will be used as replacement for VisualStudio 2010 and older.
