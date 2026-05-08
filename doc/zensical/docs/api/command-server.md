# Command Server

TCP socket server for a command handler.

The socket server will open the TCP port set by settings_shell_port (default 9800) and starts a new thread and [`Command Handler`](command-handler.md) for each incoming connection.

> **Note:** The server is only available if libfluidsynth has been compiled with network support (enable-network). Without network support, all related functions will return FLUID_FAILED or NULL.

## Functions

### `new_fluid_server()` {#new_fluid_server}

```c
fluid_server_t * new_fluid_server(fluid_settings_t *settings, fluid_synth_t *synth, fluid_midi_router_t *router)
```

### `new_fluid_server2()` {#new_fluid_server2}

```c
fluid_server_t * new_fluid_server2(fluid_settings_t *settings, fluid_synth_t *synth, fluid_midi_router_t *router, fluid_player_t *player)
```

### `delete_fluid_server()` {#delete_fluid_server}

```c
void delete_fluid_server(fluid_server_t *server)
```

### `fluid_server_join()` {#fluid_server_join}

```c
int fluid_server_join(fluid_server_t *server)
```
