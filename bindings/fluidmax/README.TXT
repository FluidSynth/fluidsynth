Well, this is fluidsynth~ for Max/MSP...

Take a look here:
  http://www.ircam.fr/equipes/temps-reel/maxmsp/fluidsynth.html

In order to build the fluidmax.mcp CodeWarrior (8) project you want to 
check out the FluidSynth source tree into a directory on the same level 
than (parallel to) this directory.

The following lines have to be added to the fluidsynth_priv.h file:

/* begin fluidsynth_priv.h */
#if defined(__POWERPC__) && !(defined(__APPLE__) && defined(__MACH__))
#include "config_maxmsp43.h"
#endif
/* end fluidsynth_priv.h */

The fluidsynth directory in this directory contains only a version.h
which is just added for the case that the version.h file in the 
fluidsynth checkout is not (auto-)"maked" or generated otherwise.

For now the file fluid_sys.c is not included to the CW project. 
Instead there (here) is a file called fluidmax_fakefuns.c implementing 
some necessary functions.

Using fluid_sys.c with the macro MACOS 9 defined almost works.
Its only the prototype of the function NewTimerProc() causing a 
compilation error (no prototype) and didn't find any good include
to fix this...

The following three functions used in fluid_settings are missing:
  fluid_shell_settings()
  fluid_audio_driver_settings()
  fluid_midi_driver_settings()


KNOWN BUGS:
 - Unloading SoungFonts doesn't work (would probably be fixed if using the MACOS 9 macro)
 
 
TODO
 - get rid of fluidmax_fakefuns.c
 - implement log function properly
 - fix unloading
 - allow access to SFs by name (rather than id only)
 - avoid double loading of Sfs and loading of SFs with the same name
 