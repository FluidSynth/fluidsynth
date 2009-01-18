******************************************************************
*
*  This is fluidsynth~,
*  a derivative for Max/MSP of the Fluid Synth soundfont synthesizer.
* 
*  Fluid Synth is written by Peter Hanappe et al.
*  See http://www.fluidsynth.org/.
* 
*  Max/MSP integration by Norbert Schnell, IRCAM - Centre Pompidou
*  Norbert.Schnell@ircam.fr
*
*  Take a look here:
*    http://www.ircam.fr/equipes/temps-reel/maxmsp/fluidsynth.html
*
_____________________________________________________________________
LICENSE

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.
 
See file COPYING.LIB in the CVS root directory for further information on licensing terms.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

_____________________________________________________________________
SYSTEM REQUIREMENTS

This version of fluidsynth~ was initially created and tested using Max/MSP 4.3 on Mac 10.3 and should work on all following versions of Max/MSP and Mac OS.

_____________________________________________________________________
RELASE NOTES for version 01/2009 (release 13)

 - adapted to current FluidSynth code base (after years without activity)
 - no new features or relevant changes!

_____________________________________________________________________
RELASE NOTES up to version 10/2005 (release 10, 11 and 12)

 - added micro-tuning methods (to be added to help patch)
 - fixed bug restricting argument of tuning-octave message to int (11)
 - fixed voice stealing (12)

_____________________________________________________________________
RELASE NOTES up to version 09/2004 (release 7 to 9)

 - bug fix: now polyphony and # of midi channel arguments take effect (release 9)
 - added message resample permitting to chose resampling interpolation method (release 8)
 - added names (= file name without path and postfix) for soundfonts (7)
 - fixed bug of crash for "info channels" in case of channel(s) without defined preset
 - dropped Mac OS 9 version
 - extended help patch

_____________________________________________________________________
RELASE NOTES of version 04/2004 (release 6)

 - fixed several bugs of the initial release (especially related to the translation of paths
 - introduced names for soundfonts (without path and .sf2 postfix)
 - introduced a message "info" in order to query the current state of tuning-octave

_____________________________________________________________________
SVN DIRECTORY

svn repository: https://resonance.org/svn/fluidsynth/trunk/fluidsynth
Max/MSP implementation directory: bindings/fluidmax

The fluidsynth directory in the fluidmax directory contains only a version.h which is just added for the case that the version.h file in the fluidsynth checkout is not (auto-)"maked" or generated otherwise.

For now the file fluid_sys.c is not included to the CW project. Instead there is a file called fluidmax_fakefuns.c implementing some necessary functions.

_____________________________________________________________________
KNOWN BUGS:

_____________________________________________________________________
TODO
 - implement log function properly
