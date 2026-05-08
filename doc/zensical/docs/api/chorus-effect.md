# Effect - Chorus

Functions for configuring the built-in chorus effect

## Enumerations

### `fluid_chorus_mod` {#fluid_chorus_mod}

| Value | Description |
|-------|-------------|
| `FLUID_CHORUS_MOD_SINE` |  |
| `FLUID_CHORUS_MOD_TRIANGLE` |  |

Chorus modulation waveform type.

## Functions

### `fluid_synth_set_chorus_on()` {#fluid_synth_set_chorus_on}

```c
void fluid_synth_set_chorus_on(fluid_synth_t *synth, int on)
```

Enable or disable all chorus groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `on` | TRUE to enable chorus, FALSE to disable |

Deprecated

Use [`fluid_synth_chorus_on()`](chorus-effect.md#fluid_synth_chorus_on) in new code instead.

### `fluid_synth_chorus_on()` {#fluid_synth_chorus_on}

```c
int fluid_synth_chorus_on(fluid_synth_t *synth, int fx_group, int on)
```

Enable or disable chorus on one or all groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter will be applied to all fx groups. |
| `on` | TRUE to enable chorus, FALSE to disable |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_set_chorus()` {#fluid_synth_set_chorus}

```c
int fluid_synth_set_chorus(fluid_synth_t *synth, int nr, double level, double speed, double depth_ms, int type)
```

Set chorus parameters to all fx groups. Keep in mind, that the needed CPU time is proportional to 'nr'. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `nr` | Chorus voice count (0-99, CPU time consumption proportional to this value) |
| `level` | Chorus level (0.0-10.0) |
| `speed` | Chorus speed in Hz (0.1-5.0) |
| `depth_ms` | Chorus depth (max value depends on synth sample-rate, 0.0-21.0 is safe for sample-rate values up to 96KHz) |
| `type` | Chorus waveform type ([`fluid_chorus_mod`](chorus-effect.md#fluid_chorus_mod)) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

Deprecated

Use the individual chorus setter functions in new code instead.

Keep in mind, that the needed CPU time is proportional to 'nr'.

### `fluid_synth_set_chorus_nr()` {#fluid_synth_set_chorus_nr}

```c
int fluid_synth_set_chorus_nr(fluid_synth_t *synth, int nr)
```

Set the chorus voice count of all groups.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `nr` | Chorus voice count (0-99, CPU time consumption proportional to this value) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

Deprecated

Use [`fluid_synth_set_chorus_group_nr()`](chorus-effect.md#fluid_synth_set_chorus_group_nr) in new code instead.

### `fluid_synth_set_chorus_level()` {#fluid_synth_set_chorus_level}

```c
int fluid_synth_set_chorus_level(fluid_synth_t *synth, double level)
```

Set the chorus level of all groups.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `level` | Chorus level (0.0-10.0) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

Deprecated

Use [`fluid_synth_set_chorus_group_level()`](chorus-effect.md#fluid_synth_set_chorus_group_level) in new code instead.

### `fluid_synth_set_chorus_speed()` {#fluid_synth_set_chorus_speed}

```c
int fluid_synth_set_chorus_speed(fluid_synth_t *synth, double speed)
```

Set the chorus speed of all groups.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `speed` | Chorus speed in Hz (0.1-5.0) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

Deprecated

Use [`fluid_synth_set_chorus_group_speed()`](chorus-effect.md#fluid_synth_set_chorus_group_speed) in new code instead.

### `fluid_synth_set_chorus_depth()` {#fluid_synth_set_chorus_depth}

```c
int fluid_synth_set_chorus_depth(fluid_synth_t *synth, double depth_ms)
```

Set the chorus depth of all groups.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `depth_ms` | Chorus depth (max value depends on synth sample-rate, 0.0-21.0 is safe for sample-rate values up to 96KHz) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

Deprecated

Use [`fluid_synth_set_chorus_group_depth()`](chorus-effect.md#fluid_synth_set_chorus_group_depth) in new code instead.

### `fluid_synth_set_chorus_type()` {#fluid_synth_set_chorus_type}

```c
int fluid_synth_set_chorus_type(fluid_synth_t *synth, int type)
```

Set the chorus type of all groups.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `type` | Chorus waveform type ([`fluid_chorus_mod`](chorus-effect.md#fluid_chorus_mod)) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

Deprecated

Use [`fluid_synth_set_chorus_group_type()`](chorus-effect.md#fluid_synth_set_chorus_group_type) in new code instead.

### `fluid_synth_get_chorus_nr()` {#fluid_synth_get_chorus_nr}

```c
int fluid_synth_get_chorus_nr(fluid_synth_t *synth)
```

Get chorus voice number (delay line count) value of all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Chorus voice count

Deprecated

Use [`fluid_synth_get_chorus_group_nr()`](chorus-effect.md#fluid_synth_get_chorus_group_nr) in new code instead.

### `fluid_synth_get_chorus_level()` {#fluid_synth_get_chorus_level}

```c
double fluid_synth_get_chorus_level(fluid_synth_t *synth)
```

Get chorus level of all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Chorus level value

Deprecated

Use [`fluid_synth_get_chorus_group_level()`](chorus-effect.md#fluid_synth_get_chorus_group_level) in new code instead.

### `fluid_synth_get_chorus_speed()` {#fluid_synth_get_chorus_speed}

```c
double fluid_synth_get_chorus_speed(fluid_synth_t *synth)
```

Get chorus speed in Hz of all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Chorus speed in Hz

Deprecated

Use [`fluid_synth_get_chorus_group_speed()`](chorus-effect.md#fluid_synth_get_chorus_group_speed) in new code instead.

### `fluid_synth_get_chorus_depth()` {#fluid_synth_get_chorus_depth}

```c
double fluid_synth_get_chorus_depth(fluid_synth_t *synth)
```

Get chorus depth of all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Chorus depth

Deprecated

Use [`fluid_synth_get_chorus_group_depth()`](chorus-effect.md#fluid_synth_get_chorus_group_depth) in new code instead.

### `fluid_synth_get_chorus_type()` {#fluid_synth_get_chorus_type}

```c
int fluid_synth_get_chorus_type(fluid_synth_t *synth)
```

Get chorus waveform type of all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** Chorus waveform type ([`fluid_chorus_mod`](chorus-effect.md#fluid_chorus_mod))

Deprecated

Use [`fluid_synth_get_chorus_group_type()`](chorus-effect.md#fluid_synth_get_chorus_group_type) in new code instead.

### `fluid_synth_set_chorus_group_nr()` {#fluid_synth_set_chorus_group_nr}

```c
int fluid_synth_set_chorus_group_nr(fluid_synth_t *synth, int fx_group, int nr)
```

Set chorus voice count nr to one or all chorus groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter will be applied to all groups. |
| `nr` | Voice count to set. Must be in the range indicated by settings_synth_chorus_nr |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_set_chorus_group_level()` {#fluid_synth_set_chorus_group_level}

```c
int fluid_synth_set_chorus_group_level(fluid_synth_t *synth, int fx_group, double level)
```

Set chorus output level to one or all chorus groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter will be applied to all groups. |
| `level` | Output level to set. Must be in the range indicated by settings_synth_chorus_level |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_set_chorus_group_speed()` {#fluid_synth_set_chorus_group_speed}

```c
int fluid_synth_set_chorus_group_speed(fluid_synth_t *synth, int fx_group, double speed)
```

Set chorus lfo speed to one or all chorus groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter will be applied to all groups. |
| `speed` | Lfo speed to set. Must be in the range indicated by settings_synth_chorus_speed |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_set_chorus_group_depth()` {#fluid_synth_set_chorus_group_depth}

```c
int fluid_synth_set_chorus_group_depth(fluid_synth_t *synth, int fx_group, double depth_ms)
```

Set chorus lfo depth to one or all chorus groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter will be applied to all groups. |
| `depth_ms` | lfo depth to set. Must be in the range indicated by settings_synth_chorus_depth |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_set_chorus_group_type()` {#fluid_synth_set_chorus_group_type}

```c
int fluid_synth_set_chorus_group_type(fluid_synth_t *synth, int fx_group, int type)
```

Set chorus lfo waveform type to one or all chorus groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group. Must be in the range `-1 to (`. If -1 the parameter will be applied to all groups. |
| `type` | Lfo waveform type to set. ([`fluid_chorus_mod`](chorus-effect.md#fluid_chorus_mod)) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_get_chorus_group_nr()` {#fluid_synth_get_chorus_group_nr}

```c
int fluid_synth_get_chorus_group_nr(fluid_synth_t *synth, int fx_group, int *nr)
```

Get chorus count nr of one or all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group from which to fetch the chorus voice count. Must be in the range `-1 to (`. If -1 the parameter common to all fx groups is fetched. |
| `nr` | valid pointer on value to return. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_get_chorus_group_level()` {#fluid_synth_get_chorus_group_level}

```c
int fluid_synth_get_chorus_group_level(fluid_synth_t *synth, int fx_group, double *level)
```

Get chorus output level of one or all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group from which chorus level to fetch. Must be in the range `-1 to (`. If -1 the parameter common to all fx groups is fetched. |
| `level` | valid pointer on value to return. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_get_chorus_group_speed()` {#fluid_synth_get_chorus_group_speed}

```c
int fluid_synth_get_chorus_group_speed(fluid_synth_t *synth, int fx_group, double *speed)
```

Get chorus waveform lfo speed of one or all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance. |
| `fx_group` | Index of the fx group from which lfo speed to fetch. Must be in the range `-1 to (`. If -1 the parameter common to all fx groups is fetched. |
| `speed` | valid pointer on value to return. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise.

### `fluid_synth_get_chorus_group_depth()` {#fluid_synth_get_chorus_group_depth}

```c
int fluid_synth_get_chorus_group_depth(fluid_synth_t *synth, int fx_group, double *depth_ms)
```

Get chorus lfo depth of one or all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `fx_group` | Index of the fx group from which lfo depth to fetch. Must be in the range `-1 to (`. If -1 the parameter common to all fx groups is fetched. |
| `depth_ms` | valid pointer on value to return. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_get_chorus_group_type()` {#fluid_synth_get_chorus_group_type}

```c
int fluid_synth_get_chorus_group_type(fluid_synth_t *synth, int fx_group, int *type)
```

Get chorus waveform type of one or all fx groups. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `fx_group` | Index of the fx group from which to fetch the waveform type. Must be in the range `-1 to (`. If -1 the parameter common to all fx groups is fetched. |
| `type` | valid pointer on waveform type to return ([`fluid_chorus_mod`](chorus-effect.md#fluid_chorus_mod)) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise
