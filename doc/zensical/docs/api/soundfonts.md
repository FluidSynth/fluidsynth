# SoundFonts

SoundFont related functions

This part of the API contains functions, defines and types that are mostly only used by internal or custom SoundFont loaders or client code that modifies loaded presets, SoundFonts or voices directly.

## Subgroups

- [SoundFont Generators](generators.md)

- [SoundFont Modulators](modulators.md)

- [SoundFont Loader](soundfont-loader.md)

- [Voice Manipulation](voices.md)

## Functions

### `fluid_synth_pin_preset()` {#fluid_synth_pin_preset}

```c
int fluid_synth_pin_preset(fluid_synth_t *synth, int sfont_id, int bank_num, int preset_num)
```

Pins all samples of the given preset.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `sfont_id` | ID of a loaded SoundFont |
| `bank_num` | MIDI bank number |
| `preset_num` | MIDI program number |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the preset was found, pinned and loaded into memory successfully. [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise. Note that [`FLUID_OK`](misc.md#FLUID_OK) is returned, even if `synth.dynamic-sample-loading` is disabled or the preset doesn't support dynamic-sample-loading.

> **Note:** This function is only useful if settings_synth_dynamic-sample-loading is enabled. By default, dynamic-sample-loading is disabled and all samples are kept in memory. Furthermore, this is only useful for presets which support dynamic-sample-loading (currently, only preset loaded with the default soundfont loader do).

**Since:** 2.2.0

### `fluid_synth_unpin_preset()` {#fluid_synth_unpin_preset}

```c
int fluid_synth_unpin_preset(fluid_synth_t *synth, int sfont_id, int bank_num, int preset_num)
```

Unpin all samples of the given preset.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `sfont_id` | ID of a loaded SoundFont |
| `bank_num` | MIDI bank number |
| `preset_num` | MIDI program number |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if preset was found, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

[`fluid_synth_pin_preset()`](soundfonts.md#fluid_synth_pin_preset). If the preset is not currently used, its samples will be unloaded.

> **Note:** Only useful for presets loaded with the default soundfont loader and only if settings_synth_dynamic-sample-loading is enabled.

**Since:** 2.2.0
