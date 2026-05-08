# Effect - Reverb

Functions for configuring the built-in reverb effect

## Functions

### `fluid_synth_set_reverb_on()` {#fluid_synth_set_reverb_on}

```c
void fluid_synth_set_reverb_on(fluid_synth_t *synth, int on)
```

Enable or disable reverb effect. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `on` | TRUE to enable chorus, FALSE to disable |

Deprecated

Use [`fluid_synth_reverb_on()`](reverb-effect.md#fluid_synth_reverb_on) instead.

### `fluid_synth_reverb_on()` {#fluid_synth_reverb_on}

```c
int fluid_synth_reverb_on(fluid_synth_t *synth, int fx_group, int on)
```

Enable or disable reverb on one fx group unit. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter will be applied to all fx groups. |
| `on` | TRUE to enable reverb, FALSE to disable |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_set_reverb()` {#fluid_synth_set_reverb}

```c
int fluid_synth_set_reverb(fluid_synth_t *synth, double roomsize, double damping, double width, double level)
```

Set reverb parameters to all groups.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `roomsize` | Reverb room size value (0.0-1.0) |
| `damping` | Reverb damping value (0.0-1.0) |
| `width` | Reverb width value (0.0-100.0) |
| `level` | Reverb level value (0.0-1.0) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

Deprecated

Use the individual reverb setter functions in new code instead.

### `fluid_synth_set_reverb_roomsize()` {#fluid_synth_set_reverb_roomsize}

```c
int fluid_synth_set_reverb_roomsize(fluid_synth_t *synth, double roomsize)
```

Set reverb roomsize of all groups.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `roomsize` | Reverb room size value (0.0-1.0) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

Deprecated

Use [`fluid_synth_set_reverb_group_roomsize()`](reverb-effect.md#fluid_synth_set_reverb_group_roomsize) in new code instead.

### `fluid_synth_set_reverb_damp()` {#fluid_synth_set_reverb_damp}

```c
int fluid_synth_set_reverb_damp(fluid_synth_t *synth, double damping)
```

Set reverb damping of all groups.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `damping` | Reverb damping value (0.0-1.0) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

Deprecated

Use [`fluid_synth_set_reverb_group_damp()`](reverb-effect.md#fluid_synth_set_reverb_group_damp) in new code instead.

### `fluid_synth_set_reverb_width()` {#fluid_synth_set_reverb_width}

```c
int fluid_synth_set_reverb_width(fluid_synth_t *synth, double width)
```

Set reverb width of all groups.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `width` | Reverb width value (0.0-100.0) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

Deprecated

Use [`fluid_synth_set_reverb_group_width()`](reverb-effect.md#fluid_synth_set_reverb_group_width) in new code instead.

### `fluid_synth_set_reverb_level()` {#fluid_synth_set_reverb_level}

```c
int fluid_synth_set_reverb_level(fluid_synth_t *synth, double level)
```

Set reverb level of all groups.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `level` | Reverb level value (0.0-1.0) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

Deprecated

Use [`fluid_synth_set_reverb_group_level()`](reverb-effect.md#fluid_synth_set_reverb_group_level) in new code instead.

### `fluid_synth_get_reverb_roomsize()` {#fluid_synth_get_reverb_roomsize}

```c
double fluid_synth_get_reverb_roomsize(fluid_synth_t *synth)
```

Get reverb room size of all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Reverb room size (0.0-1.2)

Deprecated

Use [`fluid_synth_get_reverb_group_roomsize()`](reverb-effect.md#fluid_synth_get_reverb_group_roomsize) in new code instead.

### `fluid_synth_get_reverb_damp()` {#fluid_synth_get_reverb_damp}

```c
double fluid_synth_get_reverb_damp(fluid_synth_t *synth)
```

Get reverb damping of all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Reverb damping value (0.0-1.0)

Deprecated

Use [`fluid_synth_get_reverb_group_damp()`](reverb-effect.md#fluid_synth_get_reverb_group_damp) in new code instead.

### `fluid_synth_get_reverb_level()` {#fluid_synth_get_reverb_level}

```c
double fluid_synth_get_reverb_level(fluid_synth_t *synth)
```

Get reverb level of all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Reverb level value (0.0-1.0)

Deprecated

Use [`fluid_synth_get_reverb_group_level()`](reverb-effect.md#fluid_synth_get_reverb_group_level) in new code instead.

### `fluid_synth_get_reverb_width()` {#fluid_synth_get_reverb_width}

```c
double fluid_synth_get_reverb_width(fluid_synth_t *synth)
```

Get reverb width of all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Reverb width value (0.0-100.0)

Deprecated

Use [`fluid_synth_get_reverb_group_width()`](reverb-effect.md#fluid_synth_get_reverb_group_width) in new code instead.

### `fluid_synth_set_reverb_group_roomsize()` {#fluid_synth_set_reverb_group_roomsize}

```c
int fluid_synth_set_reverb_group_roomsize(fluid_synth_t *synth, int fx_group, double roomsize)
```

Set reverb roomsize to one or all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter will be applied to all fx groups. |
| `roomsize` | roomsize value to set. Must be in the range indicated by synth.reverb.room-size setting. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_set_reverb_group_damp()` {#fluid_synth_set_reverb_group_damp}

```c
int fluid_synth_set_reverb_group_damp(fluid_synth_t *synth, int fx_group, double damping)
```

Set reverb damp to one or all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter will be applied to all fx groups. |
| `damping` | damping value to set. Must be in the range indicated by synth.reverb.damp setting. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_set_reverb_group_width()` {#fluid_synth_set_reverb_group_width}

```c
int fluid_synth_set_reverb_group_width(fluid_synth_t *synth, int fx_group, double width)
```

Set reverb width to one or all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter will be applied to all fx groups. |
| `width` | width value to set. Must be in the range indicated by synth.reverb.width setting. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_set_reverb_group_level()` {#fluid_synth_set_reverb_group_level}

```c
int fluid_synth_set_reverb_group_level(fluid_synth_t *synth, int fx_group, double level)
```

Set reverb level to one or all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter will be applied to all fx groups. |
| `level` | output level to set. Must be in the range indicated by synth.reverb.level setting. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_get_reverb_group_roomsize()` {#fluid_synth_get_reverb_group_roomsize}

```c
int fluid_synth_get_reverb_group_roomsize(fluid_synth_t *synth, int fx_group, double *roomsize)
```

get reverb roomsize of one or all groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter common to all fx groups is fetched. |
| `roomsize` | valid pointer on the value to return. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_get_reverb_group_damp()` {#fluid_synth_get_reverb_group_damp}

```c
int fluid_synth_get_reverb_group_damp(fluid_synth_t *synth, int fx_group, double *damping)
```

get reverb damp of one or all groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter common to all fx groups is fetched. |
| `damping` | valid pointer on the value to return. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_get_reverb_group_width()` {#fluid_synth_get_reverb_group_width}

```c
int fluid_synth_get_reverb_group_width(fluid_synth_t *synth, int fx_group, double *width)
```

get reverb width of one or all groups 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter common to all fx groups is fetched. |
| `width` | valid pointer on the value to return. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_get_reverb_group_level()` {#fluid_synth_get_reverb_group_level}

```c
int fluid_synth_get_reverb_group_level(fluid_synth_t *synth, int fx_group, double *level)
```

get reverb level of one or all groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter common to all fx groups is fetched. |
| `level` | valid pointer on the value to return. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.
