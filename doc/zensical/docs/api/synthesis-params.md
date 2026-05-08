# Synthesis Parameters

Functions to control and query synthesis parameters like gain and polyphony count.

## Enumerations

### `fluid_interp` {#fluid_interp}

| Value | Description |
|-------|-------------|
| `FLUID_INTERP_NONE` |  |
| `FLUID_INTERP_LINEAR` |  |
| `FLUID_INTERP_4THORDER` |  |
| `FLUID_INTERP_7THORDER` |  |
| `FLUID_INTERP_DEFAULT` |  |
| `FLUID_INTERP_HIGHEST` |  |

Synthesis interpolation method.

### `fluid_synth_add_mod` {#fluid_synth_add_mod}

| Value | Description |
|-------|-------------|
| `FLUID_SYNTH_OVERWRITE` |  |
| `FLUID_SYNTH_ADD` |  |

Enum used with [`fluid_synth_add_default_mod()`](synthesis-params.md#fluid_synth_add_default_mod) to specify how to handle duplicate modulators.

## Functions

### `fluid_synth_count_midi_channels()` {#fluid_synth_count_midi_channels}

```c
int fluid_synth_count_midi_channels(fluid_synth_t *synth)
```

Get the total count of MIDI channels. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Count of MIDI channels

### `fluid_synth_count_audio_channels()` {#fluid_synth_count_audio_channels}

```c
int fluid_synth_count_audio_channels(fluid_synth_t *synth)
```

Get the total count of audio channels. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Count of audio channel stereo pairs (1 = 2 channels, 2 = 4, etc)

### `fluid_synth_count_audio_groups()` {#fluid_synth_count_audio_groups}

```c
int fluid_synth_count_audio_groups(fluid_synth_t *synth)
```

Get the total number of allocated audio channels. Usually identical to the number of audio channels. Can be employed by LADSPA effects subsystem.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Count of audio group stereo pairs (1 = 2 channels, 2 = 4, etc)

### `fluid_synth_count_effects_channels()` {#fluid_synth_count_effects_channels}

```c
int fluid_synth_count_effects_channels(fluid_synth_t *synth)
```

Get the total number of allocated effects channels. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Count of allocated effects channels

### `fluid_synth_count_effects_groups()` {#fluid_synth_count_effects_groups}

```c
int fluid_synth_count_effects_groups(fluid_synth_t *synth)
```

Get the total number of allocated effects units.

This is the same number as initially provided by the setting settings_synth_effects-groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Count of allocated effects units

### `fluid_synth_set_sample_rate()` {#fluid_synth_set_sample_rate}

```c
void fluid_synth_set_sample_rate(fluid_synth_t *synth, float sample_rate)
```

Set up an event to change the sample-rate of the synth during the next rendering call. 

> **Warning:** This function is broken-by-design! Don't use it! Starting with fluidsynth 2.4.4 it's a no-op. Instead, specify the sample-rate when creating the synth.

Deprecated

As of fluidsynth 2.1.0 this function has been deprecated. Changing the sample-rate is generally not considered to be a realtime use-case, as it always produces some audible artifact ("click", "pop") on the dry sound and effects (because LFOs for chorus and reverb need to be reinitialized). The sample-rate change may also require memory allocation deep down in the effect units. However, this memory allocation may fail and there is no way for the caller to know that, because the actual change of the sample-rate is executed during rendering. This function cannot (must not) do the sample-rate change itself, otherwise the synth needs to be locked down, causing rendering to block. Esp. do not use this function if this `synth` instance is used by an audio driver, because the audio driver cannot be notified by this sample-rate change. Long story short: don't use it.

```c
fluid_synth_t*synth;//assumeinitialized
//\[...\]
//sample-ratechangeneeded?Deletetheaudiodriver,ifany.
delete_fluid_audio_driver(adriver);
//thendeletethesynth
delete_fluid_synth(synth);
//updatethesample-rate
fluid_settings_setnum(settings,"synth.sample-rate",22050.0);
//andre-createobjects
synth=new_fluid_synth(settings);
adriver=new_fluid_audio_driver(settings,synth);
```

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `sample_rate` | New sample-rate (Hz) |

**Since:** 1.1.2

### `fluid_synth_set_gain()` {#fluid_synth_set_gain}

```c
void fluid_synth_set_gain(fluid_synth_t *synth, float gain)
```

Set synth output gain value. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `gain` | Gain value (function clamps value to the range 0.0 to 10.0) |

### `fluid_synth_get_gain()` {#fluid_synth_get_gain}

```c
float fluid_synth_get_gain(fluid_synth_t *synth)
```

Get synth output gain value. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Synth gain value (0.0 to 10.0)

### `fluid_synth_set_polyphony()` {#fluid_synth_set_polyphony}

```c
int fluid_synth_set_polyphony(fluid_synth_t *synth, int polyphony)
```

Set synthesizer polyphony (max number of voices). 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `polyphony` | Polyphony to assign |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.0.6

### `fluid_synth_get_polyphony()` {#fluid_synth_get_polyphony}

```c
int fluid_synth_get_polyphony(fluid_synth_t *synth)
```

Get current synthesizer polyphony (max number of voices). 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Synth polyphony value.

**Since:** 1.0.6

### `fluid_synth_get_active_voice_count()` {#fluid_synth_get_active_voice_count}

```c
int fluid_synth_get_active_voice_count(fluid_synth_t *synth)
```

Get current number of active voices.

I.e. the no. of voices that have been started and have not yet finished. Unless called from synthesis context, this number does not necessarily have to be equal to the number of voices currently processed by the DSP loop, see below. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Number of currently active voices.

**Since:** 1.1.0

> **Note:** To generate accurate continuous statistics of the voice count, caller should ensure this function is called synchronously with the audio synthesis process. This can be done in the [`new_fluid_audio_driver2()`](audio-driver.md#new_fluid_audio_driver2) audio callback function for example. Otherwise every call to this function may return different voice counts as it may change after any (concurrent) call to fluid_synth_write_*() made by e.g. an audio driver or the applications audio rendering thread.

### `fluid_synth_get_internal_bufsize()` {#fluid_synth_get_internal_bufsize}

```c
int fluid_synth_get_internal_bufsize(fluid_synth_t *synth)
```

Get the internal synthesis buffer size value. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Internal buffer size in audio frames.

### `fluid_synth_set_interp_method()` {#fluid_synth_set_interp_method}

```c
int fluid_synth_set_interp_method(fluid_synth_t *synth, int chan, int interp_method)
```

Set synthesis interpolation method on one or all MIDI channels. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel to set interpolation method on or -1 for all channels |
| `interp_method` | Interpolation method ([`fluid_interp`](synthesis-params.md#fluid_interp)) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_add_default_mod()` {#fluid_synth_add_default_mod}

```c
int fluid_synth_add_default_mod(fluid_synth_t *synth, const fluid_mod_t *mod, int mode)
```

Adds the specified modulator `mod` as default modulator to the synth. `mod` will take effect for any subsequently created voice. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `mod` | Modulator info (values copied, passed in object can be freed immediately afterwards) |
| `mode` | Determines how to handle an existing identical modulator ([`fluid_synth_add_mod`](synthesis-params.md#fluid_synth_add_mod)) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** Not realtime safe (due to internal memory allocation) and therefore should not be called from synthesis context at the risk of stalling audio output.

### `fluid_synth_remove_default_mod()` {#fluid_synth_remove_default_mod}

```c
int fluid_synth_remove_default_mod(fluid_synth_t *synth, const fluid_mod_t *mod)
```

Removes the specified modulator `mod` from the synth's default modulator list. [`fluid_mod_test_identity()`](modulators.md#fluid_mod_test_identity) will be used to test modulator matching. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | synth instance |
| `mod` | The modulator to remove |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if a matching modulator was found and successfully removed, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** Not realtime safe (due to internal memory freeing) and therefore should not be called from synthesis context at the risk of stalling audio output.
