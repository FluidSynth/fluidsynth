# SoundFont Loader

Create custom SoundFont loaders

It is possible to add new SoundFont loaders to the synthesizer. This API allows for virtual SoundFont files to be loaded and synthesized, which may not actually be SoundFont files, as long as they can be represented by the SoundFont synthesis model.

To add a new SoundFont loader to the synthesizer, call [`fluid_synth_add_sfloader()`](soundfont-loader.md#fluid_synth_add_sfloader) and pass a pointer to an [`fluid_sfloader_t`](Types.md#fluid_sfloader_t) instance created by [`new_fluid_sfloader()`](soundfont-loader.md#new_fluid_sfloader). On creation, you must specify a callback function `load` that will be called for every file attempting to load it and if successful returns a [`fluid_sfont_t`](Types.md#fluid_sfont_t) instance, or NULL if it fails.

The [`fluid_sfont_t`](Types.md#fluid_sfont_t) structure contains a callback to obtain the name of the SoundFont. It contains two functions to iterate though the contained presets, and one function to obtain a preset corresponding to a bank and preset number. This function should return a [`fluid_preset_t`](Types.md#fluid_preset_t) instance.

The [`fluid_preset_t`](Types.md#fluid_preset_t) instance contains some functions to obtain information from the preset (name, bank, number). The most important callback is the noteon function. The noteon function is called by fluidsynth internally and should call [`fluid_synth_alloc_voice()`](voice-control.md#fluid_synth_alloc_voice) for every sample that has to be played. [`fluid_synth_alloc_voice()`](voice-control.md#fluid_synth_alloc_voice) expects a pointer to a [`fluid_sample_t`](Types.md#fluid_sample_t) instance and returns a pointer to the opaque [`fluid_voice_t`](Types.md#fluid_voice_t) structure. To set or increment the values of a generator, use [`fluid_voice_gen_set()`](voices.md#fluid_voice_gen_set) or [`fluid_voice_gen_incr()`](voices.md#fluid_voice_gen_incr). When you are finished initializing the voice call `fluid_voice_start()` to start playing the synthesis voice.

## Enumerations

### `` {#}

| Value | Description |
|-------|-------------|
| `FLUID_PRESET_SELECTED` |  |
| `FLUID_PRESET_UNSELECTED` |  |
| `FLUID_SAMPLE_DONE` |  |
| `FLUID_PRESET_PIN` |  |
| `FLUID_PRESET_UNPIN` |  |

Some notification enums for presets and samples.

### `fluid_sample_type` {#fluid_sample_type}

| Value | Description |
|-------|-------------|
| `FLUID_SAMPLETYPE_MONO` |  |
| `FLUID_SAMPLETYPE_RIGHT` |  |
| `FLUID_SAMPLETYPE_LEFT` |  |
| `FLUID_SAMPLETYPE_LINKED` |  |
| `FLUID_SAMPLETYPE_OGG_VORBIS` |  |
| `FLUID_SAMPLETYPE_ROM` |  |

Indicates the type of a sample used by the `_fluid_sample_t::sampletype` field.

This enum corresponds to the `SFSampleLink` enum in the SoundFont spec. One `flag` may be bit-wise OR-ed with one `value`.

## Types

### `fluid_sfloader_load_t` {#fluid_sfloader_load_t}

```c
typedef typedef fluid_sfont_t *(* fluid_sfloader_load_t) (fluid_sfloader_t *loader, const char *filename);
```

Method to load an instrument file (does not actually need to be a real file name, could be another type of string identifier that the *loader* understands).

**Parameters:**

| Name | Description |
|------|-------------|
| `loader` | SoundFont loader |
| `filename` | File name or other string identifier |

**Returns:** The loaded instrument file (SoundFont) or NULL if an error occurred.

### `fluid_sfloader_free_t` {#fluid_sfloader_free_t}

```c
typedef typedef void(* fluid_sfloader_free_t) (fluid_sfloader_t *loader);
```

The free method should free the memory allocated for a fluid_sfloader_t instance in addition to any private data.

**Parameters:**

| Name | Description |
|------|-------------|
| `loader` | SoundFont loader |

[`delete_fluid_sfloader()`](soundfont-loader.md#delete_fluid_sfloader) to ensure proper cleanup of the [`fluid_sfloader_t`](Types.md#fluid_sfloader_t) struct. If no private data needs to be freed, setting this to [`delete_fluid_sfloader()`](soundfont-loader.md#delete_fluid_sfloader) is sufficient.

### `fluid_sfloader_callback_open_t` {#fluid_sfloader_callback_open_t}

```c
typedef typedef void *(* fluid_sfloader_callback_open_t) (const char *filename);
```

Opens the file or memory indicated by `filename` in binary read mode.

**Returns:** returns a file handle on success, NULL otherwise

`filename` matches the string provided during the [`fluid_synth_sfload()`](soundfont-management.md#fluid_synth_sfload) call.

### `fluid_sfloader_callback_read_t` {#fluid_sfloader_callback_read_t}

```c
typedef typedef int(* fluid_sfloader_callback_read_t) (void *buf, fluid_long_long_t count, void *handle);
```

Reads `count` bytes to the specified buffer `buf`.

**Returns:** returns [`FLUID_OK`](misc.md#FLUID_OK) if exactly `count` bytes were successfully read, else returns [`FLUID_FAILED`](misc.md#FLUID_FAILED) and leaves *buf* unmodified.

### `fluid_sfloader_callback_seek_t` {#fluid_sfloader_callback_seek_t}

```c
typedef typedef int(* fluid_sfloader_callback_seek_t) (void *handle, fluid_long_long_t offset, int origin);
```

Same purpose and behaviour as fseek.

**Parameters:**

| Name | Description |
|------|-------------|
| `origin` | either `SEEK_SET`, `SEEK_CUR` or `SEEK_END` |

**Returns:** returns [`FLUID_OK`](misc.md#FLUID_OK) if the seek was successfully performed while not seeking beyond a buffer or file, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_sfloader_callback_close_t` {#fluid_sfloader_callback_close_t}

```c
typedef typedef int(* fluid_sfloader_callback_close_t) (void *handle);
```

Closes the handle returned by [`fluid_sfloader_callback_open_t`](soundfont-loader.md#fluid_sfloader_callback_open_t) and frees used resources.

**Returns:** returns [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) on error

### `fluid_sfloader_callback_tell_t` {#fluid_sfloader_callback_tell_t}

```c
typedef typedef fluid_long_long_t(* fluid_sfloader_callback_tell_t) (void *handle);
```

**Returns:** returns current file offset or [`FLUID_FAILED`](misc.md#FLUID_FAILED) on error

### `fluid_sfont_get_name_t` {#fluid_sfont_get_name_t}

```c
typedef typedef const char *(* fluid_sfont_get_name_t) (fluid_sfont_t *sfont);
```

Method to return the name of a virtual SoundFont.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | Virtual SoundFont |

**Returns:** The name of the virtual SoundFont.

### `fluid_sfont_get_preset_t` {#fluid_sfont_get_preset_t}

```c
typedef typedef fluid_preset_t *(* fluid_sfont_get_preset_t) (fluid_sfont_t *sfont, int bank, int prenum);
```

Get a virtual SoundFont preset by bank and program numbers.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | Virtual SoundFont |
| `bank` | MIDI bank number (0-16383) |
| `prenum` | MIDI preset number (0-127) |

**Returns:** Should return an allocated virtual preset or NULL if it could not be found.

### `fluid_sfont_iteration_start_t` {#fluid_sfont_iteration_start_t}

```c
typedef typedef void(* fluid_sfont_iteration_start_t) (fluid_sfont_t *sfont);
```

Start virtual SoundFont preset iteration method.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | Virtual SoundFont |

### `fluid_sfont_iteration_next_t` {#fluid_sfont_iteration_next_t}

```c
typedef typedef fluid_preset_t *(* fluid_sfont_iteration_next_t) (fluid_sfont_t *sfont);
```

Virtual SoundFont preset iteration function.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | Virtual SoundFont |

**Returns:** NULL when no more presets are available, otherwise the a pointer to the current preset

### `fluid_sfont_free_t` {#fluid_sfont_free_t}

```c
typedef typedef int(* fluid_sfont_free_t) (fluid_sfont_t *sfont);
```

Method to free a virtual SoundFont bank.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | Virtual SoundFont to free. |

**Returns:** Should return 0 when it was able to free all resources or non-zero if some of the samples could not be freed because they are still in use, in which case the free will be tried again later, until success.

[`delete_fluid_sfont()`](soundfont-loader.md#delete_fluid_sfont) to ensure proper cleanup of the [`fluid_sfont_t`](Types.md#fluid_sfont_t) struct. If no private data needs to be freed, setting this to [`delete_fluid_sfont()`](soundfont-loader.md#delete_fluid_sfont) is sufficient.

### `fluid_preset_get_name_t` {#fluid_preset_get_name_t}

```c
typedef typedef const char *(* fluid_preset_get_name_t) (fluid_preset_t *preset);
```

Method to get a virtual SoundFont preset name.

**Parameters:**

| Name | Description |
|------|-------------|
| `preset` | Virtual SoundFont preset |

**Returns:** Should return the name of the preset. The returned string must be valid for the duration of the virtual preset (or the duration of the SoundFont, in the case of preset iteration).

### `fluid_preset_get_banknum_t` {#fluid_preset_get_banknum_t}

```c
typedef typedef int(* fluid_preset_get_banknum_t) (fluid_preset_t *preset);
```

Method to get a virtual SoundFont preset MIDI bank number.

**Parameters:**

| Name | Description |
|------|-------------|
| `preset` | Virtual SoundFont preset |

**Returns:** The bank number of the preset

### `fluid_preset_get_num_t` {#fluid_preset_get_num_t}

```c
typedef typedef int(* fluid_preset_get_num_t) (fluid_preset_t *preset);
```

Method to get a virtual SoundFont preset MIDI program number.

**Parameters:**

| Name | Description |
|------|-------------|
| `preset` | Virtual SoundFont preset |

**Returns:** The program number of the preset

### `fluid_preset_noteon_t` {#fluid_preset_noteon_t}

```c
typedef typedef int(* fluid_preset_noteon_t) (fluid_preset_t *preset, fluid_synth_t *synth, int chan, int key, int vel);
```

Method to handle a noteon event (synthesize the instrument).

**Parameters:**

| Name | Description |
|------|-------------|
| `preset` | Virtual SoundFont preset |
| `synth` | Synthesizer instance |
| `chan` | MIDI channel number of the note on event |
| `key` | MIDI note number (0-127) |
| `vel` | MIDI velocity (0-127) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success (0) or [`FLUID_FAILED`](misc.md#FLUID_FAILED) (-1) otherwise

Call [`fluid_synth_alloc_voice()`](voice-control.md#fluid_synth_alloc_voice) for every sample that has to be played. [`fluid_synth_alloc_voice()`](voice-control.md#fluid_synth_alloc_voice) expects a pointer to a [`fluid_sample_t`](Types.md#fluid_sample_t) structure and returns a pointer to the opaque [`fluid_voice_t`](Types.md#fluid_voice_t) structure. To set or increment the values of a generator, use [`fluid_voice_gen_set()`](voices.md#fluid_voice_gen_set) or [`fluid_voice_gen_incr()`](voices.md#fluid_voice_gen_incr). When you are finished initializing the voice call `fluid_voice_start()` to start playing the synthesis voice. Starting with FluidSynth 1.1.0 all voices created will be started at the same time.

### `fluid_preset_free_t` {#fluid_preset_free_t}

```c
typedef typedef void(* fluid_preset_free_t) (fluid_preset_t *preset);
```

Method to free a virtual SoundFont preset.

**Parameters:**

| Name | Description |
|------|-------------|
| `preset` | Virtual SoundFont preset |

**Returns:** Should return 0

[`delete_fluid_preset()`](soundfont-loader.md#delete_fluid_preset) to ensure proper cleanup of the [`fluid_preset_t`](Types.md#fluid_preset_t) struct. If no private data needs to be freed, setting this to [`delete_fluid_preset()`](soundfont-loader.md#delete_fluid_preset) is sufficient.

## Functions

### `new_fluid_sfloader()` {#new_fluid_sfloader}

```c
fluid_sfloader_t * new_fluid_sfloader(fluid_sfloader_load_t load, fluid_sfloader_free_t free)
```

Creates a new SoundFont loader.

**Parameters:**

| Name | Description |
|------|-------------|
| `load` | Pointer to a function that provides a [`fluid_sfont_t`](Types.md#fluid_sfont_t) (see [`fluid_sfloader_load_t`](soundfont-loader.md#fluid_sfloader_load_t)). |
| `free` | Pointer to a function that destroys this instance (see [`fluid_sfloader_free_t`](soundfont-loader.md#fluid_sfloader_free_t)). Unless any private data needs to be freed it is sufficient to set this to [`delete_fluid_sfloader()`](soundfont-loader.md#delete_fluid_sfloader). |

**Returns:** the SoundFont loader instance on success, NULL otherwise.

### `delete_fluid_sfloader()` {#delete_fluid_sfloader}

```c
void delete_fluid_sfloader(fluid_sfloader_t *loader)
```

Frees a SoundFont loader created with [`new_fluid_sfloader()`](soundfont-loader.md#new_fluid_sfloader).

**Parameters:**

| Name | Description |
|------|-------------|
| `loader` | The SoundFont loader instance to free. |

### `new_fluid_defsfloader()` {#new_fluid_defsfloader}

```c
fluid_sfloader_t * new_fluid_defsfloader(fluid_settings_t *settings)
```

Creates a default soundfont2 loader that can be used with [`fluid_synth_add_sfloader()`](soundfont-loader.md#fluid_synth_add_sfloader). By default every synth instance has an initial default soundfont loader instance. Calling this function is usually only necessary to load a soundfont from memory, by providing custom callback functions via [`fluid_sfloader_set_callbacks()`](soundfont-loader.md#fluid_sfloader_set_callbacks).

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | A settings instance obtained by [`new_fluid_settings()`](settings.md#new_fluid_settings) |

**Returns:** A default soundfont2 loader struct

### `fluid_sfloader_set_callbacks()` {#fluid_sfloader_set_callbacks}

```c
int fluid_sfloader_set_callbacks(fluid_sfloader_t *loader, fluid_sfloader_callback_open_t open, fluid_sfloader_callback_read_t read, fluid_sfloader_callback_seek_t seek, fluid_sfloader_callback_tell_t tell, fluid_sfloader_callback_close_t close)
```

Set custom callbacks to be used upon soundfont loading.

**Parameters:**

| Name | Description |
|------|-------------|
| `loader` | The SoundFont loader instance. |
| `open` | A function implementing [`fluid_sfloader_callback_open_t`](soundfont-loader.md#fluid_sfloader_callback_open_t). |
| `read` | A function implementing [`fluid_sfloader_callback_read_t`](soundfont-loader.md#fluid_sfloader_callback_read_t). |
| `seek` | A function implementing [`fluid_sfloader_callback_seek_t`](soundfont-loader.md#fluid_sfloader_callback_seek_t). |
| `tell` | A function implementing [`fluid_sfloader_callback_tell_t`](soundfont-loader.md#fluid_sfloader_callback_tell_t). |
| `close` | A function implementing [`fluid_sfloader_callback_close_t`](soundfont-loader.md#fluid_sfloader_callback_close_t). |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the callbacks have been successfully set, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

*doc/fluidsynth_sfload_mem.c* as an example.

### `fluid_sfloader_set_data()` {#fluid_sfloader_set_data}

```c
int fluid_sfloader_set_data(fluid_sfloader_t *loader, void *data)
```

Specify private data to be used by [`fluid_sfloader_load_t`](soundfont-loader.md#fluid_sfloader_load_t).

**Parameters:**

| Name | Description |
|------|-------------|
| `loader` | The SoundFont loader instance. |
| `data` | The private data to store. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_sfloader_get_data()` {#fluid_sfloader_get_data}

```c
void * fluid_sfloader_get_data(fluid_sfloader_t *loader)
```

Obtain private data previously set with [`fluid_sfloader_set_data()`](soundfont-loader.md#fluid_sfloader_set_data).

**Parameters:**

| Name | Description |
|------|-------------|
| `loader` | The SoundFont loader instance. |

**Returns:** The private data or NULL if none explicitly set before.

### `new_fluid_sfont()` {#new_fluid_sfont}

```c
fluid_sfont_t * new_fluid_sfont(fluid_sfont_get_name_t get_name, fluid_sfont_get_preset_t get_preset, fluid_sfont_iteration_start_t iter_start, fluid_sfont_iteration_next_t iter_next, fluid_sfont_free_t free)
```

Creates a new virtual SoundFont instance structure.

**Parameters:**

| Name | Description |
|------|-------------|
| `get_name` | A function implementing [`fluid_sfont_get_name_t`](soundfont-loader.md#fluid_sfont_get_name_t). |
| `get_preset` | A function implementing [`fluid_sfont_get_preset_t`](soundfont-loader.md#fluid_sfont_get_preset_t). |
| `iter_start` | A function implementing [`fluid_sfont_iteration_start_t`](soundfont-loader.md#fluid_sfont_iteration_start_t), or NULL if preset iteration not needed. |
| `iter_next` | A function implementing [`fluid_sfont_iteration_next_t`](soundfont-loader.md#fluid_sfont_iteration_next_t), or NULL if preset iteration not needed. |
| `free` | A function implementing [`fluid_sfont_free_t`](soundfont-loader.md#fluid_sfont_free_t). |

**Returns:** The soundfont instance on success or NULL otherwise.

### `delete_fluid_sfont()` {#delete_fluid_sfont}

```c
int delete_fluid_sfont(fluid_sfont_t *sfont)
```

Destroys a SoundFont instance created with [`new_fluid_sfont()`](soundfont-loader.md#new_fluid_sfont).

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | The SoundFont instance to destroy. |

**Returns:** Always returns 0.

[`fluid_sfont_free_t`](soundfont-loader.md#fluid_sfont_free_t).

### `fluid_sfont_set_data()` {#fluid_sfont_set_data}

```c
int fluid_sfont_set_data(fluid_sfont_t *sfont, void *data)
```

Set private data to use with a SoundFont instance.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | The SoundFont instance. |
| `data` | The private data to store. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_sfont_get_data()` {#fluid_sfont_get_data}

```c
void * fluid_sfont_get_data(fluid_sfont_t *sfont)
```

Retrieve the private data of a SoundFont instance.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | The SoundFont instance. |

**Returns:** The private data or NULL if none explicitly set before.

### `fluid_sfont_get_default_mod()` {#fluid_sfont_get_default_mod}

```c
int fluid_sfont_get_default_mod(fluid_sfont_t *sfont, fluid_mod_t **mod_out)
```

Retrieve a deep copy of all default modulators attached to the provided `sfont` instance.

If a SoundFont has any default modulators set, the synth's default modulators (see [`fluid_synth_add_default_mod()`](synthesis-params.md#fluid_synth_add_default_mod)) will be ignored. A SF2 will have default modulators attached when the file contained a DMOD chunk. For a DLS file, fluidsynth will always add (custom) default modulators to allow the file being synthesized as accurately as possible.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | The SoundFont instance. |
| `mod_out` | A reference to a fluid_mod_t array. The provided pointer variable will be populated by fluidsynth to point to an array. The caller is responsible for freeing the array with [`fluid_free()`](misc.md#fluid_free). |

**Returns:** [`FLUID_FAILED`](misc.md#FLUID_FAILED) on error (e.g. out of memory). Otherwise it contains the number of modulators saved into the buffer. The caller must always free the buffer, even if the return value is zero!

**Since:** 2.5.0

> **Note:** This function involves memory allocation and is therefore not realtime safe!

> **Warning:** This function is not thread-safe. So only call this function when no synthesis thread is concurrently using this `sfont` instance!

### `fluid_sfont_set_default_mod()` {#fluid_sfont_set_default_mod}

```c
int fluid_sfont_set_default_mod(fluid_sfont_t *sfont, const fluid_mod_t *mods, int nmods)
```

Sets the default modulators of a SoundFont instance.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | The SoundFont instance. |
| `mods` | Pointer to an array of default modulators. A deep copy will be performed. |
| `nmods` | Number of modulators in the provided array. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

> **Note:** If `mods` is a zero-length array or `mods` is NULL, the default modulators attached to this SoundFont will be unset, causing the synth's default modulators to be added to voices again. The behavior is undefined if the array `mods` contains multiple identical modulators (i.e. [`fluid_mod_test_identity()`](modulators.md#fluid_mod_test_identity) evaluates to true). Further note that this function involves memory allocation and is therefore not realtime safe!

> **Warning:** This function is not thread-safe. So only call this function when no synthesis thread is concurrently using this `sfont` instance!

**Since:** 2.5.0

### `fluid_sfont_get_id()` {#fluid_sfont_get_id}

```c
int fluid_sfont_get_id(fluid_sfont_t *sfont)
```

Retrieve the unique ID of a SoundFont instance.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | The SoundFont instance. |

**Returns:** The SoundFont ID.

### `fluid_sfont_get_name()` {#fluid_sfont_get_name}

```c
const char * fluid_sfont_get_name(fluid_sfont_t *sfont)
```

Retrieve the name of a SoundFont instance.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | The SoundFont instance. |

**Returns:** The name of the SoundFont.

### `fluid_sfont_get_preset()` {#fluid_sfont_get_preset}

```c
fluid_preset_t * fluid_sfont_get_preset(fluid_sfont_t *sfont, int bank, int prenum)
```

Retrieve the preset assigned the a SoundFont instance for the given bank and preset number.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | The SoundFont instance. |
| `bank` | bank number of the preset |
| `prenum` | program number of the preset |

**Returns:** The preset instance or NULL if none found.

### `fluid_sfont_iteration_start()` {#fluid_sfont_iteration_start}

```c
void fluid_sfont_iteration_start(fluid_sfont_t *sfont)
```

Starts / re-starts virtual preset iteration in a SoundFont.

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | Virtual SoundFont instance |

### `fluid_sfont_iteration_next()` {#fluid_sfont_iteration_next}

```c
fluid_preset_t * fluid_sfont_iteration_next(fluid_sfont_t *sfont)
```

Virtual SoundFont preset iteration function.

Returns preset information to the caller and advances the internal iteration state to the next preset for subsequent calls. 

**Parameters:**

| Name | Description |
|------|-------------|
| `sfont` | The SoundFont instance. |

**Returns:** NULL when no more presets are available, otherwise the a pointer to the current preset

### `new_fluid_preset()` {#new_fluid_preset}

```c
fluid_preset_t * new_fluid_preset(fluid_sfont_t *parent_sfont, fluid_preset_get_name_t get_name, fluid_preset_get_banknum_t get_bank, fluid_preset_get_num_t get_num, fluid_preset_noteon_t noteon, fluid_preset_free_t free)
```

Create a virtual SoundFont preset instance.

**Parameters:**

| Name | Description |
|------|-------------|
| `parent_sfont` | The SoundFont instance this preset shall belong to |
| `get_name` | A function implementing [`fluid_preset_get_name_t`](soundfont-loader.md#fluid_preset_get_name_t) |
| `get_bank` | A function implementing [`fluid_preset_get_banknum_t`](soundfont-loader.md#fluid_preset_get_banknum_t) |
| `get_num` | A function implementing [`fluid_preset_get_num_t`](soundfont-loader.md#fluid_preset_get_num_t) |
| `noteon` | A function implementing [`fluid_preset_noteon_t`](soundfont-loader.md#fluid_preset_noteon_t) |
| `free` | A function implementing [`fluid_preset_free_t`](soundfont-loader.md#fluid_preset_free_t) |

**Returns:** The preset instance on success, NULL otherwise.

### `delete_fluid_preset()` {#delete_fluid_preset}

```c
void delete_fluid_preset(fluid_preset_t *preset)
```

Destroys a SoundFont preset instance created with [`new_fluid_preset()`](soundfont-loader.md#new_fluid_preset).

**Parameters:**

| Name | Description |
|------|-------------|
| `preset` | The SoundFont preset instance to destroy. |

[`fluid_preset_free_t`](soundfont-loader.md#fluid_preset_free_t).

### `fluid_preset_set_data()` {#fluid_preset_set_data}

```c
int fluid_preset_set_data(fluid_preset_t *preset, void *data)
```

Set private data to use with a SoundFont preset instance.

**Parameters:**

| Name | Description |
|------|-------------|
| `preset` | The SoundFont preset instance. |
| `data` | The private data to store. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_preset_get_data()` {#fluid_preset_get_data}

```c
void * fluid_preset_get_data(fluid_preset_t *preset)
```

Retrieve the private data of a SoundFont preset instance.

**Parameters:**

| Name | Description |
|------|-------------|
| `preset` | The SoundFont preset instance. |

**Returns:** The private data or NULL if none explicitly set before.

### `fluid_preset_get_name()` {#fluid_preset_get_name}

```c
const char * fluid_preset_get_name(fluid_preset_t *preset)
```

Retrieves the presets name by executing the `get_name` function provided on its creation.

**Parameters:**

| Name | Description |
|------|-------------|
| `preset` | The SoundFont preset instance. |

**Returns:** Pointer to a NULL-terminated string containing the presets name.

### `fluid_preset_get_banknum()` {#fluid_preset_get_banknum}

```c
int fluid_preset_get_banknum(fluid_preset_t *preset)
```

Retrieves the presets bank number by executing the `get_bank` function provided on its creation.

**Parameters:**

| Name | Description |
|------|-------------|
| `preset` | The SoundFont preset instance. |

**Returns:** The bank number of `preset`.

### `fluid_preset_get_num()` {#fluid_preset_get_num}

```c
int fluid_preset_get_num(fluid_preset_t *preset)
```

Retrieves the presets (instrument) number by executing the `get_num` function provided on its creation.

**Parameters:**

| Name | Description |
|------|-------------|
| `preset` | The SoundFont preset instance. |

**Returns:** The number of `preset`.

### `fluid_preset_get_sfont()` {#fluid_preset_get_sfont}

```c
fluid_sfont_t * fluid_preset_get_sfont(fluid_preset_t *preset)
```

Retrieves the presets parent SoundFont instance.

**Parameters:**

| Name | Description |
|------|-------------|
| `preset` | The SoundFont preset instance. |

**Returns:** The parent SoundFont of `preset`.

### `new_fluid_sample()` {#new_fluid_sample}

```c
fluid_sample_t * new_fluid_sample(void)
```

Create a new sample instance.

**Returns:** The sample on success, NULL otherwise.

### `delete_fluid_sample()` {#delete_fluid_sample}

```c
void delete_fluid_sample(fluid_sample_t *sample)
```

Destroy a sample instance previously created with [`new_fluid_sample()`](soundfont-loader.md#new_fluid_sample).

**Parameters:**

| Name | Description |
|------|-------------|
| `sample` | The sample to destroy. |

### `fluid_sample_sizeof()` {#fluid_sample_sizeof}

```c
size_t fluid_sample_sizeof(void)
```

Returns the size of the fluid_sample_t structure.

**Returns:** Size of fluid_sample_t in bytes

> **Note:** It is recommend to zero initialize the memory before using the object.

> **Warning:** Do NOT allocate samples on the stack and assign them to a voice!

### `fluid_sample_set_name()` {#fluid_sample_set_name}

```c
int fluid_sample_set_name(fluid_sample_t *sample, const char *name)
```

Set the name of a SoundFont sample.

**Parameters:**

| Name | Description |
|------|-------------|
| `sample` | SoundFont sample |
| `name` | Name to assign to sample (20 chars in length + zero terminator) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_sample_set_sound_data()` {#fluid_sample_set_sound_data}

```c
int fluid_sample_set_sound_data(fluid_sample_t *sample, short *data, char *data24, unsigned int nbframes, unsigned int sample_rate, short copy_data)
```

Assign sample data to a SoundFont sample.

**Parameters:**

| Name | Description |
|------|-------------|
| `sample` | SoundFont sample |
| `data` | Buffer containing 16 bit (mono-)audio sample data |
| `data24` | If not NULL, pointer to the least significant byte counterparts of each sample data point in order to create 24 bit audio samples |
| `nbframes` | Number of samples in *data* |
| `sample_rate` | Sampling rate of the sample data |
| `copy_data` | TRUE to copy the sample data (and automatically free it upon [`delete_fluid_sample()`](soundfont-loader.md#delete_fluid_sample)), FALSE to use it directly (and not free it) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** If *copy_data* is FALSE, data should have 8 unused frames at start and 8 unused frames at the end and *nbframes* should be >=48

### `fluid_sample_set_loop()` {#fluid_sample_set_loop}

```c
int fluid_sample_set_loop(fluid_sample_t *sample, unsigned int loop_start, unsigned int loop_end)
```

Set the loop of a sample.

**Parameters:**

| Name | Description |
|------|-------------|
| `sample` | SoundFont sample |
| `loop_start` | Start sample index of the loop. |
| `loop_end` | End index of the loop (must be a valid sample as it marks the last sample to be played). |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_sample_set_pitch()` {#fluid_sample_set_pitch}

```c
int fluid_sample_set_pitch(fluid_sample_t *sample, int root_key, int fine_tune)
```

Set the pitch of a sample.

**Parameters:**

| Name | Description |
|------|-------------|
| `sample` | SoundFont sample |
| `root_key` | Root MIDI note of sample (0-127) |
| `fine_tune` | Fine tune in cents |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_add_sfloader()` {#fluid_synth_add_sfloader}

```c
void fluid_synth_add_sfloader(fluid_synth_t *synth, fluid_sfloader_t *loader)
```

Add a SoundFont loader to the synth. This function takes ownership of `loader` and frees it automatically upon `synth` destruction. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `loader` | Loader API structure |

> **Note:** Should only be called before any SoundFont files are loaded.

### `fluid_synth_get_channel_preset()` {#fluid_synth_get_channel_preset}

```c
fluid_preset_t * fluid_synth_get_channel_preset(fluid_synth_t *synth, int chan)
```

Get active preset on a MIDI channel. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |

**Returns:** Preset or NULL if no preset active on `chan`

> **Note:** Should only be called from within synthesis thread, which includes SoundFont loader preset noteon methods. Not thread safe otherwise.
