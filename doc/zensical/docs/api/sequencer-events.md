# Sequencer Events

Create, modify, query and destroy sequencer events.

## Enumerations

### `fluid_seq_event_type` {#fluid_seq_event_type}

| Value | Description |
|-------|-------------|
| `FLUID_SEQ_NOTE` |  |
| `FLUID_SEQ_NOTEON` |  |
| `FLUID_SEQ_NOTEOFF` |  |
| `FLUID_SEQ_ALLSOUNDSOFF` |  |
| `FLUID_SEQ_ALLNOTESOFF` |  |
| `FLUID_SEQ_BANKSELECT` |  |
| `FLUID_SEQ_PROGRAMCHANGE` |  |
| `FLUID_SEQ_PROGRAMSELECT` |  |
| `FLUID_SEQ_PITCHBEND` |  |
| `FLUID_SEQ_PITCHWHEELSENS` |  |
| `FLUID_SEQ_MODULATION` |  |
| `FLUID_SEQ_SUSTAIN` |  |
| `FLUID_SEQ_CONTROLCHANGE` |  |
| `FLUID_SEQ_PAN` |  |
| `FLUID_SEQ_VOLUME` |  |
| `FLUID_SEQ_REVERBSEND` |  |
| `FLUID_SEQ_CHORUSSEND` |  |
| `FLUID_SEQ_TIMER` |  |
| `FLUID_SEQ_CHANNELPRESSURE` |  |
| `FLUID_SEQ_KEYPRESSURE` |  |
| `FLUID_SEQ_SYSTEMRESET` |  |
| `FLUID_SEQ_UNREGISTERING` |  |
| `FLUID_SEQ_SCALE` |  |
| `FLUID_SEQ_LASTEVENT` |  |

Sequencer event type enumeration.

## Functions

### `new_fluid_event()` {#new_fluid_event}

```c
fluid_event_t * new_fluid_event(void)
```

Create a new sequencer event structure. 

**Returns:** New sequencer event structure or NULL if out of memory

### `delete_fluid_event()` {#delete_fluid_event}

```c
void delete_fluid_event(fluid_event_t *evt)
```

Delete a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure created by [`new_fluid_event()`](sequencer-events.md#new_fluid_event). |

### `fluid_event_set_source()` {#fluid_event_set_source}

```c
void fluid_event_set_source(fluid_event_t *evt, fluid_seq_id_t src)
```

Set source of a sequencer event. `src` must be a unique sequencer ID or -1 if not set. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `src` | Unique sequencer ID |

### `fluid_event_set_dest()` {#fluid_event_set_dest}

```c
void fluid_event_set_dest(fluid_event_t *evt, fluid_seq_id_t dest)
```

Set destination of this sequencer event, i.e. the sequencer client this event will be sent to. `dest` must be a unique sequencer ID. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `dest` | The destination unique sequencer ID |

### `fluid_event_timer()` {#fluid_event_timer}

```c
void fluid_event_timer(fluid_event_t *evt, void *data)
```

Set a sequencer event to be a timer event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `data` | User supplied data pointer |

### `fluid_event_note()` {#fluid_event_note}

```c
void fluid_event_note(fluid_event_t *evt, int channel, short key, short vel, unsigned int duration)
```

Set a sequencer event to be a note duration event.

Before fluidsynth 2.2.0, this event type was naively implemented when used in conjunction with [`fluid_sequencer_register_fluidsynth()`](sequencer.md#fluid_sequencer_register_fluidsynth), because it simply enqueued a [`fluid_event_noteon()`](sequencer-events.md#fluid_event_noteon) and [`fluid_event_noteoff()`](sequencer-events.md#fluid_event_noteoff). A handling for overlapping notes was not implemented. Starting with 2.2.0, this changes: If a [`fluid_event_note()`](sequencer-events.md#fluid_event_note) is already playing, while another [`fluid_event_note()`](sequencer-events.md#fluid_event_note) arrives on the same `channel` and `key`, the earlier event will be canceled. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `key` | MIDI note number (0-127) |
| `vel` | MIDI velocity value (1-127) |
| `duration` | Duration of note in the time scale used by the sequencer |

> **Note:** The application should decide whether to use only Notes with duration, or separate NoteOn and NoteOff events.

> **Warning:** Calling this function with `vel` or `duration` being zero results in undefined behavior!

### `fluid_event_noteon()` {#fluid_event_noteon}

```c
void fluid_event_noteon(fluid_event_t *evt, int channel, short key, short vel)
```

Set a sequencer event to be a note on event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `key` | MIDI note number (0-127) |
| `vel` | MIDI velocity value (0-127) |

> **Note:** Since fluidsynth 2.2.2, this function will give you a `FLUID_SEQ_NOTEOFF` when called with `vel` being zero.

### `fluid_event_noteoff()` {#fluid_event_noteoff}

```c
void fluid_event_noteoff(fluid_event_t *evt, int channel, short key)
```

Set a sequencer event to be a note off event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `key` | MIDI note number (0-127) |

### `fluid_event_all_sounds_off()` {#fluid_event_all_sounds_off}

```c
void fluid_event_all_sounds_off(fluid_event_t *evt, int channel)
```

Set a sequencer event to be an all sounds off event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |

### `fluid_event_all_notes_off()` {#fluid_event_all_notes_off}

```c
void fluid_event_all_notes_off(fluid_event_t *evt, int channel)
```

Set a sequencer event to be a all notes off event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |

### `fluid_event_bank_select()` {#fluid_event_bank_select}

```c
void fluid_event_bank_select(fluid_event_t *evt, int channel, short bank_num)
```

Set a sequencer event to be a bank select event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `bank_num` | MIDI bank number (0-16383) |

### `fluid_event_program_change()` {#fluid_event_program_change}

```c
void fluid_event_program_change(fluid_event_t *evt, int channel, int preset_num)
```

Set a sequencer event to be a program change event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `val` | MIDI program number (0-127) |

### `fluid_event_program_select()` {#fluid_event_program_select}

```c
void fluid_event_program_select(fluid_event_t *evt, int channel, unsigned int sfont_id, short bank_num, short preset_num)
```

Set a sequencer event to be a program select event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `sfont_id` | SoundFont ID number |
| `bank_num` | MIDI bank number (0-16383) |
| `preset_num` | MIDI preset number (0-127) |

### `fluid_event_control_change()` {#fluid_event_control_change}

```c
void fluid_event_control_change(fluid_event_t *evt, int channel, short control, int val)
```

Set a sequencer event to be a MIDI control change event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `control` | MIDI control number (0-127) |
| `val` | MIDI control value (0-127) |

### `fluid_event_pitch_bend()` {#fluid_event_pitch_bend}

```c
void fluid_event_pitch_bend(fluid_event_t *evt, int channel, int val)
```

Set a sequencer event to be a pitch bend event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `pitch` | MIDI pitch bend value (0-16383, 8192 = no bend) |

### `fluid_event_pitch_wheelsens()` {#fluid_event_pitch_wheelsens}

```c
void fluid_event_pitch_wheelsens(fluid_event_t *evt, int channel, int val)
```

Set a sequencer event to be a pitch wheel sensitivity event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `value` | MIDI pitch wheel sensitivity value in semitones |

### `fluid_event_modulation()` {#fluid_event_modulation}

```c
void fluid_event_modulation(fluid_event_t *evt, int channel, int val)
```

Set a sequencer event to be a modulation event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `val` | MIDI modulation value (0-127) |

### `fluid_event_sustain()` {#fluid_event_sustain}

```c
void fluid_event_sustain(fluid_event_t *evt, int channel, int val)
```

Set a sequencer event to be a MIDI sustain event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `val` | MIDI sustain value (0-127) |

### `fluid_event_pan()` {#fluid_event_pan}

```c
void fluid_event_pan(fluid_event_t *evt, int channel, int val)
```

Set a sequencer event to be a stereo pan event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `val` | MIDI panning value (0-127, 0=left, 64 = middle, 127 = right) |

### `fluid_event_volume()` {#fluid_event_volume}

```c
void fluid_event_volume(fluid_event_t *evt, int channel, int val)
```

Set a sequencer event to be a volume event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `val` | Volume value (0-127) |

### `fluid_event_reverb_send()` {#fluid_event_reverb_send}

```c
void fluid_event_reverb_send(fluid_event_t *evt, int channel, int val)
```

Set a sequencer event to be a reverb send event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `val` | Reverb amount (0-127) |

### `fluid_event_chorus_send()` {#fluid_event_chorus_send}

```c
void fluid_event_chorus_send(fluid_event_t *evt, int channel, int val)
```

Set a sequencer event to be a chorus send event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `val` | Chorus amount (0-127) |

### `fluid_event_key_pressure()` {#fluid_event_key_pressure}

```c
void fluid_event_key_pressure(fluid_event_t *evt, int channel, short key, int val)
```

Set a sequencer event to be a polyphonic aftertouch event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `key` | MIDI note number (0-127) |
| `val` | Aftertouch amount (0-127) |

**Since:** 2.0.0

### `fluid_event_channel_pressure()` {#fluid_event_channel_pressure}

```c
void fluid_event_channel_pressure(fluid_event_t *evt, int channel, int val)
```

Set a sequencer event to be a channel-wide aftertouch event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `channel` | MIDI channel number |
| `val` | Aftertouch amount (0-127) |

**Since:** 1.1.0

### `fluid_event_system_reset()` {#fluid_event_system_reset}

```c
void fluid_event_system_reset(fluid_event_t *evt)
```

Set a sequencer event to be a midi system reset event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Since:** 1.1.0

### `fluid_event_unregistering()` {#fluid_event_unregistering}

```c
void fluid_event_unregistering(fluid_event_t *evt)
```

Set a sequencer event to be an unregistering event. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Since:** 1.1.0

### `fluid_event_scale()` {#fluid_event_scale}

```c
void fluid_event_scale(fluid_event_t *evt, double new_scale)
```

Set a sequencer event to be a scale change event. Useful for scheduling tempo changes. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `new_scale` | The new time scale to apply to the sequencer, see [`fluid_sequencer_set_time_scale()`](sequencer.md#fluid_sequencer_set_time_scale) |

**Since:** 2.2.0

### `fluid_event_from_midi_event()` {#fluid_event_from_midi_event}

```c
int fluid_event_from_midi_event(fluid_event_t *, const fluid_midi_event_t *)
```

Transforms an incoming MIDI event (from a MIDI driver or MIDI router) to a sequencer event.

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |
| `event` | MIDI event |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) or [`FLUID_FAILED`](misc.md#FLUID_FAILED)

> **Note:** This function copies the fields of the MIDI event into the provided sequencer event. Calling applications must create the sequencer event and set additional fields such as the source and destination of the sequencer event.

```c
//...getMIDIevent,e.g.usingplayer_callback()

//SendMIDIeventtosequencertoplay
fluid_event_t*evt=new_fluid_event();
fluid_event_set_source(evt,-1);
fluid_event_set_dest(evt,seqid);
fluid_event_from_midi_event(evt,event);
fluid_sequencer_send_at(sequencer,evt,50,0);//relativetime
delete_fluid_event(evt);
```

**Since:** 2.2.7

### `fluid_event_get_type()` {#fluid_event_get_type}

```c
int fluid_event_get_type(fluid_event_t *evt)
```

Get the event type ([`fluid_seq_event_type`](sequencer-events.md#fluid_seq_event_type)) field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** Event type ([`fluid_seq_event_type`](sequencer-events.md#fluid_seq_event_type)).

### `fluid_event_get_source()` {#fluid_event_get_source}

```c
fluid_seq_id_t fluid_event_get_source(fluid_event_t *evt)
```

Get the source sequencer client from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** source field of the sequencer event

### `fluid_event_get_dest()` {#fluid_event_get_dest}

```c
fluid_seq_id_t fluid_event_get_dest(fluid_event_t *evt)
```

Get the dest sequencer client from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** dest field of the sequencer event

### `fluid_event_get_channel()` {#fluid_event_get_channel}

```c
int fluid_event_get_channel(fluid_event_t *evt)
```

Get the MIDI channel field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** MIDI zero-based channel number

### `fluid_event_get_key()` {#fluid_event_get_key}

```c
short fluid_event_get_key(fluid_event_t *evt)
```

Get the MIDI note field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** MIDI note number (0-127)

### `fluid_event_get_velocity()` {#fluid_event_get_velocity}

```c
short fluid_event_get_velocity(fluid_event_t *evt)
```

Get the MIDI velocity field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** MIDI velocity value (0-127)

### `fluid_event_get_control()` {#fluid_event_get_control}

```c
short fluid_event_get_control(fluid_event_t *evt)
```

Get the MIDI control number field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** MIDI control number (0-127)

### `fluid_event_get_value()` {#fluid_event_get_value}

```c
int fluid_event_get_value(fluid_event_t *evt)
```

Get the value field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** Value field of event.

`FLUID_SEQ_PROGRAMCHANGE`, `FLUID_SEQ_PROGRAMSELECT` (preset_num), `FLUID_SEQ_PITCHWHEELSENS`, `FLUID_SEQ_MODULATION`, `FLUID_SEQ_SUSTAIN`, `FLUID_SEQ_CONTROLCHANGE`, `FLUID_SEQ_PAN`, `FLUID_SEQ_VOLUME`, `FLUID_SEQ_REVERBSEND`, `FLUID_SEQ_CHORUSSEND`.

### `fluid_event_get_program()` {#fluid_event_get_program}

```c
int fluid_event_get_program(fluid_event_t *evt)
```

Get the MIDI program field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** MIDI program number (0-127)

`FLUID_SEQ_PROGRAMCHANGE` and `FLUID_SEQ_PROGRAMSELECT` event types.

### `fluid_event_get_data()` {#fluid_event_get_data}

```c
void * fluid_event_get_data(fluid_event_t *evt)
```

Get the data field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** Data field of event.

`FLUID_SEQ_TIMER` event type.

### `fluid_event_get_duration()` {#fluid_event_get_duration}

```c
unsigned int fluid_event_get_duration(fluid_event_t *evt)
```

Get the duration field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** Note duration value in the time scale used by the sequencer (by default milliseconds)

`FLUID_SEQ_NOTE` event type.

### `fluid_event_get_bank()` {#fluid_event_get_bank}

```c
short fluid_event_get_bank(fluid_event_t *evt)
```

Get the MIDI bank field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** MIDI bank number (0-16383)

`FLUID_SEQ_BANKSELECT` and `FLUID_SEQ_PROGRAMSELECT` event types.

### `fluid_event_get_pitch()` {#fluid_event_get_pitch}

```c
int fluid_event_get_pitch(fluid_event_t *evt)
```

Get the pitch field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** MIDI pitch bend pitch value (0-16383, 8192 = no bend)

`FLUID_SEQ_PITCHBEND` event type.

### `fluid_event_get_scale()` {#fluid_event_get_scale}

```c
double fluid_event_get_scale(fluid_event_t *evt)
```

Gets time scale field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** SoundFont identifier value.

`FLUID_SEQ_SCALE` event type.

### `fluid_event_get_sfont_id()` {#fluid_event_get_sfont_id}

```c
unsigned int fluid_event_get_sfont_id(fluid_event_t *evt)
```

Get the SoundFont ID field from a sequencer event structure. 

**Parameters:**

| Name | Description |
|------|-------------|
| `evt` | Sequencer event structure |

**Returns:** SoundFont identifier value.

`FLUID_SEQ_PROGRAMSELECT` event type.
