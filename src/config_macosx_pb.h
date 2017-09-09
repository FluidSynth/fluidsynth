
#define VERSION  "1.0.x"

#define MACINTOSH

/* define to support DARWIN */
#define DARWIN

#define HAVE_STRING_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_MATH_H 1
#define HAVE_STDARG_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_FCNTL_H 1
#define HAVE_UNISTD_H 1
#define HAVE_LIMITS_H 1
#define HAVE_PTHREAD_H 1

#define WORDS_BIGENDIAN 1

#define DEBUG 1

#undef WITH_PROFILING

#define WITHOUT_SERVER 1
#define COREAUDIO_SUPPORT 1
#define COREMIDI_SUPPORT 1

/* define to support the MidiShare driver */
/*
#define MIDISHARE_SUPPORT 1
#define MIDISHARE_DRIVER 1
#define PORTAUDIO_SUPPORT 1
#define __Types__
*/

typedef int socklen_t
