# MIDI Input

MIDI Input Subsystem

There are multiple ways to send MIDI events to the synthesizer. They can come from MIDI files, from external MIDI sequencers or raw MIDI event sources, can be modified via MIDI routers and also generated manually.

The interface connecting all sources and sinks of MIDI events in libfluidsynth is [`handle_midi_event_func_t`](midi-input.md#handle_midi_event_func_t).

## Subgroups

- [MIDI Events](midi-events.md)

- [MIDI Router](midi-router.md)

- [MIDI Driver](midi-driver.md)

- [MIDI File Player](midi-player.md)

## Types

### `handle_midi_event_func_t` {#handle_midi_event_func_t}

```c
typedef typedef int(* handle_midi_event_func_t) (void *data, fluid_midi_event_t *event);
```

Generic callback function for MIDI event handler.

**Parameters:**

| Name | Description |
|------|-------------|
| `data` | User defined data pointer |
| `event` | The MIDI event |

**Returns:** Should return [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

- from [`MIDI File Player`](midi-player.md), [`MIDI Router`](midi-router.md) or [`MIDI Driver`](midi-driver.md)
- to [`MIDI Router`](midi-router.md) via [`fluid_midi_router_handle_midi_event()`](midi-router.md#fluid_midi_router_handle_midi_event)
- or to [`Synthesizer`](synth.md) via [`fluid_synth_handle_midi_event()`](midi-input.md#fluid_synth_handle_midi_event).

Additionally, there is a translation layer to pass MIDI events to a [`MIDI Sequencer`](sequencer.md) via [`fluid_sequencer_add_midi_event_to_buffer()`](sequencer.md#fluid_sequencer_add_midi_event_to_buffer).

### `handle_midi_tick_func_t` {#handle_midi_tick_func_t}

```c
typedef typedef int(* handle_midi_tick_func_t) (void *data, int tick);
```

Generic callback function fired once by MIDI tick change.

**Parameters:**

| Name | Description |
|------|-------------|
| `data` | User defined data pointer |
| `tick` | The current (zero-based) tick, which triggered the callback |

**Returns:** Should return [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

It can be used to sync external elements with the beat, or stop / loop the song on a given tick. Ticks being BPM-dependent, you can manipulate values such as bars or beats, without having to care about BPM.

For example, this callback loops the song whenever it reaches the 5th bar :

```c
inthandle_tick(void*data,inttick)
{
fluid_player_t*player=(fluid_player_t*)data;
intppq=192;//FromMIDIheader
intbeatsPerBar=4;//Fromthesong'stimesignature
intloopBar=5;
intloopTick=(loopBar-1)*ppq*beatsPerBar;

if(tick==loopTick)
{
returnfluid_player_seek(player,0);
}

returnFLUID_OK;
}
```

## Functions

### `fluid_synth_handle_midi_event()` {#fluid_synth_handle_midi_event}

```c
int fluid_synth_handle_midi_event(void *data, fluid_midi_event_t *event)
```

Handle MIDI event from MIDI router, used as a callback function. 

**Parameters:**

| Name | Description |
|------|-------------|
| `data` | FluidSynth instance |
| `event` | MIDI event to handle |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise
