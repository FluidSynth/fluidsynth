#define MACINTOSH

#define HAVE_STRING_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_MATH_H 1
#define HAVE_STDARG_H 1
#define WORDS_BIGENDIAN 1

#undef WITH_PROFILING

/* define to support the MidiShare driver */
#define MIDISHARE_SUPPORT 1
#define MIDISHARE_DRIVER 1
#define PORTAUDIO_SUPPORT 1
#define PORTMIDI_SUPPORT 1
#define __Types__

/* define to support DARWIN */
#define DARWIN

typedef int socklen_t
