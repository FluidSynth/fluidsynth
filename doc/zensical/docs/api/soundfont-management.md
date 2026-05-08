# SoundFont Management

Functions to load and unload SoundFonts.

## Functions

### `fluid_synth_sfload()` {#fluid_synth_sfload}

```c
int fluid_synth_sfload(fluid_synth_t *synth, const char *filename, int reset_presets)
```

Load a SoundFont file.

The `filename` is passed onto and interpreted by the SoundFont loaders. On success, the newly loaded SoundFont will be put on top of the SoundFont stack. Presets are searched starting from the SoundFont on the top of the stack, working the way down the stack until a preset is found.

If the SoundFont is structural defect, it will be rejected and the function will fail.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `filename` | File to load |
| `reset_presets` | TRUE to re-assign presets for all MIDI channels (equivalent to calling [`fluid_synth_program_reset()`](midi-messages.md#fluid_synth_program_reset)) |

**Returns:** SoundFont ID on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) on error

> **Note:** Since FluidSynth 2.2.0 `filename` is treated as an UTF8 encoded string on Windows. FluidSynth will convert it to wide-char internally and then pass it to `_wfopen()`. Before FluidSynth 2.2.0, `filename` was treated as ANSI string on Windows. All other platforms directly pass it to `fopen()` without any conversion (usually, UTF8 is accepted).

### `fluid_synth_sfreload()` {#fluid_synth_sfreload}

```c
int fluid_synth_sfreload(fluid_synth_t *synth, int id)
```

Reload a SoundFont. The SoundFont retains its ID and index on the SoundFont stack. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `id` | ID of SoundFont to reload |

**Returns:** SoundFont ID on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) on error

### `fluid_synth_sfunload()` {#fluid_synth_sfunload}

```c
int fluid_synth_sfunload(fluid_synth_t *synth, int id, int reset_presets)
```

Schedule a SoundFont for unloading.

If the SoundFont isn't used anymore by any playing voices, it will be unloaded immediately.

If any samples of the given SoundFont are still required by active voices, the SoundFont will be unloaded in a lazy manner, once those voices have finished synthesizing. If you call [`delete_fluid_synth()`](synth.md#delete_fluid_synth), all voices will be destroyed and the SoundFont will be unloaded in any case. Once this function returned, [`fluid_synth_sfcount()`](soundfont-management.md#fluid_synth_sfcount) and similar functions will behave as if the SoundFont has already been unloaded, even though the lazy-unloading is still pending.

> **Note:** This lazy-unloading mechanism was broken between FluidSynth 1.1.4 and 2.1.5 . As a consequence, SoundFonts scheduled for lazy-unloading may be never freed under certain conditions. Calling [`delete_fluid_synth()`](synth.md#delete_fluid_synth) does not recover this situation either.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `id` | ID of SoundFont to unload |
| `reset_presets` | TRUE to re-assign presets for all MIDI channels |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the given `id` was found, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_add_sfont()` {#fluid_synth_add_sfont}

```c
int fluid_synth_add_sfont(fluid_synth_t *synth, fluid_sfont_t *sfont)
```

Add a SoundFont. The SoundFont will be added to the top of the SoundFont stack and ownership is transferred to `synth`. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `sfont` | SoundFont to add |

**Returns:** New assigned SoundFont ID or [`FLUID_FAILED`](misc.md#FLUID_FAILED) on error

### `fluid_synth_remove_sfont()` {#fluid_synth_remove_sfont}

```c
int fluid_synth_remove_sfont(fluid_synth_t *synth, fluid_sfont_t *sfont)
```

Remove a SoundFont from the SoundFont stack without deleting it. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `sfont` | SoundFont to remove |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if `sfont` successfully removed, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** The SoundFont should only be freed after there are no presets referencing it. This can only be ensured by the SoundFont loader and therefore this function should not normally be used.

### `fluid_synth_sfcount()` {#fluid_synth_sfcount}

```c
int fluid_synth_sfcount(fluid_synth_t *synth)
```

Count number of loaded SoundFont files. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Count of loaded SoundFont files.

### `fluid_synth_get_sfont()` {#fluid_synth_get_sfont}

```c
fluid_sfont_t * fluid_synth_get_sfont(fluid_synth_t *synth, unsigned int num)
```

Get SoundFont by index. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `num` | SoundFont index on the stack (starting from 0 for top of stack). |

**Returns:** SoundFont instance or NULL if invalid index

> **Note:** Caller should be certain that SoundFont is not deleted (unloaded) for the duration of use of the returned pointer.

### `fluid_synth_get_sfont_by_id()` {#fluid_synth_get_sfont_by_id}

```c
fluid_sfont_t * fluid_synth_get_sfont_by_id(fluid_synth_t *synth, int id)
```

Get SoundFont by ID. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `id` | SoundFont ID |

**Returns:** SoundFont instance or NULL if invalid ID

> **Note:** Caller should be certain that SoundFont is not deleted (unloaded) for the duration of use of the returned pointer.

### `fluid_synth_get_sfont_by_name()` {#fluid_synth_get_sfont_by_name}

```c
fluid_sfont_t * fluid_synth_get_sfont_by_name(fluid_synth_t *synth, const char *name)
```

Get SoundFont by name. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `name` | Name of SoundFont |

**Returns:** SoundFont instance or NULL if invalid name

**Since:** 1.1.0

> **Note:** Caller should be certain that SoundFont is not deleted (unloaded) for the duration of use of the returned pointer.

### `fluid_synth_set_bank_offset()` {#fluid_synth_set_bank_offset}

```c
int fluid_synth_set_bank_offset(fluid_synth_t *synth, int sfont_id, int offset)
```

Offset the bank numbers of a loaded SoundFont, i.e. subtract `offset` from any bank number when assigning instruments.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `sfont_id` | ID of a loaded SoundFont |
| `offset` | Bank offset value to apply to all instruments |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the offset was set successfully, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_get_bank_offset()` {#fluid_synth_get_bank_offset}

```c
int fluid_synth_get_bank_offset(fluid_synth_t *synth, int sfont_id)
```

Get bank offset of a loaded SoundFont. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `sfont_id` | ID of a loaded SoundFont |

**Returns:** SoundFont bank offset value
