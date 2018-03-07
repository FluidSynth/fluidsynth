/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */


#ifndef _FLUIDSYNTH_PRIV_H
#define _FLUIDSYNTH_PRIV_H

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#if HAVE_MATH_H
#include <math.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if HAVE_STDARG_H
#include <stdarg.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if HAVE_LIMITS_H
#include <limits.h>
#endif

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif

#if HAVE_IO_H
#include <io.h>
#endif

/** Integer types  */
#if HAVE_STDINT_H
#include <stdint.h>
#endif

#include "fluidsynth.h"


/***************************************************************
 *
 *         BASIC TYPES
 */

#if defined(WITH_FLOAT)
typedef float fluid_real_t;
#else
typedef double fluid_real_t;
#endif


typedef int fluid_socket_t;
#define INVALID_SOCKET -1

#if defined(SUPPORTS_VLA)
#  define FLUID_DECLARE_VLA(_type, _name, _len) \
     _type _name[_len]
#else
#  define FLUID_DECLARE_VLA(_type, _name, _len) \
     _type* _name = g_newa(_type, (_len))
#endif

#define TRUE 1
#define FALSE 0


/** Atomic types */
typedef struct {
    volatile int value;
} fluid_atomic_int_t;
typedef struct {
    volatile unsigned value;
} fluid_atomic_uint_t;
typedef struct {
    volatile float value;
} fluid_atomic_float_t;

/***************************************************************
 *
 *       FORWARD DECLARATIONS
 */
typedef struct _fluid_env_data_t fluid_env_data_t;
typedef struct _fluid_adriver_definition_t fluid_adriver_definition_t;
typedef struct _fluid_channel_t fluid_channel_t;
typedef struct _fluid_tuning_t fluid_tuning_t;
typedef struct _fluid_hashtable_t  fluid_hashtable_t;
typedef struct _fluid_client_t fluid_client_t;
typedef struct _fluid_server_socket_t fluid_server_socket_t;
typedef struct _fluid_sample_timer_t fluid_sample_timer_t;
typedef struct _fluid_zone_range_t fluid_zone_range_t;

/***************************************************************
 *
 *                      CONSTANTS
 */

#define FLUID_BUFSIZE                64         /**< FluidSynth internal buffer size (in samples) */
#define FLUID_MAX_EVENTS_PER_BUFSIZE 1024       /**< Maximum queued MIDI events per #FLUID_BUFSIZE */
#define FLUID_MAX_RETURN_EVENTS      1024       /**< Maximum queued synthesis thread return events */
#define FLUID_MAX_EVENT_QUEUES       16         /**< Maximum number of unique threads queuing events */
#define FLUID_DEFAULT_AUDIO_RT_PRIO  60         /**< Default setting for audio.realtime-prio */
#define FLUID_DEFAULT_MIDI_RT_PRIO   50         /**< Default setting for midi.realtime-prio */
#define FLUID_NUM_MOD                64         /**< Maximum number of modulators in a voice */

/***************************************************************
 *
 *                      SYSTEM INTERFACE
 */
typedef FILE*  fluid_file;

#define FLUID_MALLOC(_n)             malloc(_n)
#define FLUID_REALLOC(_p,_n)         realloc(_p,_n)
#define FLUID_NEW(_t)                (_t*)malloc(sizeof(_t))
#define FLUID_ARRAY(_t,_n)           (_t*)malloc((_n)*sizeof(_t))
#define FLUID_FREE(_p)               free(_p)
#define FLUID_FOPEN(_f,_m)           fopen(_f,_m)
#define FLUID_FCLOSE(_f)             fclose(_f)
#define FLUID_FREAD(_p,_s,_n,_f)     fread(_p,_s,_n,_f)
#define FLUID_FSEEK(_f,_n,_set)      fseek(_f,_n,_set)
#define FLUID_FTELL(_f)              ftell(_f)
#define FLUID_MEMCPY(_dst,_src,_n)   memcpy(_dst,_src,_n)
#define FLUID_MEMSET(_s,_c,_n)       memset(_s,_c,_n)
#define FLUID_STRLEN(_s)             strlen(_s)
#define FLUID_STRCMP(_s,_t)          strcmp(_s,_t)
#define FLUID_STRNCMP(_s,_t,_n)      strncmp(_s,_t,_n)
#define FLUID_STRCPY(_dst,_src)      strcpy(_dst,_src)

#define FLUID_STRNCPY(_dst,_src,_n) \
do { strncpy(_dst,_src,_n); \
    (_dst)[(_n)-1]=0; \
}while(0)

#define FLUID_STRCHR(_s,_c)          strchr(_s,_c)
#define FLUID_STRRCHR(_s,_c)         strrchr(_s,_c)

#ifdef strdup
    #define FLUID_STRDUP(s)          strdup(s)
#else
    #define FLUID_STRDUP(s)          FLUID_STRCPY(FLUID_MALLOC(FLUID_STRLEN(s) + 1), s)
#endif

#define FLUID_SPRINTF                sprintf
#define FLUID_FPRINTF                fprintf

#if (defined(WIN32) && _MSC_VER < 1900) || defined(MINGW32)
    /* need to make sure we use a C99 compliant implementation of (v)snprintf(),
     * i.e. not microsofts non compliant extension _snprintf() as it doesnt
     * reliably null-terminates the buffer
     */
    #define FLUID_SNPRINTF           g_snprintf
#else
    #define FLUID_SNPRINTF           snprintf
#endif

#if (defined(WIN32) && _MSC_VER < 1500) || defined(MINGW32)
    #define FLUID_VSNPRINTF          g_vsnprintf
#else
    #define FLUID_VSNPRINTF          vsnprintf
#endif

#if defined(WIN32) && !defined(MINGW32)
    #define FLUID_STRCASECMP         _stricmp
#else
    #define FLUID_STRCASECMP         strcasecmp
#endif

#if defined(WIN32) && !defined(MINGW32)
    #define FLUID_STRNCASECMP         _strnicmp
#else
    #define FLUID_STRNCASECMP         strncasecmp
#endif


#define fluid_clip(_val, _min, _max) \
{ (_val) = ((_val) < (_min))? (_min) : (((_val) > (_max))? (_max) : (_val)); }

#if WITH_FTS
#define FLUID_PRINTF                 post
#define FLUID_FLUSH()
#else
#define FLUID_PRINTF                 printf
#define FLUID_FLUSH()                fflush(stdout)
#endif

#define FLUID_LOG                    fluid_log

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#ifndef M_LN2
#define M_LN2 0.69314718055994530941723212145818
#endif

#ifndef M_LN10
#define M_LN10 2.3025850929940456840179914546844
#endif

#define FLUID_ASSERT(a,b)
#define FLUID_ASSERT_P(a,b)

#define FLUID_LIKELY(x)      __builtin_expect(!!(x), 1)
#define FLUID_UNLIKELY(x)    __builtin_expect(!!(x), 0)

char* fluid_error(void);


/* Internationalization */
#define _(s) s


#endif /* _FLUIDSYNTH_PRIV_H */
