# Audio Driver

Functions for managing audio drivers.

Defines functions for creating audio driver output. Use [`new_fluid_audio_driver()`](audio-driver.md#new_fluid_audio_driver) to create a new audio driver for a given synth and configuration settings.

The function [`new_fluid_audio_driver2()`](audio-driver.md#new_fluid_audio_driver2) can be used if custom audio processing is desired before the audio is sent to the audio driver (although it is not as efficient).

**See also:** `Creating the audio driver`

## Types

### `fluid_audio_func_t` {#fluid_audio_func_t}

```c
typedef typedef int(* fluid_audio_func_t) (void *data, int len, int nfx, float *fx\[\], int nout, float *out\[\]);
```

Callback function type used with [`new_fluid_audio_driver2()`](audio-driver.md#new_fluid_audio_driver2) to allow for custom user audio processing before the audio is sent to the driver.

**Parameters:**

| Name | Description |
|------|-------------|
| `data` | The user data parameter as passed to [`new_fluid_audio_driver2()`](audio-driver.md#new_fluid_audio_driver2). |
| `len` | Count of audio frames to synthesize. |
| `nfx` | Count of arrays in `fx`. |
| `fx` | Array of buffers to store effects audio to. Buffers may alias with buffers of `out`. |
| `nout` | Count of arrays in `out`. |
| `out` | Array of buffers to store (dry) audio to. Buffers may alias with buffers of `fx`. |

**Returns:** Should return [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) if an error occurred.

[`fluid_synth_process()`](audio-rendering.md#fluid_synth_process).

> **Note:** Whereas [`fluid_synth_process()`](audio-rendering.md#fluid_synth_process) allows aliasing buffers, there is the guarantee that `out` and `fx` buffers provided by fluidsynth's audio drivers never alias. This prevents downstream applications from e.g. applying a custom effect accidentally to the same buffer multiple times.

> **Note:** Also note that the Jack driver is currently the only driver that has dedicated `fx` buffers (but only if settings_audio_jack_multi is true). All other drivers do not provide `fx` buffers. In this case, users are encouraged to mix the effects into the provided dry buffers when calling [`fluid_synth_process()`](audio-rendering.md#fluid_synth_process). ```c intmyCallback(void*,intlen,intnfx,float*fx\[\],intnout,float*out\[\]) { intret; if(nfx==0) { float*fxb\[4\]={out\[0\],out\[1\],out\[0\],out\[1\]}; ret=fluid_synth_process(synth,len,sizeof(fxb)/sizeof(fxb\[0\]),fxb,nout,out); } else { ret=fluid_synth_process(synth,len,nfx,fx,nout,out); } //...client-code... returnret; } ``` `fluidsynth_process.c` .

## Functions

### `new_fluid_audio_driver()` {#new_fluid_audio_driver}

```c
fluid_audio_driver_t * new_fluid_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
```

Create a new audio driver.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | Configuration settings used to select and create the audio driver. |
| `synth` | Synthesizer instance for which the audio driver is created for. |

**Returns:** The new audio driver instance or NULL on error

`synth` instance with a defined set of configuration `settings`. The `settings` instance must be the same that you have passed to [`new_fluid_synth()`](synth.md#new_fluid_synth) when creating the `synth` instance. Otherwise the behaviour is undefined.

> **Note:** As soon as an audio driver is created, the `synth` starts rendering audio. This means that all necessary initialization and sound-setup should have been completed before calling this function. Thus, of all object types in use (synth, midi player, sequencer, etc.) the audio driver should always be the last one to be created and the first one to be deleted! Also refer to the order of object creation in the code examples. Deleting and re-creating the audio driver is supported. However, only settings marked as realtime can reconfigure an already created `synth`.

### `new_fluid_audio_driver2()` {#new_fluid_audio_driver2}

```c
fluid_audio_driver_t * new_fluid_audio_driver2(fluid_settings_t *settings, fluid_audio_func_t func, void *data)
```

Create a new audio driver.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | Configuration settings used to select and create the audio driver. |
| `func` | Function called to fill audio buffers for audio playback |
| `data` | User defined data pointer to pass to `func` |

**Returns:** The new audio driver instance or NULL on error

[`new_fluid_audio_driver()`](audio-driver.md#new_fluid_audio_driver) but allows for custom audio processing before audio is sent to audio driver. It is the responsibility of the callback `func` to render the audio into the buffers. If `func` uses a fluid_synth_t `synth`, the `settings` instance must be the same that you have passed to [`new_fluid_synth()`](synth.md#new_fluid_synth) when creating the `synth` instance. Otherwise the behaviour is undefined.

> **Note:** Not as efficient as [`new_fluid_audio_driver()`](audio-driver.md#new_fluid_audio_driver).

> **Note:** As soon as an audio driver is created, a new thread is spawned starting to make callbacks to `func`. This means that all necessary sound-setup should be completed after this point, thus of all object types in use (synth, midi player, sequencer, etc.) the audio driver should always be the last one to be created and the first one to be deleted! Also refer to the order of object creation in the code examples.

### `delete_fluid_audio_driver()` {#delete_fluid_audio_driver}

```c
void delete_fluid_audio_driver(fluid_audio_driver_t *driver)
```

Deletes an audio driver instance.

**Parameters:**

| Name | Description |
|------|-------------|
| `driver` | Audio driver instance to delete |

### `fluid_audio_driver_register()` {#fluid_audio_driver_register}

```c
int fluid_audio_driver_register(const char **adrivers)
```

Registers audio drivers to use

**Parameters:**

| Name | Description |
|------|-------------|
| `adrivers` | NULL-terminated array of audio drivers to register. Pass NULL to register all available drivers. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if all the audio drivers requested by the user are supported by fluidsynth and have been successfully registered. Otherwise [`FLUID_FAILED`](misc.md#FLUID_FAILED) is returned and this function has no effect.

[`new_fluid_settings()`](settings.md#new_fluid_settings), all audio drivers are initialized once. In the past this has caused segfaults and application crashes due to buggy soundcard drivers.

This function enables the user to only initialize specific audio drivers when settings instances are created. Therefore pass a NULL-terminated array of C-strings containing the `names` of audio drivers to register for the usage with fluidsynth. The `names` are the same as being used for the `audio.driver` setting.

By default all audio drivers fluidsynth has been compiled with are registered, so calling this function is optional.

> **Warning:** This function may only be called if no thread is residing in fluidsynth's API and no instances of any kind are alive (e.g. as it would be the case right after fluidsynth's initial creation). Else the behaviour is undefined. Furthermore any attempt of using audio drivers that have not been registered is undefined behaviour!

> **Note:** This function is not thread safe and will never be!

**Since:** 1.1.9
