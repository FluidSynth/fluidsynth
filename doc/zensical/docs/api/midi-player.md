# MIDI File Player

Parse standard MIDI files and emit MIDI events.

## Enumerations

### `fluid_player_status` {#fluid_player_status}

| Value | Description |
|-------|-------------|
| `FLUID_PLAYER_READY` |  |
| `FLUID_PLAYER_PLAYING` |  |
| `FLUID_PLAYER_STOPPING` |  |
| `FLUID_PLAYER_DONE` |  |

MIDI File Player status enum. 

**Since:** 1.1.0

### `fluid_player_set_tempo_type` {#fluid_player_set_tempo_type}

| Value | Description |
|-------|-------------|
| `FLUID_PLAYER_TEMPO_INTERNAL` |  |
| `FLUID_PLAYER_TEMPO_EXTERNAL_BPM` |  |
| `FLUID_PLAYER_TEMPO_EXTERNAL_MIDI` |  |
| `FLUID_PLAYER_TEMPO_NBR` |  |

MIDI File Player tempo enum. 

**Since:** 2.2.0

## Functions

### `new_fluid_player()` {#new_fluid_player}

```c
fluid_player_t * new_fluid_player(fluid_synth_t *synth)
```

Create a new MIDI player. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | Fluid synthesizer instance to create player for |

**Returns:** New MIDI player instance or NULL on error (out of memory)

### `delete_fluid_player()` {#delete_fluid_player}

```c
void delete_fluid_player(fluid_player_t *player)
```

Delete a MIDI player instance. 

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |

> **Warning:** Do not call when the synthesizer associated to this `player` renders audio, i.e. an audio driver is running or any other synthesizer thread concurrently calls [`fluid_synth_process()`](audio-rendering.md#fluid_synth_process) or [`fluid_synth_nwrite_float()`](audio-rendering.md#fluid_synth_nwrite_float) or fluid_synth_write_*() !

### `fluid_player_add()` {#fluid_player_add}

```c
int fluid_player_add(fluid_player_t *player, const char *midifile)
```

Add a MIDI file to a player queue. 

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |
| `midifile` | File name of the MIDI file to add |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) or [`FLUID_FAILED`](misc.md#FLUID_FAILED)

### `fluid_player_add_mem()` {#fluid_player_add_mem}

```c
int fluid_player_add_mem(fluid_player_t *player, const void *buffer, size_t len)
```

Add a MIDI file to a player queue, from a buffer in memory. 

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |
| `buffer` | Pointer to memory containing the bytes of a complete MIDI file. The data is copied, so the caller may free or modify it immediately without affecting the playlist. |
| `len` | Length of the buffer, in bytes. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) or [`FLUID_FAILED`](misc.md#FLUID_FAILED)

### `fluid_player_play()` {#fluid_player_play}

```c
int fluid_player_play(fluid_player_t *player)
```

Activates play mode for a MIDI player if not already playing. 

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_player_stop()` {#fluid_player_stop}

```c
int fluid_player_stop(fluid_player_t *player)
```

Pauses the MIDI playback.

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

[`fluid_player_seek()`](midi-player.md#fluid_player_seek) for this purpose.

### `fluid_player_join()` {#fluid_player_join}

```c
int fluid_player_join(fluid_player_t *player)
```

Wait for a MIDI player until the playback has been stopped. 

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |

**Returns:** Always [`FLUID_OK`](misc.md#FLUID_OK)

> **Warning:** The player may still be used by a concurrently running synth context. Hence it is not safe to immediately delete the player afterwards. Also refer to [`delete_fluid_player()`](midi-player.md#delete_fluid_player).

### `fluid_player_set_loop()` {#fluid_player_set_loop}

```c
int fluid_player_set_loop(fluid_player_t *player, int loop)
```

Enable looping of a MIDI player

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |
| `loop` | Times left to loop the playlist. -1 means loop infinitely. |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

**Since:** 1.1.0

### `fluid_player_set_tempo()` {#fluid_player_set_tempo}

```c
int fluid_player_set_tempo(fluid_player_t *player, int tempo_type, double tempo)
```

Set the tempo of a MIDI player. The player can be controlled by internal tempo coming from MIDI file tempo change or controlled by external tempo expressed in BPM or in micro seconds per quarter note.

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance. Must be a valid pointer. |
| `tempo_type` | Must a be value of [`fluid_player_set_tempo_type`](midi-player.md#fluid_player_set_tempo_type) and indicates the meaning of tempo value and how the player will be controlled, see below. |
| `tempo` | Tempo value or multiplier. |

`FLUID_PLAYER_TEMPO_INTERNAL`, the player will be controlled by internal MIDI file tempo changes. If there are no tempo change in the MIDI file a default value of 120 bpm is used. The `tempo` parameter is used as a multiplier factor that must be in the range (0.001 to 1000). For example, if the current MIDI file tempo is 120 bpm and the multiplier value is 0.5 then this tempo will be slowed down to 60 bpm. At creation, the player is set to be controlled by internal tempo with a multiplier factor set to 1.0.

`FLUID_PLAYER_TEMPO_EXTERNAL_BPM`, the player will be controlled by the external tempo value provided by the tempo parameter in bpm (i.e in quarter notes per minute) which must be in the range (1 to 60000000).

`FLUID_PLAYER_TEMPO_EXTERNAL_MIDI`, similar as FLUID_PLAYER_TEMPO_EXTERNAL_BPM, but the tempo parameter value is in micro seconds per quarter note which must be in the range (1 to 60000000). Using micro seconds per quarter note is convenient when the tempo value is derived from MIDI clock realtime messages.

> **Note:** When the player is controlled by an external tempo (`FLUID_PLAYER_TEMPO_EXTERNAL_BPM` or `FLUID_PLAYER_TEMPO_EXTERNAL_MIDI`) it continues to memorize the most recent internal tempo change coming from the MIDI file so that next call to [`fluid_player_set_tempo()`](midi-player.md#fluid_player_set_tempo) with `FLUID_PLAYER_TEMPO_INTERNAL` will set the player to follow this internal tempo.

> **Warning:** If the function is called when no MIDI file is loaded or currently playing, it would have caused a division by zero in fluidsynth 2.2.7 and earlier. Starting with 2.2.8, the new tempo change will be stashed and applied later.

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if success or [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise (incorrect parameters).

**Since:** 2.2.0

### `fluid_player_set_midi_tempo()` {#fluid_player_set_midi_tempo}

```c
int fluid_player_set_midi_tempo(fluid_player_t *player, int tempo)
```

Set the tempo of a MIDI player. 

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |
| `tempo` | Tempo to set playback speed to (in microseconds per quarter note, as per MIDI file spec) |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

> **Note:** Tempo change events contained in the MIDI file can override the specified tempo at any time!

Deprecated

Use [`fluid_player_set_tempo()`](midi-player.md#fluid_player_set_tempo) instead.

### `fluid_player_set_bpm()` {#fluid_player_set_bpm}

```c
int fluid_player_set_bpm(fluid_player_t *player, int bpm)
```

Set the tempo of a MIDI player in beats per minute. 

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |
| `bpm` | Tempo in beats per minute |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

> **Note:** Tempo change events contained in the MIDI file can override the specified BPM at any time!

Deprecated

Use [`fluid_player_set_tempo()`](midi-player.md#fluid_player_set_tempo) instead.

### `fluid_player_set_playback_callback()` {#fluid_player_set_playback_callback}

```c
int fluid_player_set_playback_callback(fluid_player_t *player, handle_midi_event_func_t handler, void *handler_data)
```

Change the MIDI callback function.

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |
| `handler` | Pointer to callback function |
| `handler_data` | Parameter sent to the callback function |

**Returns:** FLUID_OK

[`fluid_synth_handle_midi_event()`](midi-input.md#fluid_synth_handle_midi_event), but can optionally be changed to a user-defined function instead, for intercepting all MIDI messages sent to the synth. You can also use a midi router as the callback function to modify the MIDI messages before sending them to the synth.

**Since:** 1.1.4

### `fluid_player_set_tick_callback()` {#fluid_player_set_tick_callback}

```c
int fluid_player_set_tick_callback(fluid_player_t *player, handle_midi_tick_func_t handler, void *handler_data)
```

Add a listener function for every MIDI tick change.

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |
| `handler` | Pointer to callback function |
| `handler_data` | Opaque parameter to be sent to the callback function |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK)

**Since:** 2.2.0

### `fluid_player_get_status()` {#fluid_player_get_status}

```c
int fluid_player_get_status(fluid_player_t *player)
```

Get MIDI player status. 

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |

**Returns:** Player status ([`fluid_player_status`](midi-player.md#fluid_player_status))

**Since:** 1.1.0

### `fluid_player_get_current_tick()` {#fluid_player_get_current_tick}

```c
int fluid_player_get_current_tick(fluid_player_t *player)
```

Get the number of tempo ticks passed. 

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |

**Returns:** The number of tempo ticks passed

**Since:** 1.1.7

### `fluid_player_get_total_ticks()` {#fluid_player_get_total_ticks}

```c
int fluid_player_get_total_ticks(fluid_player_t *player)
```

Looks through all available MIDI tracks and gets the absolute tick of the very last event to play. 

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |

**Returns:** Total tick count of the sequence

**Since:** 1.1.7

### `fluid_player_get_bpm()` {#fluid_player_get_bpm}

```c
int fluid_player_get_bpm(fluid_player_t *player)
```

Get the tempo currently used by a MIDI player. The player can be controlled by internal tempo coming from MIDI file tempo change or controlled by external tempo see [`fluid_player_set_tempo()`](midi-player.md#fluid_player_set_tempo). 

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance. Must be a valid pointer. |

**Returns:** MIDI player tempo in BPM or FLUID_FAILED if error.

**Since:** 1.1.7

### `fluid_player_get_division()` {#fluid_player_get_division}

```c
int fluid_player_get_division(fluid_player_t *player)
```

Get the division currently used by a MIDI player. The player can be controlled by internal tempo coming from MIDI file tempo change or controlled by external tempo see [`fluid_player_set_tempo()`](midi-player.md#fluid_player_set_tempo). 

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance. Must be a valid pointer. |

**Returns:** MIDI player division or FLUID_FAILED if error.

**Since:** 2.3.2

### `fluid_player_get_midi_tempo()` {#fluid_player_get_midi_tempo}

```c
int fluid_player_get_midi_tempo(fluid_player_t *player)
```

Get the tempo currently used by a MIDI player. The player can be controlled by internal tempo coming from MIDI file tempo change or controlled by external tempo see [`fluid_player_set_tempo()`](midi-player.md#fluid_player_set_tempo).

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance. Must be a valid pointer. |

**Returns:** Tempo of the MIDI player (in microseconds per quarter note, as per MIDI file spec) or FLUID_FAILED if error.

**Since:** 1.1.7

### `fluid_player_seek()` {#fluid_player_seek}

```c
int fluid_player_seek(fluid_player_t *player, int ticks)
```

Seek in the currently playing file.

**Parameters:**

| Name | Description |
|------|-------------|
| `player` | MIDI player instance |
| `ticks` | the position to seek to in the current file |

**Returns:** [`FLUID_FAILED`](misc.md#FLUID_FAILED) if ticks is negative or after the latest tick of the file \[or, since 2.1.3, if another seek operation is currently in progress\], [`FLUID_OK`](misc.md#FLUID_OK) otherwise.

[`fluid_player_set_playback_callback()`](midi-player.md#fluid_player_set_playback_callback)). If the player's status is `FLUID_PLAYER_PLAYING` and a previous seek operation has not been completed yet, [`FLUID_FAILED`](misc.md#FLUID_FAILED) is returned.

**Since:** 2.0.0
