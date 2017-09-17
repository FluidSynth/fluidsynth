dnl Some additional autoconf macros

AC_DEFUN([AC_MIDISHARE],
[
  AC_ARG_ENABLE(midishare, AS_HELP_STRING([--enable-midishare],
	  [Compile MIDISHARE support (default=auto)]),
     midishare=$enableval, midishare=yes)

  MIDISHARE_SUPPORT=0 

  if test "x$midishare" != "xno"; then

    AC_CHECK_HEADERS(MidiShare.h)

    if test "${ac_cv_header_MidiShare_h}" = "yes"; then

       MIDISHARE_SUPPORT=1 

	midishare_found=yes
 	AC_CHECK_LIB([MidiShare], [MidiOpen],, [midishare_found=no])
 
 	if test "x$midishare_found" = "xyes" ; then
 	    MIDISHARE_SUPPORT=1
 	    AC_DEFINE(MIDISHARE_SUPPORT, 1, [Define to enable MidiShare driver])
	fi
 
 	if test "x$midishare_found" = "xno" ; then
       	    AC_MSG_WARN([ *** Could not find the required MidiShare library])
 	fi dnl  midishare_found = yes test
 
     else
         AC_MSG_WARN([ *** Could not find MidiShare.h, disabling MidiShare driver])
     fi	dnl  midishare.h header test
   fi	dnl  enable_midishare != no?
])

AC_DEFUN([AC_OSS_AUDIO],
[
  AC_ARG_ENABLE(oss-support,
    [  --disable-oss-support   Do not compile OSS support (default=auto)],
    enable_oss_support=$enableval, enable_oss_support="yes")

  OSS_SUPPORT=0

  if test "x$enable_oss_support" != "xno"; then
    AC_CHECK_HEADERS(fcntl.h sys/ioctl.h sys/soundcard.h machine/soundcard.h)
    if test "${ac_cv_header_fcntl_h}" = "yes" && \
     test "${ac_cv_header_sys_ioctl_h}" = "yes"; then
      if test "${ac_cv_header_sys_soundcard_h}" = "yes" || \
       test "${ac_cv_header_machine_soundcard_h}" = "yes"; then
	OSS_SUPPORT=1
	AC_DEFINE(OSS_SUPPORT, 1, [Define to enable OSS driver])
      else
        AC_MSG_WARN([ *** Could not find soundcard.h, disabling OSS driver])
      fi	dnl  soundcard.h header test
    else
      AC_MSG_WARN([ *** Could not find fcntl.h and/or ioctl.h which are required for sound and midi support])
    fi		dnl  fcntl.h & ioctl.h header test
  fi		dnl  enable_oss_support != no?
])

dnl Configure Paths for readline (Josh Green 2003-06-10)
dnl
dnl AM_PATH_READLINE([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for readline, and define READLINE_CFLAGS and
dnl READLINE_LIBS as appropriate.
dnl enables arguments --with-readline-prefix=

AC_DEFUN([AM_PATH_READLINE],
[dnl Save the original CFLAGS, and LIBS
save_CFLAGS="$CFLAGS"
save_LIBS="$LIBS"
readline_found=yes

dnl
dnl Setup configure options
dnl
AC_ARG_WITH(readline-prefix,
  [  --with-readline-prefix=PATH  Path where readline is (optional)],
  [readline_prefix="$withval"], [readline_prefix=""])

AC_MSG_CHECKING(for readline)

dnl Add readline to the LIBS path
READLINE_LIBS="-lreadline"

if test "${readline_prefix}" != "" ; then
  READLINE_LIBS="-L${readline_prefix}/lib $READLINE_LIBS"
  READLINE_CFLAGS="-I${readline_prefix}/include"
else
  READLINE_CFLAGS=""
fi

LIBS="$READLINE_LIBS $LIBS"
CFLAGS="$READLINE_CFLAGS $CFLAGS"

AC_TRY_COMPILE([
#include <stdio.h>
#include <readline/readline.h>
], [
#ifndef readline
   return (1);
#else
   return (0);
#endif
],
  [AC_MSG_RESULT(found.)],
  [AC_MSG_RESULT(not present.)
   readline_found=no]
)

CFLAGS="$save_CFLAGS"
LIBS="$save_LIBS"

if test "x$readline_found" = "xyes" ; then
   ifelse([$1], , :, [$1])
else
   READLINE_CFLAGS=""
   READLINE_LIBS=""
   ifelse([$2], , :, [$2])
fi

dnl That should be it.  Now just export out symbols:
AC_SUBST(READLINE_CFLAGS)
AC_SUBST(READLINE_LIBS)
])
