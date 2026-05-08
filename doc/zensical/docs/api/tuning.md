# MIDI Tuning

The functions in this section implement the MIDI Tuning Standard interface.

## Functions

### `fluid_synth_activate_key_tuning()` {#fluid_synth_activate_key_tuning}

```c
int fluid_synth_activate_key_tuning(fluid_synth_t *synth, int bank, int prog, const char *name, const double *pitch, int apply)
```

Set the tuning of the entire MIDI note scale. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `bank` | Tuning bank number (0-127), not related to MIDI instrument bank |
| `prog` | Tuning preset number (0-127), not related to MIDI instrument program |
| `name` | Label name for this tuning |
| `pitch` | Array of pitch values (length of 128, each value is number of cents, for example normally note 0 is 0.0, 1 is 100.0, 60 is 6000.0, etc). Pass NULL to create a equal tempered (normal) scale. |
| `apply` | TRUE to apply new tuning in realtime to existing notes which are using the replaced tuning (if any), FALSE otherwise |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.1.0

### `fluid_synth_activate_octave_tuning()` {#fluid_synth_activate_octave_tuning}

```c
int fluid_synth_activate_octave_tuning(fluid_synth_t *synth, int bank, int prog, const char *name, const double *pitch, int apply)
```

Activate an octave tuning on every octave in the MIDI note scale. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `bank` | Tuning bank number (0-127), not related to MIDI instrument bank |
| `prog` | Tuning preset number (0-127), not related to MIDI instrument program |
| `name` | Label name for this tuning |
| `pitch` | Array of pitch values (length of 12 for each note of an octave starting at note C, values are number of offset cents to add to the normal tuning amount) |
| `apply` | TRUE to apply new tuning in realtime to existing notes which are using the replaced tuning (if any), FALSE otherwise |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.1.0

### `fluid_synth_tune_notes()` {#fluid_synth_tune_notes}

```c
int fluid_synth_tune_notes(fluid_synth_t *synth, int bank, int prog, int len, const int *keys, const double *pitch, int apply)
```

Set tuning values for one or more MIDI notes for an existing tuning. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `bank` | Tuning bank number (0-127), not related to MIDI instrument bank |
| `prog` | Tuning preset number (0-127), not related to MIDI instrument program |
| `len` | Number of MIDI notes to assign |
| `key` | Array of MIDI key numbers (length of 'len', values 0-127) |
| `pitch` | Array of pitch values (length of 'len', values are number of cents from MIDI note 0) |
| `apply` | TRUE to apply tuning change in realtime to existing notes using the specified tuning, FALSE otherwise |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** Prior to version 1.1.0 it was an error to specify a tuning that didn't already exist. Starting with 1.1.0, the default equal tempered scale will be used as a basis, if no tuning exists for the given bank and prog.

### `fluid_synth_activate_tuning()` {#fluid_synth_activate_tuning}

```c
int fluid_synth_activate_tuning(fluid_synth_t *synth, int chan, int bank, int prog, int apply)
```

Activate a tuning scale on a MIDI channel. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `bank` | Tuning bank number (0-127), not related to MIDI instrument bank |
| `prog` | Tuning preset number (0-127), not related to MIDI instrument program |
| `apply` | TRUE to apply tuning change to active notes, FALSE otherwise |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.1.0

> **Note:** A default equal tempered scale will be created, if no tuning exists on the given bank and prog.

### `fluid_synth_deactivate_tuning()` {#fluid_synth_deactivate_tuning}

```c
int fluid_synth_deactivate_tuning(fluid_synth_t *synth, int chan, int apply)
```

Clear tuning scale on a MIDI channel (use default equal tempered scale). 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `apply` | TRUE to apply tuning change to active notes, FALSE otherwise |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.1.0

### `fluid_synth_tuning_iteration_start()` {#fluid_synth_tuning_iteration_start}

```c
void fluid_synth_tuning_iteration_start(fluid_synth_t *synth)
```

Start tuning iteration. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

### `fluid_synth_tuning_iteration_next()` {#fluid_synth_tuning_iteration_next}

```c
int fluid_synth_tuning_iteration_next(fluid_synth_t *synth, int *bank, int *prog)
```

Advance to next tuning. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `bank` | Location to store MIDI bank number of next tuning scale |
| `prog` | Location to store MIDI program number of next tuning scale |

**Returns:** 1 if tuning iteration advanced, 0 if no more tunings

### `fluid_synth_tuning_dump()` {#fluid_synth_tuning_dump}

```c
int fluid_synth_tuning_dump(fluid_synth_t *synth, int bank, int prog, char *name, int len, double *pitch)
```

Get the entire note tuning for a given MIDI bank and program. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `bank` | MIDI bank number of tuning |
| `prog` | MIDI program number of tuning |
| `name` | Location to store tuning name or NULL to ignore |
| `len` | Maximum number of chars to store to 'name' (including NULL byte) |
| `pitch` | Array to store tuning scale to or NULL to ignore (len of 128) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if matching tuning was found, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise
