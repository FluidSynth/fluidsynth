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

include_directories (
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/src/rvoice
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
  set ( fluid_dbus_SOURCES fluid_rtkit.c fluid_rtkit.h )
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
  set ( fluid_lash_SOURCES fluid_lash.c fluid_lash.h )
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
  set ( fluid_ladspa_SOURCES fluid_ladspa.c fluid_ladspa.h )
endif ( LADSPA_SUPPORT )

if ( MIDISHARE_SUPPORT )
  set ( fluid_midishare_SOURCES drivers/fluid_midishare.c )
  include_directories ( ${MidiShare_INCLUDE_DIRS} )
endif ( MIDISHARE_SUPPORT )

set ( libfluidsynth_SOURCES 
    fluid_adriver.c 
    fluid_adriver.h 
    fluid_chan.c 
    fluid_chan.h 
    fluid_chorus.c 
    fluid_chorus.h 
    fluid_cmd.c 
    fluid_cmd.h
    fluid_conv.c 
    fluid_conv.h 
    fluid_defsfont.c 
    fluid_defsfont.h 
    fluid_event.c 
    fluid_event_priv.h 
    fluid_event_queue.c 
    fluid_event_queue.h 
    fluid_gen.c 
    fluid_gen.h 
    fluid_hash.c 
    fluid_hash.h 
    rvoice/fluid_iir_filter.c
    rvoice/fluid_iir_filter.h
    rvoice/fluid_lfo.h
    rvoice/fluid_adsr_env.h
    rvoice/fluid_rvoice.h
    rvoice/fluid_rvoice.c
    rvoice/fluid_rvoice_dsp.c
    fluid_list.c 
    fluid_list.h 
    fluid_mdriver.c 
    fluid_mdriver.h 
    fluid_midi.c 
    fluid_midi.h 
    fluid_midi_router.c 
    fluid_midi_router.h 
    fluid_mod.c 
    fluid_mod.h 
    fluid_phase.h 
    fluid_ramsfont.c 
    fluid_ramsfont.h 
    fluid_rev.c 
    fluid_rev.h 
    fluid_seqbind.c 
    fluid_seq.c 
    fluid_settings.c 
    fluid_settings.h 
    fluid_sfont.h 
    fluid_synth.c 
    fluid_synth.h 
    fluidsynth_priv.h 
    fluid_sys.c 
    fluid_sys.h 
    fluid_tuning.c 
    fluid_tuning.h 
    fluid_voice.c 
    fluid_voice.h 
    fluid_filerenderer.c 
    drivers/fluid_aufile.c 
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

add_library ( libfluidsynth SHARED 
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
)

set_target_properties ( libfluidsynth 
  PROPERTIES
    OUTPUT_NAME "fluidsynth" 
    VERSION ${LT_VERSION_INFO}
    SOVERSION ${LT_VERSION_CURRENT}
    LT_VERSION_CURRENT ${LT_VERSION_CURRENT}
    LT_VERSION_AGE ${LT_VERSION_AGE}
    LT_VERSION_REVISION ${LT_VERSION_REVISION}
)

if ( LIBFLUID_CPPFLAGS ) 
  set_target_properties ( libfluidsynth 
    PROPERTIES COMPILE_FLAGS ${LIBFLUID_CPPFLAGS} )
endif ( LIBFLUID_CPPFLAGS ) 

include ( CreateLibtoolFile )

create_libtool_file ( libfluidsynth "lib${LIB_SUFFIX}" )

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

install ( TARGETS fluidsynth libfluidsynth
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib${LIB_SUFFIX}
  ARCHIVE DESTINATION lib${LIB_SUFFIX}
)
