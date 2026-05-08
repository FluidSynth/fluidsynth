# MIDI Driver

Functions for managing MIDI drivers.

The available MIDI drivers depend on your platform. See settings_midi for all available configuration options.

To create a MIDI driver, you need to specify a source for the MIDI events to be forwarded to via the [`fluid_midi_event_t`](Types.md#fluid_midi_event_t) callback. Normally this will be either a [`MIDI Router`](midi-router.md) via [`fluid_midi_router_handle_midi_event()`](midi-router.md#fluid_midi_router_handle_midi_event) or the synthesizer via [`fluid_synth_handle_midi_event()`](midi-input.md#fluid_synth_handle_midi_event).

But you can also write your own handler function that preprocesses the events and forwards them on to the router or synthesizer instead.

## Functions

### `new_fluid_midi_driver()` {#new_fluid_midi_driver}

```c
fluid_midi_driver_t * new_fluid_midi_driver(fluid_settings_t *settings, handle_midi_event_func_t handler, void *event_handler_data)
```

Create a new MIDI driver instance.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | Settings used to configure new MIDI driver. See settings_midi for available options. |
| `handler` | MIDI handler callback (for example: [`fluid_midi_router_handle_midi_event()`](midi-router.md#fluid_midi_router_handle_midi_event) for MIDI router) |
| `event_handler_data` | Caller defined data to pass to 'handler' |

**Returns:** New MIDI driver instance or NULL on error

### `delete_fluid_midi_driver()` {#delete_fluid_midi_driver}

```c
void delete_fluid_midi_driver(fluid_midi_driver_t *driver)
```

Delete a MIDI driver instance. 

**Parameters:**

| Name | Description |
|------|-------------|
| `driver` | MIDI driver to delete |
