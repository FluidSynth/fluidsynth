# Android support in Fluidsynth

Android support is done as OpenSLES audio driver.

Android also has Android MIDI API which is exposed only in Android Java API.

## Usage

You will have to enable it with "audio.driver" setting as "opensles":

```
fluid_settings_setstr (settings_handle, "audio.driver", "opensles");
```

## Custom SoundFont loader

Since Android file access is quite limited and there is no common place
to store soundfonts unlike Linux desktop (/usr/share/sounds/sf2), you
will most likely have to provide custom soundfont loader.

Fluidsynth 1.9.x comes with `fluid_sfloader_set_callbacks()` which brings
[customizible file/stream reader](https://github.com/FluidSynth/fluidsynth/issues/241) (open/read/seek/tell/close). It is useful to implement simplified
custom SF loader e.g. with Android assets or OBB streams.

