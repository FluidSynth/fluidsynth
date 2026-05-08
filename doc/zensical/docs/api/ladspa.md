# Effect - LADSPA

Functions for configuring the LADSPA effects unit

This header defines useful functions for programmatically manipulating the ladspa effects unit of the synth that can be retrieved via [`fluid_synth_get_ladspa_fx()`](ladspa.md#fluid_synth_get_ladspa_fx).

Using any of those functions requires fluidsynth to be compiled with LADSPA support. Else all of those functions are useless dummies.

## Functions

### `fluid_ladspa_is_active()` {#fluid_ladspa_is_active}

```c
int fluid_ladspa_is_active(fluid_ladspa_fx_t *fx)
```

### `fluid_ladspa_activate()` {#fluid_ladspa_activate}

```c
int fluid_ladspa_activate(fluid_ladspa_fx_t *fx)
```

### `fluid_ladspa_deactivate()` {#fluid_ladspa_deactivate}

```c
int fluid_ladspa_deactivate(fluid_ladspa_fx_t *fx)
```

### `fluid_ladspa_reset()` {#fluid_ladspa_reset}

```c
int fluid_ladspa_reset(fluid_ladspa_fx_t *fx)
```

### `fluid_ladspa_check()` {#fluid_ladspa_check}

```c
int fluid_ladspa_check(fluid_ladspa_fx_t *fx, char *err, int err_size)
```

### `fluid_ladspa_host_port_exists()` {#fluid_ladspa_host_port_exists}

```c
int fluid_ladspa_host_port_exists(fluid_ladspa_fx_t *fx, const char *name)
```

### `fluid_ladspa_add_buffer()` {#fluid_ladspa_add_buffer}

```c
int fluid_ladspa_add_buffer(fluid_ladspa_fx_t *fx, const char *name)
```

### `fluid_ladspa_buffer_exists()` {#fluid_ladspa_buffer_exists}

```c
int fluid_ladspa_buffer_exists(fluid_ladspa_fx_t *fx, const char *name)
```

### `fluid_ladspa_add_effect()` {#fluid_ladspa_add_effect}

```c
int fluid_ladspa_add_effect(fluid_ladspa_fx_t *fx, const char *effect_name, const char *lib_name, const char *plugin_name)
```

### `fluid_ladspa_effect_can_mix()` {#fluid_ladspa_effect_can_mix}

```c
int fluid_ladspa_effect_can_mix(fluid_ladspa_fx_t *fx, const char *name)
```

### `fluid_ladspa_effect_set_mix()` {#fluid_ladspa_effect_set_mix}

```c
int fluid_ladspa_effect_set_mix(fluid_ladspa_fx_t *fx, const char *name, int mix, float gain)
```

### `fluid_ladspa_effect_port_exists()` {#fluid_ladspa_effect_port_exists}

```c
int fluid_ladspa_effect_port_exists(fluid_ladspa_fx_t *fx, const char *effect_name, const char *port_name)
```

### `fluid_ladspa_effect_set_control()` {#fluid_ladspa_effect_set_control}

```c
int fluid_ladspa_effect_set_control(fluid_ladspa_fx_t *fx, const char *effect_name, const char *port_name, float val)
```

### `fluid_ladspa_effect_link()` {#fluid_ladspa_effect_link}

```c
int fluid_ladspa_effect_link(fluid_ladspa_fx_t *fx, const char *effect_name, const char *port_name, const char *name)
```

### `fluid_synth_get_ladspa_fx()` {#fluid_synth_get_ladspa_fx}

```c
fluid_ladspa_fx_t * fluid_synth_get_ladspa_fx(fluid_synth_t *synth)
```

Return the LADSPA effects instance used by FluidSynth

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** pointer to LADSPA fx or NULL if compiled without LADSPA support or LADSPA is not active
