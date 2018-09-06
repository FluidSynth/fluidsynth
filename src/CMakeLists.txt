# FluidSynth - A Software Synthesizer
#
# Copyright (C) 2003-2010 Peter Hanappe and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2.1 of
# the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA

# CMake based build system. Pedro Lopez-Cabanillas <plcl@users.sf.net>

include_directories (
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/drivers
    ${CMAKE_SOURCE_DIR}/src/synth
    ${CMAKE_SOURCE_DIR}/src/rvoice
    ${CMAKE_SOURCE_DIR}/src/midi
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/src/sfloader
    ${CMAKE_SOURCE_DIR}/src/bindings
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    ${PTHREADS_INCLUDE_DIR}
)

include_directories (
    SYSTEM
    ${GLIB_INCLUDE_DIRS}
)

# ************ library ************

if ( READLINE_SUPPORT )
  include_directories ( ${READLINE_INCLUDE_DIR} )
endif ( READLINE_SUPPORT )

if ( PULSE_SUPPORT )
  set ( fluid_pulse_SOURCES drivers/fluid_pulse.c )
  include_directories ( ${PULSE_INCLUDE_DIRS} )
endif ( PULSE_SUPPORT )

if ( ALSA_SUPPORT )
  set ( fluid_alsa_SOURCES drivers/fluid_alsa.c )
  include_directories ( ${ALSA_INCLUDE_DIRS} )
endif ( ALSA_SUPPORT )

if ( COREAUDIO_SUPPORT )
  set ( fluid_coreaudio_SOURCES drivers/fluid_coreaudio.c )
endif ( COREAUDIO_SUPPORT )

if ( COREMIDI_SUPPORT )
  set ( fluid_coremidi_SOURCES drivers/fluid_coremidi.c )
endif ( COREMIDI_SUPPORT )

if ( DBUS_SUPPORT )
  set ( fluid_dbus_SOURCES bindings/fluid_rtkit.c bindings/fluid_rtkit.h )
  include_directories ( ${DBUS_INCLUDE_DIRS} )
endif ( DBUS_SUPPORT )

if ( JACK_SUPPORT )
  set ( fluid_jack_SOURCES drivers/fluid_jack.c )
  include_directories ( ${JACK_INCLUDE_DIRS} )
endif ( JACK_SUPPORT )

if ( PORTAUDIO_SUPPORT )
  set ( fluid_portaudio_SOURCES drivers/fluid_portaudio.c )
  include_directories ( ${PORTAUDIO_INCLUDE_DIRS} )
endif ( PORTAUDIO_SUPPORT )

if ( WINDOWS_SUPPORT )
  set ( fluid_windows_SOURCES drivers/fluid_dsound.c drivers/fluid_winmidi.c )
endif ( WINDOWS_SUPPORT )

if ( OSS_SUPPORT )
  set ( fluid_oss_SOURCES drivers/fluid_oss.c )
endif ( OSS_SUPPORT )

if ( LASH_SUPPORT )
  set ( fluid_lash_SOURCES bindings/fluid_lash.c bindings/fluid_lash.h )
  include_directories ( ${LASH_INCLUDE_DIRS})
endif ( LASH_SUPPORT )

if ( DART_SUPPORT )
  set ( fluid_dart_SOURCES drivers/fluid_dart.c )
  include_directories ( ${DART_INCLUDE_DIRS} )
endif ( DART_SUPPORT )

if ( LIBSNDFILE_SUPPORT )
  include_directories ( ${LIBSNDFILE_INCLUDE_DIRS} )
endif ( LIBSNDFILE_SUPPORT )

if ( MIDISHARE_SUPPORT )
  set ( fluid_midishare_SOURCES drivers/fluid_midishare.c )
  include_directories ( ${MidiShare_INCLUDE_DIRS} )
endif ( MIDISHARE_SUPPORT )

set ( config_SOURCES ${CMAKE_BINARY_DIR}/config.h )

aux_source_directory(. DIR_ROOT_SRCS)
aux_source_directory(utils DIR_UTILS_SRCS)
aux_source_directory(sfloader DIR_SFLOADER_SRCS)
aux_source_directory(rvoice DIR_RVOICE_SRCS)
aux_source_directory(synth DIR_SYNTH_SRCS)
aux_source_directory(midi DIR_MIDI_SRCS)
aux_source_directory(drivers DIR_AUDIO_SRCS)
aux_source_directory(bindings DIR_BINDINGS_SRCS)

set ( public_HEADERS
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/audio.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/event.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/gen.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/ladspa.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/log.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/midi.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/misc.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/mod.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/seq.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/seqbind.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/settings.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/sfont.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/shell.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/synth.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/types.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/voice.h
    ${CMAKE_BINARY_DIR}/include/fluidsynth/version.h
)

set ( public_main_HEADER
    ${CMAKE_BINARY_DIR}/include/fluidsynth.h
)

configure_file ( ${CMAKE_SOURCE_DIR}/include/fluidsynth/version.h.in 
                 ${CMAKE_BINARY_DIR}/include/fluidsynth/version.h )
configure_file ( ${CMAKE_SOURCE_DIR}/include/fluidsynth.cmake
                 ${public_main_HEADER} )

if ( WIN32 AND NOT MINGW )
include(generate_product_version)
generate_product_version(
    VersionFilesOutputVariable
    NAME "Fluidsynth"
    BUNDLE "Fluidsynth"
    VERSION_MAJOR ${FLUIDSYNTH_VERSION_MAJOR}
    VERSION_MINOR ${FLUIDSYNTH_VERSION_MINOR}
    VERSION_PATCH ${FLUIDSYNTH_VERSION_MICRO}
    VERSION_REVISION 0
    COMMENTS "Fluidsynth"
    COMPANY_NAME "Fluidsynth LGPL"
    ORIGINAL_FILENAME "libfluidsynth.dll"
    FILE_DESCRIPTION "Fluidsynth"
)
endif ( WIN32 AND NOT MINGW )

add_library ( libfluidsynth-OBJ OBJECT
    ${config_SOURCES}
    ${fluid_alsa_SOURCES}
    ${fluid_coreaudio_SOURCES}
    ${fluid_coremidi_SOURCES}
    ${fluid_dart_SOURCES}
    ${fluid_dbus_SOURCES}
    ${fluid_jack_SOURCES}
    ${fluid_lash_SOURCES}
    ${fluid_midishare_SOURCES}
    ${fluid_oss_SOURCES}
    ${fluid_portaudio_SOURCES}
    ${fluid_pulse_SOURCES}
    ${fluid_windows_SOURCES}
    ${DIR_SYNTH_SRCS}
    ${DIR_UTILS_SRCS}
    ${DIR_AUDIO_SRCS}
    ${DIR_MIDI_SRCS}
    ${DIR_RVOICE_SRCS}
    ${DIR_SFLOADER_SRCS}
    ${DIR_BINDINGS_SRCS}
    ${public_HEADERS}
    ${public_main_HEADER}
    ${VersionFilesOutputVariable}
)

if ( LIBFLUID_CPPFLAGS )
  set_target_properties ( libfluidsynth-OBJ
    PROPERTIES COMPILE_FLAGS ${LIBFLUID_CPPFLAGS} )
endif ( LIBFLUID_CPPFLAGS )

# note: by default this target creates a shared object (or dll). To build a
# static library instead, set the option BUILD_SHARED_LIBS to FALSE.
add_library ( libfluidsynth $<TARGET_OBJECTS:libfluidsynth-OBJ> )

if ( MACOSX_FRAMEWORK )
     set_property ( SOURCE ${public_HEADERS}
         PROPERTY MACOSX_PACKAGE_LOCATION Headers/fluidsynth
     )
    set_target_properties ( libfluidsynth
      PROPERTIES
        OUTPUT_NAME "FluidSynth"
        FRAMEWORK TRUE
        PUBLIC_HEADER "${public_main_HEADER}"
        FRAMEWORK_VERSION "${LIB_VERSION_CURRENT}"
        INSTALL_NAME_DIR ""
        VERSION ${LIB_VERSION_INFO}
        SOVERSION ${LIB_VERSION_CURRENT}
    )
elseif ( OS2 )
    set_target_properties ( libfluidsynth
      PROPERTIES
        PUBLIC_HEADER "${public_HEADERS}"
        OUTPUT_NAME "fluidsynth"
        VERSION ${LIB_VERSION_INFO}
        SOVERSION ${LIB_VERSION_CURRENT}
    )
elseif ( WIN32 )
  set_target_properties ( libfluidsynth
    PROPERTIES
      PUBLIC_HEADER "${public_HEADERS}"
      ARCHIVE_OUTPUT_NAME "fluidsynth"
      PREFIX "lib"
      OUTPUT_NAME "fluidsynth-${LIB_VERSION_CURRENT}"
      VERSION ${LIB_VERSION_INFO}
      SOVERSION ${LIB_VERSION_CURRENT}
    )
else ( MACOSX_FRAMEWORK )
  set_target_properties ( libfluidsynth
    PROPERTIES
      PUBLIC_HEADER "${public_HEADERS}"
      PREFIX "lib"
      OUTPUT_NAME "fluidsynth"
      VERSION ${LIB_VERSION_INFO}
      SOVERSION ${LIB_VERSION_CURRENT}
  )
endif ( MACOSX_FRAMEWORK )

target_link_libraries ( libfluidsynth
    ${GLIB_LIBRARIES}
    ${GMODULE_LIBRARIES}
    ${LASH_LIBRARIES}
    ${JACK_LIBRARIES}
    ${ALSA_LIBRARIES}
    ${PULSE_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
    ${LIBSNDFILE_LIBRARIES}
    ${DBUS_LIBRARIES}
    ${READLINE_LIBS}
    ${DART_LIBS}
    ${COREAUDIO_LIBS}
    ${COREMIDI_LIBS}
    ${WINDOWS_LIBS}
    ${MidiShare_LIBS}
    ${LIBFLUID_LIBS}
)

# ************ CLI program ************

set ( fluidsynth_SOURCES fluidsynth.c )

add_executable ( fluidsynth
    ${fluidsynth_SOURCES}
)

if ( FLUID_CPPFLAGS )
  set_target_properties ( fluidsynth
    PROPERTIES COMPILE_FLAGS ${FLUID_CPPFLAGS} )
endif ( FLUID_CPPFLAGS )

target_link_libraries ( fluidsynth
    libfluidsynth
    ${FLUID_LIBS}
)

if ( MACOSX_FRAMEWORK )
  install ( TARGETS fluidsynth libfluidsynth
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    FRAMEWORK DESTINATION ${FRAMEWORK_INSTALL_DIR}
    ARCHIVE DESTINATION ${FRAMEWORK_INSTALL_DIR}
)
else ( MACOSX_FRAMEWORK )
  install ( TARGETS fluidsynth libfluidsynth
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    PUBLIC_HEADER DESTINATION ${INCLUDE_INSTALL_DIR}/fluidsynth 
)
   install ( FILES ${public_main_HEADER} DESTINATION ${INCLUDE_INSTALL_DIR} )
endif ( MACOSX_FRAMEWORK )

