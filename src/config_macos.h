#define VERSION FLUIDSYNTH_VERSION

#define MACOS9
#define MACINTOSH

#define HAVE_STRING_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_MATH_H 1
#define HAVE_STDARG_H 1
#define WORDS_BIGENDIAN 1
#define HAVE_LIMITS_H 1
#define HAVE_FCNTL_H 1

#undef WITH_PROFILING
#define WITHOUT_SERVER 1

/**** define to use the macintosh sound manager driver*/
#define SNDMAN_SUPPORT 1
/**** define to use the portaudio driver */
/* #define PORTAUDIO_SUPPORT 1 */

/**** define to use the MidiShare driver */
/* #define MIDISHARE_SUPPORT 1 */
/* #define MIDISHARE_DRIVER 1 */
/* #define MidiSharePPC_68k */
