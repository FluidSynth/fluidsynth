# Command Interface

Control and configuration interface

The command interface allows you to send textual commands to the synthesizer, to parse a command file, or to read commands from the stdin or other input streams (like a TCP socket).

For a full list of available commands, type `help` in the [`Command Shell`](command-shell.md) or send the same command via a command handler. Further documentation can be found at [https://github.com/FluidSynth/fluidsynth/wiki/UserManual#shell-commands](https://github.com/FluidSynth/fluidsynth/wiki/UserManual#shell-commands)

## Subgroups

- [Command Handler](command-handler.md)

- [Command Shell](command-shell.md)

- [Command Server](command-server.md)

## Functions

### `fluid_get_stdin()` {#fluid_get_stdin}

```c
fluid_istream_t fluid_get_stdin(void)
```

Get standard in stream handle. 

**Returns:** Standard in stream.

### `fluid_get_stdout()` {#fluid_get_stdout}

```c
fluid_ostream_t fluid_get_stdout(void)
```

Get standard output stream handle. 

**Returns:** Standard out stream.

### `fluid_get_userconf()` {#fluid_get_userconf}

```c
char * fluid_get_userconf(char *buf, int len)
```

### `fluid_get_sysconf()` {#fluid_get_sysconf}

```c
char * fluid_get_sysconf(char *buf, int len)
```
