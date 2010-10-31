# FluidSynth - A Software Synthesizer
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
    ${GLIB_INCLUDEDIR}
    ${GLIB_INCLUDE_DIRS}
)

# ************ library ************

if ( READLINE_SUPPORT )
  include_directories ( ${READLINE_INCLUDE_DIR} )
endif ( READLINE_SUPPORT )

if ( PULSE_SUPPORT )
  set ( fluid_pulse_SOURCES drivers/fluid_pulse.c )
  include_directories ( ${PULSE_INCLUDEDIR} ${PULSE_INCLUDE_DIRS} )
endif ( PULSE_SUPPORT )

if ( ALSA_SUPPORT )
  set ( fluid_alsa_SOURCES drivers/fluid_alsa.c )
  include_directories ( ${ALSA_INCLUDEDIR} ${ALSA_INCLUDE_DIRS} )
endif ( ALSA_SUPPORT )

if ( COREAUDIO_SUPPORT )
  set ( fluid_coreaudio_SOURCES drivers/fluid_coreaudio.c )
endif ( COREAUDIO_SUPPORT )

if ( COREMIDI_SUPPORT )
  set ( fluid_coremidi_SOURCES drivers/fluid_coremidi.c )
endif ( COREMIDI_SUPPORT )

if ( DBUS_SUPPORT )
  set ( fluid_dbus_SOURCES bindings/fluid_rtkit.c bindings/fluid_rtkit.h )
  include_directories ( ${DBUS_INCLUDEDIR} ${DBUS_INCLUDE_DIRS} )
endif ( DBUS_SUPPORT )

if ( JACK_SUPPORT )
  set ( fluid_jack_SOURCES drivers/fluid_jack.c )
  include_directories ( ${JACK_INCLUDEDIR} ${JACK_INCLUDE_DIRS} )
endif ( JACK_SUPPORT )

if ( PORTAUDIO_SUPPORT )
  set ( fluid_portaudio_SOURCES drivers/fluid_portaudio.c )
  include_directories ( ${PORTAUDIO_INCLUDEDIR} ${PORTAUDIO_INCLUDE_DIRS} )
endif ( PORTAUDIO_SUPPORT )

if ( WINDOWS_SUPPORT )
  set ( fluid_windows_SOURCES fluid_dll.c drivers/fluid_dsound.c drivers/fluid_winmidi.c )
endif ( WINDOWS_SUPPORT )

if ( OSS_SUPPORT )
  set ( fluid_oss_SOURCES drivers/fluid_oss.c )
endif ( OSS_SUPPORT )

if ( LASH_SUPPORT OR LADCCA_SUPPORT )
  set ( fluid_lash_SOURCES bindings/fluid_lash.c bindings/fluid_lash.h )
  include_directories ( ${LASH_INCLUDEDIR} ${LADCCA_INCLUDEDIR}
                        ${LASH_INCLUDE_DIRS} ${LADCCA_INCLUDE_DIRS} )
endif ( LASH_SUPPORT OR LADCCA_SUPPORT )

if ( DART_SUPPORT )
  set ( fluid_dart_SOURCES drivers/fluid_dart.c )
  include_directories ( ${DART_INCLUDE_DIRS} )
endif ( DART_SUPPORT )

if ( LIBSNDFILE_SUPPORT )
  include_directories ( ${LIBSNDFILE_INCLUDEDIR} ${LIBSNDFILE_INCLUDE_DIRS} )
endif ( LIBSNDFILE_SUPPORT )

if ( LADSPA_SUPPORT )
  set ( fluid_ladspa_SOURCES bindings/fluid_ladspa.c bindings/fluid_ladspa.h )
endif ( LADSPA_SUPPORT )

if ( MIDISHARE_SUPPORT )
  set ( fluid_midishare_SOURCES drivers/fluid_midishare.c )
  include_directories ( ${MidiShare_INCLUDE_DIRS} )
endif ( MIDISHARE_SUPPORT )

set ( config_SOURCES ${CMAKE_BINARY_DIR}/config.h )
if ( MSVC )
    set ( config_SOURCES
          ${config_SOURCES}
          ${CMAKE_BINARY_DIR}/config_win32.h )
endif ( MSVC )

set ( libfluidsynth_SOURCES
    utils/fluid_conv.c
    utils/fluid_conv.h
    utils/fluid_hash.c
    utils/fluid_hash.h
    utils/fluid_list.c
    utils/fluid_list.h
    utils/fluid_ringbuffer.c
    utils/fluid_ringbuffer.h
    utils/fluid_settings.c
    utils/fluid_settings.h
    utils/fluidsynth_priv.h
    utils/fluid_sys.c
    utils/fluid_sys.h
    sfloader/fluid_defsfont.c
    sfloader/fluid_defsfont.h
    sfloader/fluid_ramsfont.c
    sfloader/fluid_ramsfont.h
    sfloader/fluid_sfont.h
    rvoice/fluid_adsr_env.c
    rvoice/fluid_adsr_env.h
    rvoice/fluid_chorus.c
    rvoice/fluid_chorus.h
    rvoice/fluid_iir_filter.c
    rvoice/fluid_iir_filter.h
    rvoice/fluid_lfo.c
    rvoice/fluid_lfo.h
    rvoice/fluid_rvoice.h
    rvoice/fluid_rvoice.c
    rvoice/fluid_rvoice_dsp.c
    rvoice/fluid_rvoice_event.h
    rvoice/fluid_rvoice_event.c
    rvoice/fluid_rvoice_mixer.h
    rvoice/fluid_rvoice_mixer.c
    rvoice/fluid_phase.h
    rvoice/fluid_rev.c
    rvoice/fluid_rev.h
    synth/fluid_chan.c
    synth/fluid_chan.h
    synth/fluid_event.c
    synth/fluid_event_priv.h
    synth/fluid_event_queue.h
    synth/fluid_gen.c
    synth/fluid_gen.h
    synth/fluid_mod.c
    synth/fluid_mod.h
    synth/fluid_synth.c
    synth/fluid_synth.h
    synth/fluid_tuning.c
    synth/fluid_tuning.h
    synth/fluid_voice.c
    synth/fluid_voice.h
    midi/fluid_midi.c
    midi/fluid_midi.h
    midi/fluid_midi_router.c
    midi/fluid_midi_router.h
    midi/fluid_seqbind.c
    midi/fluid_seq.c
    drivers/fluid_adriver.c
    drivers/fluid_adriver.h
    drivers/fluid_mdriver.c
    drivers/fluid_mdriver.h
    drivers/fluid_aufile.c
    bindings/fluid_cmd.c
    bindings/fluid_cmd.h
    bindings/fluid_filerenderer.c
)

set ( public_HEADERS
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/audio.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/event.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/gen.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/log.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/midi.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/misc.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/mod.h
    ${CMAKE_SOURCE_DIR}/include/fluidsynth/ramsfont.h
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
    ${CMAKE_SOURCE_DIR}/include/fluidsynth.h
)

link_directories (
    ${GLIB_LIBDIR}
    ${GLIB_LIBRARY_DIRS}
    ${LASH_LIBDIR}
    ${LASH_LIBRARY_DIRS}
    ${LADCCA_LIBDIR}
    ${LADCCA_LIBRARY_DIRS}
    ${JACK_LIBDIR}
    ${JACK_LIBRARY_DIRS}
    ${ALSA_LIBDIR}
    ${ALSA_LIBRARY_DIRS}
    ${PULSE_LIBDIR}
    ${PULSE_LIBRARY_DIRS}
    ${PORTAUDIO_LIBDIR}
    ${PORTAUDIO_LIBRARY_DIRS}
    ${LIBSNDFILE_LIBDIR}
    ${LIBSNDFILE_LIBRARY_DIRS}
    ${DBUS_LIBDIR}
    ${DBUS_LIBRARY_DIRS}
)


# note: by default this target creates a shared object (or dll). To build a
# static library instead, set the option BUILD_SHARED_LIBS to FALSE.
add_library ( libfluidsynth
    ${config_SOURCES}
    ${fluid_alsa_SOURCES}
    ${fluid_coreaudio_SOURCES}
    ${fluid_coremidi_SOURCES}
    ${fluid_dart_SOURCES}
    ${fluid_dbus_SOURCES}
    ${fluid_jack_SOURCES}
    ${fluid_lash_SOURCES}
    ${fluid_ladspa_SOURCES}
    ${fluid_midishare_SOURCES}
    ${fluid_oss_SOURCES}
    ${fluid_portaudio_SOURCES}
    ${fluid_pulse_SOURCES}
    ${fluid_windows_SOURCES}
    ${libfluidsynth_SOURCES}
    ${public_HEADERS}
    ${public_main_HEADER}
)

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
else ( MACOSX_FRAMEWORK )
  set_target_properties ( libfluidsynth
    PROPERTIES
      PREFIX "lib"
      OUTPUT_NAME "fluidsynth"
      VERSION ${LIB_VERSION_INFO}
      SOVERSION ${LIB_VERSION_CURRENT}
  )
endif ( MACOSX_FRAMEWORK )

if ( LIBFLUID_CPPFLAGS )
  set_target_properties ( libfluidsynth
    PROPERTIES COMPILE_FLAGS ${LIBFLUID_CPPFLAGS} )
endif ( LIBFLUID_CPPFLAGS )

target_link_libraries ( libfluidsynth
    ${GLIB_LIBRARIES}
    ${LASH_LIBRARIES}
    ${LADCCA_LIBRARIES}
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
)
else ( MACOSX_FRAMEWORK )
  install ( TARGETS fluidsynth libfluidsynth
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}${LIB_SUFFIX}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}${LIB_SUFFIX}
)
endif ( MACOSX_FRAMEWORK )

