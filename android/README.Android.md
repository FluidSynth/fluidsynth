# Android support in Fluidsynth

Android support is done as Oboe and OpenSLES audio drivers.

Android also has Android MIDI API which is exposed only in Android Java API.

## Usage

By default, "oboe" will be the default driver for Android. You can enable "opensles" instead too, with "audio.driver" setting:

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

The Android implementation comes with native Asset sfloader too. However, usage is a bit tricky because AssetManager needs to be passed from Java code (even though we use AAssetManager API).
Use `Java_fluidsynth_androidextensions_NativeHandler_setAssetManagerContext()` to initialize the this loader, then call `new_fluid_android_asset_sfloader()` to create a new sfloader. If you already have AAssetManager instance, then the first JNI function is ignorable and you only have to specify the manager to the second function.
