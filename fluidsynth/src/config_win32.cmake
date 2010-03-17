#pragma once

#cmakedefine HAVE_IO_H @HAVE_IO_H@

#define DSOUND_SUPPORT 1
#define WINMIDI_SUPPORT 1

#define snprintf _snprintf
#define strcasecmp _stricmp

#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#pragma warning(disable : 4244)
#pragma warning(disable : 4101)
#pragma warning(disable : 4305)
#pragma warning(disable : 4996)

typedef int socklen_t;
