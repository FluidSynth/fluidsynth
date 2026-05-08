# MIDI Channel Messages

The MIDI channel message functions are mostly directly named after their counterpart MIDI messages. They are a high-level interface to controlling the synthesizer, playing notes and changing note and channel parameters.

## Functions

### `fluid_synth_noteon()` {#fluid_synth_noteon}

```c
int fluid_synth_noteon(fluid_synth_t *synth, int chan, int key, int vel)
```

Send a note-on event to a FluidSynth object.

This function will take care of proper legato playing. If a note on channel `chan` is already playing at the given key `key`, it will be released (even if it is sustained). In other words, overlapping notes are not allowed. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `key` | MIDI note number (0-127) |
| `vel` | MIDI velocity (0-127, 0=noteoff) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_noteoff()` {#fluid_synth_noteoff}

```c
int fluid_synth_noteoff(fluid_synth_t *synth, int chan, int key)
```

Sends a note-off event to a FluidSynth object. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `key` | MIDI note number (0-127) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise (may just mean that no voices matched the note off event)

### `fluid_synth_cc()` {#fluid_synth_cc}

```c
int fluid_synth_cc(fluid_synth_t *synth, int chan, int ctrl, int val)
```

Send a MIDI controller event on a MIDI channel.

Most CCs are 7-bits wide in FluidSynth. There are a few exceptions which may be 14-bits wide as are documented here: [https://github.com/FluidSynth/fluidsynth/wiki/FluidFeatures#midi-control-change-implementation-chart](https://github.com/FluidSynth/fluidsynth/wiki/FluidFeatures#midi-control-change-implementation-chart)

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `num` | MIDI controller number (0-127) |
| `val` | MIDI controller value (0-127) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** This function supports MIDI Global Controllers which will be sent to all channels of the basic channel if this basic channel is in mode OmniOff/Mono. This is accomplished by sending the CC one MIDI channel below the basic channel of the receiver. Examples: let a synthesizer with 16 MIDI channels:

- Let a basic channel 7 in mode 3 (Omni Off, Mono). If MIDI channel 6 is disabled it could be used as CC global for all channels belonging to basic channel 7.
- Let a basic channel 0 in mode 3. If MIDI channel 15 is disabled it could be used as CC global for all channels belonging to basic channel 0.

> **Warning:** Contrary to the MIDI Standard, this function does not clear LSB controllers, when MSB controllers are received.

### `fluid_synth_get_cc()` {#fluid_synth_get_cc}

```c
int fluid_synth_get_cc(fluid_synth_t *synth, int chan, int ctrl, int *pval)
```

Get current MIDI controller value on a MIDI channel. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `num` | MIDI controller number (0-127) |
| `pval` | Location to store MIDI controller value (0-127) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_sysex()` {#fluid_synth_sysex}

```c
int fluid_synth_sysex(fluid_synth_t *synth, const char *data, int len, char *response, int *response_len, int *handled, int dryrun)
```

Process a MIDI SYSEX (system exclusive) message. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `data` | Buffer containing SYSEX data (not including 0xF0 and 0xF7) |
| `len` | Length of data in buffer |
| `response` | Buffer to store response to or NULL to ignore |
| `response_len` | IN/OUT parameter, in: size of response buffer, out: amount of data written to response buffer (if [`FLUID_FAILED`](misc.md#FLUID_FAILED) is returned and this value is non-zero, it indicates the response buffer is too small) |
| `handled` | Optional location to store boolean value if message was recognized and handled or not (set to TRUE if it was handled) |
| `dryrun` | TRUE to just do a dry run but not actually execute the SYSEX command (useful for checking if a SYSEX message would be handled) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.1.0

> **Note:** When Fluidsynth receives an XG System Mode ON message, it compares the `synth` 's deviceID directly with the deviceID of the SysEx message. This is contrary to the XG spec (page 42), which requires to only compare the lower nibble. However, following the XG spec seems to break drum channels for a lot of MIDI files out there and therefore we've decided for this customization. If you rely on XG System Mode ON messages, make sure to set the setting settings_synth_device-id to match the deviceID provided in the SysEx message (in most cases, this will be `deviceID=16`).

```c
SYSEXformat(0xF0and0xF7bytesshallnotbepassedtothisfunction):
Non-realtime:0xF00x7E<DeviceId>\[BODY\]0xF7
Realtime:0xF00x7F<DeviceId>\[BODY\]0xF7
Tuningmessages:0xF00x7E/0x7F<DeviceId>0x08<subID2>\[BODY\]<ChkSum>0xF7
GSDT1messages:0xF00x41<DeviceId>0x420x12[ADDRESS(3bytes)]\[DATA\]<ChkSum>0xF7
```

### `fluid_synth_pitch_bend()` {#fluid_synth_pitch_bend}

```c
int fluid_synth_pitch_bend(fluid_synth_t *synth, int chan, int val)
```

Set the MIDI pitch bend controller value on a MIDI channel. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `val` | MIDI pitch bend value (0-16383 with 8192 being center) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_get_pitch_bend()` {#fluid_synth_get_pitch_bend}

```c
int fluid_synth_get_pitch_bend(fluid_synth_t *synth, int chan, int *ppitch_bend)
```

Get the MIDI pitch bend controller value on a MIDI channel. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `ppitch_bend` | Location to store MIDI pitch bend value (0-16383 with 8192 being center) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_pitch_wheel_sens()` {#fluid_synth_pitch_wheel_sens}

```c
int fluid_synth_pitch_wheel_sens(fluid_synth_t *synth, int chan, int val)
```

Set MIDI pitch wheel sensitivity on a MIDI channel. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `val` | Pitch wheel sensitivity value in semitones |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_get_pitch_wheel_sens()` {#fluid_synth_get_pitch_wheel_sens}

```c
int fluid_synth_get_pitch_wheel_sens(fluid_synth_t *synth, int chan, int *pval)
```

Get MIDI pitch wheel sensitivity on a MIDI channel. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `pval` | Location to store pitch wheel sensitivity value in semitones |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** Sometime AFTER v1.0 API freeze.

### `fluid_synth_program_change()` {#fluid_synth_program_change}

```c
int fluid_synth_program_change(fluid_synth_t *synth, int chan, int program)
```

Send a program change event on a MIDI channel. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `prognum` | MIDI program number (0-127) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_channel_pressure()` {#fluid_synth_channel_pressure}

```c
int fluid_synth_channel_pressure(fluid_synth_t *synth, int chan, int val)
```

Set the MIDI channel pressure controller value. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `val` | MIDI channel pressure value (0-127) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_key_pressure()` {#fluid_synth_key_pressure}

```c
int fluid_synth_key_pressure(fluid_synth_t *synth, int chan, int key, int val)
```

Set the MIDI polyphonic key pressure controller value. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `key` | MIDI key number (0-127) |
| `val` | MIDI key pressure value (0-127) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 2.0.0

### `fluid_synth_bank_select()` {#fluid_synth_bank_select}

```c
int fluid_synth_bank_select(fluid_synth_t *synth, int chan, int bank)
```

Set instrument bank number on a MIDI channel. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `bank` | MIDI bank number |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** This function does not change the instrument currently assigned to `chan`, as it is usually called prior to [`fluid_synth_program_change()`](midi-messages.md#fluid_synth_program_change). If you still want instrument changes to take effect immediately, call [`fluid_synth_program_reset()`](midi-messages.md#fluid_synth_program_reset) after having set up the bank configuration.

### `fluid_synth_sfont_select()` {#fluid_synth_sfont_select}

```c
int fluid_synth_sfont_select(fluid_synth_t *synth, int chan, int sfont_id)
```

Set SoundFont ID on a MIDI channel. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `sfont_id` | ID of a loaded SoundFont |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** This function does not change the instrument currently assigned to `chan`, as it is usually called prior to [`fluid_synth_bank_select()`](midi-messages.md#fluid_synth_bank_select) or [`fluid_synth_program_change()`](midi-messages.md#fluid_synth_program_change). If you still want instrument changes to take effect immediately, call [`fluid_synth_program_reset()`](midi-messages.md#fluid_synth_program_reset) after having selected the soundfont.

### `fluid_synth_program_select()` {#fluid_synth_program_select}

```c
int fluid_synth_program_select(fluid_synth_t *synth, int chan, int sfont_id, int bank_num, int preset_num)
```

Select an instrument on a MIDI channel by SoundFont ID, bank and program numbers. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `sfont_id` | ID of a loaded SoundFont |
| `bank_num` | MIDI bank number |
| `preset_num` | MIDI program number |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_program_select_by_sfont_name()` {#fluid_synth_program_select_by_sfont_name}

```c
int fluid_synth_program_select_by_sfont_name(fluid_synth_t *synth, int chan, const char *sfont_name, int bank_num, int preset_num)
```

Select an instrument on a MIDI channel by SoundFont name, bank and program numbers. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `sfont_name` | Name of a loaded SoundFont |
| `bank_num` | MIDI bank number |
| `preset_num` | MIDI program number |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.1.0

### `fluid_synth_get_program()` {#fluid_synth_get_program}

```c
int fluid_synth_get_program(fluid_synth_t *synth, int chan, int *sfont_id, int *bank_num, int *preset_num)
```

Get current SoundFont ID, bank number and program number for a MIDI channel. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `sfont_id` | Location to store SoundFont ID |
| `bank_num` | Location to store MIDI bank number |
| `preset_num` | Location to store MIDI program number |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_unset_program()` {#fluid_synth_unset_program}

```c
int fluid_synth_unset_program(fluid_synth_t *synth, int chan)
```

Set the preset of a MIDI channel to an unassigned state. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.1.1

> **Note:** Channel retains its SoundFont ID and bank numbers, while the program number is set to an "unset" state. MIDI program changes may re-assign a preset if one matches.

### `fluid_synth_program_reset()` {#fluid_synth_program_reset}

```c
int fluid_synth_program_reset(fluid_synth_t *synth)
```

Resend a bank select and a program change for every channel and assign corresponding instruments. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_system_reset()` {#fluid_synth_system_reset}

```c
int fluid_synth_system_reset(fluid_synth_t *synth)
```

Send MIDI system reset command (big red 'panic' button), turns off notes, resets controllers and restores initial basic channel configuration. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_all_notes_off()` {#fluid_synth_all_notes_off}

```c
int fluid_synth_all_notes_off(fluid_synth_t *synth, int chan)
```

Turn off all voices that are playing on the given MIDI channel, by putting them into release phase. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1), (chan=-1 selects all channels) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.1.4

### `fluid_synth_all_sounds_off()` {#fluid_synth_all_sounds_off}

```c
int fluid_synth_all_sounds_off(fluid_synth_t *synth, int chan)
```

Immediately stop all voices on the given MIDI channel (skips release phase). 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1), (chan=-1 selects all channels) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.1.4

### `fluid_synth_set_gen()` {#fluid_synth_set_gen}

```c
int fluid_synth_set_gen(fluid_synth_t *synth, int chan, int param, float value)
```

Apply an offset to a SoundFont generator on a MIDI channel.

This function allows to set an offset for the specified destination generator in realtime. The offset will be applied immediately to all voices that are currently and subsequently playing on the given MIDI channel. This functionality works equivalent to using NRPN MIDI messages to manipulate synthesis parameters. See SoundFont spec, paragraph 8.1.3, for details on SoundFont generator parameters and valid ranges, as well as paragraph 9.6 for details on NRPN messages. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `param` | SoundFont generator ID ([`fluid_gen_type`](generators.md#fluid_gen_type)) |
| `value` | Offset value (in native units of the generator) to assign to the MIDI channel |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

### `fluid_synth_get_gen()` {#fluid_synth_get_gen}

```c
float fluid_synth_get_gen(fluid_synth_t *synth, int chan, int param)
```

Retrieve the generator NRPN offset assigned to a MIDI channel.

The value returned is in native units of the generator. By default, the offset is zero. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `chan` | MIDI channel number (0 to MIDI channel count - 1) |
| `param` | SoundFont generator ID ([`fluid_gen_type`](generators.md#fluid_gen_type)) |

**Returns:** Current NRPN generator offset value assigned to the MIDI channel
