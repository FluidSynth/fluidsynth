# Logging

Logging interface

The default logging function of the fluidsynth prints its messages to the stderr. The synthesizer uses five level of messages: `FLUID_PANIC`, `FLUID_ERR`, `FLUID_WARN`, `FLUID_INFO`, and `FLUID_DBG`.

A client application can install a new log function to handle the messages differently. In the following example, the application sets a callback function to display `FLUID_PANIC` messages in a dialog, and ignores all other messages by setting the log function to NULL:

```c
fluid_set_log_function(FLUID_PANIC,show_dialog,(void*)root_window);
fluid_set_log_function(FLUID_ERR,NULL,NULL);
fluid_set_log_function(FLUID_WARN,NULL,NULL);
fluid_set_log_function(FLUID_DBG,NULL,NULL);
```

> **Note:** The logging configuration is global and not tied to a specific synthesizer instance. That means that all synthesizer instances created in the same process share the same logging configuration.

## Enumerations

### `fluid_log_level` {#fluid_log_level}

| Value | Description |
|-------|-------------|
| `FLUID_PANIC` |  |
| `FLUID_ERR` |  |
| `FLUID_WARN` |  |
| `FLUID_INFO` |  |
| `FLUID_DBG` |  |
| `LAST_LOG_LEVEL` |  |

FluidSynth log levels.

## Types

### `fluid_log_function_t` {#fluid_log_function_t}

```c
typedef typedef void(* fluid_log_function_t) (int level, const char *message, void *data);
```

Log function handler callback type used by [`fluid_set_log_function()`](logging.md#fluid_set_log_function).

**Parameters:**

| Name | Description |
|------|-------------|
| `level` | Log level ([`fluid_log_level`](logging.md#fluid_log_level)) |
| `message` | Log message text |
| `data` | User data pointer supplied to [`fluid_set_log_function()`](logging.md#fluid_set_log_function). |

## Functions

### `fluid_set_log_function()` {#fluid_set_log_function}

```c
fluid_log_function_t fluid_set_log_function(int level, fluid_log_function_t fun, void *data)
```

Installs a new log function for a specified log level. 

**Parameters:**

| Name | Description |
|------|-------------|
| `level` | Log level to install handler for. |
| `fun` | Callback function handler to call for logged messages |
| `data` | User supplied data pointer to pass to log function |

**Returns:** The previously installed function.

### `fluid_default_log_function()` {#fluid_default_log_function}

```c
void fluid_default_log_function(int level, const char *message, void *data)
```

Default log function which prints to the stderr. 

**Parameters:**

| Name | Description |
|------|-------------|
| `level` | Log level |
| `message` | Log message |
| `data` | User supplied data (not used) |

### `fluid_log()` {#fluid_log}

```c
int fluid_log(int level, const char *fmt,...)
```

Print a message to the log. 

**Parameters:**

| Name | Description |
|------|-------------|
| `level` | Log level ([`fluid_log_level`](logging.md#fluid_log_level)). |
| `fmt` | Printf style format string for log message |
| `...` | Arguments for printf 'fmt' message string |

**Returns:** Always returns [`FLUID_FAILED`](misc.md#FLUID_FAILED)
