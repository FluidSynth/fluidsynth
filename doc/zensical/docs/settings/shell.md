# Shell (command line) settings


## `shell.prompt` {#settings_shell_prompt}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `(empty string)` |

In dump mode we set the prompt to "" (empty string). The ui cannot easily handle lines, which don't end with cr. Changing the prompt cannot be done through a command, because the current shell does not handle empty arguments.

## `shell.port` {#settings_shell_port}

| Property | Value |
|----------|-------|
| Type | `int` |
| Default | `9800` |
| Min | `1` |
| Max | `65535` |

The shell can be used in a client/server mode. This setting controls what TCP/IP port the server uses.
