dnl --------------------------------------------------
dnl configure.in for the If I Were U Synthesizer
dnl --------------------------------------------------

AC_INIT(src/fluidsynth.c)

FLUIDSYNTH_VERSION_MAJOR=1
FLUIDSYNTH_VERSION_MINOR=0
FLUIDSYNTH_VERSION_MICRO=1
FLUIDSYNTH_VERSION=$FLUIDSYNTH_VERSION_MAJOR.$FLUIDSYNTH_VERSION_MINOR.$FLUIDSYNTH_VERSION_MICRO

AC_SUBST(FLUIDSYNTH_VERSION_MAJOR)
AC_SUBST(FLUIDSYNTH_VERSION_MINOR)
AC_SUBST(FLUIDSYNTH_VERSION_MICRO)
AC_SUBST(FLUIDSYNTH_VERSION)

AM_INIT_AUTOMAKE(fluidsynth, $FLUIDSYNTH_VERSION)

dnl Convert to quoted string for version.h substitution
FLUIDSYNTH_VERSION="\"$VERSION\""

LIBFLUIDSYNTH_MAJ=1
LIBFLUIDSYNTH_MIN=0
AC_SUBST(LIBFLUIDSYNTH_MAJ)
AC_SUBST(LIBFLUIDSYNTH_MIN)

AM_CONFIG_HEADER(src/config.h)

dnl Check for programs
AC_PROG_CC
AC_PROG_INSTALL
dnl AC_PROG_RANLIB
AM_PROG_LIBTOOL
AC_PROG_MAKE_SET

dnl Check for libraries
AC_CHECK_LIB(pthread, pthread_create)

dnl Check for header files
AC_HEADER_STDC
AC_CHECK_HEADERS(string.h stdlib.h stdio.h math.h errno.h stdarg.h unistd.h sys/mman.h sys/types.h sys/time.h sys/stat.h fcntl.h sys/socket.h netinet/in.h netinet/tcp.h arpa/inet.h limits.h pthread.h signal.h)

dnl Compiler and machine specs
AC_C_INLINE 
AC_C_BIGENDIAN


ENABLE_FLOAT_SAMPLES=yes
AC_ARG_ENABLE(double,
  [  --enable-double         double floating point for dsp [default=float]],
  ENABLE_FLOAT_SAMPLES=no,
  ENABLE_FLOAT_SAMPLES=yes)
if test "x$ENABLE_FLOAT_SAMPLES" = "xyes" ; then 
  AC_DEFINE(WITH_FLOAT, 1, [Define to do all DSP in single floating point precision])
fi

profiling_flag=no
AC_ARG_ENABLE(profiling,
  [  --enable-profiling      profile the dsp code [default=no]],
  profiling_flag=yes,
  profiling_flag=no)
if test "x$profiling_flag" = "xyes" ; then 
  AC_DEFINE(WITH_PROFILING, 1, [Define to profile the DSP code])
fi

longlong=no
AC_ARG_ENABLE(longlong,
  [  --enable-longlong       use long long integers, where appropriate [default=yes]],
  longlong=yes,
  longlong=no)
if test "x$longlong" = "xyes" ; then 
  AC_DEFINE(USE_LONGLONG, 1, [Define to use long long type, where appropriate])
fi

SSE=no
AC_ARG_ENABLE(SSE,
  [  --enable-SSE            Use the SSE instructions of Pentium3+     [default=no]],
  SSE=yes,
  SSE=no)
if test "x$SSE" = "xyes" ; then 
  AC_DEFINE(ENABLE_SSE, 1, [Use the SSE instructions of Pentium 3 and higher])
fi

ENABLE_LADSPA=no
AC_ARG_ENABLE(ladspa,
  [  --enable-ladspa         Include LADSPA effect unit [default=no]],
  ENABLE_LADSPA=yes,
  ENABLE_LADSPA=no)
if test "x$ENABLE_LADSPA" = "xyes" ; then 
   AC_DEFINE(LADSPA, 1, [Include the LADSPA Fx unit])
   dnl LADSPA plugins are loaded as dynamic libraries.
   dnl In this case, -ldl is required for linking.   
   AC_CHECK_LIB(dl, dlopen)
fi


ENABLE_FUNCTIONCHECK=no
AC_ARG_ENABLE(functioncheck,
  [  --enable-functioncheck  enable profiling using FunctionCheck. Turns on debugging. [default=no]],
  ENABLE_FUNCTIONCHECK=yes,
  ENABLE_FUNCTIONCHECK=no)

FCLDFLAGS=""
FCCFLAGS=""
if test "${ENABLE_FUNCTIONCHECK}" = "yes"; then
  FCCFLAGS=`fc-config --cflags`
  FCLDFLAGS=`fc-config --libs`
fi

AC_SUBST(FCLDFLAGS)


if test "${ENABLE_FUNCTIONCHECK}" = "yes"; then
  ENABLE_DEBUG=yes
else
  ENABLE_DEBUG=no
fi

AC_ARG_ENABLE(debug,
  [  --enable-debug          enable debugging [default=no]],
  ENABLE_DEBUG=yes)


case "$ENABLE_DEBUG" in
no) 
  CFLAGS="${FCCFLAGS} -O2 -fomit-frame-pointer -funroll-all-loops -finline-functions -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wstrict-prototypes -Wno-unused -Winline"
  AC_DEFINE(DEBUG, 0, [Define to activate debugging message])
  ;;

yes)
  if test "${ENABLE_FUNCTIONCHECK}" = "yes"; then
    CFLAGS="${FCCFLAGS} -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wstrict-prototypes -Wno-unused"
  else
    CFLAGS="${FCCFLAGS} -g -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wstrict-prototypes -Wno-unused"
  fi 
  AC_DEFINE(DEBUG, 1, [Define to activate debugging message]) 
  ;;

*)
  CFLAGS="${FCCFLAGS} -O2 -fomit-frame-pointer -funroll-all-loops -finline-functions -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wstrict-prototypes -Wno-unused -Winline"
  AC_DEFINE(DEBUG, 0, [Define to activate debugging message])
  ;;

esac




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


dnl - Sound support check (ALSA/OSS)
AC_SOUND


dnl - Check support for MidiShare
AC_MIDISHARE


dnl - Check support for JACK

dnl PKG_CHECK_MODULES(JACK, jack, HAVE_JACK=true, HAVE_JACK=false)
dnl 
dnl AC_SUBST(JACK_CFLAGS)
dnl AC_SUBST(JACK_LIBS)


dnl - Check support for JACK
AC_JACK



dnl
dnl - Check support for CoreAudio
dnl
fluid_enable_coreaudio="no"
AC_ARG_ENABLE(coreaudio,
[  --enable-coreaudio        enable CoreAudio support],[
  case "$enableval" in
    "yes")
      fluid_enable_coreaudio="yes"
      ;;
    "no")
      fluid_enable_coreaudio="no"
      ;;
    *)
      AC_MSG_ERROR([must use --enable-coreaudio(=yes/no) or --disable-coreaudio])
      ;;
  esac
])

COREAUDIO_SUPPORT=0 
if test "$fluid_enable_coreaudio" = "yes"; then
  AC_CHECK_HEADER(AudioHardware.h, COREAUDIO_FOUND="yes", COREAUDIO_FOUND="no")
  if test "$COREAUDIO_FOUND" = "yes"; then
     AC_DEFINE(COREAUDIO_SUPPORT, 1, [whether or not we are supporting CoreAudio])
     COREAUDIO_SUPPORT=1 
  fi
else
  COREAUDIO_FOUND="no"
fi


dnl
dnl The following scripts checks for readline and ncurses support.
dnl I copied and adapted it from DataDisplayDebugger's (DDD)
dnl configure.in, written by Andreas Zeller <zeller@gnu.org>.
dnl

WITH_READLINE=yes
have_termcap=false

AC_ARG_WITH(readline,
[  --with-readline         Use the readline library for line editing [default=yes]],[
  if test "$withval" = "no"; then
    WITH_READLINE=no
  fi
])

dnl Look for the tgetent() function - either in libtermcap, libcurses
dnl
dnl On FreeBSD systems, `-lmytinfo' is preferred to `-lncurses'.
dnl Reported by Vincenzo Capuano <vcapuano@esoc.esa.de>
dnl
dnl On Linux ELF systems, `-lncurses' is preferred to `-ltermcap'.
dnl Reported by jdassen@wi.leidenuniv.nl (J.H.M. Dassen)
dnl
if test "x${WITH_READLINE}" = "xyes"; then
  _termlib="mytinfo ncurses curses termcap terminfo termlib"
  for termlib in ${_termlib}; do
     AC_CHECK_LIB(${termlib}, tgetent, [LIBREADLINE="-l${termlib}"; have_termcap=true; break])
  done
fi
AC_SUBST(LIBREADLINE)

if test "x${have_termcap}" = "xtrue"; then
  LIBREADLINE="-lreadline $LIBREADLINE"; 
  AC_DEFINE(WITH_READLINE, 1, [Define to use the readline library for line editing])
fi


dnl
dnl ladcca stuff
dnl
fluid_enable_ladcca="yes"
AC_ARG_ENABLE(ladcca,
[  --disable-ladcca        disable LADCCA support],[
  case "$enableval" in
    "yes")
      ;;
    "no")
      fluid_enable_ladcca="no"
      ;;
    *)
      AC_MSG_ERROR([must use --enable-ladcca(=yes/no) or --disable-ladcca])
      ;;
  esac
])

if test "$fluid_enable_ladcca" = "yes"; then
  PKG_CHECK_MODULES(LADCCA, ladcca-1.0 >= 0.3, LADCCA_FOUND="yes", LADCCA_FOUND="no")
  AC_SUBST(LADCCA_CFLAGS)
  AC_SUBST(LADCCA_LIBS)

  if test "$LADCCA_FOUND" = "yes"; then
     AC_DEFINE(HAVE_LADCCA, 1, [whether or not we are supporting ladcca])
  fi
else
  LADCCA_FOUND="no"
fi
AM_CONDITIONAL(LADCCA, test "$LADCCA_FOUND" = "yes")





AC_OUTPUT(Makefile src/Makefile doc/Makefile include/Makefile include/fluidsynth/Makefile include/fluidsynth/version.h fluidsynth.pc)


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

dnl if test "${AUFILE_SUPPORT}" = "1"; then
dnl   echo "Sound file:            yes"
dnl else
dnl   echo "Sound file:            no"
dnl fi

if test "x${WITH_READLINE}" = "xyes"; then
  echo "Readline:              yes"
else
  echo "Readline:              no"
fi

if test "${LADCCA_FOUND}" = "yes"; then
  echo "LADCCA support:        yes"
else
  echo "LADCCA support:        no"
fi

if test "${ENABLE_DEBUG}" = "yes"; then
  echo "Debug:                 yes"
else
  echo "Debug:                 no"
fi

if test "${profiling_flag}" = "yes"; then
  echo "Profiling:             yes"
else
  echo "Profiling:             no"
fi

dnl if test "${ENABLE_FUNCTIONCHECK}" = "yes"; then
dnl   echo "FunctionCheck:         yes"
dnl else
dnl   echo "FunctionCheck:         no"
dnl fi

if test "${ENABLE_LADSPA}" = "yes"; then
  echo "LADSPA support:        yes"
else
  echo "LADSPA support:        no"
fi



if test "${longlong}" = "yes"; then
  echo "use long long:         yes"
else
  echo "use long long:         no"
fi

if test "${SSE}" = "yes"; then
  echo "Pentium 3+ SSE:        yes"
else
  echo "Pentium 3+ SSE:        no"
fi

echo "**************************************************************"
echo
