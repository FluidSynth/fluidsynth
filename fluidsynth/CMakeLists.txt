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

project ( FluidSynth C )
cmake_minimum_required ( VERSION 2.6 )
set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_admin )

# FluidSynth package name
set ( PACKAGE "fluidsynth" )

# FluidSynth package version
set ( FLUIDSYNTH_VERSION_MAJOR 1 )
set ( FLUIDSYNTH_VERSION_MINOR 1 )
set ( FLUIDSYNTH_VERSION_MICRO 3 )
set ( VERSION "${FLUIDSYNTH_VERSION_MAJOR}.${FLUIDSYNTH_VERSION_MINOR}.${FLUIDSYNTH_VERSION_MICRO}" )      
set ( FLUIDSYNTH_VERSION "\"${VERSION}\"" )

# libfluidsynth - Library version
# *** NOTICE ***
# Update library version upon each release (follow these steps in order)
# if any source code changes: REVISION++
# if any interfaces added/removed/changed: REVISION=0
# if any interfaces removed/changed (compatibility broken): CURRENT++
# if any interfaces have been added: AGE++
# if any interfaces have been removed/changed (compatibility broken): AGE=0
# This is not exactly the same algorithm as the libtool one, but the results are the same.
set ( LT_VERSION_CURRENT 1 )
set ( LT_VERSION_AGE 4 )
set ( LT_VERSION_REVISION 1 )
set ( LT_VERSION_INFO "${LT_VERSION_CURRENT}.${LT_VERSION_AGE}.${LT_VERSION_REVISION}" )

# Options disabled by default
option ( enable-floats "enable type float instead of double for DSP samples" off )
option ( enable-profiling "profile the dsp code" off )
option ( enable-ladspa "enable LADSPA effect units" off )
option ( enable-trap-on-fpe "enable SIGFPE trap on Floating Point Exceptions" off )
option ( enable-fpe-check "enable Floating Point Exception checks and debug messages" off )
option ( enable-debug "enable debugging (default=no)" off )
option ( enable-ladcca "compile LADCCA support if it is available (deprecated)" off )

# Options enabled by default
option ( enable-libsndfile "compile libsndfile support (if it is available)" on )
option ( enable-aufile "compile support for sound file output" on )
option ( enable-pulseaudio "compile PulseAudio support (if it is available)" on ) 
option ( enable-alsa "compile ALSA support (if it is available)" on )
option ( enable-portaudio "compile PortAudio support (if it is available)" on )
option ( enable-jack "compile JACK support (if it is available)" on )
option ( enable-coreaudio "compile CoreAudio support (if it is available)" on )
option ( enable-coremidi "compile CoreMIDI support (if it is available)" on )
option ( enable-midishare "compile MidiShare support (if it is available)" on )
option ( enable-dart "compile DART support (if it is available)" on )
option ( enable-readline "compile readline lib line editing (if it is available)" on )
option ( enable-lash "compile LASH support (if it is available)" on )
option ( enable-dbus "compile DBUS support (if it is available)" on )
option ( BUILD_SHARED_LIBS "Build a shared object or DLL" on )

# Initialize the library directory name suffix.
if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set ( _init_lib_suffix "64" )
else ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set ( _init_lib_suffix "" )
endif ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
set ( LIB_SUFFIX ${_init_lib_suffix} CACHE STRING 
      "library directory name suffix (32/64/nothing)" )
mark_as_advanced ( LIB_SUFFIX )

# Basic C library checks
include ( CheckSTDC )
include ( CheckIncludeFile )
check_include_file ( string.h HAVE_STRING_H )
check_include_file ( stdlib.h HAVE_STDLIB_H ) 
check_include_file ( stdio.h HAVE_STDIO_H ) 
check_include_file ( math.h HAVE_MATH_H ) 
check_include_file ( errno.h HAVE_ERRNO_H ) 
check_include_file ( stdarg.h HAVE_STDARG_H ) 
check_include_file ( unistd.h HAVE_UNISTD_H ) 
check_include_file ( memory.h HAVE_MEMORY_H )
check_include_file ( sys/mman.h HAVE_SYS_MMAN_H ) 
check_include_file ( sys/types.h HAVE_SYS_TYPES_H ) 
check_include_file ( sys/time.h HAVE_SYS_TIME_H ) 
check_include_file ( sys/stat.h HAVE_SYS_STAT_H ) 
check_include_file ( sys/ioctl.h HAVE_SYS_IOCTL_H )
check_include_file ( fcntl.h HAVE_FCNTL_H ) 
check_include_file ( sys/socket.h HAVE_SYS_SOCKET_H ) 
check_include_file ( netinet/in.h HAVE_NETINET_IN_H ) 
check_include_file ( netinet/tcp.h HAVE_NETINET_TCP_H ) 
check_include_file ( arpa/inet.h HAVE_ARPA_INET_H ) 
check_include_file ( limits.h  HAVE_LIMITS_H )
check_include_file ( pthread.h HAVE_PTHREAD_H ) 
check_include_file ( signal.h HAVE_SIGNAL_H ) 
include ( TestInline )
include ( TestVLA )
include ( TestBigEndian )
test_big_endian ( WORDS_BIGENDIAN )

set ( LIBFLUID_CPPFLAGS )
set ( LIBFLUID_LIBS )
set ( FLUID_CPPFLAGS )
set ( FLUID_LIBS )

# Options for the GNU C compiler only
if ( CMAKE_COMPILER_IS_GNUCC )
  if ( NOT APPLE )
    set ( CMAKE_EXE_LINKER_FLAGS 
          "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed" )
    set ( CMAKE_SHARED_LINKER_FLAGS 
          "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined" )
  endif ( NOT APPLE )
  set ( GNUCC_WARNING_FLAGS "-Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wstrict-prototypes -Wno-unused-parameter -Wno-cast-qual")
  set ( CMAKE_C_FLAGS_DEBUG "-g -DDEBUG ${GNUCC_WARNING_FLAGS}" )
  set ( CMAKE_C_FLAGS_RELEASE "-O2 -fomit-frame-pointer -funroll-all-loops -finline-functions -DNDEBUG ${GNUCC_WARNING_FLAGS}" )
  set ( CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -fomit-frame-pointer -funroll-all-loops -finline-functions -DNDEBUG ${GNUCC_WARNING_FLAGS}" )
endif ( CMAKE_COMPILER_IS_GNUCC )

# Windows
set ( WINDOWS_SUPPORT )
set ( WINDOWS_LIBS )
set ( MINGW32 )
if ( WIN32 )
  include ( CheckIncludeFiles )
  check_include_file ( windows.h HAVE_WINDOWS_H )
  check_include_file ( io.h HAVE_IO_H )
  check_include_file ( dsound.h HAVE_DSOUND_H )
  check_include_files ( "windows.h;mmsystem.h" HAVE_MMSYSTEM_H )
  set ( WINDOWS_SUPPORT ${HAVE_WINDOWS_H} )
  set ( WINDOWS_LIBS "dsound;winmm;ws2_32" )
  set ( LIBFLUID_CPPFLAGS "-DFLUIDSYNTH_DLL_EXPORTS" )
  set ( FLUID_CPPFLAGS "-DFLUIDSYNTH_NOT_A_DLL" )
  set ( CMAKE_DEBUG_POSTFIX "_debug" )
  # MinGW compiler (a Windows GCC port)
  if ( MINGW ) 
    set ( MINGW32 1 )
    add_definitions ( -mms-bitfields )
  endif  ( MINGW )
else ( WIN32 )
# Check PThreads, but not in Windows 
  find_package ( Pthreads REQUIRED )
  set ( HAVE_LIBPTHREAD ${PTHREADS_FOUND} )
  set ( LIBFLUID_LIBS "m" )
endif ( WIN32 )

# IBM OS/2
set ( DART_SUPPORT )
set ( DART_LIBS )  
if ( ${CMAKE_SYSTEM} MATCHES "os2" )
  set ( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Zbin-files" )
  set ( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Zbin-files -no-undefined" )
  if ( enable-dart )
    check_include_file ( os2me.h HAVE_DART_H )
    set ( DART_SUPPORT ${HAVE_DART_H} )
    set ( DART_INCLUDE_DIRS )
  endif ( enable-dart )
endif ( ${CMAKE_SYSTEM} MATCHES "os2" )

# Solaris / SunOS
if ( ${CMAKE_SYSTEM} MATCHES "SunOS" ) 
  set ( FLUID_LIBS "${FLUID_LIBS};nsl;socket" ) 
  set ( LIBFLUID_LIBS "${LIBFLUID_LIBS};nsl;socket" ) 
endif ( ${CMAKE_SYSTEM} MATCHES "SunOS" )

# Apple Mac OSX
set ( COREAUDIO_SUPPORT )
set ( COREAUDIO_LIBS ) 
set ( COREMIDI_SUPPORT )
set ( COREMIDI_LIBS )
set ( DARWIN ) 
if ( ${CMAKE_SYSTEM} MATCHES "Darwin" )
  set ( DARWIN 1 )
  set ( CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX} )
  if ( enable-coreaudio )
    check_include_file ( CoreAudio/AudioHardware.h COREAUDIO_FOUND )
    if ( COREAUDIO_FOUND )
      set ( COREAUDIO_SUPPORT ${COREAUDIO_FOUND} )
      set ( COREAUDIO_LIBS "-Wl,-framework,CoreAudio" )
    endif ( COREAUDIO_FOUND )
  endif ( enable-coreaudio )
  if ( enable-coremidi )
    check_include_file ( CoreMIDI/MIDIServices.h COREMIDI_FOUND )
    if ( COREMIDI_FOUND )
      set ( COREMIDI_SUPPORT ${COREMIDI_FOUND} )
      set ( COREMIDI_LIBS "-Wl,-framework,CoreMIDI,-framework,CoreServices" )
    endif ( COREMIDI_FOUND )
  endif ( enable-coremidi )
endif ( ${CMAKE_SYSTEM} MATCHES "Darwin" )

set ( WITH_FLOAT )
if ( enable-floats )
    set ( WITH_FLOAT 1 )
endif ( enable-floats )

set ( WITH_PROFILING )
if ( enable-profiling )
    set ( WITH_PROFILING 1 )
endif ( enable-profiling ) 

set ( HAVE_LIBDL )
set ( LADSPA_SUPPORT )
if ( enable-ladspa )
  check_include_file ( ladspa.h LADSPA_SUPPORT )
  if ( LADSPA_SUPPORT )
    set ( LADSPA 1 )
    if ( CMAKE_DL_LIBS )
      set ( HAVE_LIBDL 1 )
      set ( LIBFLUID_LIBS "${LIBFLUID_LIBS};${CMAKE_DL_LIBS}" )
    endif ( CMAKE_DL_LIBS )    
  endif ( LADSPA_SUPPORT )        
endif ( enable-ladspa )

set ( ENABLE_TRAPONFPE )
set ( TRAP_ON_FPE )
if ( enable-trap-on-fpe AND NOT APPLE AND NOT WIN32 )
    set ( ENABLE_TRAPONFPE 1 )
    set ( TRAP_ON_FPE 1 )
endif ( enable-trap-on-fpe AND NOT APPLE AND NOT WIN32 )

set ( ENABLE_FPECHECK )
set ( FPE_CHECK ) 
if ( enable-fpe-check )
    set ( ENABLE_FPECHECK 1 )
    set ( FPE_CHECK 1 )
endif ( enable-fpe-check )
  
if ( enable-debug )  
    set ( CMAKE_BUILD_TYPE "Debug" CACHE STRING
          "Choose the build type, options: Debug Release RelWithDebInfo" FORCE )
endif ( enable-debug )

set ( ENABLE_DEBUG )
set ( DEBUG )
if ( CMAKE_BUILD_TYPE MATCHES "Debug" )
    set ( ENABLE_DEBUG 1 )
    set ( DEBUG 1 )
endif ( CMAKE_BUILD_TYPE MATCHES "Debug" )

# Mandatory tool: pkg-config
find_package ( PkgConfig REQUIRED )

# Mandatory libraries: glib and gthread
set ( GLIB_LIBRARIES ) 
pkg_check_modules ( GLIB REQUIRED glib-2.0>=2.6.5 gthread-2.0>=2.6.5 )

# Optional features
set ( LIBSNDFILE_SUPPORT )
set ( LIBSNDFILE_HASVORBIS )
set ( LIBSNDFILE_LIBRARIES )
if ( enable-libsndfile )
  pkg_check_modules ( LIBSNDFILE sndfile>=1.0.0 )
  set ( LIBSNDFILE_SUPPORT ${LIBSNDFILE_FOUND} )
  if ( LIBSNDFILE_SUPPORT ) 
    pkg_check_modules ( LIBSNDFILE_VORBIS sndfile>=1.0.18 )
    set ( LIBSNDFILE_HASVORBIS ${LIBSNDFILE_VORBIS_FOUND} )
  endif ( LIBSNDFILE_SUPPORT ) 
endif ( enable-libsndfile )

set ( AUFILE_SUPPORT )
if ( enable-aufile )
  set ( AUFILE_SUPPORT 1 )
endif ( enable-aufile )

set ( PULSE_SUPPORT )
set ( PULSE_LIBRARIES ) 
if ( enable-pulseaudio )
  pkg_check_modules ( PULSE libpulse-simple>=0.9.8 )
  set ( PULSE_SUPPORT ${PULSE_FOUND} )
endif ( enable-pulseaudio )

set ( ALSA_SUPPORT )
set ( ALSA_LIBRARIES ) 
if ( enable-alsa )
  pkg_check_modules ( ALSA alsa>=0.9.1 )
  set ( ALSA_SUPPORT ${ALSA_FOUND} )
endif ( enable-alsa )

set ( PORTAUDIO_SUPPORT )
set ( PORTAUDIO_LIBRARIES ) 
if ( enable-portaudio )
  pkg_check_modules ( PORTAUDIO portaudio-2.0>=19 )
  set ( PORTAUDIO_SUPPORT ${PORTAUDIO_FOUND} )
endif ( enable-portaudio )

find_package ( OSS QUIET )
set ( OSS_SUPPORT ${OSS_FOUND} )

set ( MIDISHARE_SUPPORT )
set ( MidiShare_LIBS )
if ( enable_midishare )
  find_package ( MidiShare )
  set ( MIDISHARE_SUPPORT ${MidiShare_FOUND} )
endif ( enable_midishare )

set ( JACK_SUPPORT )
set ( JACK_LIBRARIES ) 
if ( enable-jack )
  pkg_check_modules ( JACK jack )
  set ( JACK_SUPPORT ${JACK_FOUND} )
endif ( enable-jack )

set ( WITH_READLINE )
set ( READLINE_LIBS )
if ( enable-readline )
  find_package ( Readline )
  set ( FOUND_READLINE ${HAVE_READLINE} )
  if ( HAVE_READLINE )
    set ( WITH_READLINE 1 )
    set ( READLINE_LIBS ${READLINE_LIBRARIES} )
  endif ( HAVE_READLINE )
endif ( enable-readline )

set ( LASH_SUPPORT )
set ( LASH_LIBRARIES ) 
if ( enable-lash )
  pkg_check_modules ( LASH lash-1.0>=0.3 )
  if ( LASH_FOUND )
    set ( LASH_SUPPORT 1 )
    add_definitions ( -DHAVE_LASH )
  endif ( LASH_FOUND )
endif ( enable-lash )

set ( LADCCA_SUPPORT )
set ( LADCCA_LIBRARIES ) 
if ( enable-ladcca )
  pkg_check_modules ( LADCCA ladcca-1.0>=0.3 )
  set ( LADCCA_SUPPORT ${LADCCA_FOUND} )
endif ( enable-ladcca )

set ( DBUS_SUPPORT )
set ( DBUS_LIBRARIES )
if ( enable-dbus )
    pkg_check_modules ( DBUS dbus-1>=1.0.0 )
    set ( DBUS_SUPPORT ${DBUS_FOUND} )
endif ( enable-dbus )
 
# General configuration file
configure_file ( ${CMAKE_SOURCE_DIR}/src/config.cmake 
                 ${CMAKE_BINARY_DIR}/config.h )
add_definitions ( -DHAVE_CONFIG_H )

# Extra configuration file for MS VisualC compiler
if ( MSVC )
  configure_file ( ${CMAKE_SOURCE_DIR}/src/config_win32.cmake 
                   ${CMAKE_BINARY_DIR}/config_win32.h )
endif ( MSVC )

# Process subdirectories 
add_subdirectory ( src )
add_subdirectory ( include )
add_subdirectory ( doc )

# Extra targets for Unix build environments
if ( UNIX )
    # RPM spec
    configure_file ( fluidsynth.spec.in 
        ${CMAKE_BINARY_DIR}/fluidsynth.spec IMMEDIATE @ONLY ) 
    
    # pkg-config support
    set ( prefix "${CMAKE_INSTALL_PREFIX}" )
    set ( exec_prefix "\${prefix}" )
    set ( libdir "\${exec_prefix}/lib${LIB_SUFFIX}" )
    set ( includedir "\${prefix}/include" )
    configure_file ( fluidsynth.pc.in 
        ${CMAKE_BINARY_DIR}/fluidsynth.pc IMMEDIATE @ONLY )
    install ( FILES ${CMAKE_BINARY_DIR}/fluidsynth.pc
        DESTINATION lib${LIB_SUFFIX}/pkgconfig )
    
    # uninstall custom target
    configure_file ( "${CMAKE_SOURCE_DIR}/cmake_admin/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)
    add_custom_target ( uninstall
        "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
    
    # tarball custom target
    add_custom_target ( tarball
        COMMAND mkdir -p ${PACKAGE}-${VERSION}
        COMMAND cp -r bindings ${PACKAGE}-${VERSION}
        COMMAND cp -r cmake_admin ${PACKAGE}-${VERSION}
        COMMAND cp -r doc ${PACKAGE}-${VERSION}
        COMMAND cp -r include ${PACKAGE}-${VERSION}
        COMMAND cp -r src ${PACKAGE}-${VERSION}
        COMMAND cp AUTHORS ChangeLog CMakeLists.txt COPYING ${PACKAGE}.* INSTALL NEWS README* THANKS TODO ${PACKAGE}-${VERSION}
#       COMMAND tar -cj --exclude .svn --exclude Makefile.am -f ${PACKAGE}-${VERSION}.tar.bz2 ${PACKAGE}-${VERSION}
#       COMMAND tar -cz --exclude .svn --exclude Makefile.am -f ${PACKAGE}-${VERSION}.tar.gz ${PACKAGE}-${VERSION}
#       COMMAND zip -qr ${PACKAGE}-${VERSION}.zip ${PACKAGE}-${VERSION} -x '*.svn*' -x '*Makefile.am'
        COMMAND tar -cj --exclude .svn -f ${PACKAGE}-${VERSION}.tar.bz2 ${PACKAGE}-${VERSION}
        COMMAND tar -cz --exclude .svn -f ${PACKAGE}-${VERSION}.tar.gz ${PACKAGE}-${VERSION}
        COMMAND zip -qr ${PACKAGE}-${VERSION}.zip ${PACKAGE}-${VERSION} -x '*.svn*'
        COMMAND rm -rf ${PACKAGE}-${VERSION}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    # tarball custom target
    add_custom_target ( tarball-autotools
        COMMAND mkdir -p ${PACKAGE}-${VERSION}
        COMMAND cp -r bindings ${PACKAGE}-${VERSION}
        COMMAND cp -r cmake_admin ${PACKAGE}-${VERSION}
        COMMAND cp -r doc ${PACKAGE}-${VERSION}
        COMMAND cp -r include ${PACKAGE}-${VERSION}
        COMMAND cp -r src ${PACKAGE}-${VERSION}
        COMMAND cp AUTHORS ChangeLog CMakeLists.txt COPYING ${PACKAGE}.* INSTALL NEWS README* THANKS TODO ${PACKAGE}-${VERSION}
        COMMAND cp acinclude.m4 autogen.sh configure.ac install-sh missing mkinstalldirs Makefile.am ${PACKAGE}-${VERSION}
        # Files below are added when running autogen.sh
        # COMMAND cp -r autom4te.cache ${PACKAGE}-${VERSION}
        COMMAND cp -r m4 ${PACKAGE}-${VERSION}
        COMMAND cp aclocal.m4 compile config.guess config.sub configure depcomp ltmain.sh Makefile.in ${PACKAGE}-${VERSION}
        COMMAND tar -cj --exclude .svn -f ${PACKAGE}-${VERSION}.tar.bz2 ${PACKAGE}-${VERSION}
        COMMAND tar -cz --exclude .svn -f ${PACKAGE}-${VERSION}.tar.gz ${PACKAGE}-${VERSION}
        COMMAND zip -qr ${PACKAGE}-${VERSION}.zip ${PACKAGE}-${VERSION} -x '*.svn*'
        COMMAND rm -rf ${PACKAGE}-${VERSION}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

endif ( UNIX )

include ( report )
