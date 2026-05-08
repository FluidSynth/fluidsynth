# Synthesis Voice Control

Low-level access to synthesis voices.

## Functions

### `fluid_synth_start()` {#fluid_synth_start}

```c
int fluid_synth_start(fluid_synth_t *synth, unsigned int id, fluid_preset_t *preset, int audio_chan, int midi_chan, int key, int vel)
```

Create and start voices using an arbitrary preset and a MIDI note on event.

Using this function is only supported when the setting `synth.dynamic-sample-loading` is false! 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `id` | Voice group ID to use (can be used with [`fluid_synth_stop()`](voice-control.md#fluid_synth_stop)). |
| `preset` | Preset to synthesize |
| `audio_chan` | Unused currently, set to 0 |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `key` | MIDI note number (0-127) |
| `vel` | MIDI velocity number (1-127) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** Should only be called from within synthesis thread, which includes SoundFont loader preset noteon method.

### `fluid_synth_stop()` {#fluid_synth_stop}

```c
int fluid_synth_stop(fluid_synth_t *synth, unsigned int id)
```

Stop notes for a given note event voice ID. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `id` | Voice note event ID |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** In FluidSynth versions prior to 1.1.0 [`FLUID_FAILED`](misc.md#FLUID_FAILED) would be returned if no matching voice note event ID was found. Versions after 1.1.0 only return [`FLUID_FAILED`](misc.md#FLUID_FAILED) if an error occurs.

### `fluid_synth_alloc_voice()` {#fluid_synth_alloc_voice}

```c
fluid_voice_t * fluid_synth_alloc_voice(fluid_synth_t *synth, fluid_sample_t *sample, int channum, int key, int vel)
```

Allocate a synthesis voice. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `sample` | Sample to assign to the voice |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `key` | MIDI note number for the voice (0-127) |
| `vel` | MIDI velocity for the voice (0-127) |

**Returns:** Allocated synthesis voice or NULL on error

> **Note:** Should only be called from within synthesis thread, which includes SoundFont loader preset noteon method.

### `fluid_synth_start_voice()` {#fluid_synth_start_voice}

```c
void fluid_synth_start_voice(fluid_synth_t *synth, fluid_voice_t *voice)
```

Activate a voice previously allocated with [`fluid_synth_alloc_voice()`](voice-control.md#fluid_synth_alloc_voice). 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `voice` | Voice to activate |

> **Note:** Should only be called from within synthesis thread, which includes SoundFont loader preset noteon method.

### `fluid_synth_get_voicelist()` {#fluid_synth_get_voicelist}

```c
void fluid_synth_get_voicelist(fluid_synth_t *synth, fluid_voice_t *buf\[\], int bufsize, int ID)
```

Get list of currently playing voices. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `buf` | Array to store voices to (NULL terminated if not filled completely) |
| `bufsize` | Count of indexes in buf |
| `id` | Voice ID to search for or < 0 to return list of all playing voices |

> **Note:** Should only be called from within synthesis thread, which includes SoundFont loader preset noteon methods. Voices are only guaranteed to remain unchanged until next synthesis process iteration.
