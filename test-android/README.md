# Android test runner

It is meant to be an Android app that runs those fluidsynth tests under `../test` directory.

It is not immediately doable at this moment because everything is based on ctest where each source has `main()` function that cannot be more than one within a shared library. Either every single test source file has to be compiled separately, or renaming `main()` to something else and have separate runnable (latter is much simpler).

But so far this app makes sure that it loads `libfluidsynth.so` without problem.

## Building

You have to build fluidsynth for Android first, and then copy
`libfluidsynth.so` and all the dependencies into `app/src/main/jniLibs/{ABI}` (`armeabi-v7a` / `arm64-v8a` / `x86` / `x86_64`).
