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

This version of fluidsynth~ was created and tested with Max/MSP 4.3 on Mac OS X (10.3) Panther.
The Mac OS 9 version of the external was not tested.

_____________________________________________________________________
RELASE NOTES of version 05/2004 (release 7)

 - fixed bug of crash for "info channels" in case of channel(s) without defined preset
 - rebuilt Mac OS 9 version
 - extended help patch
_____________________________________________________________________
RELASE NOTES of version 04/2004 (release 6)

 - fixed several bugs of the initial release (especially related to the translation of paths
 - introduced names for soundfonts (without path and .sf2 postfix)
 - introduced a message "info" in order to query the current state of fluidsynth~

_____________________________________________________________________
CVS DIRECTORY

cvs repository: savannah.nongnu.org:/cvsroot/fluid
Max/MSP implementation directory: bindings/fluidmax

The fluidsynth directory in the fluidmax directory contains only a version.h which is just added for the case that the version.h file in the fluidsynth checkout is not (auto-)"maked" or generated otherwise.

For now the file fluid_sys.c is not included to the CW project. Instead there is a file called fluidmax_fakefuns.c implementing some necessary functions.

_____________________________________________________________________
KNOWN BUGS:

_____________________________________________________________________
TODO
 - get rid of fluidmax_fakefuns.c
 - implement log function properly
