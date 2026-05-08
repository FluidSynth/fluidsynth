# File Renderer

Functions for managing file renderers and triggering the rendering.

The file renderer is only used to render a MIDI file to audio as fast as possible. Please see `Fast file renderer for non-realtime MIDI file rendering` for a full example.

If you are looking for a way to write audio generated from realtime events (for example from an external sequencer or a MIDI controller) to a file, please have a look at the `file` [`Audio Driver`](audio-driver.md) instead.

## Functions

### `new_fluid_file_renderer()` {#new_fluid_file_renderer}

```c
fluid_file_renderer_t * new_fluid_file_renderer(fluid_synth_t *synth)
```

### `delete_fluid_file_renderer()` {#delete_fluid_file_renderer}

```c
void delete_fluid_file_renderer(fluid_file_renderer_t *dev)
```

### `fluid_file_renderer_process_block()` {#fluid_file_renderer_process_block}

```c
int fluid_file_renderer_process_block(fluid_file_renderer_t *dev)
```

### `fluid_file_set_encoding_quality()` {#fluid_file_set_encoding_quality}

```c
int fluid_file_set_encoding_quality(fluid_file_renderer_t *dev, double q)
```
