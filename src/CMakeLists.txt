# FluidSynth - A Software Synthesizer
#
# Copyright (C) 2003-2025 Peter Hanappe and others.
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

# ************ library ************

if ( PULSE_SUPPORT )
  set ( fluid_pulse_SOURCES drivers/fluid_pulse.c )
endif ( PULSE_SUPPORT )

if ( ALSA_SUPPORT )
  set ( fluid_alsa_SOURCES drivers/fluid_alsa.c )
endif ( ALSA_SUPPORT )

if ( COREAUDIO_SUPPORT )
  set ( fluid_coreaudio_SOURCES drivers/fluid_coreaudio.c )
endif ( COREAUDIO_SUPPORT )

if ( COREMIDI_SUPPORT )
  set ( fluid_coremidi_SOURCES drivers/fluid_coremidi.c )
endif ( COREMIDI_SUPPORT )

if ( DBUS_SUPPORT )
  set ( fluid_dbus_SOURCES bindings/fluid_rtkit.c bindings/fluid_rtkit.h )
endif ( DBUS_SUPPORT )

if ( JACK_SUPPORT )
  set ( fluid_jack_SOURCES drivers/fluid_jack.c )
endif ( JACK_SUPPORT )

if ( PIPEWIRE_SUPPORT )
  set ( fluid_pipewire_SOURCES drivers/fluid_pipewire.c )
endif ( PIPEWIRE_SUPPORT )

if ( PORTAUDIO_SUPPORT )
  set ( fluid_portaudio_SOURCES drivers/fluid_portaudio.c )
endif ( PORTAUDIO_SUPPORT )

if ( DSOUND_SUPPORT )
  set ( fluid_dsound_SOURCES drivers/fluid_dsound.c )
endif ( DSOUND_SUPPORT )

if ( WASAPI_SUPPORT )
  set ( fluid_wasapi_SOURCES drivers/fluid_wasapi.c )
endif ( WASAPI_SUPPORT )

if ( WAVEOUT_SUPPORT )
  set ( fluid_waveout_SOURCES drivers/fluid_waveout.c )
endif ( WAVEOUT_SUPPORT )

if ( WINMIDI_SUPPORT )
  set ( fluid_winmidi_SOURCES drivers/fluid_winmidi.c )
endif ( WINMIDI_SUPPORT )

if ( SDL3_SUPPORT )
  set ( fluid_sdl3_SOURCES drivers/fluid_sdl3.c )
endif ( SDL3_SUPPORT )

if ( OSS_SUPPORT )
  set ( fluid_oss_SOURCES drivers/fluid_oss.c )
endif ( OSS_SUPPORT )

if ( DART_SUPPORT )
  set ( fluid_dart_SOURCES drivers/fluid_dart.c )
endif ( DART_SUPPORT )

if ( KAI_SUPPORT )
  set ( fluid_kai_SOURCES drivers/fluid_kai.c )
endif ( KAI_SUPPORT )

if ( MIDISHARE_SUPPORT )
  set ( fluid_midishare_SOURCES drivers/fluid_midishare.c )
endif ( MIDISHARE_SUPPORT )

if ( AUFILE_SUPPORT )
  set ( fluid_aufile_SOURCES drivers/fluid_aufile.c )
endif ( AUFILE_SUPPORT )

if ( ENABLE_NATIVE_DLS )
  set ( fluid_dls_SOURCES sfloader/fluid_dls.cpp sfloader/fluid_dls.h )
endif ( ENABLE_NATIVE_DLS )

if ( LIBINSTPATCH_SUPPORT )
  set ( fluid_libinstpatch_SOURCES sfloader/fluid_instpatch.c sfloader/fluid_instpatch.h )
endif ( LIBINSTPATCH_SUPPORT )

if ( OPENSLES_SUPPORT )
  set ( fluid_opensles_SOURCES drivers/fluid_opensles.c )
endif ( OPENSLES_SUPPORT )

if ( OBOE_SUPPORT )
  set ( fluid_oboe_SOURCES drivers/fluid_oboe.cpp )
endif ( OBOE_SUPPORT )

set ( config_SOURCES ${FluidSynth_BINARY_DIR}/config.h )

set ( libfluidsynth_SOURCES
    gentables/fluid_ct2hz.cpp
    gentables/fluid_cb2amp.cpp
    gentables/fluid_concave.cpp
    gentables/fluid_convex.cpp
    gentables/fluid_pan.cpp
    gentables/fluid_interp_coeff.cpp
    gentables/fluid_interp_coeff_linear.cpp
    gentables/fluid_interp_coeff_sinc7.cpp
    gentables/ConstExprArr.hpp
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
    sfloader/fluid_sfont.h
    sfloader/fluid_sfont.c
    sfloader/fluid_sffile.c
    sfloader/fluid_sffile.h
    sfloader/fluid_samplecache.c
    sfloader/fluid_samplecache.h
    rvoice/fluid_adsr_env.c
    rvoice/fluid_adsr_env.h
    rvoice/fluid_chorus.c
    rvoice/fluid_chorus.h
    rvoice/fluid_iir_filter_impl.cpp
    rvoice/fluid_iir_filter.c
    rvoice/fluid_iir_filter.h
    rvoice/fluid_lfo.c
    rvoice/fluid_lfo.h
    rvoice/fluid_rvoice.h
    rvoice/fluid_rvoice.c
    rvoice/fluid_rvoice_dsp.cpp
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
    synth/fluid_event.h
    synth/fluid_gen.c
    synth/fluid_gen.h
    synth/fluid_mod.c
    synth/fluid_mod.h
    synth/fluid_synth.c
    synth/fluid_synth.h
    synth/fluid_synth_monopoly.c
    synth/fluid_tuning.c
    synth/fluid_tuning.h
    synth/fluid_voice.c
    synth/fluid_voice.h
    midi/fluid_midi.c
    midi/fluid_midi.h
    midi/fluid_midi_router.c
    midi/fluid_midi_router.h
    midi/fluid_seqbind.c
    midi/fluid_seqbind_notes.cpp
    midi/fluid_seq.c
    midi/fluid_seq_queue.cpp
    drivers/fluid_adriver.c
    drivers/fluid_adriver.h
    drivers/fluid_mdriver.c
    drivers/fluid_mdriver.h
    bindings/fluid_cmd.c
    bindings/fluid_cmd.h
    bindings/fluid_filerenderer.c
    bindings/fluid_ladspa.c
    bindings/fluid_ladspa.h
)

set ( fluid_osal_SOURCES utils/fluid_sys_${osal}.h )
if ( osal STREQUAL "cpp11" )
  set ( fluid_osal_SOURCES ${fluid_osal_SOURCES} utils/fluid_sys_${osal}.cpp )
else ( )
  set ( fluid_osal_SOURCES ${fluid_osal_SOURCES} utils/fluid_sys_${osal}.c )
endif ( )

set ( fluid_file_SOURCES
    utils/fluid_file.h
    utils/fluid_file.cpp
)

if ( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  set_source_files_properties(rvoice/fluid_iir_filter_impl.cpp PROPERTIES COMPILE_FLAGS "-fno-math-errno -ffast-math")
elseif ( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
  set_source_files_properties(rvoice/fluid_iir_filter_impl.cpp PROPERTIES COMPILE_FLAGS "/fp:fast")
endif ( )


set ( public_HEADERS
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/audio.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/event.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/gen.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/ladspa.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/log.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/midi.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/misc.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/mod.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/seq.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/seqbind.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/settings.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/sfont.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/shell.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/synth.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/types.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/voice.h
    ${FluidSynth_BINARY_DIR}/include/fluidsynth/version.h
)

set ( public_main_HEADER
    ${FluidSynth_BINARY_DIR}/include/fluidsynth.h
)

configure_file ( ${FluidSynth_SOURCE_DIR}/include/fluidsynth/version.h.in
                 ${FluidSynth_BINARY_DIR}/include/fluidsynth/version.h )
configure_file ( ${FluidSynth_SOURCE_DIR}/include/fluidsynth.cmake
                 ${public_main_HEADER} )

if ( WIN32 )
    set(TARGET_TYPE "VFT_DLL")
    set(TARGET_FILENAME "libfluidsynth-${LIB_VERSION_CURRENT}.dll")
    set(PRODUCT_NAME "FluidSynth Library")
    set(VersionFilesOutputVariable "${FluidSynth_BINARY_DIR}/src/VersionInfoLib.rc")
    configure_file(${FluidSynth_SOURCE_DIR}/cmake_admin/VersionResource.rc.in ${VersionFilesOutputVariable} @ONLY)
endif ( WIN32 )

add_library ( libfluidsynth-OBJ OBJECT
    ${config_SOURCES}
    ${fluid_alsa_SOURCES}
    ${fluid_aufile_SOURCES}
    ${fluid_coreaudio_SOURCES}
    ${fluid_coremidi_SOURCES}
    ${fluid_dart_SOURCES}
    ${fluid_dbus_SOURCES}
    ${fluid_jack_SOURCES}
    ${fluid_kai_SOURCES}
    ${fluid_pipewire_SOURCES}
    ${fluid_midishare_SOURCES}
    ${fluid_opensles_SOURCES}
    ${fluid_oboe_SOURCES}
    ${fluid_oss_SOURCES}
    ${fluid_portaudio_SOURCES}
    ${fluid_pulse_SOURCES}
    ${fluid_dsound_SOURCES}
    ${fluid_wasapi_SOURCES}
    ${fluid_waveout_SOURCES}
    ${fluid_winmidi_SOURCES}
    ${fluid_sdl3_SOURCES}
    ${fluid_dls_SOURCES}
    ${fluid_libinstpatch_SOURCES}
    ${fluid_osal_SOURCES}
    ${fluid_file_SOURCES}
    ${libfluidsynth_SOURCES}
    ${public_HEADERS}
    ${public_main_HEADER}
    ${VersionFilesOutputVariable}
)

target_include_directories ( libfluidsynth-OBJ PRIVATE
    ${FluidSynth_BINARY_DIR}
    ${FluidSynth_BINARY_DIR}/include
    ${FluidSynth_SOURCE_DIR}/src
    ${FluidSynth_SOURCE_DIR}/src/drivers
    ${FluidSynth_SOURCE_DIR}/src/synth
    ${FluidSynth_SOURCE_DIR}/src/rvoice
    ${FluidSynth_SOURCE_DIR}/src/midi
    ${FluidSynth_SOURCE_DIR}/src/utils
    ${FluidSynth_SOURCE_DIR}/src/sfloader
    ${FluidSynth_SOURCE_DIR}/src/bindings
    ${FluidSynth_SOURCE_DIR}/include
    ${GCEM_INCLUDE_DIR}
)

if ( LIBFLUID_CPPFLAGS )
  set_target_properties ( libfluidsynth-OBJ
    PROPERTIES COMPILE_FLAGS ${LIBFLUID_CPPFLAGS} )
endif ( LIBFLUID_CPPFLAGS )

# The CMake Xcode generator fails to build the framework unless it references
# a source file. Create an empty one in the build directory to use for this
# purpose.
if ( MACOSX_FRAMEWORK )
  set ( MACOSX_FRAMEWORK_SOURCE_HACK ${CMAKE_BINARY_DIR}/empty_file_for_linking_issue.cpp )
  file ( TOUCH ${MACOSX_FRAMEWORK_SOURCE_HACK} )
endif ( MACOSX_FRAMEWORK )

# Note: by default this target creates a shared object (or dll). To build a
# static library instead, set the option BUILD_SHARED_LIBS to FALSE.
# Further note: The headers must be explicitly added here to have CMake install
# them correctly in case of MACOSX_FRAMEWORK
add_library ( libfluidsynth
    $<TARGET_OBJECTS:libfluidsynth-OBJ>
    ${public_main_HEADER}
    ${public_HEADERS}
    ${MACOSX_FRAMEWORK_SOURCE_HACK}
)

if ( MACOSX_FRAMEWORK )
    set_source_files_properties ( ${public_HEADERS}
         PROPERTIES MACOSX_PACKAGE_LOCATION Headers/fluidsynth
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
      PREFIX "lib"
      OUTPUT_NAME "fluidsynth-${LIB_VERSION_CURRENT}"
      IMPORT_PREFIX "lib"
      ARCHIVE_OUTPUT_NAME "fluidsynth-${LIB_VERSION_CURRENT}"
      VERSION ${LIB_VERSION_INFO}
      SOVERSION ${LIB_VERSION_CURRENT}
    )
elseif ( CYGWIN )
  set_target_properties ( libfluidsynth
    PROPERTIES
      PUBLIC_HEADER "${public_HEADERS}"
      PREFIX "cyg"
      OUTPUT_NAME "fluidsynth"
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

# Since CMake 3.12, OBJECT libraries can be linked to with target_link_libraries().
# See https://cmake.org/cmake/help/latest/command/target_link_libraries.html#linking-object-libraries
# Object Libraries may "link" to other libraries to get usage requirements,
# but since they do not have a link step nothing is done with their object files.
target_link_libraries ( libfluidsynth-OBJ PUBLIC
    ${DART_LIBS}
    ${KAI_LIBS}
    ${COREAUDIO_LIBS}
    ${COREMIDI_LIBS}
    ${WINDOWS_LIBS}
    ${LIBFLUID_LIBS}
)

if ( TARGET OpenMP::OpenMP_C AND HAVE_OPENMP )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC OpenMP::OpenMP_C )
endif()

if ( TARGET GLib2::glib-2 )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC GLib2::glib-2 GLib2::gthread-2 )
endif()

if ( TARGET SndFile::sndfile AND LIBSNDFILE_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC SndFile::sndfile )
endif()

if ( PULSE_SUPPORT )
    # need to include PULSEAUDIO_INCLUDE_DIR to make it compile with homebrew on Mac
    target_include_directories( libfluidsynth-OBJ PRIVATE ${PULSEAUDIO_INCLUDE_DIRS} ${PULSEAUDIO_INCLUDE_DIR} )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC ${PULSEAUDIO_LIBRARIES} )
endif()

if ( TARGET ALSA::ALSA AND ALSA_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC ALSA::ALSA )
endif()

if ( TARGET PortAudio::PortAudio AND PORTAUDIO_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC PortAudio::PortAudio )
endif()

if ( TARGET Jack::Jack AND JACK_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC Jack::Jack )
endif()

if ( TARGET PipeWire::PipeWire AND PIPEWIRE_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC PipeWire::PipeWire )
endif()

if ( TARGET dbus-1 AND DBUS_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC dbus-1 )
endif()

if ( TARGET GLib2::gmodule-2 AND LADSPA_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC GLib2::gmodule-2 )
endif()

if ( TARGET InstPatch::libinstpatch AND LIBINSTPATCH_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC InstPatch::libinstpatch )
endif()

if ( SDL3_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC SDL3::SDL3 )
endif()

if ( TARGET oboe::oboe AND OBOE_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC oboe::oboe )
endif()

if ( TARGET Readline::Readline AND READLINE_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC Readline::Readline )
endif()

if ( DART_SUPPORT )
    target_include_directories ( libfluidsynth-OBJ PUBLIC ${DART_INCLUDE_DIRS} )
endif ( DART_SUPPORT )

if ( KAI_SUPPORT )
    target_include_directories ( libfluidsynth-OBJ PUBLIC ${KAI_INCLUDE_DIRS} )
endif ( KAI_SUPPORT )

if ( MIDISHARE_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC MidiShare::MidiShare )
endif ( MIDISHARE_SUPPORT )

if ( OPENSLES_SUPPORT )
    target_link_libraries( libfluidsynth-OBJ PUBLIC OpenSLES::OpenSLES )
endif ( OPENSLES_SUPPORT )

# This doesn't install any object file at all!
# it is only a trick for exporting targets
install( TARGETS libfluidsynth-OBJ
    EXPORT FluidSynthTargets
    OBJECTS )

# Here are applied/linked the OBJECT library dependencies
target_link_libraries ( libfluidsynth PRIVATE libfluidsynth-OBJ )

# ************ CLI program ************

set ( fluidsynth_SOURCES fluidsynth.c )

if ( WASAPI_SUPPORT )
  set ( fluidsynth_SOURCES ${fluidsynth_SOURCES} fluid_wasapi_device_enumerate.c )
endif ( WASAPI_SUPPORT )

if ( WIN32 )
    set(TARGET_TYPE "VFT_APP")
    set(TARGET_FILENAME "fluidsynth.exe")
    set(PRODUCT_NAME "FluidSynth CLI")
    set(VersionFilesOutputVariable "${FluidSynth_BINARY_DIR}/src/VersionInfo.rc")
    configure_file(${FluidSynth_SOURCE_DIR}/cmake_admin/VersionResource.rc.in ${VersionFilesOutputVariable} @ONLY)
endif ( WIN32 )

set ( getopt_SOURCES "")
if ( WIN32 AND NOT HAVE_GETOPT_H )
    set ( getopt_SOURCES
        "${FluidSynth_SOURCE_DIR}/contrib/getopt/win/getopt.c"
        "${FluidSynth_SOURCE_DIR}/contrib/getopt/win/getopt.h"
    )
endif ( WIN32 AND NOT HAVE_GETOPT_H )

add_executable ( fluidsynth
    ${fluidsynth_SOURCES}
    ${getopt_SOURCES}
    ${fluid_file_SOURCES}
    ${VersionFilesOutputVariable}
)

set_target_properties ( fluidsynth
    PROPERTIES IMPORT_PREFIX "" )

if ( FLUID_CPPFLAGS )
  set_target_properties ( fluidsynth
    PROPERTIES COMPILE_FLAGS ${FLUID_CPPFLAGS} )
endif ( FLUID_CPPFLAGS )

target_include_directories ( fluidsynth PRIVATE
    ${FluidSynth_BINARY_DIR}
    ${FluidSynth_BINARY_DIR}/include
    ${FluidSynth_SOURCE_DIR}/src/bindings
    ${FluidSynth_SOURCE_DIR}/src/midi
    ${FluidSynth_SOURCE_DIR}/src/rvoice
    ${FluidSynth_SOURCE_DIR}/src/sfloader
    ${FluidSynth_SOURCE_DIR}/src/synth
    ${FluidSynth_SOURCE_DIR}/src/utils
    ${FluidSynth_SOURCE_DIR}/src/
    ${FluidSynth_SOURCE_DIR}/include
)

if ( WIN32 AND NOT HAVE_GETOPT_H )
  target_include_directories ( fluidsynth PRIVATE
        "${FluidSynth_SOURCE_DIR}/contrib/getopt/win"
  )
endif ( WIN32 AND NOT HAVE_GETOPT_H )

target_link_libraries ( fluidsynth PRIVATE
    libfluidsynth
    ${FLUID_LIBS}
)

if ( TARGET OpenMP::OpenMP_C AND HAVE_OPENMP )
    target_link_libraries ( fluidsynth PRIVATE OpenMP::OpenMP_C )
endif()

if ( TARGET Systemd::libsystemd AND SYSTEMD_SUPPORT )
    target_link_libraries ( fluidsynth PRIVATE Systemd::libsystemd )
endif()

if ( TARGET GLib2::glib-2 ) # because g_file_test()
    target_link_libraries ( fluidsynth PRIVATE GLib2::glib-2 )
endif()

if ( SDL3_SUPPORT )
    target_link_libraries ( fluidsynth PUBLIC SDL3::SDL3 )
endif ( SDL3_SUPPORT )

if ( TARGET PipeWire::PipeWire AND PIPEWIRE_SUPPORT ) # because pw_init() etc.
    target_link_libraries ( fluidsynth PRIVATE PipeWire::PipeWire )
endif()

if ( MINGW AND enable-unicode )
    target_link_options( fluidsynth PRIVATE -municode )
endif()

if ( MACOSX_FRAMEWORK )
  install ( TARGETS fluidsynth libfluidsynth
    EXPORT FluidSynthTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FRAMEWORK DESTINATION "${FRAMEWORK_INSTALL_PREFIX}/${FRAMEWORK_INSTALL_DIR}"
    ARCHIVE DESTINATION "${FRAMEWORK_INSTALL_PREFIX}/${FRAMEWORK_INSTALL_DIR}"
)
else ( MACOSX_FRAMEWORK )
  install ( TARGETS fluidsynth libfluidsynth
    EXPORT FluidSynthTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fluidsynth
)
   install ( FILES ${public_main_HEADER} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )
endif ( MACOSX_FRAMEWORK )

# Exported targets.

# build_interface: for the libfluidsynth target when imported from the build directory.
# install_interface: for the target when imported from the installed directory.
target_include_directories(libfluidsynth PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/;${PROJECT_BINARY_DIR}/include/>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if (BUILD_SHARED_LIBS)
    set(target-type shared)
else ()
    set(target-type static)
endif ()

# installation of the exported targets
install(EXPORT FluidSynthTargets
        FILE FluidSynth-${target-type}-targets.cmake
        NAMESPACE FluidSynth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fluidsynth
)
