# 🛠️ Building FluidSynth with CMake

* [Build Requirements](#requirements)
* [Common Tips for compiling from source](#common-tips-for-compiling-from-source)
* [Building on Linux](#building-on-linux)
* [Building with MSYS2 on Windows](#building-with-msys2-on-windows)
* [Building with Cygwin on Windows](#building-with-cygwin-on-windows)
* [Building with MinGW on Windows](#building-with-mingw-on-windows)
* [Building with Visual Studio on Windows](#building-with-visual-studio-on-windows)
* [Building on OS X](#building-on-os-x)
* [Cross-Compilation](#cross-compilation)
* [Installation](#installation)
* [Output library naming conventions](#output-library-naming-conventions)

## What is CMake?

CMake is a cross-platform build system, that can be used to replace the old auto-tools, providing a nice building environment and advanced features. Some of these features are: 

  * Out of sources build: CMake allows you to build your software into a directory different to the source tree. You can safely delete the build directory and all its contents once you are done. 
  * Multiple generators: classic makefiles can be generated for Unix and MinGW, but also Visual Studio, XCode and Eclipse CDT projects among other types. 
  * Graphic front-ends for configuration and build options. 

More information and documentation is available at the [CMake project site](http://www.cmake.org)

CMake is free software. You can get the [sources and pre-compiled packages](http://www.cmake.org/cmake/resources/software.html) for Linux and other systems. 

You need CMake 3.13 or later to build FluidSynth. However, we **recommend to use CMake 3.18 or later**.

## Requirements

A minimal build of fluidsynth has the following requirements:

  * A C compiler. [GCC](http://gcc.gnu.org/) is the primary compiler, including the [MinGW](http://mingw.org) port for Windows, and the [Apple port](http://developer.apple.com/tools/gcc_overview.html) for Mac OS X. MS Visual C is also supported. Technically, _any_ C90 compliant compiler should work.
  * Starting with fluidsynth 2.5.0, a C++11 compliant compiler will be required.
  * CMake >= 3.13.0
  * make, gmake, nmake, or an IDE as [MS Visual Studio Express](http://www.microsoft.com/express/), [Eclipse CDT](http://www.eclipse.org/cdt/), [Xcode](http://developer.apple.com/tools/xcode/) and others.
  * [pkg-config](http://pkg-config.freedesktop.org/wiki/) tool.
  *  Starting with version 2.5.0, fluidsynth additionally requires a submodule [gcem](https://github.com/kthohr/gcem), that GitHub unfortunately doesn't include in the auto-generated source archive. If gcem is unavailable (i.e. the `gcem` directory in the source archive root is empty), it will be automatically downloaded from GitHub, i.e. **an internet connection will be required during the build**.


<details><summary>Some other libraries can be <strong>optionally</strong> included at build time, providing additional functionality. Click here to view them.</summary><br />

  * C++17 compliant compiler and standard library
  * [glib and gthread](http://www.gtk.org) libraries.
  * [libsndfile](http://www.mega-nerd.com/libsndfile/) - Allows for rendering MIDI to numerous audio file formats. Otherwise, only rendering to RAW PCM files is supported.
  * [libinstpatch](https://github.com/swami/libinstpatch) - Enables loading of Downloadable Sounds (DLS) files as alternative Soundfont format
  * [JACK](http://jackaudio.org) - Jack Audio Connection Kit, inter-application audio routing found on Linux and Mac OS X
  * [ALSA](http://www.alsa-project.org) - Modern audio system found on Linux. FluidSynth supports audio output and the ALSA MIDI sequencer.
  * [OSS](http://www.opensound.com) - Open Sound System (the older Unix sound system)
  * [OpenSLES](https://www.khronos.org/opensles/) - A royalty-free, cross-platform, hardware-accelerated audio API tuned for embedded systems (e.g. Android)
  * [Oboe](https://github.com/google/oboe) - An audio library for real-time audio apps on Android.
  * [PortAudio](http://www.portaudio.com) - Free, cross-platform, audio I/O library
  * [PulseAudio](http://www.pulseaudio.org) - Linux sound server
  * CoreAudio - Audio system found on Mac OS X
  * DirectSound - Sound drivers found on Windows sytems
  * [WaveOut](https://docs.microsoft.com/de-at/windows/win32/directshow/audio-renderer--waveout--filter) - Alternative to DirectSound driver on Windows
  * WinMIDI - MIDI driver on Windows
  * [MidiShare](http://midishare.sourceforge.net) - Cross-platform MIDI environment
  * [SDL2](https://www.libsdl.org/) - Cross-platform audio library
  * [LASH](http://www.nongnu.org/lash) - LASH Audio Session Handler, session management support for ALSA and Jack applications
  * [LADSPA](http://www.ladspa.org) - Audio plugin architecture (FluidSynth can use LADSPA plugins)
  * Readline - For improved command line editing features in the built-in shell

</details>


## Common Tips for compiling from source

Get the FluidSynth sources [somewhere](Download). 

First, **execute CMake from the build directory**, providing the source directory location and optionally, the build options. There are several ways. 

  * From a command line shell: 
        
```bash
$ cd fluidsynth-x.y.z/
$ mkdir build
$ cd build
$ cmake ..
```

The last command (`cmake ..`) can be modified by adding options. Common modifications include e.g.: 

  * `-Denable-portaudio=1` - PortAudio is disabled by default, this will enable that functionality 
  * `-DLIB_SUFFIX=""` - make sure we don't put anything in /lib64, this is needed on 64 bit Debian/Ubuntu platforms 
  * [`-DCMAKE_INSTALL_PREFIX=/usr`](https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html) - this will effectively _overwrite_ rather than override an existing FluidSynth installation 
  * [`-DCMAKE_BUILD_TYPE=Debug`](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html) - if you want to help out and find bugs, this will make it easier to debug (but much worse performance)
  * `-Denable-ubsan=1` - even better debugging support by using UBSan and ASan.
  * [`-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded`](https://cmake.org/cmake/help/latest/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html) - if you want to use a statically-linked MSVC runtime library (requires CMake >= 3.15, use [this hack](https://github.com/FluidSynth/fluidsynth/blob/7f11a9bf5c7304e04309a6ec9fc515ee815524bf/CMakeLists.txt#L229-L249) if your CMake is too old)

Valid values for boolean (`enable-xxxx`) options: 1, 0, yes, no, on, off. 

  * There are also several alternative CMake front-ends, if you don't want to use the command line interface: 
    * ncurses based program, for Linux and Unix: `ccmake`
    * GUI, Qt-based program, multiplatform: `cmake-gui`
    * GUI, Windows native program: `CMakeSetup.exe`

Then, **execute the build command**. If you used the Makefiles generator (the default in Linux and other Unix systems) then execute `make`, `gmake` or `mingw32-make`. If you generated a project file, e.g. a MSVS solution, use your IDE to build it.

There are many targets available. To see a complete list of them, type: 

```bash
$ make help
```

The build process usually hides the compiler command lines. To get a full, verbose build log: 

```bash
$ make VERBOSE=1
```

There is a `clean` target, but not a `distclean` one. You should use a build directory different to the source tree (as demonstrated in the command line example above). In this case, the `distclean` target would be equivalent to simply removing the build directory. 

After fluidsynth has been built successfully, it is recommended to **execute the unit tests**:

```bash
$ make check
```

To compile the developer documentation you need [Doxygen](http://www.doxygen.org) installed. Use this command from the root build directory: 

```bash
$ make doxygen
```

### If something fails

If there is an error message while executing CMake, this probably means that a required package is missing in your system. You should install the missing component and run CMake again.

If there is an error when executing the build process, after running a flawless CMake configuration process, this means that there may be an error in the source code, or in the build system, 
or something incompatible in 3rd party libraries.

You can provide feedback and ask for help, by sending a report containing your problem(s) to the FluidSynth development mailing list. Make sure to **provide the CMake output and the full, verbose build log**.
http://lists.nongnu.org/mailman/listinfo/fluid-dev


## Building on Linux

If your distribution already has a packaged FluidSynth, you may simply install it using the package manager, [as described here](https://github.com/FluidSynth/fluidsynth/wiki/Download#distributions). You can also grab the build dependencies automatically by running a command such as 

```bash
sudo apt-get build-dep fluidsynth --no-install-recommends
```

which should work under Debian/Ubuntu. To build from source please follow the [common tips](#common-tips-for-compiling-from-source).

### Note
After (un)installation (with `sudo make install` or `sudo make uninstall`) you might need update the dynamic linker cache by running `sudo ldconfig` or you'll end up with a new executable calling the old library! You are experiencing some `error while loading shared libraries: libfluidsynth.so`? Update the linker cache as well! No root permission? Set the `LD_LIBRARY_PATH` environment variable to include the path where you've installed libfluidsynth.so, like

```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```


## Building with MSYS2 on Windows
This is probably the easiest and most straightforward method.

### Requirements
* [MSYS2](http://www.msys2.org/)
* DirectX SDK for dsound. You probably already have this installed, so just ignore it. If you encounter issues with CMake later on, then you can [download it directly from Microsoft](http://msdn.microsoft.com/en-us/directx/default.aspx).

### Environment setup
1. Install MSYS2 in your preferred location and launch it.
2. Run `pacman -Syyu`. Let it update, close the window when requested
3. Launch MSYS2 again, either from the mingw32 (32 bit) or mingw64 (64 bit) shell. Choose based on your machine architecture.
4. Run `pacman -Syyu` and update again. This time it should pull more packages.
5. Once done, install all the dependencies needed by FluidSynth by running:

```Batchfile
pacman -S make mingw-w64-x86_64-pkg-config mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake glib2-devel mingw-w64-x86_64-glib2 mingw-w64-x86_64-libsndfile
```

**This is just an example for 64-bit systems. If you are running the 32 bit version, replace `x86_64` by `i686`.**

### Building
1. Grab FluidSynths latest release tarball, as described [here](https://github.com/FluidSynth/fluidsynth/wiki/Download#source-archives).
2. Unpack the tarball and navigate into the folder containing the extracted files.
3. Create a new folder, we'll call it `build`.
4. In the MSYS2 shell, navigate into that folder
5. Run cmake on the previous directory, specifying the Makefile type: `cmake -G "MSYS Makefiles" ..`
6. Run `make` to start compilation.

!!! Note
 
    Make sure to execute `fluidsynth` in the MSYS2 shell as well. 
    It won't work form the `cmd.exe` (out-of-the-box).

## Building with Cygwin on Windows
This method provides a full UNIX-like environment. _You might run into weird issues with compiled code, such as FluidSynth trying to load Linux audio drivers, or not being able to run FluidSynth without the specific Cygwin DLLs!_

### Environment setup
Install [Cygwin](https://www.cygwin.com/) following the setup, and selecting _at least_ the following packages (please note that they might already be marked for installation):

* gcc
* cmake
* glib

### Building
1. Grab FluidSynths latest release tarball, as described [here](https://github.com/FluidSynth/fluidsynth/wiki/Download#source-archives).
2. Navigate into the cloned repository and create a new folder, we'll call it 'build'.
3. Navigate into that folder and run cmake on the previous directory, specifying the Makefile type
`cmake -G "MSYS Makefiles" ..`
4. You're set. Run `make` to build.

## Building with MinGW on Windows
If you want to use MinGW you need to set up a build environment containing the glib binaries and its dependencies. For simplicity, you can download prebuilt binaries from [gtk.org](http://ftp.gnome.org/pub/gnome/binaries/):

64-bit Windows:
  * [glib-2.26](http://ftp.gnome.org/pub/gnome/binaries/win64/glib/2.26/glib_2.26.1-1_win64.zip)
  * [glib-dev-2.26](http://ftp.gnome.org/pub/gnome/binaries/win64/glib/2.26/glib-dev_2.26.1-1_win64.zip)
  * [pkg-config-0.23](http://ftp.gnome.org/pub/gnome/binaries/win64/dependencies/pkg-config_0.23-2_win64.zip)
  * [proxy-libintl](http://ftp.gnome.org/pub/gnome/binaries/win64/dependencies/proxy-libintl-dev_20100902_win64.zip)
  * [gettext-runtime-0.18](http://ftp.gnome.org/pub/gnome/binaries/win64/dependencies/gettext-runtime_0.18.1.1-2_win64.zip)

32-bit Windows:
  * [glib-2.28](http://ftp.gnome.org/pub/gnome/binaries/win32/glib/2.28/glib_2.28.8-1_win32.zip)
  * [glib-dev-2.28](http://ftp.gnome.org/pub/gnome/binaries/win32/glib/2.28/glib-dev_2.28.8-1_win32.zip)
  * [pkg-config-0.26](http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/pkg-config_0.26-1_win32.zip)
  * [proxy-libintl](http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/proxy-libintl-dev_20100902_win32.zip)
  * [gettext-runtime-0.18](http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/gettext-runtime_0.18.1.1-2_win32.zip)

Unpack all required ZIPs (see above) to the same directory, a name without spaces, for instance **C:\\freesw**. Add **C:\\freesw\\bin** to the system PATH by executing `set PATH=C:\freesw\bin;%PATH%`. To verify: Start button -&gt; Command Prompt 

```Batchfile
C:\\> pkg-config --list-all
gio-2.0               GIO - glib I/O library
gmodule-2.0           GModule - Dynamic module loader for GLib
glib-2.0              GLib - C Utility Library
gmodule-no-export-2.0 GModule - Dynamic module loader for GLib
gthread-2.0           GThread - Thread support for GLib
gobject-2.0           GObject - GLib Type, Object, Parameter and Signal Library
```

For instance, this is the layout of the directory with the dependencies installed: 
    
    C:\freesw\
    ├───bin
    ├───include
    │   └───glib-2.0
    │       ├───gio
    │       ├───glib
    │       └───gobject
    ├───lib
    │   └───glib-2.0
    │       └───include
    ├───manifest
    ├───pkgconfig
    ├───share
    │   ├───aclocal
    (...)
    

When installing CMake, make sure to let the installer add CMake to the system `PATH` environment variable, and create a desktop shortcut icon.

Obtain **dsound.h** and copy it to **C:\\freesw\\include**. Either install the Microsoft [DirectX SDK](http://msdn.microsoft.com/en-us/directx/default.aspx) or alternatively try [Google Code Search](http://www.google.com/codesearch?q=dsound.h&hl=en&btnG=Search+Code) to locate a copy of the header file. Some users have mentioned that the header contained in Wine might work.

!!! Note
 
    For simplicity, you can unzip the dependencies above directly into your MinGW installation directory. 

Get [MinGW](http://mingw.org), for convenience you can use the [TDM Bundle installer](https://sourceforge.net/projects/tdm-gcc/files/TDM-GCC%20Installer/) (use at your own risk). 

Run CMake, opening the desktop icon shortcut. 

  * Select "MinGW Makefiles". 
  * Select the directory containing FluidSynth sources, and another directory for building (e.g. **./build**). 

Start mingw32-make on the command line to build the project: 
    
    >cd
    fluidsynth-x.y.z
    >cd build
    >mingw32-make.exe
    

### Notes on compiling under Windows 10

#### Problem 1.

pkg-config was not found: Make sure the folder containing the pkg-config.exe is on the PATH env variable or copy the .exe to mingw32\bin folder.

#### Problem 2.

libintl.dll cannot be found (linker calls it intl):

Make sure libintl.dll is in a directory the linker is looking for (e.g.: `c:/mingw32/lib/gcc/mingw32/6.3.0`). It does not use the PATH variable. Make sure the filename of libintl.dll does not include the version.


## Building with Visual Studio on Windows

This approach requires to use Visual Studio 2015 Update 3 or later versions. In case you are forced to use an older version of Visual Studio, follow the preparations of the [MinGW approach](#building-with-mingw-on-windows) and refer to our [CI build script for Windows](https://github.com/FluidSynth/fluidsynth/blob/eb132d8196f11f2a84a44ce6076ad8e8b8c75e32/.appveyor.yml#L72-L76) for correct CMake invocation.

You can get Visual Studio Express Community 2017, from [Microsoft](https://www.visualstudio.com/de/vs/visual-studio-express/). _Note: the Community 2017 edition supports many languages, but only C/C++ is needed. You don't need special options, just the defaults._

Prepare your build environment with all prerequisites as described in [Building with MinGW](#building-with-mingw-on-windows). Alternatively you can install glib using the [vcpkg](https://docs.microsoft.com/en-us/cpp/vcpkg) tool:
```Batchfile
vcpkg install glib
```

Run CMake, opening the desktop icon shortcut. 

  * Select the appropriate version of Visual Studio you are going to use.
  * Select the directory containing FluidSynth sources, and another directory for building.
  * Click "Configure"
  * Press "Generate"

Go to the build directory, and double-click the file `FluidSynth.sln`, and build the solution. 

!!! Note
 
    As of fluidsynth 1.1.10 there also exists a vcpkg package:
    ```Batchfile
    vcpkg install fluidsynth
    ```

## Building on OS X

Get all requirements with [Homebrew](http://mxcl.github.com/homebrew/): 
    
    brew install fluidsynth pkg-config
    
Alternatively install libgnugetopt, readline5, libflac8-dev, libsndfile1-dev, glib2-dev, dbus1.3-dev and cmake manually.

Install XCode, "DevSDK.pkg" and "CoreAudioSDK.pkg" packages from your OS X install media. 

Run CMake and start the build as described in section [Common Tips for compiling from source](#common-tips-for-compiling-from-source).

After a successful build you may optionally install the compiled binaries as superuser:

```
make install
```
    
## Cross-Compilation

In order to cross compile fluidsynth make sure you know about [the basics of cross compilation using cmake](https://cmake.org/Wiki/CMake_Cross_Compiling).


It is highly recommend to provide cmake with a toolchain file that takes care about basic path handling like where to find the compiler for your target platform. An example toolchain file for compiling for 64-bit Windows using mingw could look like this:

```cmake
# Cross compile toolchain configuration based on:
# https://www.cmake.org/Wiki/CMake_Cross_Compiling

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# 64 bit mingw-w64 - ADJUST THIS TO YOUR NEEDS!
SET(TOOLCHAIN_PREFIX "x86_64-w64-mingw32")

# compilers to use for C and C++
FIND_PROGRAM(CMAKE_C_COMPILER NAMES ${TOOLCHAIN_PREFIX}-gcc)
FIND_PROGRAM(CMAKE_CXX_COMPILER NAMES ${TOOLCHAIN_PREFIX}-g++)
FIND_PROGRAM(CMAKE_RC_COMPILER NAMES ${TOOLCHAIN_PREFIX}-windres)
FIND_PROGRAM(PKG_CONFIG_EXECUTABLE ${TOOLCHAIN_PREFIX}-pkg-config)

# path to the target environment
SET(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX}/sys-root/mingw)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
```

Furthermore, fluidsynth's cmake build system heavily relies on using pkg-config. In order to make sure the `pkg-config` executable of the host system finds the correct libraries for the target platform, set the `PKG_CONFIG_LIBDIR` environment variable to the path that contains the `.pc` files for all relevant libraries (i.e. glib, gthread and possibly other dependencies you want to build against).

As usual create a build subdir within fluidsynth's source tree. Invoke cmake and provide the toolchain file by specifying the `-DCMAKE_TOOLCHAIN_FILE` flag. Additionally consider manually disabling any optional build dependencies that you cannot provide for the target platform.

```bash
cd fluidsynth/
mkdir build && cd build
export PKG_CONFIG_LIBDIR=/usr/x86_64-w64-mingw32/sys-root/mingw/lib/pkgconfig
cmake -DCMAKE_TOOLCHAIN_FILE=my-fluid-toolchain.cmake -Denable-libsndfile=0 -Denable-dbus=0 -Denable-pulseaudio=0 -Denable-jack=0 ..
```

If your environment is set up correctly cmake should find the compiler and the glib libraries for the target platform and exit successfully.

## Installation

Starting with 2.5.1 fluidsynth provides three cmake components:

* `fluidsynth_program`
* `fluidsynth_runtime`
* `fluidsynth_development`

Those can be used at install time to selectively install built files. E.g.:
```
$ cmake --install build --component fluidsynth_runtime
```
will install only the shared library, without installing the CLI program or the headers.


## Output Library naming conventions

Depending on the target OS and a several compilation settings, fluidsynth's cmake build system will generate library files with different naming. Below is a table which highlights the different naming conventions.

`<FULL.SOVERSION>` refers to the full version of the library (this is different from fluidsynth's release version!), e.g. `3.5.1`.

`<SOVERSION>` refers to the major part of the library version, e.g. `3`.

| OS                                         | dynamic library                                                               | static library                  |
|--------------------------------------------|-------------------------------------------------------------------------------|---------------------------------|
| MacOS Framework                            | `FluidSynth.framework/Versions/<SOVERSION>/FluidSynth`                        | ?                               |
| MacOS Universal Libs                       | `libfluidsynth.<FULL.SOVERSION>.dylib`                                        | `libfluidsynth.a`               |
| Android                                    | `libfluidsynth.so`                                                            | `libfluidsynth.a`               |
| Other Unixoids (Linux, BSD, Solaris, etc.) | `libfluidsynth.so.<VERSION>`                                                  | `libfluidsynth.a`               |
| Windows (native & MinGW)                   | `libfluidsynth-<SOVERSION>.dll` (import lib: `libfluidsynth-<SOVERSION>.lib`) | `libfluidsynth-<SOVERSION>.lib` |
| Windows (CygWin)                           | ?                                                                             | ?                               |
| OS/2                                       | `FLUIDSY3.DLL`                                                                | ?                               |
