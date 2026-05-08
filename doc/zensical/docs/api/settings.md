# Settings

Functions for settings management

To create a synthesizer object you will have to specify its settings. These settings are stored in a fluid_settings_t object. 

```c
void
my_synthesizer()
{
fluid_settings_t*settings;
fluid_synth_t*synth;
fluid_audio_driver_t*adriver;

settings=new_fluid_settings();
fluid_settings_setstr(settings,"audio.driver","alsa");
//...changesettings...
synth=new_fluid_synth(settings);
adriver=new_fluid_audio_driver(settings,synth);
//...
}
```

**See also:** `Creating and changing the settings`

## Enumerations

### `fluid_types_enum` {#fluid_types_enum}

| Value | Description |
|-------|-------------|
| `FLUID_NO_TYPE` |  |
| `FLUID_NUM_TYPE` |  |
| `FLUID_INT_TYPE` |  |
| `FLUID_STR_TYPE` |  |
| `FLUID_SET_TYPE` |  |

Settings type

Each setting has a defined type: numeric (double), integer, string or a set of values. The type of each setting can be retrieved using the function [`fluid_settings_get_type()`](settings.md#fluid_settings_get_type)

## Types

### `fluid_settings_foreach_option_t` {#fluid_settings_foreach_option_t}

```c
typedef typedef void(* fluid_settings_foreach_option_t) (void *data, const char *name, const char *option);
```

Callback function type used with [`fluid_settings_foreach_option()`](settings.md#fluid_settings_foreach_option)

**Parameters:**

| Name | Description |
|------|-------------|
| `data` | User defined data pointer |
| `name` | Setting name |
| `option` | A string option for this setting (iterates through the list) |

### `fluid_settings_foreach_t` {#fluid_settings_foreach_t}

```c
typedef typedef void(* fluid_settings_foreach_t) (void *data, const char *name, int type);
```

Callback function type used with [`fluid_settings_foreach()`](settings.md#fluid_settings_foreach)

**Parameters:**

| Name | Description |
|------|-------------|
| `data` | User defined data pointer |
| `name` | Setting name |
| `type` | Setting type ([`fluid_types_enum`](settings.md#fluid_types_enum)) |

## Functions

### `new_fluid_settings()` {#new_fluid_settings}

```c
fluid_settings_t * new_fluid_settings(void)
```

Create a new settings object

**Returns:** the pointer to the settings object

### `delete_fluid_settings()` {#delete_fluid_settings}

```c
void delete_fluid_settings(fluid_settings_t *settings)
```

Delete the provided settings object

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |

### `fluid_settings_get_type()` {#fluid_settings_get_type}

```c
int fluid_settings_get_type(fluid_settings_t *settings, const char *name)
```

Get the type of the setting with the given name

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |

**Returns:** the type for the named setting (see [`fluid_types_enum`](settings.md#fluid_types_enum)), or `FLUID_NO_TYPE` when it does not exist

### `fluid_settings_get_hints()` {#fluid_settings_get_hints}

```c
int fluid_settings_get_hints(fluid_settings_t *settings, const char *name, int *val)
```

Get the hints for the named setting as an integer bitmap

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `hints` | set to the hints associated to the setting if it exists |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if hints associated to the named setting exist, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_settings_is_realtime()` {#fluid_settings_is_realtime}

```c
int fluid_settings_is_realtime(fluid_settings_t *settings, const char *name)
```

Ask whether the setting is changeable in realtime.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |

**Returns:** TRUE if the setting is changeable in realtime, FALSE otherwise

> **Note:** Before using this function, make sure the `settings` object has already been used to create a synthesizer, a MIDI driver, an audio driver, a MIDI player, or a command handler (depending on which settings you want to query).

### `fluid_settings_setstr()` {#fluid_settings_setstr}

```c
int fluid_settings_setstr(fluid_settings_t *settings, const char *name, const char *str)
```

Set a string value for a named setting

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `str` | new string value |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the value has been set, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_settings_copystr()` {#fluid_settings_copystr}

```c
int fluid_settings_copystr(fluid_settings_t *settings, const char *name, char *str, int len)
```

Copy the value of a string setting into the provided buffer (thread safe)

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `str` | Caller supplied buffer to copy string value to |
| `len` | Size of 'str' buffer (no more than len bytes will be written, which will always include a zero terminator) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the value exists, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** A size of 256 should be more than sufficient for the string buffer.

**Since:** 1.1.0

### `fluid_settings_dupstr()` {#fluid_settings_dupstr}

```c
int fluid_settings_dupstr(fluid_settings_t *settings, const char *name, char **str)
```

Duplicate the value of a string setting

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `str` | Location to store pointer to allocated duplicate string |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the value exists and was successfully duplicated, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

[`fluid_settings_copystr()`](settings.md#fluid_settings_copystr) but allocates a new copy of the string. Caller owns the string and should free it with [`fluid_free()`](misc.md#fluid_free) when done using it.

**Since:** 1.1.0

### `fluid_settings_getstr_default()` {#fluid_settings_getstr_default}

```c
int fluid_settings_getstr_default(fluid_settings_t *settings, const char *name, char **def)
```

Get the default value of a string setting.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `def` | the default string value of the setting if it exists |

**Returns:** FLUID_OK if a default value exists, FLUID_FAILED otherwise

> **Note:** The returned string is not owned by the caller and should not be modified or freed.

### `fluid_settings_str_equal()` {#fluid_settings_str_equal}

```c
int fluid_settings_str_equal(fluid_settings_t *settings, const char *name, const char *value)
```

Test a string setting for some value.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `s` | a string to be tested |

**Returns:** TRUE if the value exists and is equal to `s`, FALSE otherwise

### `fluid_settings_setnum()` {#fluid_settings_setnum}

```c
int fluid_settings_setnum(fluid_settings_t *settings, const char *name, double val)
```

Set a numeric value for a named setting.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `val` | new setting's value |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the value has been set, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_settings_getnum()` {#fluid_settings_getnum}

```c
int fluid_settings_getnum(fluid_settings_t *settings, const char *name, double *val)
```

Get the numeric value of a named setting

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `val` | variable pointer to receive the setting's numeric value |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the value exists, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_settings_getnum_default()` {#fluid_settings_getnum_default}

```c
int fluid_settings_getnum_default(fluid_settings_t *settings, const char *name, double *val)
```

Get the default value of a named numeric (double) setting

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `val` | set to the default value if the named setting exists |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the default value of the named setting exists, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_settings_getnum_range()` {#fluid_settings_getnum_range}

```c
int fluid_settings_getnum_range(fluid_settings_t *settings, const char *name, double *min, double *max)
```

Get the range of values of a numeric setting

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `min` | setting's range lower limit |
| `max` | setting's range upper limit |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the setting's range exists, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_settings_setint()` {#fluid_settings_setint}

```c
int fluid_settings_setint(fluid_settings_t *settings, const char *name, int val)
```

Set an integer value for a setting

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `val` | new setting's integer value |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the value has been set, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_settings_getint()` {#fluid_settings_getint}

```c
int fluid_settings_getint(fluid_settings_t *settings, const char *name, int *val)
```

Get an integer value setting.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `val` | pointer to a variable to receive the setting's integer value |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the value exists, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_settings_getint_default()` {#fluid_settings_getint_default}

```c
int fluid_settings_getint_default(fluid_settings_t *settings, const char *name, int *val)
```

Get the default value of an integer setting.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `val` | set to the setting's default integer value if it exists |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the setting's default integer value exists, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_settings_getint_range()` {#fluid_settings_getint_range}

```c
int fluid_settings_getint_range(fluid_settings_t *settings, const char *name, int *min, int *max)
```

Get the range of values of an integer setting

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `min` | setting's range lower limit |
| `max` | setting's range upper limit |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the setting's range exists, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_settings_foreach_option()` {#fluid_settings_foreach_option}

```c
void fluid_settings_foreach_option(fluid_settings_t *settings, const char *name, void *data, fluid_settings_foreach_option_t func)
```

Iterate the available options for a named string setting, calling the provided callback function for each existing option.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | a setting's name |
| `data` | any user provided pointer |
| `func` | callback function to be called on each iteration |

> **Note:** Starting with FluidSynth 1.1.0 the `func` callback is called for each option in alphabetical order. Sort order was undefined in previous versions.

### `fluid_settings_option_count()` {#fluid_settings_option_count}

```c
int fluid_settings_option_count(fluid_settings_t *settings, const char *name)
```

Count option string values for a string setting.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `name` | Name of setting |

**Returns:** Count of options for this string setting (0 if none, -1 if not found or not a string setting)

**Since:** 1.1.0

### `fluid_settings_option_concat()` {#fluid_settings_option_concat}

```c
char * fluid_settings_option_concat(fluid_settings_t *settings, const char *name, const char *separator)
```

Concatenate options for a string setting together with a separator between.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | Settings object |
| `name` | Settings name |
| `separator` | String to use between options (NULL to use ", ") |

**Returns:** Newly allocated string or NULL on error (out of memory, not a valid setting `name` or not a string setting). Free the string when finished with it by using [`fluid_free()`](misc.md#fluid_free).

**Since:** 1.1.0

### `fluid_settings_foreach()` {#fluid_settings_foreach}

```c
void fluid_settings_foreach(fluid_settings_t *settings, void *data, fluid_settings_foreach_t func)
```

Iterate the existing settings defined in a settings object, calling the provided callback function for each setting.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | a settings object |
| `data` | any user provided pointer |
| `func` | callback function to be called on each iteration |

> **Note:** Starting with FluidSynth 1.1.0 the `func` callback is called for each setting in alphabetical order. Sort order was undefined in previous versions.

### `fluid_synth_get_settings()` {#fluid_synth_get_settings}

```c
fluid_settings_t * fluid_synth_get_settings(fluid_synth_t *synth)
```

Get settings assigned to a synth. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** FluidSynth settings which are assigned to the synth

## Macros

### `FLUID_HINT_BOUNDED_BELOW` {#FLUID_HINT_BOUNDED_BELOW}

Hint FLUID_HINT_BOUNDED_BELOW indicates that the LowerBound field of the FLUID_PortRangeHint should be considered meaningful. The value in this field should be considered the (inclusive) lower bound of the valid range. If FLUID_HINT_SAMPLE_RATE is also specified then the value of LowerBound should be multiplied by the sample rate.

### `FLUID_HINT_BOUNDED_ABOVE` {#FLUID_HINT_BOUNDED_ABOVE}

Hint FLUID_HINT_BOUNDED_ABOVE indicates that the UpperBound field of the FLUID_PortRangeHint should be considered meaningful. The value in this field should be considered the (inclusive) upper bound of the valid range. If FLUID_HINT_SAMPLE_RATE is also specified then the value of UpperBound should be multiplied by the sample rate.

### `FLUID_HINT_TOGGLED` {#FLUID_HINT_TOGGLED}

Hint FLUID_HINT_TOGGLED indicates that the data item should be considered a Boolean toggle. Data less than or equal to zero should be considered off or false, and data above zero should be considered on or true. FLUID_HINT_TOGGLED may not be used in conjunction with any other hint.

### `FLUID_HINT_OPTIONLIST` {#FLUID_HINT_OPTIONLIST}

Setting is a list of string options
