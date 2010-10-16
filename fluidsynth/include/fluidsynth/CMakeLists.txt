# FluidSynth - A Software Synthesize
#
# Copyright (C) 2003-2010 Peter Hanappe and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA

# CMake based build system. Pedro Lopez-Cabanillas <plcl@users.sf.net>

configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in 
                 ${CMAKE_CURRENT_BINARY_DIR}/version.h )

if ( NOT MACOSX_FRAMEWORK )
  set ( include_HEADERS 
    audio.h 
    event.h 
    gen.h 
    log.h 
    midi.h 
    misc.h 
    mod.h 
    ramsfont.h 
    seq.h 
    seqbind.h 
    settings.h 
    sfont.h 
    shell.h 
    synth.h 
    types.h 
    voice.h 
  )
  install ( 
    FILES 
      ${include_HEADERS}
      ${CMAKE_CURRENT_BINARY_DIR}/version.h
    DESTINATION 
      ${INCLUDE_INSTALL_DIR}/fluidsynth 
  )
endif ( NOT MACOSX_FRAMEWORK )
