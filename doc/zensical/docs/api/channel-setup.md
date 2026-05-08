# MIDI Channel Setup

The functions in this section provide interfaces to change the channel type and to configure basic channels, legato and portamento setups.

## User-defined

### `fluid_midi_channel_type` {#fluid_midi_channel_type}

| Value | Description |
|-------|-------------|
| `CHANNEL_TYPE_MELODIC` |  |
| `CHANNEL_TYPE_DRUM` |  |

The midi channel type used by [`fluid_synth_set_channel_type()`](channel-setup.md#fluid_synth_set_channel_type)

### `fluid_synth_set_channel_type()` {#fluid_synth_set_channel_type}

```c
int fluid_synth_set_channel_type(fluid_synth_t *synth, int chan, int type)
```

Set midi channel type 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `type` | MIDI channel type ([`fluid_midi_channel_type`](channel-setup.md#fluid_midi_channel_type)) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.1.4

## User-defined

### `fluid_channel_mode_flags` {#fluid_channel_mode_flags}

| Value | Description |
|-------|-------------|
| `FLUID_CHANNEL_POLY_OFF` |  |
| `FLUID_CHANNEL_OMNI_OFF` |  |

Channel mode bits OR-ed together so that it matches with the midi spec: poly omnion (0), mono omnion (1), poly omnioff (2), mono omnioff (3)

### `fluid_basic_channel_modes` {#fluid_basic_channel_modes}

| Value | Description |
|-------|-------------|
| `FLUID_CHANNEL_MODE_MASK` |  |
| `FLUID_CHANNEL_MODE_OMNION_POLY` |  |
| `FLUID_CHANNEL_MODE_OMNION_MONO` |  |
| `FLUID_CHANNEL_MODE_OMNIOFF_POLY` |  |
| `FLUID_CHANNEL_MODE_OMNIOFF_MONO` |  |
| `FLUID_CHANNEL_MODE_LAST` |  |

Indicates the mode a basic channel is set to

### `fluid_synth_reset_basic_channel()` {#fluid_synth_reset_basic_channel}

```c
int fluid_synth_reset_basic_channel(fluid_synth_t *synth, int chan)
```

Disables and unassigns all channels from a basic channel group.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | The synth instance. |
| `chan` | The basic channel of the group to reset or -1 to reset all channels. |

> **Note:** By default (i.e. on creation after [`new_fluid_synth()`](synth.md#new_fluid_synth) and after [`fluid_synth_system_reset()`](midi-messages.md#fluid_synth_system_reset)) a synth instance has one basic channel at channel 0 in mode `FLUID_CHANNEL_MODE_OMNION_POLY`. All other channels belong to this basic channel group. Make sure to call this function before setting any custom basic channel setup.

**Returns:** - [`FLUID_OK`](misc.md#FLUID_OK) on success.
- [`FLUID_FAILED`](misc.md#FLUID_FAILED)

    - *synth* is NULL.
    - *chan* is outside MIDI channel count.
    - *chan* isn't a basic channel.

### `fluid_synth_get_basic_channel()` {#fluid_synth_get_basic_channel}

```c
int fluid_synth_get_basic_channel(fluid_synth_t *synth, int chan, int *basic_chan_out, int *mode_chan_out, int *basic_val_out)
```

Returns poly mono mode information of any MIDI channel.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | the synth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `basic_chan_out` | Buffer to store the basic channel *chan* belongs to or [`FLUID_FAILED`](misc.md#FLUID_FAILED) if *chan* is disabled. |
| `mode_out` | Buffer to store the mode of *chan* (see [`fluid_basic_channel_modes`](channel-setup.md#fluid_basic_channel_modes)) or [`FLUID_FAILED`](misc.md#FLUID_FAILED) if *chan* is disabled. |
| `val_out` | Buffer to store the total number of channels in this basic channel group or [`FLUID_FAILED`](misc.md#FLUID_FAILED) if *chan* is disabled. |

> **Note:** If any of *basic_chan_out*, *mode_out*, *val_out* pointer is NULL the corresponding information isn't returned.

**Returns:** - [`FLUID_OK`](misc.md#FLUID_OK) on success.
- [`FLUID_FAILED`](misc.md#FLUID_FAILED)

    - *synth* is NULL.
    - *chan* is outside MIDI channel count.

### `fluid_synth_set_basic_channel()` {#fluid_synth_set_basic_channel}

```c
int fluid_synth_set_basic_channel(fluid_synth_t *synth, int chan, int mode, int val)
```

Sets a new basic channel group only. The function doesn't allow to change an existing basic channel.

The function fails if any channel overlaps any existing basic channel group. To make room if necessary, basic channel groups can be cleared using [`fluid_synth_reset_basic_channel()`](channel-setup.md#fluid_synth_reset_basic_channel).

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | the synth instance. |
| `chan` | the basic Channel number (0 to MIDI channel count-1). |
| `mode` | the MIDI mode to use for chan (see [`fluid_basic_channel_modes`](channel-setup.md#fluid_basic_channel_modes)). |
| `val` | number of channels in the group. |

> **Note:** *val* is only relevant for mode `FLUID_CHANNEL_MODE_OMNION_POLY`, `FLUID_CHANNEL_MODE_OMNION_MONO` and `FLUID_CHANNEL_MODE_OMNIOFF_MONO`. A value of 0 means all possible channels from *chan* to to next basic channel minus 1 (if any) or to MIDI channel count minus 1. Val is ignored for `FLUID_CHANNEL_MODE_OMNIOFF_POLY` as this mode implies a group of only one channel.

**Returns:** - [`FLUID_OK`](misc.md#FLUID_OK) on success.
- [`FLUID_FAILED`](misc.md#FLUID_FAILED)

    - *synth* is NULL.
    - *chan* is outside MIDI channel count.
    - *mode* is invalid.
    - *val* has a number of channels overlapping another basic channel group or been above MIDI channel count.
    - When the function fails, any existing basic channels aren't modified.

## User-defined

### `fluid_channel_legato_mode` {#fluid_channel_legato_mode}

| Value | Description |
|-------|-------------|
| `FLUID_CHANNEL_LEGATO_MODE_RETRIGGER` |  |
| `FLUID_CHANNEL_LEGATO_MODE_MULTI_RETRIGGER` |  |
| `FLUID_CHANNEL_LEGATO_MODE_LAST` |  |

Indicates the legato mode a channel is set to n1,n2,n3,.. is a legato passage. n1 is the first note, and n2,n3,n4 are played legato with previous note.

### `fluid_synth_set_legato_mode()` {#fluid_synth_set_legato_mode}

```c
int fluid_synth_set_legato_mode(fluid_synth_t *synth, int chan, int legatomode)
```

Sets the legato mode of a channel.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | the synth instance. |
| `chan` | MIDI channel number (0 to MIDI channel count - 1). |
| `legatomode` | The legato mode as indicated by [`fluid_channel_legato_mode`](channel-setup.md#fluid_channel_legato_mode). |

**Returns:** - [`FLUID_OK`](misc.md#FLUID_OK) on success.
- [`FLUID_FAILED`](misc.md#FLUID_FAILED)

    - *synth* is NULL.
    - *chan* is outside MIDI channel count.
    - *legatomode* is invalid.

### `fluid_synth_get_legato_mode()` {#fluid_synth_get_legato_mode}

```c
int fluid_synth_get_legato_mode(fluid_synth_t *synth, int chan, int *legatomode)
```

Gets the legato mode of a channel.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | the synth instance. |
| `chan` | MIDI channel number (0 to MIDI channel count - 1). |
| `legatomode` | The legato mode as indicated by [`fluid_channel_legato_mode`](channel-setup.md#fluid_channel_legato_mode). |

**Returns:** - [`FLUID_OK`](misc.md#FLUID_OK) on success.
- [`FLUID_FAILED`](misc.md#FLUID_FAILED)

    - *synth* is NULL.
    - *chan* is outside MIDI channel count.
    - *legatomode* is NULL.

## User-defined

### `fluid_channel_portamento_mode` {#fluid_channel_portamento_mode}

| Value | Description |
|-------|-------------|
| `FLUID_CHANNEL_PORTAMENTO_MODE_EACH_NOTE` |  |
| `FLUID_CHANNEL_PORTAMENTO_MODE_LEGATO_ONLY` |  |
| `FLUID_CHANNEL_PORTAMENTO_MODE_STACCATO_ONLY` |  |
| `FLUID_CHANNEL_PORTAMENTO_MODE_LAST` |  |

Indicates the portamento mode a channel is set to

### `fluid_synth_set_portamento_mode()` {#fluid_synth_set_portamento_mode}

```c
int fluid_synth_set_portamento_mode(fluid_synth_t *synth, int chan, int portamentomode)
```

Sets the portamento mode of a channel.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | the synth instance. |
| `chan` | MIDI channel number (0 to MIDI channel count - 1). |
| `portamentomode` | The portamento mode as indicated by [`fluid_channel_portamento_mode`](channel-setup.md#fluid_channel_portamento_mode). |

**Returns:** - [`FLUID_OK`](misc.md#FLUID_OK) on success.
- [`FLUID_FAILED`](misc.md#FLUID_FAILED)

    - *synth* is NULL.
    - *chan* is outside MIDI channel count.
    - *portamentomode* is invalid.

### `fluid_synth_get_portamento_mode()` {#fluid_synth_get_portamento_mode}

```c
int fluid_synth_get_portamento_mode(fluid_synth_t *synth, int chan, int *portamentomode)
```

Gets the portamento mode of a channel.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | the synth instance. |
| `chan` | MIDI channel number (0 to MIDI channel count - 1). |
| `portamentomode` | Pointer to the portamento mode as indicated by [`fluid_channel_portamento_mode`](channel-setup.md#fluid_channel_portamento_mode). |

**Returns:** - [`FLUID_OK`](misc.md#FLUID_OK) on success.
- [`FLUID_FAILED`](misc.md#FLUID_FAILED)

    - *synth* is NULL.
    - *chan* is outside MIDI channel count.
    - *portamentomode* is NULL.

## User-defined

### `fluid_channel_breath_flags` {#fluid_channel_breath_flags}

| Value | Description |
|-------|-------------|
| `FLUID_CHANNEL_BREATH_POLY` |  |
| `FLUID_CHANNEL_BREATH_MONO` |  |
| `FLUID_CHANNEL_BREATH_SYNC` |  |

Indicates the breath mode a channel is set to

### `fluid_synth_set_breath_mode()` {#fluid_synth_set_breath_mode}

```c
int fluid_synth_set_breath_mode(fluid_synth_t *synth, int chan, int breathmode)
```

Sets the breath mode of a channel.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | the synth instance. |
| `chan` | MIDI channel number (0 to MIDI channel count - 1). |
| `breathmode` | The breath mode as indicated by [`fluid_channel_breath_flags`](channel-setup.md#fluid_channel_breath_flags). |

**Returns:** - [`FLUID_OK`](misc.md#FLUID_OK) on success.
- [`FLUID_FAILED`](misc.md#FLUID_FAILED)

    - *synth* is NULL.
    - *chan* is outside MIDI channel count.

### `fluid_synth_get_breath_mode()` {#fluid_synth_get_breath_mode}

```c
int fluid_synth_get_breath_mode(fluid_synth_t *synth, int chan, int *breathmode)
```

Gets the breath mode of a channel.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | the synth instance. |
| `chan` | MIDI channel number (0 to MIDI channel count - 1). |
| `breathmode` | Pointer to the returned breath mode as indicated by [`fluid_channel_breath_flags`](channel-setup.md#fluid_channel_breath_flags). |

**Returns:** - [`FLUID_OK`](misc.md#FLUID_OK) on success.
- [`FLUID_FAILED`](misc.md#FLUID_FAILED)

    - *synth* is NULL.
    - *chan* is outside MIDI channel count.
    - *breathmode* is NULL.

## User-defined

### `fluid_portamento_time_mode` {#fluid_portamento_time_mode}

| Value | Description |
|-------|-------------|
| `FLUID_PORTAMENTO_TIME_MODE_AUTO` |  |
| `FLUID_PORTAMENTO_TIME_MODE_XG_GS` |  |
| `FLUID_PORTAMENTO_TIME_MODE_LINEAR` |  |
| `FLUID_PORTAMENTO_TIME_MODE_LAST` |  |

Indicates the portamento time mode the synthesizer is set to

### `fluid_synth_set_portamento_time_mode()` {#fluid_synth_set_portamento_time_mode}

```c
int fluid_synth_set_portamento_time_mode(fluid_synth_t *synth, int mode)
```

Sets the global portamento time mode of the synthesizer.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | the synth instance. |
| `mode` | The portamento time mode as indicated by [`fluid_portamento_time_mode`](channel-setup.md#fluid_portamento_time_mode). |

**Returns:** - [`FLUID_OK`](misc.md#FLUID_OK) on success.
- [`FLUID_FAILED`](misc.md#FLUID_FAILED)

    - *synth* is NULL.
    - *mode* is invalid.

### `fluid_synth_get_portamento_time_mode()` {#fluid_synth_get_portamento_time_mode}

```c
int fluid_synth_get_portamento_time_mode(fluid_synth_t *synth, int *mode)
```

Gets the global portamento time mode of the synthesizer.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | the synth instance. |
| `mode` | the address to store the portamento time mode to. |

**Returns:** - [`FLUID_OK`](misc.md#FLUID_OK) on success.
- [`FLUID_FAILED`](misc.md#FLUID_FAILED)

    - *synth* is NULL.
    - *mode* is NULL.
