# Voice Manipulation

Synthesis voice manipulation functions.

The interface to the synthesizer's voices. Examples on using them can be found in the source code of the default SoundFont loader (`fluid_defsfont.c`).

Most of these functions should only be called from within synthesis context, such as the SoundFont loader's noteon method.

## Enumerations

### `fluid_voice_add_mod` {#fluid_voice_add_mod}

| Value | Description |
|-------|-------------|
| `FLUID_VOICE_OVERWRITE` |  |
| `FLUID_VOICE_ADD` |  |
| `FLUID_VOICE_DEFAULT` |  |

Enum used with [`fluid_voice_add_mod()`](voices.md#fluid_voice_add_mod) to specify how to handle duplicate modulators.

## Functions

### `fluid_voice_add_mod()` {#fluid_voice_add_mod}

```c
void fluid_voice_add_mod(fluid_voice_t *voice, fluid_mod_t *mod, int mode)
```

Adds a modulator to the voice if the modulator has valid sources.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance. |
| `mod` | Modulator info (copied). |
| `mode` | Determines how to handle an existing identical modulator. `FLUID_VOICE_ADD` to add (offset) the modulator amounts, `FLUID_VOICE_OVERWRITE` to replace the modulator, `FLUID_VOICE_DEFAULT` when adding a default modulator - no duplicate should exist so don't check. |

### `fluid_voice_gen_get()` {#fluid_voice_gen_get}

```c
float fluid_voice_gen_get(fluid_voice_t *voice, int gen)
```

Get the value of a generator.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |
| `gen` | Generator ID ([`fluid_gen_type`](generators.md#fluid_gen_type)) |

**Returns:** Current generator value

### `fluid_voice_gen_set()` {#fluid_voice_gen_set}

```c
void fluid_voice_gen_set(fluid_voice_t *voice, int gen, float val)
```

Set the value of a generator.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |
| `i` | Generator ID ([`fluid_gen_type`](generators.md#fluid_gen_type)) |
| `val` | Generator value |

### `fluid_voice_gen_incr()` {#fluid_voice_gen_incr}

```c
void fluid_voice_gen_incr(fluid_voice_t *voice, int gen, float val)
```

Offset the value of a generator.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |
| `i` | Generator ID ([`fluid_gen_type`](generators.md#fluid_gen_type)) |
| `val` | Value to add to the existing value |

### `fluid_voice_get_id()` {#fluid_voice_get_id}

```c
unsigned int fluid_voice_get_id(const fluid_voice_t *voice)
```

Get the unique ID of the noteon-event.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |

**Returns:** Note on unique ID

However, before modifying an existing voice, check

- that its state is still 'playing'
- that the ID is still the same

Otherwise the voice has finished playing.

### `fluid_voice_get_channel()` {#fluid_voice_get_channel}

```c
int fluid_voice_get_channel(const fluid_voice_t *voice)
```

Return the MIDI channel the voice is playing on.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |

**Returns:** The channel assigned to this voice

> **Note:** The result of this function is only valid if the voice is playing.

**Since:** 1.1.7

### `fluid_voice_get_key()` {#fluid_voice_get_key}

```c
int fluid_voice_get_key(const fluid_voice_t *voice)
```

Return the MIDI key from the starting noteon event.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |

**Returns:** The MIDI key of the noteon event that originally turned on this voice

> **Note:** The result of this function is only valid if the voice is playing.

**Since:** 1.1.7

### `fluid_voice_get_actual_key()` {#fluid_voice_get_actual_key}

```c
int fluid_voice_get_actual_key(const fluid_voice_t *voice)
```

Return the effective MIDI key of the playing voice.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |

**Returns:** The MIDI key this voice is playing at

`fluid_voice_get_key`.

> **Note:** The result of this function is only valid if the voice is playing.

**Since:** 1.1.7

### `fluid_voice_get_velocity()` {#fluid_voice_get_velocity}

```c
int fluid_voice_get_velocity(const fluid_voice_t *voice)
```

Return the MIDI velocity from the starting noteon event.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |

**Returns:** The MIDI velocity which originally turned on this voice

> **Note:** The result of this function is only valid if the voice is playing.

**Since:** 1.1.7

### `fluid_voice_get_actual_velocity()` {#fluid_voice_get_actual_velocity}

```c
int fluid_voice_get_actual_velocity(const fluid_voice_t *voice)
```

Return the effective MIDI velocity of the playing voice.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |

**Returns:** The MIDI velocity this voice is playing at

`fluid_voice_get_velocity`.

> **Note:** The result of this function is only valid if the voice is playing.

**Since:** 1.1.7

### `fluid_voice_is_playing()` {#fluid_voice_is_playing}

```c
int fluid_voice_is_playing(const fluid_voice_t *voice)
```

Check if a voice is producing sound.

Like [`fluid_voice_is_on()`](voices.md#fluid_voice_is_on) this will return TRUE once a call to [`fluid_synth_start_voice()`](voice-control.md#fluid_synth_start_voice) has been made. Contrary to [`fluid_voice_is_on()`](voices.md#fluid_voice_is_on), this might also return TRUE after the voice received a noteoff event, as it may still be playing in release phase, or because it has been sustained or sostenuto'ed.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |

**Returns:** TRUE if playing, FALSE otherwise

### `fluid_voice_is_on()` {#fluid_voice_is_on}

```c
int fluid_voice_is_on(const fluid_voice_t *voice)
```

Check if a voice is ON.

A voice is in ON state as soon as a call to [`fluid_synth_start_voice()`](voice-control.md#fluid_synth_start_voice) has been made (which is typically done in a fluid_preset_t's noteon function). A voice stays ON as long as it has not received a noteoff event.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |

**Returns:** TRUE if on, FALSE otherwise

**Since:** 1.1.7

### `fluid_voice_is_sustained()` {#fluid_voice_is_sustained}

```c
int fluid_voice_is_sustained(const fluid_voice_t *voice)
```

Check if a voice keeps playing after it has received a noteoff due to being held by sustain.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |

**Returns:** TRUE if sustained, FALSE otherwise

**Since:** 1.1.7

### `fluid_voice_is_sostenuto()` {#fluid_voice_is_sostenuto}

```c
int fluid_voice_is_sostenuto(const fluid_voice_t *voice)
```

Check if a voice keeps playing after it has received a noteoff due to being held by sostenuto.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |

**Returns:** TRUE if sostenuto, FALSE otherwise

**Since:** 1.1.7

### `fluid_voice_optimize_sample()` {#fluid_voice_optimize_sample}

```c
int fluid_voice_optimize_sample(fluid_sample_t *s)
```

Calculate the peak volume of a sample for voice off optimization.

**Parameters:**

| Name | Description |
|------|-------------|
| `s` | Sample to optimize |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

[`fluid_voice_optimize_sample()`](voices.md#fluid_voice_optimize_sample) on each sample once.

### `fluid_voice_update_param()` {#fluid_voice_update_param}

```c
void fluid_voice_update_param(fluid_voice_t *voice, int gen)
```

Update all the synthesis parameters which depend on generator *gen*.

**Parameters:**

| Name | Description |
|------|-------------|
| `voice` | Voice instance |
| `gen` | Generator id ([`fluid_gen_type`](generators.md#fluid_gen_type)) |
