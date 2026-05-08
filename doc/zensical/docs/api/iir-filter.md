# Effect - IIR Filter

Functions for configuring the built-in IIR filter effect

## Enumerations

### `fluid_iir_filter_type` {#fluid_iir_filter_type}

| Value | Description |
|-------|-------------|
| `FLUID_IIR_DISABLED` |  |
| `FLUID_IIR_LOWPASS` |  |
| `FLUID_IIR_HIGHPASS` |  |
| `FLUID_IIR_LAST` |  |

Specifies the type of filter to use for the custom IIR filter

### `fluid_iir_filter_flags` {#fluid_iir_filter_flags}

| Value | Description |
|-------|-------------|
| `FLUID_IIR_Q_LINEAR` |  |
| `FLUID_IIR_Q_ZERO_OFF` |  |
| `FLUID_IIR_NO_GAIN_AMP` |  |
| `FLUID_IIR_BEANLAND` |  |

Specifies optional settings to use for the custom IIR filter. Can be bitwise ORed.

## Functions

### `fluid_synth_set_custom_filter()` {#fluid_synth_set_custom_filter}

```c
int fluid_synth_set_custom_filter(fluid_synth_t *, int type, int flags)
```

Configure a general-purpose IIR biquad filter.

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `type` | Type of the IIR filter to use (see [`fluid_iir_filter_type`](iir-filter.md#fluid_iir_filter_type)) |
| `flags` | Additional flags to customize this filter or zero to stay with the default (see [`fluid_iir_filter_flags`](iir-filter.md#fluid_iir_filter_flags)) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if the settings have been successfully applied, otherwise [`FLUID_FAILED`](misc.md#FLUID_FAILED)

`FLUID_IIR_DISABLED`).
