# SoundFont Modulators

SoundFont modulator functions and constants.

## Enumerations

### `fluid_mod_flags` {#fluid_mod_flags}

| Value | Description |
|-------|-------------|
| `FLUID_MOD_POSITIVE` |  |
| `FLUID_MOD_NEGATIVE` |  |
| `FLUID_MOD_UNIPOLAR` |  |
| `FLUID_MOD_BIPOLAR` |  |
| `FLUID_MOD_LINEAR` |  |
| `FLUID_MOD_CONCAVE` |  |
| `FLUID_MOD_CONVEX` |  |
| `FLUID_MOD_SWITCH` |  |
| `FLUID_MOD_GC` |  |
| `FLUID_MOD_CC` |  |
| `FLUID_MOD_CUSTOM` |  |
| `FLUID_MOD_SIN` |  |

Flags defining the polarity, mapping function and type of a modulator source. Compare with SoundFont 2.04 PDF section 8.2.

Note: Bit values do not correspond to the SoundFont spec! Also note that `FLUID_MOD_GC` and `FLUID_MOD_CC` are in the flags field instead of the source field.

### `fluid_mod_transforms` {#fluid_mod_transforms}

| Value | Description |
|-------|-------------|
| `FLUID_MOD_TRANSFORM_LINEAR` |  |
| `FLUID_MOD_TRANSFORM_ABS` |  |

Transform types for the SoundFont2 modulators as defined by SoundFont 2.04 section 8.3.

### `fluid_mod_src` {#fluid_mod_src}

| Value | Description |
|-------|-------------|
| `FLUID_MOD_NONE` |  |
| `FLUID_MOD_VELOCITY` |  |
| `FLUID_MOD_KEY` |  |
| `FLUID_MOD_KEYPRESSURE` |  |
| `FLUID_MOD_CHANNELPRESSURE` |  |
| `FLUID_MOD_PITCHWHEEL` |  |
| `FLUID_MOD_PITCHWHEELSENS` |  |

General controller (if `FLUID_MOD_GC` in flags). This corresponds to SoundFont 2.04 PDF section 8.2.1

## Types

### `fluid_mod_mapping_t` {#fluid_mod_mapping_t}

```c
typedef typedef double(* fluid_mod_mapping_t) (const fluid_mod_t *mod, int value, int range, int is_src1, void *data);
```

This function transforms or maps a modulator source value into a normalized range of `[-1.0;+1.0]`.

See [`fluid_mod_set_custom_mapping()`](modulators.md#fluid_mod_set_custom_mapping).

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance. The behavior is undefined if you modify `mod` through any of the `fluid_mod_set*()` functions from within the callback. |
| `value` | The input value from the modulator source, which will be in range `[0;16383]` if the input source value is `FLUID_MOD_PITCHWHEEL`, or `[0;127]` otherwise. |
| `range` | The value-range of the modulator source, i.e. `16384`, if the input source value is `FLUID_MOD_PITCHWHEEL`, otherwise `128`. |
| `data` | Custom data pointer, as supplied via [`fluid_mod_set_custom_mapping()`](modulators.md#fluid_mod_set_custom_mapping). |
| `is_src1` | A boolean, which, if true, indicates that the mapping function is called for source1. Otherwise, it's called for source2. Only useful if two sources have been specified with the `FLUID_MOD_CUSTOM` flag set. |

**Returns:** A value mapped into range `[-1.0;+1.0]`. For return values that exceed the mentioned range, the behavior is unspecified (i.e. it may be honored, it may be clipped, ignored, the entire modulator may be disabled, etc.).

**Since:** 2.5.0

## Functions

### `new_fluid_mod()` {#new_fluid_mod}

```c
fluid_mod_t * new_fluid_mod(void)
```

Create a new uninitialized modulator structure.

**Returns:** New allocated modulator or NULL if out of memory

### `delete_fluid_mod()` {#delete_fluid_mod}

```c
void delete_fluid_mod(fluid_mod_t *mod)
```

Free a modulator structure.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | Modulator to free |

### `fluid_mod_sizeof()` {#fluid_mod_sizeof}

```c
size_t fluid_mod_sizeof(void)
```

Returns the size of the fluid_mod_t structure.

**Returns:** Size of fluid_mod_t in bytes

### `fluid_mod_set_source1()` {#fluid_mod_set_source1}

```c
void fluid_mod_set_source1(fluid_mod_t *mod, int src, int flags)
```

Set a modulator's primary source controller and flags.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |
| `src` | Modulator source ([`fluid_mod_src`](modulators.md#fluid_mod_src) or a MIDI controller number) |
| `flags` | Flags determining mapping function and whether the source controller is a general controller (`FLUID_MOD_GC`) or a MIDI CC controller (`FLUID_MOD_CC`), see [`fluid_mod_flags`](modulators.md#fluid_mod_flags). |

### `fluid_mod_set_source2()` {#fluid_mod_set_source2}

```c
void fluid_mod_set_source2(fluid_mod_t *mod, int src, int flags)
```

Set a modulator's secondary source controller and flags.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |
| `src` | Modulator source ([`fluid_mod_src`](modulators.md#fluid_mod_src) or a MIDI controller number) |
| `flags` | Flags determining mapping function and whether the source controller is a general controller (`FLUID_MOD_GC`) or a MIDI CC controller (`FLUID_MOD_CC`), see [`fluid_mod_flags`](modulators.md#fluid_mod_flags). |

### `fluid_mod_set_dest()` {#fluid_mod_set_dest}

```c
void fluid_mod_set_dest(fluid_mod_t *mod, int dst)
```

Set the destination effect of a modulator.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |
| `dest` | Destination generator ([`fluid_gen_type`](generators.md#fluid_gen_type)) |

### `fluid_mod_set_amount()` {#fluid_mod_set_amount}

```c
void fluid_mod_set_amount(fluid_mod_t *mod, double amount)
```

Set the scale amount of a modulator.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |
| `amount` | Scale amount to assign |

### `fluid_mod_set_transform()` {#fluid_mod_set_transform}

```c
void fluid_mod_set_transform(fluid_mod_t *mod, int type)
```

Set the transform type of a modulator.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |
| `type` | Transform type, see [`fluid_mod_transforms`](modulators.md#fluid_mod_transforms) |

### `fluid_mod_set_custom_mapping()` {#fluid_mod_set_custom_mapping}

```c
void fluid_mod_set_custom_mapping(fluid_mod_t *mod, fluid_mod_mapping_t mapping_function, void *data)
```

Set a user defined mapping function of type [`fluid_mod_mapping_t`](modulators.md#fluid_mod_mapping_t) to `mod`. To use this function, specify `FLUID_MOD_CUSTOM` as source flag. 

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |
| `mapping_function` | Pointer to the mapping function to assign |
| `data` | User defined data pointer that will be passed into the mapping function, or NULL if not needed |

### `fluid_mod_get_source1()` {#fluid_mod_get_source1}

```c
int fluid_mod_get_source1(const fluid_mod_t *mod)
```

Get the primary source value from a modulator.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |

**Returns:** The primary source value ([`fluid_mod_src`](modulators.md#fluid_mod_src) or a MIDI CC controller value).

### `fluid_mod_get_flags1()` {#fluid_mod_get_flags1}

```c
int fluid_mod_get_flags1(const fluid_mod_t *mod)
```

Get primary source flags from a modulator.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |

**Returns:** The primary source flags ([`fluid_mod_flags`](modulators.md#fluid_mod_flags)).

### `fluid_mod_get_source2()` {#fluid_mod_get_source2}

```c
int fluid_mod_get_source2(const fluid_mod_t *mod)
```

Get the secondary source value from a modulator.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |

**Returns:** The secondary source value ([`fluid_mod_src`](modulators.md#fluid_mod_src) or a MIDI CC controller value).

### `fluid_mod_get_flags2()` {#fluid_mod_get_flags2}

```c
int fluid_mod_get_flags2(const fluid_mod_t *mod)
```

Get secondary source flags from a modulator.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |

**Returns:** The secondary source flags ([`fluid_mod_flags`](modulators.md#fluid_mod_flags)).

### `fluid_mod_get_dest()` {#fluid_mod_get_dest}

```c
int fluid_mod_get_dest(const fluid_mod_t *mod)
```

Get destination effect from a modulator.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |

**Returns:** Destination generator ([`fluid_gen_type`](generators.md#fluid_gen_type))

### `fluid_mod_get_amount()` {#fluid_mod_get_amount}

```c
double fluid_mod_get_amount(const fluid_mod_t *mod)
```

Get the scale amount from a modulator.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |

**Returns:** Scale amount

### `fluid_mod_get_transform()` {#fluid_mod_get_transform}

```c
int fluid_mod_get_transform(const fluid_mod_t *mod)
```

Get the transform type of a modulator.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |

**Returns:** Transform type, see [`fluid_mod_transforms`](modulators.md#fluid_mod_transforms)

### `fluid_mod_test_identity()` {#fluid_mod_test_identity}

```c
int fluid_mod_test_identity(const fluid_mod_t *mod1, const fluid_mod_t *mod2)
```

Checks if two modulators are identical in sources, flags and destination.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod1` | First modulator |
| `mod2` | Second modulator |

**Returns:** TRUE if identical, FALSE otherwise

### `fluid_mod_has_source()` {#fluid_mod_has_source}

```c
int fluid_mod_has_source(const fluid_mod_t *mod, int cc, int ctrl)
```

Check if the modulator has the given source.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |
| `cc` | Boolean value indicating if ctrl is a CC controller or not |
| `ctrl` | The source to check for (if `cc` == FALSE : a value of type [`fluid_mod_src`](modulators.md#fluid_mod_src), else the value of the MIDI CC to check for) |

**Returns:** TRUE if the modulator has the given source, FALSE otherwise.

### `fluid_mod_has_dest()` {#fluid_mod_has_dest}

```c
int fluid_mod_has_dest(const fluid_mod_t *mod, int gen)
```

Check if the modulator has the given destination.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | The modulator instance |
| `gen` | The destination generator of type [`fluid_gen_type`](generators.md#fluid_gen_type) to check for |

**Returns:** TRUE if the modulator has the given destination, FALSE otherwise.

### `fluid_mod_clone()` {#fluid_mod_clone}

```c
void fluid_mod_clone(fluid_mod_t *mod, const fluid_mod_t *src)
```

Clone the modulators destination, sources, flags and amount.

**Parameters:**

| Name | Description |
|------|-------------|
| `mod` | the modulator to store the copy to |
| `src` | the source modulator to retrieve the information from |

> **Note:** The `next` member of `mod` will be left unchanged.
