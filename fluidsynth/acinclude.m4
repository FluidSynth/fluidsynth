

AC_DEFUN(AC_MIDISHARE,
[
  midishare=no

  AC_ARG_ENABLE(midishare,
    [  --enable-midishare      Compile MIDISHARE support],
     midishare=$enableval)

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


AC_DEFUN(AC_JACK,
[

  jack_support=no

  AC_ARG_ENABLE(jack-support,
    [  --enable-jack-support   Compile JACK support],
    [jack_support=$enableval])


  JACK_SUPPORT=0 

  if test "x$jack_support" != "xno"; then

    AC_CHECK_HEADERS(jack/jack.h)

    if test "${ac_cv_header_jack_jack_h}" = "yes"; then

    dnl jack-0.6.? and above are dependent on librt.
    rt_lib_arg=-lrt
 	AC_CHECK_LIB([rt], [shm_open],, [rt_lib_arg=])

 	if test "x$rt_lib_arg" = "x" ; then
       	    AC_MSG_WARN([ *** Could not find the required rt library.  Newer versions of JACK depend on it])
 	fi
 
	jack_found=yes
 	AC_CHECK_LIB([jack], [jack_client_new],, [jack_found=no], $rt_lib_arg)
 
 	if test "x$jack_found" = "xyes" ; then
 	    JACK_SUPPORT=1
 	    AC_DEFINE(JACK_SUPPORT, 1, [Define to enable JACK driver])
	fi
 
 	if test "x$jack_found" = "xno" ; then
       	    AC_MSG_WARN([ *** Could not find the required JACK library])
 	fi dnl  jack_found = yes test
 
     else
         AC_MSG_WARN([ *** Could not find jack.h, disabling JACK driver])
     fi	dnl  jack.h header test
   fi	dnl  enable_jack_support != no?
])


dnl Copied from Josh Green's Smurf SoundFont Editor
dnl   Peter Hanappe, 21/05/2001
dnl
dnl - AC_SOUND - Determine sound compilability
dnl - Josh Green  08/08/99

AC_DEFUN(AC_SOUND,
[
  AC_ARG_ENABLE(oss-support,
    [  --disable-oss-support   Do not compile OSS support],
    enable_oss_support=no)

  AC_ARG_ENABLE(alsa-support,
    [  --disable-alsa-support  Do not compile ALSA support],
    enable_alsa_support=no)

  ALSA_SUPPORT=0
  OSS_SUPPORT=0

  AM_PATH_ALSA(0.9.0, [
    if test "x$enable_alsa_support" != "xno"; then
      ALSA_SUPPORT=1
      AC_DEFINE(ALSA_SUPPORT, 1, [Define to enable ALSA driver])
      LIBS="${LIBS} ${ALSA_LIBS}"
      CFLAGS="${CFLAGS} ${ALSA_CFLAGS}"
      COMPOPSTR="ALSA ${COMPOPSTR}"
    fi
  ])

  if test "x$enable_oss_support" != "xno"; then
    AC_CHECK_HEADERS(fcntl.h sys/ioctl.h sys/soundcard.h machine/soundcard.h)
    if test "${ac_cv_header_fcntl_h}" = "yes" && \
     test "${ac_cv_header_sys_ioctl_h}" = "yes"; then
      if test "${ac_cv_header_sys_soundcard_h}" = "yes" || \
       test "${ac_cv_header_machine_soundcard_h}" = "yes"; then
	OSS_SUPPORT=1
	AC_DEFINE(OSS_SUPPORT, 1, [Define to enable OSS driver])
	COMPOPSTR="OSS ${COMPOPSTR}"
      else
        AC_MSG_WARN([ *** Could not find soundcard.h, disabling OSS driver])
      fi	dnl  soundcard.h header test
    else
      AC_MSG_WARN([ *** Could not find fcntl.h and/or ioctl.h which are required for sound and midi support])
    fi		dnl  fcntl.h & ioctl.h header test
  fi		dnl  enable_oss_support != no?
])

dnl Josh Green ripped and modified from alsa-utils-0.9beta1 Feb 25 2001
dnl
dnl Configure Paths for Alsa
dnl Some modifications by Richard Boulton <richard-alsa@tartarus.org>
dnl Christopher Lansdown <lansdoct@cs.alfred.edu>
dnl Jaroslav Kysela <perex@suse.cz>
dnl Last modification: 07/01/2001
dnl AM_PATH_ALSA([MINIMUM-VERSION [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for libasound, and define ALSA_CFLAGS and ALSA_LIBS as appropriate.
dnl enables arguments --with-alsa-prefix=
dnl                   --with-alsa-enc-prefix=
dnl                   --disable-alsatest  (this has no effect, as yet)
dnl
AC_DEFUN(AM_PATH_ALSA,
[dnl Save the original CFLAGS, LDFLAGS, and LIBS
alsa_save_CFLAGS="$CFLAGS"
alsa_save_LDFLAGS="$LDFLAGS"
alsa_save_LIBS="$LIBS"
alsa_found=yes

dnl
dnl Get the cflags and libraries for alsa
dnl
AC_ARG_WITH(alsa-prefix,
[  --with-alsa-prefix=PFX  Prefix where Alsa library is installed(optional)],
[alsa_prefix="$withval"], [alsa_prefix=""])

AC_ARG_WITH(alsa-inc-prefix,
[  --with-alsa-inc-prefix=PFX  Prefix where include libraries are (optional)],
[alsa_inc_prefix="$withval"], [alsa_inc_prefix=""])

dnl FIXME: this is not yet implemented
AC_ARG_ENABLE(alsatest,
[  --disable-alsatest      Do not try to compile and run a test Alsa program],
[enable_alsatest=no],
[enable_alsatest=yes])

dnl Add any special include directories
AC_MSG_CHECKING(for ALSA CFLAGS)
if test "$alsa_inc_prefix" != "" ; then
	ALSA_CFLAGS="$ALSA_CFLAGS -I$alsa_inc_prefix"
	CFLAGS="$CFLAGS -I$alsa_inc_prefix"
fi
AC_MSG_RESULT($ALSA_CFLAGS)

dnl add any special lib dirs
AC_MSG_CHECKING(for ALSA LDFLAGS)
if test "$alsa_prefix" != "" ; then
	ALSA_LIBS="$ALSA_LIBS -L$alsa_prefix"
	LDFLAGS="$LDFLAGS $ALSA_LIBS"
fi

dnl add the alsa library
ALSA_LIBS="$ALSA_LIBS -lasound -lm -ldl"
LIBS="$ALSA_LIBS $LIBS"
AC_MSG_RESULT($ALSA_LIBS)

dnl Check for a working version of libasound that is of the right version.
min_alsa_version=ifelse([$1], ,0.1.1,$1)
AC_MSG_CHECKING(for libasound headers version >= $min_alsa_version)
no_alsa=""
    alsa_min_major_version=`echo $min_alsa_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    alsa_min_minor_version=`echo $min_alsa_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    alsa_min_micro_version=`echo $min_alsa_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

AC_LANG_SAVE
AC_LANG_C
AC_TRY_COMPILE([
#include <alsa/asoundlib.h>
], [
int main(void)
{
/* ensure backward compatibility */
#if !defined(SND_LIB_MAJOR) && defined(SOUNDLIB_VERSION_MAJOR)
#define SND_LIB_MAJOR SOUNDLIB_VERSION_MAJOR
#endif
#if !defined(SND_LIB_MINOR) && defined(SOUNDLIB_VERSION_MINOR)
#define SND_LIB_MINOR SOUNDLIB_VERSION_MINOR
#endif
#if !defined(SND_LIB_SUBMINOR) && defined(SOUNDLIB_VERSION_SUBMINOR)
#define SND_LIB_SUBMINOR SOUNDLIB_VERSION_SUBMINOR
#endif

#  if(SND_LIB_MAJOR > $alsa_min_major_version)
  exit(0);
#  else
#    if(SND_LIB_MAJOR < $alsa_min_major_version)
#       error not present
#    endif

#   if(SND_LIB_MINOR > $alsa_min_minor_version)
  exit(0);
#   else
#     if(SND_LIB_MINOR < $alsa_min_minor_version)
#          error not present
#      endif

#      if(SND_LIB_SUBMINOR < $alsa_min_micro_version)
#        error not present
#      endif
#    endif
#  endif
exit(0);
}
],
  [AC_MSG_RESULT(found.)],
  [AC_MSG_RESULT(not present.)
   alsa_found=no]
)
AC_LANG_RESTORE

dnl Now that we know that we have the right version, let's see if we have the library and not just the headers.
AC_CHECK_LIB([asound], [snd_seq_open],, [alsa_found=no])

dnl Restore variables
CFLAGS="$alsa_save_CFLAGS"
LDFLAGS="$alsa_save_LDFLAGS"
LIBS="$alsa_save_LIBS"

if test "x$alsa_found" = "xyes" ; then
   ifelse([$2], , :, [$2])
fi
if test "x$alsa_found" = "xno" ; then
   ifelse([$3], , :, [$3])
   ALSA_CFLAGS=""
   ALSA_LIBS=""
fi

dnl That should be it.  Now just export out symbols:
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LIBS)
])


dnl Configure Paths for readline (Josh Green 2003-06-10)
dnl
dnl AM_PATH_READLINE([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for readline, and define READLINE_CFLAGS and
dnl READLINE_LIBS as appropriate.
dnl enables arguments --with-readline-prefix=

AC_DEFUN(AM_PATH_READLINE,
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
int main(void)
{
#ifndef readline
   return (1);
#else
   return (0);
#endif
}
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
