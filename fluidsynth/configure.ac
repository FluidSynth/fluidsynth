dnl --------------------------------------------------
dnl configure.in for FluidSynth
dnl --------------------------------------------------

AC_INIT(src/fluidsynth.c)

dnl *** NOTE *** Don't forget to update library version below also

FLUIDSYNTH_VERSION_MAJOR=1
FLUIDSYNTH_VERSION_MINOR=0
FLUIDSYNTH_VERSION_MICRO=8
FLUIDSYNTH_VERSION=$FLUIDSYNTH_VERSION_MAJOR.$FLUIDSYNTH_VERSION_MINOR.$FLUIDSYNTH_VERSION_MICRO

AC_SUBST(FLUIDSYNTH_VERSION_MAJOR)
AC_SUBST(FLUIDSYNTH_VERSION_MINOR)
AC_SUBST(FLUIDSYNTH_VERSION_MICRO)
AC_SUBST(FLUIDSYNTH_VERSION)

AM_INIT_AUTOMAKE(fluidsynth, $FLUIDSYNTH_VERSION)

dnl Convert to quoted string for version.h substitution
FLUIDSYNTH_VERSION="\"$VERSION\""

dnl *** NOTICE ***
dnl Update library version upon each release (follow these steps in order)
dnl if any source code changes: LT_REVISION++
dnl if any interfaces added/removed/changed: LT_CURRENT++ and LT_REVISION=0
dnl if any interfaces have been added: LT_AGE++
dnl if any interfaces have been removed/changed (compatibility broken): LT_AGE=0

LT_CURRENT=2
LT_REVISION=2
LT_AGE=1

LT_VERSION_INFO="${LT_CURRENT}:${LT_REVISION}:${LT_AGE}"
AC_SUBST(LT_VERSION_INFO)

AM_CONFIG_HEADER(src/config.h)

CFLAGS="$CFLAGS -Wall"

dnl Check for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
dnl AC_PROG_RANLIB
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL
AC_PROG_MAKE_SET

dnl Check for libraries
AC_CHECK_LIB(pthread, pthread_create)

dnl Check for header files
AC_HEADER_STDC
AC_CHECK_HEADERS(string.h stdlib.h stdio.h math.h errno.h stdarg.h unistd.h sys/mman.h sys/types.h sys/time.h sys/stat.h fcntl.h sys/socket.h netinet/in.h netinet/tcp.h arpa/inet.h limits.h pthread.h signal.h windows.h)

dnl Compiler and machine specs
AC_C_INLINE
AC_C_BIGENDIAN

LIBFLUID_LIBS="-lm"

dnl Machine specific checks and defines
case "${host_os}" in
darwin*)
  AC_DEFINE(DARWIN, 1, [Define if building for Mac OS X Darwin])
  ;;
mingw*)
  mingw32_support="yes"
  CFLAGS="$CFLAGS -mms-bitfields"
  AC_DEFINE(MINGW32, 1, [Define if using the MinGW32 environment])
  LIBFLUID_LIBS="-ldsound -lwinmm"
  LIBFLUID_CPPFLAGS="-DFLUIDSYNTH_DLL_EXPORTS"
  LIBFLUID_LDFLAGS="-no-undefined"
  FLUID_CPPFLAGS="-DFLUIDSYNTH_NOT_A_DLL"
  ;;
esac

AC_SUBST(LIBFLUID_LIBS)
AC_SUBST(LIBFLUID_CPPFLAGS)
AC_SUBST(LIBFLUID_LDFLAGS)
AC_SUBST(FLUID_CPPFLAGS)

AM_CONDITIONAL(MINGW32_SUPPORT, test "$mingw32_support" == "yes")

AC_ARG_ENABLE(double, AS_HELP_STRING([--enable-double],
	[double floating point for dsp (default=float)]),
  ENABLE_FLOAT_SAMPLES=$enableval,
  ENABLE_FLOAT_SAMPLES=yes)
if test "x$ENABLE_FLOAT_SAMPLES" = "xyes" ; then 
  AC_DEFINE(WITH_FLOAT, 1, [Define to do all DSP in single floating point precision])
fi

AC_ARG_ENABLE(profiling, AS_HELP_STRING([--enable-profiling],
	[profile the dsp code (default=no)]),
  profiling_flag=$enableval,
  profiling_flag=no)
if test "x$profiling_flag" = "xyes" ; then 
  AC_DEFINE(WITH_PROFILING, 1, [Define to profile the DSP code])
fi

AC_ARG_ENABLE(ladspa, AS_HELP_STRING([--enable-ladspa],
	[Include LADSPA effect unit (default=no)]),
  ENABLE_LADSPA=$enableval,
  ENABLE_LADSPA=no)
if test "x$ENABLE_LADSPA" = "xyes" ; then 
   AC_DEFINE(LADSPA, 1, [Include the LADSPA Fx unit])
   dnl LADSPA plugins are loaded as dynamic libraries.
   dnl In this case, -ldl is required for linking.   
   AC_CHECK_LIB(dl, dlopen)
fi


AC_ARG_ENABLE(trap-on-fpe, AS_HELP_STRING([--enable-trap-on-fpe],
	[Enable SIGFPE trap on Floating Point Exceptions (debugging)]),
	ENABLE_TRAPONFPE=$enableval, ENABLE_TRAPONFPE=no)

if test "$ENABLE_TRAPONFPE" = "yes"; then
  AC_DEFINE(TRAP_ON_FPE, 1, [Define to enable SIGFPE assertions])
fi

AC_ARG_ENABLE(fpe-check, AS_HELP_STRING([--enable-fpe-check],
	[Enable Floating Point Exception checks and debug messages]),
	ENABLE_FPECHECK=$enableval, ENABLE_FPECHECK=no)

if test "$ENABLE_FPECHECK" = "yes"; then
  AC_DEFINE(FPE_CHECK, 1, [Define to enable FPE checks])
fi

AC_ARG_ENABLE(debug, AS_HELP_STRING([--enable-debug],
	[enable debugging (default=no)]),
  ENABLE_DEBUG=$enableval)

if test "$ENABLE_DEBUG" = "yes"; then
  CFLAGS="${CFLAGS} ${FCCFLAGS} -g -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wstrict-prototypes -Wno-unused"
  AC_DEFINE(DEBUG, 1, [Define to activate debugging message]) 
else
  CFLAGS="${CFLAGS} ${FCCFLAGS} -O2 -fomit-frame-pointer -funroll-all-loops -finline-functions -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wstrict-prototypes -Wno-unused -Winline"
  AC_DEFINE(DEBUG, 0, [Define to activate debugging message])
fi


dnl 
dnl Check support for all the drivers
dnl


dnl - Sound file support check
dnl AUFILE_SUPPORT=0
dnl 
dnl AC_ARG_ENABLE(aufile-support,
dnl   [  --disable-aufile-support Do not compile support for sound file output],
dnl   enable_aufile_support=no)
dnl 
dnl if test "x${enable_aufile_support}" != "xno"; then
dnl   AUFILE_SUPPORT=1
dnl   AC_DEFINE(AUFILE_SUPPORT, 1, [Define to activate sound output to files])
dnl fi


dnl - Check support for ALSA

AC_ARG_ENABLE(alsa-support, AS_HELP_STRING([--disable-alsa-support],
      [Do not compile ALSA support (default=auto)]),
  enable_alsa_support=$enableval, enable_alsa_support="yes")

if test "x$enable_alsa_support" != "xno"; then
  PKG_CHECK_MODULES(ALSA, alsa >= 0.9.1, ALSA_SUPPORT=1, ALSA_SUPPORT=0)
else
  ALSA_SUPPORT=0
fi

if test "$ALSA_SUPPORT" = "1"; then
  AC_DEFINE(ALSA_SUPPORT, 1, [Define to enable ALSA driver])
fi
AM_CONDITIONAL(ALSA_SUPPORT, test "$ALSA_SUPPORT" = "1")
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LIBS)


dnl - Check support for OSS audio
AC_OSS_AUDIO
AM_CONDITIONAL(OSS_SUPPORT, test "$OSS_SUPPORT" = "1")


dnl - Check support for MidiShare
AC_MIDISHARE


dnl - Check support for JACK
AC_ARG_ENABLE(jack-support, AS_HELP_STRING([--disable-jack-support],
	[disable JACK support (default=auto)]),
  enable_jack=$enableval, enable_jack="yes")

if test "x$enable_jack" != "xno"; then
  PKG_CHECK_MODULES(JACK, jack, JACK_SUPPORT=1, JACK_SUPPORT=0)
else
  JACK_SUPPORT=0
fi

if test "$JACK_SUPPORT" = "1"; then
  AC_DEFINE(JACK_SUPPORT, 1, [Define to enable JACK driver])
fi
AM_CONDITIONAL(JACK_SUPPORT, test "$JACK_SUPPORT" = "1")
AC_SUBST(JACK_CFLAGS)
AC_SUBST(JACK_LIBS)


dnl
dnl - Check support for CoreAudio
dnl
AC_CHECK_HEADER(CoreAudio/AudioHardware.h, COREAUDIO_FOUND="yes",
                COREAUDIO_FOUND="no")
AC_ARG_ENABLE(coreaudio, AS_HELP_STRING([--disable-coreaudio],
	[disable CoreAudio support (default=auto)]),
  enable_coreaudio=$enableval, enable_coreaudio="yes")

COREAUDIO_SUPPORT=0
if test "$enable_coreaudio" = "yes" -a "$COREAUDIO_FOUND" = "yes"; then
  AC_DEFINE(COREAUDIO_SUPPORT, 1, [whether or not we are supporting CoreAudio])
  COREAUDIO_SUPPORT=1
  COREAUDIO_LIBS="-Wl,-framework,CoreAudio"
fi

AM_CONDITIONAL(COREAUDIO_SUPPORT, test "$COREAUDIO_SUPPORT" = "1")
AC_SUBST(COREAUDIO_LIBS)

dnl
dnl Check for readline support (Josh Green 2003-06-10)
dnl
AC_ARG_WITH(readline,
  [  --without-readline      disable readline lib line editing (default=auto)],
  with_readline=$withval, with_readline="yes")

if test "$with_readline" != "no"; then
  AM_PATH_READLINE(FOUND_READLINE=1, FOUND_READLINE=0)
fi

dnl
dnl The following script checks for ncurses support.
dnl I copied and adapted it from DataDisplayDebugger's (DDD)
dnl configure.in, written by Andreas Zeller <zeller@gnu.org>.
dnl

dnl Look for the tgetent() function - either in libtermcap, libcurses
dnl
dnl On FreeBSD systems, `-lmytinfo' is preferred to `-lncurses'.
dnl Reported by Vincenzo Capuano <vcapuano@esoc.esa.de>
dnl
dnl On Linux ELF systems, `-lncurses' is preferred to `-ltermcap'.
dnl Reported by jdassen@wi.leidenuniv.nl (J.H.M. Dassen)
dnl
WITH_READLINE=0
have_termcap=false
if test "${FOUND_READLINE}" = "1"; then
  _termlib="mytinfo ncurses curses termcap terminfo termlib"
  for termlib in ${_termlib}; do
    AC_CHECK_LIB(${termlib}, tgetent,
      [READLINE_LIBS="$READLINE_LIBS -l${termlib}"; have_termcap=true; break])
  done
fi

if test "x${have_termcap}" = "xtrue"; then
  AC_DEFINE(WITH_READLINE, 1, [Define to use the readline library for line editing])
  WITH_READLINE=1
fi

dnl
dnl lash stuff
dnl
AC_ARG_ENABLE(lash, AS_HELP_STRING([--disable-lash],
	[disable LASH support (default=auto)]),
    fluid_enable_lash=$enableval,
    fluid_enable_lash="yes")

if test "$fluid_enable_lash" = "yes"; then
  PKG_CHECK_MODULES(LASH, lash-1.0 >= 0.3, LASH_FOUND="yes", LASH_FOUND="no")
  AC_SUBST(LASH_CFLAGS)
  AC_SUBST(LASH_LIBS)

  if test "$LASH_FOUND" = "yes"; then
     AC_DEFINE(HAVE_LASH, 1, [whether or not we are supporting lash])
  fi
else
  LASH_FOUND="no"
fi
AM_CONDITIONAL(LASH_SUPPORT, test "$LASH_FOUND" = "yes")

dnl
dnl ladcca stuff (depricated by lash and will be removed in the future)
dnl
AC_ARG_ENABLE(ladcca, AS_HELP_STRING([--disable-ladcca],
	[disable LADCCA support (default=auto)]),
    fluid_enable_ladcca=$enableval,
    fluid_enable_ladcca="yes")

if test "$fluid_enable_ladcca" = "yes" -a "$LASH_FOUND" = "no"; then
  PKG_CHECK_MODULES(LADCCA, ladcca-1.0 >= 0.3, LADCCA_FOUND="yes", LADCCA_FOUND="no")
  AC_SUBST(LADCCA_CFLAGS)
  AC_SUBST(LADCCA_LIBS)

  if test "$LADCCA_FOUND" = "yes"; then
     AC_DEFINE(HAVE_LADCCA, 1, [whether or not we are supporting ladcca])
  fi
else
  LADCCA_FOUND="no"
fi
AM_CONDITIONAL(LADCCA_SUPPORT, test "$LADCCA_FOUND" = "yes")



AC_OUTPUT([
	Makefile
	macbuild/Makefile
	sf2/Makefile
	winbuild/Makefile
	winbuild/fluidsynth/Makefile
	winbuild/fluidsynth_dll/Makefile
	winbuild/fluidsynth_lib/Makefile
	src/Makefile
	src/config_win32.h
	doc/Makefile
	include/Makefile
	include/fluidsynth/Makefile
	include/fluidsynth/version.h
	fluidsynth.pc
	fluidsynth.spec])

echo
echo "**************************************************************"
echo "Summary:"

if test "${ALSA_SUPPORT}" = "1"; then
  echo "ALSA:                  yes"
else 
  echo "ALSA:                  no"
fi

if test "${OSS_SUPPORT}" = "1"; then
  echo "OSS:                   yes"
else 
  echo "OSS:                   no"
fi

if test "${MIDISHARE_SUPPORT}" = "1"; then
  echo "MidiShare:             yes"
else
  echo "MidiShare:             no"
fi

if test "${JACK_SUPPORT}" = "1"; then
  echo "JACK:                  yes"
else
  echo "JACK:                  no"
fi

if test "${COREAUDIO_SUPPORT}" = "1"; then
  echo "CoreAudio:             yes"
else
  echo "CoreAudio:             no"
fi

if test "${ENABLE_LADSPA}" = "yes"; then
  echo "LADSPA support:        yes"
else
  echo "LADSPA support:        no"
fi

if test "${LASH_FOUND}" = "yes"; then
  echo "LASH support:          yes"
else
  echo "LASH support:          no"
fi

if test "${LADCCA_FOUND}" = "yes"; then
  echo "LADCCA support:        yes"
else
  echo "LADCCA support:        no"
fi

dnl if test "${AUFILE_SUPPORT}" = "1"; then
dnl   echo "Sound file:            yes"
dnl else
dnl   echo "Sound file:            no"
dnl fi

if test "$WITH_READLINE" = "1"; then
  echo "Readline:              yes"
else
  echo "Readline:              no"
fi

if test "${profiling_flag}" = "yes"; then
  echo "Profiling:             yes"
else
  echo "Profiling:             no"
fi

if test "${ENABLE_DEBUG}" = "yes"; then
  echo "Debug:                 yes"
else
  echo "Debug:                 no"
fi

if test "${ENABLE_TRAPONFPE}" = "yes"; then
  echo "Trap on FPE (debug):   yes"
else
  echo "Trap on FPE (debug):   no"
fi

if test "${ENABLE_FPECHECK}" = "yes"; then
  echo "Check FPE (debug):     yes"
else
  echo "Check FPE (debug):     no"
fi

echo "**************************************************************"
echo
