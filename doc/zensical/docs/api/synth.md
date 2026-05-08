# Synthesizer

SoundFont synthesizer

You create a new synthesizer with [`new_fluid_synth()`](synth.md#new_fluid_synth) and you destroy it with [`delete_fluid_synth()`](synth.md#delete_fluid_synth). Use the fluid_settings_t structure to specify the synthesizer characteristics.

You have to load a SoundFont in order to hear any sound. For that you use the [`fluid_synth_sfload()`](soundfont-management.md#fluid_synth_sfload) function.

You can use the audio driver functions to open the audio device and create a background audio thread.

The API for sending MIDI events is probably what you expect: [`fluid_synth_noteon()`](midi-messages.md#fluid_synth_noteon), [`fluid_synth_noteoff()`](midi-messages.md#fluid_synth_noteoff), ...

## Subgroups

- [Effect - LADSPA](ladspa.md)

- [MIDI Channel Messages](midi-messages.md)

- [Synthesis Voice Control](voice-control.md)

- [SoundFont Management](soundfont-management.md)

- [Effect - Reverb](reverb-effect.md)

- [Effect - Chorus](chorus-effect.md)

- [Synthesis Parameters](synthesis-params.md)

- [MIDI Tuning](tuning.md)

- [Audio Rendering](audio-rendering.md)

- [Effect - IIR Filter](iir-filter.md)

- [MIDI Channel Setup](channel-setup.md)

## Functions

### `new_fluid_synth()` {#new_fluid_synth}

```c
fluid_synth_t * new_fluid_synth(fluid_settings_t *settings)
```

Create new FluidSynth instance. 

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | Configuration parameters to use (used directly). |

**Returns:** New FluidSynth instance or NULL on error

> **Note:** The `settings` parameter is used directly, but the synth does not take ownership of it. Hence, the caller is responsible for freeing it, when no longer needed. Further note that you may modify FluidSettings of the `settings` instance. However, only synth settings marked as realtime in fluidsettings will affect the synth at runtime. [`fluid_settings_is_realtime()`](settings.md#fluid_settings_is_realtime) can be used to check this.

> **Warning:** The `settings` object should only be used by a single synth at a time. I.e. creating multiple synth instances with a single `settings` object causes undefined behavior. Once the "single synth" has been deleted, you may use the `settings` object again for another synth.

### `delete_fluid_synth()` {#delete_fluid_synth}

```c
void delete_fluid_synth(fluid_synth_t *synth)
```

Delete a FluidSynth instance. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance to delete |

> **Note:** Other users of a synthesizer instance, such as audio and MIDI drivers, should be deleted prior to freeing the FluidSynth instance.

### `fluid_synth_get_cpu_load()` {#fluid_synth_get_cpu_load}

```c
double fluid_synth_get_cpu_load(fluid_synth_t *synth)
```

Get the synth CPU load value. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Estimated CPU load value in percent (0-100)

### `fluid_synth_error()` {#fluid_synth_error}

```c
const char * fluid_synth_error(fluid_synth_t *synth)
```

Get a textual representation of the last error 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Pointer to string of last error message. Valid until the same calling thread calls another FluidSynth function which fails. String is internal and should not be modified or freed.

Deprecated

This function is not thread-safe and does not work with multiple synths. It has been deprecated. It may return "" in a future release and will eventually be removed.
