# Command Handler

Handles text commands and reading of configuration files.

## Functions

### `new_fluid_cmd_handler()` {#new_fluid_cmd_handler}

```c
fluid_cmd_handler_t * new_fluid_cmd_handler(fluid_synth_t *synth, fluid_midi_router_t *router)
```

### `new_fluid_cmd_handler2()` {#new_fluid_cmd_handler2}

```c
fluid_cmd_handler_t * new_fluid_cmd_handler2(fluid_settings_t *settings, fluid_synth_t *synth, fluid_midi_router_t *router, fluid_player_t *player)
```

### `delete_fluid_cmd_handler()` {#delete_fluid_cmd_handler}

```c
void delete_fluid_cmd_handler(fluid_cmd_handler_t *handler)
```

### `fluid_cmd_handler_set_synth()` {#fluid_cmd_handler_set_synth}

```c
void fluid_cmd_handler_set_synth(fluid_cmd_handler_t *handler, fluid_synth_t *synth)
```

### `fluid_command()` {#fluid_command}

```c
int fluid_command(fluid_cmd_handler_t *handler, const char *cmd, fluid_ostream_t out)
```

### `fluid_source()` {#fluid_source}

```c
int fluid_source(fluid_cmd_handler_t *handler, const char *filename)
```
