FluidSynth MacOS9 port
----------------------

Antoine Schmitt March 2003

1) Configuring
As Macintosh cannot run the configure shell command, you need to do it yourself. Mainly, this consists in:
- creating the include/fluidsynth/version.h file by hand, using the version.h.in file as a template, and defining the VERSION* defines by hand, using the values from the 'configure.ac' file.

Example:
#define FLUIDSYNTH_VERSION       "1.0.0"
#define FLUIDSYNTH_VERSION_MAJOR 1
#define FLUIDSYNTH_VERSION_MINOR 0
#define FLUIDSYNTH_VERSION_MICRO 0

2) src/config_macos.h file
In this file, you can define which audio driver to use. Only the SoundManager driver has been tested (SNDMAN_SUPPORT). You can choose the PORTAUDIO driver.

3) Compiler, etc..
This project has been compiled with CodeWarrior 4.0 on MacOS9.2.2
