# 🤖 Build for Android

This page demonstrates how to cross-compile the upstream version of fluidsynth for Android, along with **all the dependencies** that fluidsynth requires. Other users have taken shortcuts by custom modifications to fluidsynth's code base that we do not officially support.

You may find more information about FluidSynth on Android [in the /doc folder](https://github.com/FluidSynth/fluidsynth/tree/master/doc/android).

## Preface

So, you want to compile fluidsynth for Android? Are you aware of the fact that fluidsynth is written in C? That means you cannot benefit from nicely portable bytecode, i.e. compile once and deploy for all smartphone architectures and Android versions out there. The code being compiled will be **native** to the device and Android NDK you are targeting. 
Thus, you will need to deploy fat binaries to your users. Even worse: fluidsynth depends on glib that we use as OSAL library. In order to build the official upstream version of fluidsynth you will have to cross compile glib and it's dependencies first. And again: you'll have to do this for all architectures and Android versions you want to support!

fluidsynth is licensed under **LGPL-2.1**; [make sure you understand what that means](LicensingFAQ) when using it in your app!

Note that glib and many of its dependencies are licensed under the LGPL, too. If you **statically link** any of them into your proprietary, closed-source app, you violate the LGPL and end up with a serious license infringement!
Further note that fluidsynth can be optionally compiled with dependencies that are GPL licensed. In this case **fluidsynth and your app become GPL licensed**! If you don't respect those terms and understand its consequences, you are a potential subject to legal prosecution.

When using the methods and scripts explained here, there is no guarantee of any kind that they work as expected, esp. not in a commercial product. **NO warranty is provided; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE**.

If, despite all that, you still want to use fluidsynth on Android, ...well thank you, here's a guide that hopefully turns out to be useful and gives you an idea of the cross-compilation process as a whole.

If you just want to use fluidsynth on Android without going through all the compilation trouble yourself, you may use the build artifacts from the CI build (cf. README.md).

If you have questions or need support, contact our mailing list:
https://lists.nongnu.org/mailman/listinfo/fluid-dev

## Prerequisites

#### This guide...

* assumes that you are on a Linux machine.
* requires an active internet connection (for downloading dependency libraries).
* assumes that you have a solid understanding of how to compile C/C++ programs from source under Linux.
* assumes that you know about Autotools, CMake, Make, Clang, GCC, Shell.
* recommends that you are a C/C++ programmer (so that you are able to understand and fix compilation errors).
* uses the [Standalone Toolchain approach of the Android NDK](https://developer.android.com/ndk/guides/standalone_toolchain).
* uses clang for compilation as shipped by the Android NDK.
* has been successfully tested for Android NDK r17b and r17c.
* exemplary compiles for `arm-eabi`.

#### Android NDK

The NDK has a few prerequisites itself. Unfortunately, they don't seem to be documented. E.g. the `clang60` compiler shipped with the NDK may require `libtinfo.so.5` to be installed on your host system. So, make sure `libncurses5` is installed.
Keep an eye open for errors like

```
android-ndk-toolchain/bin/clang60: error while loading shared libraries: libtinfo.so.5: cannot open shared object file: No such file or directory
```

to capture even more missing dependencies that the NDK requires and make sure they are installed on your host system.

## Getting started

* Read the preface above!
* [Seriously, read the preface](#Preface)!!
* Download the Android NDK
* Download [fluid-android-prepare.sh](https://gist.github.com/derselbst/885641b86b8bd0e5fd1b1d846d4b286b) and [fluid-android-compile.sh](https://gist.github.com/derselbst/88917bf177d25cfc1067a37b442dd5ff)
* Open `fluid-android-prepare.sh` and study it
* Adjust the `$DEV` and `$NDK` variables to your needs
* Adjust the target architecture and `$ANDROID_API` to your needs
* Open the `bash` shell, source the `fluid-android-prepare.sh`, make a folder
```shell
source fluid-android-prepare.sh
mkdir $DEV
```
* Provide the extracted NDK folder (e.g. android-ndk-r17b) at `$NDK` (i.e. move it there)
* Move `android-prepare.sh` and `android-compile.sh` into `$DEV`
* `cd $DEV`
* In the same shell, where you've just sourced `fluid-android-prepare.sh`, start the build process by
```shell
bash fluid-android-compile.sh
```

That's it. All dependencies are downloaded and compiled one after another. No errors should occur. 
Once done you should end up with a directory `$PREFIX` containing all binaries needed to run fluidsynth on Android (about 100 MiB on arm-eabi) that you can integrate into your favorite environment or IDE 
(cf. [Using prebuilt libraries on Android](Using-prebuilt-libraries-on-Android.md)).

## If compilation of dependencies fails

Study the output of autotools. Keep an eye open for any warnings that might come up. Also, study autotools `config.log`. You might try to ask for help at our mailing list. However, preferably you should contact the respective project directly.

## If compilation of fluidsynth fails

Ask for help at our mailing list. Provide as much information as possible! (Commands, log files, etc.) If you have already identified a bug in fluidsynth's build system or elsewhere, feel free to open a ticket directly here at GitHub.
