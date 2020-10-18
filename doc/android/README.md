# Android support in Fluidsynth

Fluidsynth supports Android audio outputs by Oboe and OpenSLES audio drivers.

If you are new to Fluidsynth on Android, check out [Hector Ricardo's Hello World App for Android](https://medium.com/swlh/creating-a-fluidsynth-hello-world-app-for-android-5e112454a8eb).

Android also has Android MIDI API which is exposed only in Android Java API, but it is not exposed as a native API, therefore there is no `mdriver` support for Android. There is an example MidiDeviceService implementation for Fluidsynth at: https://github.com/atsushieno/fluidsynth-midi-service-j

## Usage

`libfluidsynth.so` and `libfluidsynth-assetloader.so` are the library that should be packaged into apk. The latter is for asset-based "sfloader".

By default, "oboe" is the default driver for Android. You can also explicitly specify "opensles" instead, with "audio.driver" setting:

```
fluid_settings_setstr (settings_handle, "audio.driver", "opensles");
```

## Custom SoundFont loader

Since Android file access is quite limited and there is no common place
to store soundfonts unlike Linux desktop (e.g. `/usr/share/sounds/sf2`), you
will most likely have to provide custom soundfont loader.

Since version 2.0.0 Fluidsynth comes with `fluid_sfloader_set_callbacks()` which brings
[customizible file/stream reader](https://github.com/FluidSynth/fluidsynth/issues/241) (open/read/seek/tell/close). It is useful to implement simplified
custom SF loader e.g. with Android assets or OBB streams.

The Android implementation is in separate library called `libfluidsynth-assetloader.so`. It comes with native Asset sfloader. However, its usage is a bit tricky because AssetManager needs to be passed from Java code (even though we use AAssetManager API).
Use `Java_fluidsynth_androidextensions_NativeHandler_setAssetManagerContext()` to initialize the this loader, then call `new_fluid_android_asset_sfloader()` to create a new sfloader. If you already have AAssetManager instance, then the first JNI function is ignorable and you only have to specify the manager to the second function.

There is [an example source code](https://github.com/atsushieno/fluidsynth-midi-service-j/blob/a2a56b/fluidsynthjna/src/main/java/fluidsynth/androidextensions/AndroidNativeAssetSoundFontLoader.kt#L17) on how to do it.

## Building

In this directory the Cerbero build system is (ab)used for cross-compiling Fluidsynth's dependencies for Android. The entrypoint is `Makefile.android`. If you are looking for a step by step introduction guide for cross-compiling Fluidsynth, [you'll find it in the wiki](https://github.com/FluidSynth/fluidsynth/wiki/BuildingForAndroid).

By default, you are supposed to provide `PKG_CONFIG_PATH` to glib etc. as well as oboe. There is nothing special.

However, in reality, Oboe does not come up with an official package specification, so you will have to create it manually... unless you use `oboe-1.0.pc` in this directory as well as the build system set up here.
