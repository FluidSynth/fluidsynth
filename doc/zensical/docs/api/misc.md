# Miscellaneous

Miscellaneous utility functions and defines

## Functions

### `fluid_is_soundfont()` {#fluid_is_soundfont}

```c
int fluid_is_soundfont(const char *filename)
```

Check if a file is a SoundFont file.

**Parameters:**

| Name | Description |
|------|-------------|
| `filename` | Path to the file to check |

**Returns:** TRUE if it could be a SF2, SF3 or DLS file, FALSE otherwise

`fopen()`, `fread()` and `fseek()` to identify known Soundfont formats. If fluidsynth was built with DLS support, this function will also identify DLS files.

> **Note:** This function only checks whether certain RIFF chunks are present in the file. A call to [`fluid_synth_sfload()`](soundfont-management.md#fluid_synth_sfload) might still fail, as it imposes much stricter structural checks.

### `fluid_is_midifile()` {#fluid_is_midifile}

```c
int fluid_is_midifile(const char *filename)
```

Check if a file is a MIDI file. 

**Parameters:**

| Name | Description |
|------|-------------|
| `filename` | Path to the file to check |

**Returns:** TRUE if it could be a MIDI file, FALSE otherwise

### `fluid_free()` {#fluid_free}

```c
void fluid_free(void *ptr)
```

Wrapper for free() that satisfies at least C90 requirements.

**Parameters:**

| Name | Description |
|------|-------------|
| `ptr` | Pointer to memory region that should be freed |

> **Note:** Only use this function when the API documentation explicitly says so. Otherwise use adequate `delete_fluid_*` functions.

> **Warning:** Calling C-std lib `free()` on memory that is advised to be freed with [`fluid_free()`](misc.md#fluid_free) results in undefined behaviour! (cf.: "Potential Errors Passing CRT Objects Across DLL Boundaries" found in MS Docs)

**Since:** 2.0.7

## Macros

### `FLUID_OK` {#FLUID_OK}

Value that indicates success, used by most libfluidsynth functions.

> **Note:** This was not publicly defined prior to libfluidsynth 1.1.0. When writing code which should also be compatible with older versions, something like the following can be used:

```c
#include<fluidsynth.h>

#ifndefFLUID_OK
#defineFLUID_OK(0)
#defineFLUID_FAILED(-1)
#endif
```

**Since:** 1.1.0

### `FLUID_FAILED` {#FLUID_FAILED}

Value that indicates failure, used by most libfluidsynth functions.

> **Note:** See [`FLUID_OK`](misc.md#FLUID_OK) for more details.

**Since:** 1.1.0
