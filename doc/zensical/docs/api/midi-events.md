# MIDI Events

Functions to create, modify, query and delete MIDI events.

These functions are intended to be used in MIDI routers and other filtering and processing functions in the MIDI event path. If you want to simply send MIDI messages to the synthesizer, you can use the more convenient [`MIDI Channel Messages`](midi-messages.md) interface.

## Functions

### `new_fluid_midi_event()` {#new_fluid_midi_event}

```c
fluid_midi_event_t * new_fluid_midi_event(void)
```

Create a MIDI event structure. 

**Returns:** New MIDI event structure or NULL when out of memory.

### `delete_fluid_midi_event()` {#delete_fluid_midi_event}

```c
void delete_fluid_midi_event(fluid_midi_event_t *event)
```

Delete MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |

### `fluid_midi_event_set_type()` {#fluid_midi_event_set_type}

```c
int fluid_midi_event_set_type(fluid_midi_event_t *evt, int type)
```

Set the event type field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `type` | Event type field (MIDI status byte without channel) |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

### `fluid_midi_event_get_type()` {#fluid_midi_event_get_type}

```c
int fluid_midi_event_get_type(const fluid_midi_event_t *evt)
```

Get the event type field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |

**Returns:** Event type field (MIDI status byte without channel)

### `fluid_midi_event_set_channel()` {#fluid_midi_event_set_channel}

```c
int fluid_midi_event_set_channel(fluid_midi_event_t *evt, int chan)
```

Set the channel field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `chan` | MIDI channel field |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

### `fluid_midi_event_get_channel()` {#fluid_midi_event_get_channel}

```c
int fluid_midi_event_get_channel(const fluid_midi_event_t *evt)
```

Get the channel field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |

**Returns:** Channel field

### `fluid_midi_event_get_key()` {#fluid_midi_event_get_key}

```c
int fluid_midi_event_get_key(const fluid_midi_event_t *evt)
```

Get the key field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |

**Returns:** MIDI note number (0-127)

### `fluid_midi_event_set_key()` {#fluid_midi_event_set_key}

```c
int fluid_midi_event_set_key(fluid_midi_event_t *evt, int key)
```

Set the key field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `v` | MIDI note number (0-127) |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

### `fluid_midi_event_get_velocity()` {#fluid_midi_event_get_velocity}

```c
int fluid_midi_event_get_velocity(const fluid_midi_event_t *evt)
```

Get the velocity field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |

**Returns:** MIDI velocity number (0-127)

### `fluid_midi_event_set_velocity()` {#fluid_midi_event_set_velocity}

```c
int fluid_midi_event_set_velocity(fluid_midi_event_t *evt, int vel)
```

Set the velocity field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `v` | MIDI velocity value |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

### `fluid_midi_event_get_control()` {#fluid_midi_event_get_control}

```c
int fluid_midi_event_get_control(const fluid_midi_event_t *evt)
```

Get the control number of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |

**Returns:** MIDI control number

### `fluid_midi_event_set_control()` {#fluid_midi_event_set_control}

```c
int fluid_midi_event_set_control(fluid_midi_event_t *evt, int ctrl)
```

Set the control field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `v` | MIDI control number |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

### `fluid_midi_event_get_value()` {#fluid_midi_event_get_value}

```c
int fluid_midi_event_get_value(const fluid_midi_event_t *evt)
```

Get the value field from a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |

**Returns:** Value field

### `fluid_midi_event_set_value()` {#fluid_midi_event_set_value}

```c
int fluid_midi_event_set_value(fluid_midi_event_t *evt, int val)
```

Set the value field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `v` | Value to assign |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

### `fluid_midi_event_get_program()` {#fluid_midi_event_get_program}

```c
int fluid_midi_event_get_program(const fluid_midi_event_t *evt)
```

Get the program field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |

**Returns:** MIDI program number (0-127)

### `fluid_midi_event_set_program()` {#fluid_midi_event_set_program}

```c
int fluid_midi_event_set_program(fluid_midi_event_t *evt, int val)
```

Set the program field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `val` | MIDI program number (0-127) |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

### `fluid_midi_event_get_pitch()` {#fluid_midi_event_get_pitch}

```c
int fluid_midi_event_get_pitch(const fluid_midi_event_t *evt)
```

Get the pitch field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |

**Returns:** Pitch value (14 bit value, 0-16383, 8192 is center)

### `fluid_midi_event_set_pitch()` {#fluid_midi_event_set_pitch}

```c
int fluid_midi_event_set_pitch(fluid_midi_event_t *evt, int val)
```

Set the pitch field of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `val` | Pitch value (14 bit value, 0-16383, 8192 is center) |

**Returns:** Always returns FLUID_OK

### `fluid_midi_event_set_sysex()` {#fluid_midi_event_set_sysex}

```c
int fluid_midi_event_set_sysex(fluid_midi_event_t *evt, void *data, int size, int dynamic)
```

Assign sysex data to a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `data` | Pointer to SYSEX data |
| `size` | Size of SYSEX data in bytes |
| `dynamic` | TRUE if the SYSEX data has been dynamically allocated and should be freed when the event is freed (only applies if event gets destroyed with [`delete_fluid_midi_event()`](midi-events.md#delete_fluid_midi_event)) |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

### `fluid_midi_event_set_text()` {#fluid_midi_event_set_text}

```c
int fluid_midi_event_set_text(fluid_midi_event_t *evt, void *data, int size, int dynamic)
```

Assign text data to a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `data` | Pointer to text data |
| `size` | Size of text data in bytes |
| `dynamic` | TRUE if the data has been dynamically allocated and should be freed when the event is freed via [`delete_fluid_midi_event()`](midi-events.md#delete_fluid_midi_event) |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

**Since:** 2.0.0

### `fluid_midi_event_get_text()` {#fluid_midi_event_get_text}

```c
int fluid_midi_event_get_text(fluid_midi_event_t *evt, void **data, int *size)
```

Get the text of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `data` | Pointer to return text data on. |
| `size` | Pointer to return text size on. |

**Returns:** Returns [`FLUID_OK`](misc.md#FLUID_OK) if `data` and `size` previously set by [`fluid_midi_event_set_text()`](midi-events.md#fluid_midi_event_set_text) have been successfully retrieved. Else [`FLUID_FAILED`](misc.md#FLUID_FAILED) is returned and `data` and `size` are not changed.

**Since:** 2.0.3

### `fluid_midi_event_set_lyrics()` {#fluid_midi_event_set_lyrics}

```c
int fluid_midi_event_set_lyrics(fluid_midi_event_t *evt, void *data, int size, int dynamic)
```

Assign lyric data to a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `data` | Pointer to lyric data |
| `size` | Size of lyric data in bytes |
| `dynamic` | TRUE if the data has been dynamically allocated and should be freed when the event is freed via [`delete_fluid_midi_event()`](midi-events.md#delete_fluid_midi_event) |

**Returns:** Always returns [`FLUID_OK`](misc.md#FLUID_OK)

**Since:** 2.0.0

### `fluid_midi_event_get_lyrics()` {#fluid_midi_event_get_lyrics}

```c
int fluid_midi_event_get_lyrics(fluid_midi_event_t *evt, void **data, int *size)
```

Get the lyric of a MIDI event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | MIDI event structure |
| `data` | Pointer to return lyric data on. |
| `size` | Pointer to return lyric size on. |

**Returns:** Returns [`FLUID_OK`](misc.md#FLUID_OK) if `data` and `size` previously set by [`fluid_midi_event_set_lyrics()`](midi-events.md#fluid_midi_event_set_lyrics) have been successfully retrieved. Else [`FLUID_FAILED`](misc.md#FLUID_FAILED) is returned and `data` and `size` are not changed.

**Since:** 2.0.3
