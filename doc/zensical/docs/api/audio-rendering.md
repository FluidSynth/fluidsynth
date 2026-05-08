# Audio Rendering

The functions in this section can be used to render audio directly to memory buffers. They are used internally by the [`Audio Driver`](audio-driver.md) and [`File Renderer`](file-renderer.md), but can also be used manually for custom processing of the rendered audio.

> **Note:** Please note that all following functions block during rendering. If your goal is to render realtime audio, ensure that you call these functions from a high-priority thread with little to no other duties other than calling the rendering functions.

> **Warning:** If a concurrently running thread calls any other sound affecting synth function (e.g. [`fluid_synth_noteon()`](midi-messages.md#fluid_synth_noteon), [`fluid_synth_cc()`](midi-messages.md#fluid_synth_cc), etc.) it is unspecified whether the event triggered by such a call will be effective in the recently synthesized audio. While this is inaudible when only requesting small chunks from the synth with every call (cf. [`fluid_synth_get_internal_bufsize()`](synthesis-params.md#fluid_synth_get_internal_bufsize)), it will become evident when requesting larger sample chunks: With larger sample chunks it will get harder for the synth to react on those spontaneously occurring events in time (like events received from a MIDI driver, or directly made synth API calls). In those realtime scenarios, prefer requesting smaller sample chunks from the synth with each call, to avoid poor quantization of your events in the synthesized audio. This issue is not applicable when using the MIDI player or sequencer for event dispatching. Also refer to the documentation of settings_audio_period-size.

## Functions

### `fluid_synth_write_s16()` {#fluid_synth_write_s16}

```c
int fluid_synth_write_s16(fluid_synth_t *synth, int len, void *lout, int loff, int lincr, void *rout, int roff, int rincr)
```

Synthesize a block of 16 bit audio samples to audio buffers. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `len` | Count of audio frames to synthesize |
| `lout` | Array of 16 bit words to store left channel of audio |
| `loff` | Offset index in 'lout' for first sample |
| `lincr` | Increment between samples stored to 'lout' |
| `rout` | Array of 16 bit words to store right channel of audio |
| `roff` | Offset index in 'rout' for first sample |
| `rincr` | Increment between samples stored to 'rout' |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** Should only be called from synthesis thread.

> **Note:** Reverb and Chorus are mixed to `lout` resp. `rout`.

> **Note:** Dithering is performed when converting from internal floating point to 16 bit audio.

### `fluid_synth_write_s24()` {#fluid_synth_write_s24}

```c
int fluid_synth_write_s24(fluid_synth_t *synth, int len, void *lout, int loff, int lincr, void *rout, int roff, int rincr)
```

Synthesize a block of 24 bit audio samples to audio buffers. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `len` | Count of audio frames to synthesize |
| `lout` | Array of 32 bit words to store left channel of audio |
| `loff` | Offset index in 'lout' for first sample |
| `lincr` | Increment between samples stored to 'lout' |
| `rout` | Array of 32 bit words to store right channel of audio |
| `roff` | Offset index in 'rout' for first sample |
| `rincr` | Increment between samples stored to 'rout' |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** Should only be called from synthesis thread.

> **Note:** Reverb and Chorus are mixed to `lout` resp. `rout`.

> **Note:** Output is left-aligned signed 24-bit PCM (24-in-32), produced by clearing the least significant byte after 32-bit conversion from internal floating point. No dithering is performed.

### `fluid_synth_write_s32()` {#fluid_synth_write_s32}

```c
int fluid_synth_write_s32(fluid_synth_t *synth, int len, void *lout, int loff, int lincr, void *rout, int roff, int rincr)
```

Synthesize a block of 32 bit audio samples to audio buffers. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `len` | Count of audio frames to synthesize |
| `lout` | Array of 32 bit words to store left channel of audio |
| `loff` | Offset index in 'lout' for first sample |
| `lincr` | Increment between samples stored to 'lout' |
| `rout` | Array of 32 bit words to store right channel of audio |
| `roff` | Offset index in 'rout' for first sample |
| `rincr` | Increment between samples stored to 'rout' |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** Should only be called from synthesis thread.

> **Note:** Reverb and Chorus are mixed to `lout` resp. `rout`.

> **Note:** Output is signed 32-bit PCM (int32_t), produced by round-and-clip conversion from internal floating point. No dithering is performed.

### `fluid_synth_write_float()` {#fluid_synth_write_float}

```c
int fluid_synth_write_float(fluid_synth_t *synth, int len, void *lout, int loff, int lincr, void *rout, int roff, int rincr)
```

Synthesize a block of floating point audio samples to audio buffers. 

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `len` | Count of audio frames to synthesize |
| `lout` | Array of floats to store left channel of audio |
| `loff` | Offset index in 'lout' for first sample |
| `lincr` | Increment between samples stored to 'lout' |
| `rout` | Array of floats to store right channel of audio |
| `roff` | Offset index in 'rout' for first sample |
| `rincr` | Increment between samples stored to 'rout' |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** Should only be called from synthesis thread.

> **Note:** Reverb and Chorus are mixed to `lout` resp. `rout`.

### `fluid_synth_nwrite_float()` {#fluid_synth_nwrite_float}

```c
int fluid_synth_nwrite_float(fluid_synth_t *synth, int len, float **left, float **right, float **fx_left, float **fx_right)
```

Synthesize a block of floating point audio to separate audio buffers (multi-channel rendering).

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `len` | Count of audio frames to synthesize |
| `left` | Array of float buffers to store left channel of planar audio (as many as `synth.audio-channels` buffers, each of `len` in size) |
| `right` | Array of float buffers to store right channel of planar audio (size: ditto) |
| `fx_left` | Since 1.1.7: If not `NULL`, array of float buffers to store left effect channels (as many as `synth.effects-channels` buffers, each of `len` in size) |
| `fx_right` | Since 1.1.7: If not `NULL`, array of float buffers to store right effect channels (size: ditto) |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise

> **Note:** Should only be called from synthesis thread.

Deprecated

[`fluid_synth_nwrite_float()`](audio-rendering.md#fluid_synth_nwrite_float) is deprecated and will be removed in a future release. It may continue to work or it may return [`FLUID_FAILED`](misc.md#FLUID_FAILED) in the future. Consider using the more powerful and flexible [`fluid_synth_process()`](audio-rendering.md#fluid_synth_process).

Usage example: 

```c
constintFramesToRender=64;
intchannels;
//retrievenumberofstereoaudiochannels
fluid_settings_getint(settings,"synth.audio-channels",&channels);

//weneedtwiceasmany(mono-)buffers
channels*=2;

//fluid_synth_nwrite_floatrendersplanaraudio,e.g.ifsynth.audio-channels==16:
//eachmidichannelgetsrenderedtoitsownstereobuffer,ratherthanhaving
//onebufferandinterleavedPCM
float**mix_buf=newfloat*\[channels\];
for(inti=0;i<channels;i++)
{
mix_buf\[i\]=newfloat\[FramesToRender\];
}

//retrievenumberof(stereo)effectchannels(internallyhardcodedtoreverb(firstchan)
//andchrous(secondchan))
fluid_settings_getint(settings,"synth.effects-channels",&channels);
channels*=2;

float**fx_buf=newfloat*\[channels\];
for(inti=0;i<channels;i++)
{
fx_buf\[i\]=newfloat\[FramesToRender\];
}

float**mix_buf_l=mix_buf;
float**mix_buf_r=&mix_buf\[channels/2\];

float**fx_buf_l=fx_buf;
float**fx_buf_r=&fx_buf\[channels/2\];

fluid_synth_nwrite_float(synth,FramesToRender,mix_buf_l,mix_buf_r,fx_buf_l,fx_buf_r)
```

### `fluid_synth_process()` {#fluid_synth_process}

```c
int fluid_synth_process(fluid_synth_t *synth, int len, int nfx, float *fx\[\], int nout, float *out\[\])
```

Synthesize floating point audio to stereo audio channels (implements the default interface [`fluid_audio_func_t`](audio-driver.md#fluid_audio_func_t)).

**Parameters:**

| Name | Description |
|------|-------------|
| `synth` | FluidSynth instance |
| `len` | Count of audio frames to synthesize and store in every single buffer provided by `out` and `fx`. Zero value is permitted, the function does nothing and return FLUID_OK. |
| `nfx` | Count of arrays in `fx`. Must be a multiple of 2 (because of stereo) and in the range `0 <= nfx/2 <= (`. Note that zero value is valid and allows to skip mixing effects in all fx output buffers. |
| `fx` | Array of buffers to store effects audio to. Buffers may alias with buffers of `out`. Individual NULL buffers are permitted and will cause to skip mixing any audio into that buffer. |
| `nout` | Count of arrays in `out`. Must be a multiple of 2 (because of stereo) and in the range `0 <= nout/2 <= `. Note that zero value is valid and allows to skip mixing dry audio in all out output buffers. |
| `out` | Array of buffers to store (dry) audio to. Buffers may alias with buffers of `fx`. Individual NULL buffers are permitted and will cause to skip mixing any audio into that buffer. |

**Returns:** [`FLUID_OK`](misc.md#FLUID_OK) on success, [`FLUID_FAILED`](misc.md#FLUID_FAILED) otherwise,

- `fx == NULL` while `nfx > 0`, or `out == NULL` while `nout > 0`.
- `nfx` or `nout` not multiple of 2.
- `len < 0`.
- `nfx` or `nout` exceed the range explained above.

**mix** audio to a given number of planar audio buffers. Therefore pass `nout = N*2` float buffers to `out` in order to render the synthesized audio to `N` stereo channels. Each float buffer must be able to hold `len` elements.

`out` contains an array of planar buffers for normal, dry, stereo audio (alternating left and right). Like: 

```c
out\[0\]=left_buffer_audio_channel_0
out\[1\]=right_buffer_audio_channel_0
out\[2\]=left_buffer_audio_channel_1
out\[3\]=right_buffer_audio_channel_1
...
out\[(i*2+0)%nout\]=left_buffer_audio_channel_i
out\[(i*2+1)%nout\]=right_buffer_audio_channel_i
```

for zero-based channel index `i`. The buffer layout of `fx` used for storing effects like reverb and chorus looks similar: 

```c
fx\[0\]=left_buffer_channel_of_reverb_unit_0
fx\[1\]=right_buffer_channel_of_reverb_unit_0
fx\[2\]=left_buffer_channel_of_chorus_unit_0
fx\[3\]=right_buffer_channel_of_chorus_unit_0
fx\[4\]=left_buffer_channel_of_reverb_unit_1
fx\[5\]=right_buffer_channel_of_reverb_unit_1
fx\[6\]=left_buffer_channel_of_chorus_unit_1
fx\[7\]=right_buffer_channel_of_chorus_unit_1
fx\[8\]=left_buffer_channel_of_reverb_unit_2
...
fx\[((k*fluid_synth_count_effects_channels()+j)*2+0)%nfx\]=left_buffer_for_effect_channel_j_of_unit_k
fx\[((k*fluid_synth_count_effects_channels()+j)*2+1)%nfx\]=right_buffer_for_effect_channel_j_of_unit_k
```

`0 <= k < ` is a zero-based index denoting the effects unit and `0 <= j < ` is a zero-based index denoting the effect channel within unit `k`.

Any playing voice is assigned to audio channels based on the MIDI channel it's playing on: Let `chan` be the zero-based MIDI channel index an arbitrary voice is playing on. To determine the audio channel and effects unit it is going to be rendered to use:

`i = chan % `

`k = chan % `

> **Note:** The owner of the sample buffers must zero them out before calling this function, because any synthesized audio is mixed (i.e. added) to the buffers. E.g. if [`fluid_synth_process()`](audio-rendering.md#fluid_synth_process) is called from a custom audio driver process function (see [`new_fluid_audio_driver2()`](audio-driver.md#new_fluid_audio_driver2)), the audio driver takes care of zeroing the buffers.

> **Note:** No matter how many buffers you pass in, [`fluid_synth_process()`](audio-rendering.md#fluid_synth_process) will always render all audio channels to the buffers in `out` and all effects channels to the buffers in `fx`, provided that `nout > 0` and `nfx > 0` respectively. If `nout/2 < ` it will wrap around. Same is true for effects audio if `nfx/2 < (`. See usage examples below.

> **Note:** Should only be called from synthesis thread.
