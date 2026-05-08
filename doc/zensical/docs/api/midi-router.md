# MIDI Router

Rule based transformation and filtering of MIDI events.

## Enumerations

### `fluid_midi_router_rule_type` {#fluid_midi_router_rule_type}

| Value | Description |
|-------|-------------|
| `FLUID_MIDI_ROUTER_RULE_NOTE` |  |
| `FLUID_MIDI_ROUTER_RULE_CC` |  |
| `FLUID_MIDI_ROUTER_RULE_PROG_CHANGE` |  |
| `FLUID_MIDI_ROUTER_RULE_PITCH_BEND` |  |
| `FLUID_MIDI_ROUTER_RULE_CHANNEL_PRESSURE` |  |
| `FLUID_MIDI_ROUTER_RULE_KEY_PRESSURE` |  |
| `FLUID_MIDI_ROUTER_RULE_COUNT` |  |

MIDI router rule type.

**Since:** 1.1.0

## Functions

### `new_fluid_midi_router()` {#new_fluid_midi_router}

```c
fluid_midi_router_t * new_fluid_midi_router(fluid_settings_t *settings, handle_midi_event_func_t handler, void *event_handler_data)
```

Create a new midi router.

**Parameters:**

| Name | Description |
|------|-------------|
| `settings` | Settings used to configure MIDI router |
| `handler` | MIDI event callback. |
| `event_handler_data` | Caller defined data pointer which gets passed to 'handler' |

**Returns:** New MIDI router instance or NULL on error

The MIDI handler callback should process the possibly filtered/modified MIDI events from the MIDI router and forward them on to a synthesizer for example. The function [`fluid_synth_handle_midi_event()`](midi-input.md#fluid_synth_handle_midi_event) can be used for *handle* and a [`fluid_synth_t`](Types.md#fluid_synth_t) passed as the *event_handler_data* parameter for this purpose.

### `delete_fluid_midi_router()` {#delete_fluid_midi_router}

```c
void delete_fluid_midi_router(fluid_midi_router_t *handler)
```

Delete a MIDI router instance. 

**Parameters:**

| Name | Description |
|------|-------------|
| `router` | MIDI router to delete |

### `fluid_midi_router_set_default_rules()` {#fluid_midi_router_set_default_rules}

```c
int fluid_midi_router_set_default_rules(fluid_midi_router_t *router)
```

Set a MIDI router to use default "unity" rules.

**Parameters:**

| Name | Description |
|------|-------------|
| `router` | Router to set to default rules. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.1.0

### `fluid_midi_router_clear_rules()` {#fluid_midi_router_clear_rules}

```c
int fluid_midi_router_clear_rules(fluid_midi_router_t *router)
```

Clear all rules in a MIDI router.

**Parameters:**

| Name | Description |
|------|-------------|
| `router` | Router to clear all rules from |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

**Since:** 1.1.0

### `fluid_midi_router_add_rule()` {#fluid_midi_router_add_rule}

```c
int fluid_midi_router_add_rule(fluid_midi_router_t *router, fluid_midi_router_rule_t *rule, int type)
```

Add a rule to a MIDI router. 

**Parameters:**

| Name | Description |
|------|-------------|
| `router` | MIDI router |
| `rule` | Rule to add (used directly and should not be accessed again following a successful call to this function). |
| `type` | The type of rule to add ([`fluid_midi_router_rule_type`](midi-router.md#fluid_midi_router_rule_type)) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise (invalid rule for example)

**Since:** 1.1.0

### `new_fluid_midi_router_rule()` {#new_fluid_midi_router_rule}

```c
fluid_midi_router_rule_t * new_fluid_midi_router_rule(void)
```

Create a new MIDI router rule.

**Returns:** Newly allocated router rule or NULL if out of memory.

**Since:** 1.1.0

### `delete_fluid_midi_router_rule()` {#delete_fluid_midi_router_rule}

```c
void delete_fluid_midi_router_rule(fluid_midi_router_rule_t *rule)
```

Free a MIDI router rule.

**Parameters:**

| Name | Description |
|------|-------------|
| `rule` | Router rule to free |

**Since:** 1.1.0

### `fluid_midi_router_rule_set_chan()` {#fluid_midi_router_rule_set_chan}

```c
void fluid_midi_router_rule_set_chan(fluid_midi_router_rule_t *rule, int min, int max, float mul, int add)
```

Set the channel portion of a rule.

**Parameters:**

| Name | Description |
|------|-------------|
| `rule` | MIDI router rule |
| `min` | Minimum value for rule match |
| `max` | Maximum value for rule match |
| `mul` | Value which is multiplied by matching event's channel value (1.0 to not modify) |
| `add` | Value which is added to matching event's channel value (0 to not modify) |

*min* and *max* parameters define a channel range window to match incoming events to. If *min* is less than or equal to *max* then an event is matched if its channel is within the defined range (including *min* and *max*). If *min* is greater than *max* then rule is inverted and matches everything except in *between* the defined range (so *min* and *max* would match).

The *mul* and *add* values are used to modify event channel values prior to sending the event, if the rule matches.

**Since:** 1.1.0

### `fluid_midi_router_rule_set_param1()` {#fluid_midi_router_rule_set_param1}

```c
void fluid_midi_router_rule_set_param1(fluid_midi_router_rule_t *rule, int min, int max, float mul, int add)
```

Set the first parameter portion of a rule.

**Parameters:**

| Name | Description |
|------|-------------|
| `rule` | MIDI router rule |
| `min` | Minimum value for rule match |
| `max` | Maximum value for rule match |
| `mul` | Value which is multiplied by matching event's 1st parameter value (1.0 to not modify) |
| `add` | Value which is added to matching event's 1st parameter value (0 to not modify) |

Pitch bend values have a maximum value of 16383 (8192 is pitch bend center) and all other events have a max of 127. All events have a minimum value of 0.

The *min* and *max* parameters define a parameter range window to match incoming events to. If *min* is less than or equal to *max* then an event is matched if its 1st parameter is within the defined range (including *min* and *max*). If *min* is greater than *max* then rule is inverted and matches everything except in *between* the defined range (so *min* and *max* would match).

The *mul* and *add* values are used to modify event 1st parameter values prior to sending the event, if the rule matches.

**Since:** 1.1.0

### `fluid_midi_router_rule_set_param2()` {#fluid_midi_router_rule_set_param2}

```c
void fluid_midi_router_rule_set_param2(fluid_midi_router_rule_t *rule, int min, int max, float mul, int add)
```

Set the second parameter portion of a rule.

**Parameters:**

| Name | Description |
|------|-------------|
| `rule` | MIDI router rule |
| `min` | Minimum value for rule match |
| `max` | Maximum value for rule match |
| `mul` | Value which is multiplied by matching event's 2nd parameter value (1.0 to not modify) |
| `add` | Value which is added to matching event's 2nd parameter value (0 to not modify) |

All applicable 2nd parameters have the range 0-127.

The *min* and *max* parameters define a parameter range window to match incoming events to. If *min* is less than or equal to *max* then an event is matched if its 2nd parameter is within the defined range (including *min* and *max*). If *min* is greater than *max* then rule is inverted and matches everything except in *between* the defined range (so *min* and *max* would match).

The *mul* and *add* values are used to modify event 2nd parameter values prior to sending the event, if the rule matches.

**Since:** 1.1.0

### `fluid_midi_router_handle_midi_event()` {#fluid_midi_router_handle_midi_event}

```c
int fluid_midi_router_handle_midi_event(void *data, fluid_midi_event_t *event)
```

Handle a MIDI event through a MIDI router instance. 

**Parameters:**

| Name | Description |
|------|-------------|
| `data` | MIDI router instance [`fluid_midi_router_t`](Types.md#fluid_midi_router_t), its a void * so that this function can be used as a callback for other subsystems ([`new_fluid_midi_driver()`](midi-driver.md#new_fluid_midi_driver) for example). |
| `event` | MIDI event to handle |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) if all rules were applied successfully, [`FLUID_FAILED`](misc.md#FLUID_FAILED) if an error occurred while applying a rule or (since 2.2.2) the event was ignored because a parameter was out-of-range after the rule had been applied. See the note below.

In default mode, a noteon event is just forwarded to the synth's 'noteon' function, a 'CC' event to the synth's 'CC' function and so on.

The router can be used to:

- filter messages (for example: Pass sustain pedal CCs only to selected channels)
- split the keyboard (noteon with notenr < x: to ch 1, >x to ch 2)
- layer sounds (for each noteon received on ch 1, create a noteon on ch1, ch2, ch3,...)
- velocity scaling (for each noteon event, scale the velocity by 1.27 to give DX7 users a chance)
- velocity switching ("v <=100: Angel Choir; V > 100: Hell's Bells")
- get rid of aftertouch
- ...

> **Note:** Each input event has values (ch, par1, par2) that could be changed by a rule. After a rule has been applied on any value and the value is out of range, the event can be either ignored or the value can be clamped depending on the type of the event:

- To get full benefice of the rule the value is clamped and the event passed to the output.
- To avoid MIDI messages conflicts at the output, the event is ignored (i.e not passed to the output).

    - ch out of range: event is ignored regardless of the event type.
    - par1 out of range: event is ignored for PROG_CHANGE or CONTROL_CHANGE type, par1 is clamped otherwise.
    - par2 out of range: par2 is clamped regardless of the event type.

### `fluid_midi_dump_prerouter()` {#fluid_midi_dump_prerouter}

```c
int fluid_midi_dump_prerouter(void *data, fluid_midi_event_t *event)
```

MIDI event callback function to display event information to stdout 

**Parameters:**

| Name | Description |
|------|-------------|
| `data` | MIDI router instance |
| `event` | MIDI event data |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

[`handle_midi_event_func_t`](midi-input.md#handle_midi_event_func_t) function type, used for displaying MIDI event information between the MIDI driver and router to stdout. Useful for adding into a MIDI router chain for debugging MIDI events.

### `fluid_midi_dump_postrouter()` {#fluid_midi_dump_postrouter}

```c
int fluid_midi_dump_postrouter(void *data, fluid_midi_event_t *event)
```

MIDI event callback function to display event information to stdout 

**Parameters:**

| Name | Description |
|------|-------------|
| `data` | MIDI router instance |
| `event` | MIDI event data |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

[`handle_midi_event_func_t`](midi-input.md#handle_midi_event_func_t) function type, used for displaying MIDI event information between the MIDI driver and router to stdout. Useful for adding into a MIDI router chain for debugging MIDI events.
