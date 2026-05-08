# MIDI Sequencer

MIDI event sequencer.

The MIDI sequencer can be used to play MIDI events in a more flexible way than using the MIDI file player, which expects the events to be stored as Standard MIDI Files. Using the sequencer, you can provide the events one by one, with an optional timestamp for scheduling.

## Subgroups

- [Sequencer Events](sequencer-events.md)

## Types

### `fluid_event_callback_t` {#fluid_event_callback_t}

```c
typedef typedef void(* fluid_event_callback_t) (unsigned int time, fluid_event_t *event, fluid_sequencer_t *seq, void *data);
```

Event callback prototype for destination clients.

**Parameters:**

| Name | Description |
|------|-------------|
| `time` | Current sequencer tick value (see [`fluid_sequencer_get_tick()`](sequencer.md#fluid_sequencer_get_tick)). |
| `event` | The event being received |
| `seq` | The sequencer instance |
| `data` | User defined data registered with the client |

> **Note:** `time` may not be of the same tick value as the scheduled event! In fact, depending on the sequencer's scale and the synth's sample-rate, `time` may be a few ticks too late. Although this itself is inaudible, it is important to consider, when you use this callback for enqueuing additional events over and over again with [`fluid_sequencer_send_at()`](sequencer.md#fluid_sequencer_send_at): If you enqueue new events with a relative tick value you might introduce a timing error, which causes your sequence to sound e.g. slower than it's supposed to be. If this is your use-case, make sure to enqueue events with an absolute tick value.

## Functions

### `new_fluid_sequencer()` {#new_fluid_sequencer}

```c
fluid_sequencer_t * new_fluid_sequencer(void)
```

Create a new sequencer object which uses the system timer.

**Returns:** New sequencer instance

[`new_fluid_sequencer2()`](sequencer.md#new_fluid_sequencer2) to specify whether the system timer or [`fluid_sequencer_process()`](sequencer.md#fluid_sequencer_process) is used to advance the sequencer.

Deprecated As of fluidsynth 2.1.1 the use of the system timer has been deprecated.

### `new_fluid_sequencer2()` {#new_fluid_sequencer2}

```c
fluid_sequencer_t * new_fluid_sequencer2(int use_system_timer)
```

Create a new sequencer object.

**Parameters:**

| Name | Description |
|------|-------------|
| `use_system_timer` | If TRUE, sequencer will advance at the rate of the system clock. If FALSE, call [`fluid_sequencer_process()`](sequencer.md#fluid_sequencer_process) to advance the sequencer. |

**Returns:** New sequencer instance

> **Note:** As of fluidsynth 2.1.1 the use of the system timer has been deprecated.

**Since:** 1.1.0

### `delete_fluid_sequencer()` {#delete_fluid_sequencer}

```c
void delete_fluid_sequencer(fluid_sequencer_t *seq)
```

Free a sequencer object.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer to delete |

> **Note:** Before fluidsynth 2.1.1 registered sequencer clients may not be fully freed by this function.

### `fluid_sequencer_get_use_system_timer()` {#fluid_sequencer_get_use_system_timer}

```c
int fluid_sequencer_get_use_system_timer(fluid_sequencer_t *seq)
```

Check if a sequencer is using the system timer or not.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |

**Returns:** TRUE if system timer is being used, FALSE otherwise.

Deprecated

As of fluidsynth 2.1.1 the usage of the system timer has been deprecated.

**Since:** 1.1.0

### `fluid_sequencer_register_client()` {#fluid_sequencer_register_client}

```c
fluid_seq_id_t fluid_sequencer_register_client(fluid_sequencer_t *seq, const char *name, fluid_event_callback_t callback, void *data)
```

Register a sequencer client.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |
| `name` | Name of sequencer client |
| `callback` | Sequencer client callback or NULL for a source client. |
| `data` | User data to pass to the *callback* |

**Returns:** Unique sequencer ID or [`FLUID_FAILED`](misc.md#FLUID_FAILED) on error

> **Note:** Implementations are encouraged to explicitly unregister any registered client with [`fluid_sequencer_unregister_client()`](sequencer.md#fluid_sequencer_unregister_client) before deleting the sequencer.

### `fluid_sequencer_unregister_client()` {#fluid_sequencer_unregister_client}

```c
void fluid_sequencer_unregister_client(fluid_sequencer_t *seq, fluid_seq_id_t id)
```

Unregister a previously registered client.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |
| `id` | Client ID as returned by [`fluid_sequencer_register_client()`](sequencer.md#fluid_sequencer_register_client). |

### `fluid_sequencer_count_clients()` {#fluid_sequencer_count_clients}

```c
int fluid_sequencer_count_clients(fluid_sequencer_t *seq)
```

Count a sequencers registered clients.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |

**Returns:** Count of sequencer clients.

### `fluid_sequencer_get_client_id()` {#fluid_sequencer_get_client_id}

```c
fluid_seq_id_t fluid_sequencer_get_client_id(fluid_sequencer_t *seq, int index)
```

Get a client ID from its index (order in which it was registered).

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |
| `index` | Index of register client |

**Returns:** Client ID or [`FLUID_FAILED`](misc.md#FLUID_FAILED) if not found

### `fluid_sequencer_get_client_name()` {#fluid_sequencer_get_client_name}

```c
char * fluid_sequencer_get_client_name(fluid_sequencer_t *seq, fluid_seq_id_t id)
```

Get the name of a registered client.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |
| `id` | Client ID |

**Returns:** Client name or NULL if not found. String is internal and should not be modified or freed.

### `fluid_sequencer_client_is_dest()` {#fluid_sequencer_client_is_dest}

```c
int fluid_sequencer_client_is_dest(fluid_sequencer_t *seq, fluid_seq_id_t id)
```

Check if a client is a destination client.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |
| `id` | Client ID |

**Returns:** TRUE if client is a destination client, FALSE otherwise or if not found

### `fluid_sequencer_process()` {#fluid_sequencer_process}

```c
void fluid_sequencer_process(fluid_sequencer_t *seq, unsigned int msec)
```

Advance a sequencer.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |
| `msec` | Time to advance sequencer to (absolute time since sequencer start). |

[`fluid_sequencer_register_fluidsynth()`](sequencer.md#fluid_sequencer_register_fluidsynth)), the synth will take care of calling [`fluid_sequencer_process()`](sequencer.md#fluid_sequencer_process). Otherwise it is up to the user to advance the sequencer manually.

**Since:** 1.1.0

### `fluid_sequencer_send_now()` {#fluid_sequencer_send_now}

```c
void fluid_sequencer_send_now(fluid_sequencer_t *seq, fluid_event_t *evt)
```

Send an event immediately.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |
| `evt` | Event to send (not copied, used directly) |

### `fluid_sequencer_send_at()` {#fluid_sequencer_send_at}

```c
int fluid_sequencer_send_at(fluid_sequencer_t *seq, fluid_event_t *evt, unsigned int time, int absolute)
```

Schedule an event for sending at a later time.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |
| `evt` | Event to send (will be copied into internal queue) |
| `time` | Time value in ticks (in milliseconds with the default time scale of 1000). |
| `absolute` | TRUE if *time* is absolute sequencer time (time since sequencer creation), FALSE if relative to current time. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** The sequencer sorts events according to their timestamp `time`. For events that have the same timestamp, fluidsynth (as of version 2.2.0) uses the following order, according to which events will be dispatched to the client's callback function.

- `FLUID_SEQ_NOTEOFF` events precede any other event type.
- `FLUID_SEQ_SYSTEMRESET` events are only preceded by `FLUID_SEQ_NOTEOFF` events.
- `FLUID_SEQ_UNREGISTERING` events succeed `FLUID_SEQ_SYSTEMRESET` and precede other event type.
- `FLUID_SEQ_NOTEON` and `FLUID_SEQ_NOTE` events succeed any other event type.
- Otherwise the order is undefined. Or mathematically: `FLUID_SEQ_NOTEOFF` < `FLUID_SEQ_SYSTEMRESET` < `FLUID_SEQ_UNREGISTERING` < ... < (`FLUID_SEQ_NOTEON` && `FLUID_SEQ_NOTE`)

> **Warning:** Be careful with relative ticks when sending many events! See [`fluid_event_callback_t`](sequencer.md#fluid_event_callback_t) for details.

### `fluid_sequencer_remove_events()` {#fluid_sequencer_remove_events}

```c
void fluid_sequencer_remove_events(fluid_sequencer_t *seq, fluid_seq_id_t source, fluid_seq_id_t dest, int type)
```

Remove events from the event queue.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |
| `source` | Source client ID to match or -1 for wildcard |
| `dest` | Destination client ID to match or -1 for wildcard |
| `type` | Event type to match or -1 for wildcard ([`fluid_seq_event_type`](sequencer-events.md#fluid_seq_event_type)) |

### `fluid_sequencer_get_tick()` {#fluid_sequencer_get_tick}

```c
unsigned int fluid_sequencer_get_tick(fluid_sequencer_t *seq)
```

Get the current tick of the sequencer scaled by the time scale currently set.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |

**Returns:** Current tick value

### `fluid_sequencer_set_time_scale()` {#fluid_sequencer_set_time_scale}

```c
void fluid_sequencer_set_time_scale(fluid_sequencer_t *seq, double scale)
```

Set the time scale of a sequencer.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object |
| `scale` | Sequencer scale value in ticks per second (default is 1000 for 1 tick per millisecond) |

> **Note:** May only be called from a sequencer callback or initially when no event dispatching happens. Otherwise it will mess up your event timing, because you have zero control over which events are affected by the scale change.

### `fluid_sequencer_get_time_scale()` {#fluid_sequencer_get_time_scale}

```c
double fluid_sequencer_get_time_scale(fluid_sequencer_t *seq)
```

Get a sequencer's time scale.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer object. |

**Returns:** Time scale value in ticks per second.

### `fluid_sequencer_register_fluidsynth()` {#fluid_sequencer_register_fluidsynth}

```c
fluid_seq_id_t fluid_sequencer_register_fluidsynth(fluid_sequencer_t *seq, fluid_synth_t *synth)
```

Registers a synthesizer as a destination client of the given sequencer.

**Parameters:**

| Name | Description |
|------|-------------|
| `seq` | Sequencer instance |
| `synth` | Synthesizer instance |

**Returns:** Sequencer client ID, or [`FLUID_FAILED`](misc.md#FLUID_FAILED) on error.

[`fluid_sequencer_register_client()`](sequencer.md#fluid_sequencer_register_client), that allows you to easily process and render enqueued sequencer events with fluidsynth's synthesizer. The client being registered will be named `fluidsynth`.

> **Note:** Implementations are encouraged to explicitly unregister this client either by calling [`fluid_sequencer_unregister_client()`](sequencer.md#fluid_sequencer_unregister_client) or by sending an unregistering event to the sequencer. Before fluidsynth 2.1.1 this was mandatory to avoid memory leaks.

```c
fluid_seq_id_tseqid=fluid_sequencer_register_fluidsynth(seq,synth);

//...dowork

fluid_event_t*evt=new_fluid_event();
fluid_event_set_source(evt,-1);
fluid_event_set_dest(evt,seqid);
fluid_event_unregistering(evt);

//unregisterthe"fluidsynth"clientimmediately
fluid_sequencer_send_now(seq,evt);
delete_fluid_event(evt);
delete_fluid_synth(synth);
delete_fluid_sequencer(seq);
```

### `fluid_sequencer_add_midi_event_to_buffer()` {#fluid_sequencer_add_midi_event_to_buffer}

```c
int fluid_sequencer_add_midi_event_to_buffer(void *data, fluid_midi_event_t *event)
```

Transforms an incoming MIDI event (from a MIDI driver or MIDI router) to a sequencer event and adds it to the sequencer queue for sending as soon as possible.

**Parameters:**

| Name | Description |
|------|-------------|
| `data` | The sequencer, must be a valid [`fluid_sequencer_t`](Types.md#fluid_sequencer_t) |
| `event` | MIDI event |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) or [`FLUID_FAILED`](misc.md#FLUID_FAILED)

[`handle_midi_event_func_t`](midi-input.md#handle_midi_event_func_t).

**Since:** 1.1.0
