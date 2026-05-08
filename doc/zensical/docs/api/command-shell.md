# Command Shell

Interactive shell to control and configure a synthesizer instance.

If you need a platform independent way to get the standard input and output streams, use [`fluid_get_stdin()`](command-interface.md#fluid_get_stdin) and [`fluid_get_stdout()`](command-interface.md#fluid_get_stdout).

For a full list of available commands, type `help` in the shell.

## Functions

### `new_fluid_shell()` {#new_fluid_shell}

```c
fluid_shell_t * new_fluid_shell(fluid_settings_t *settings, fluid_cmd_handler_t *handler, fluid_istream_t in, fluid_ostream_t out, int thread)
```

### `fluid_usershell()` {#fluid_usershell}

```c
void fluid_usershell(fluid_settings_t *settings, fluid_cmd_handler_t *handler)
```

### `delete_fluid_shell()` {#delete_fluid_shell}

```c
void delete_fluid_shell(fluid_shell_t *shell)
```
