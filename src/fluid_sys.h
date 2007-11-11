/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */


/**

   This header contains a bunch of (mostly) system and machine
   dependent functions:

   - timers
   - current time in milliseconds and microseconds
   - debug logging
   - profiling
   - memory locking
   - checking for floating point exceptions

 */

#ifndef _FLUID_SYS_H
#define _FLUID_SYS_H

#include "fluidsynth_priv.h"


void fluid_sys_config(void);
void fluid_log_config(void);
void fluid_time_config(void);


/*
 * Utility functions
 */
char *fluid_strtok (char **str, char *delim);


/**

  Additional debugging system, separate from the log system. This
  allows to print selected debug messages of a specific subsystem.

 */

extern unsigned int fluid_debug_flags;

#if DEBUG

enum fluid_debug_level {
  FLUID_DBG_DRIVER = 1
};

int fluid_debug(int level, char * fmt, ...);

#else
#define fluid_debug
#endif


/** fluid_curtime() returns the current time in milliseconds. This time
    should only be used in relative time measurements.  */

/** fluid_utime() returns the time in micro seconds. this time should
    only be used to measure duration (relative times). */

#if defined(WIN32)
#define fluid_curtime()   GetTickCount()

double fluid_utime(void);

#elif defined(MACOS9)
#include <OSUtils.h>
#include <Timer.h>

unsigned int fluid_curtime();
#define fluid_utime()  0.0

#else

unsigned int fluid_curtime(void);
double fluid_utime(void);

#endif



/**
    Timers

 */

/* if the callback function returns 1 the timer will continue; if it
   returns 0 it will stop */
typedef int (*fluid_timer_callback_t)(void* data, unsigned int msec);

typedef struct _fluid_timer_t fluid_timer_t;

fluid_timer_t* new_fluid_timer(int msec, fluid_timer_callback_t callback,
					    void* data, int new_thread, int auto_destroy);

int delete_fluid_timer(fluid_timer_t* timer);
int fluid_timer_join(fluid_timer_t* timer);
int fluid_timer_stop(fluid_timer_t* timer);

/**

    Muteces

*/

#if defined(MACOS9)
typedef int fluid_mutex_t;
#define fluid_mutex_init(_m)      { (_m) = 0; }
#define fluid_mutex_destroy(_m)
#define fluid_mutex_lock(_m)
#define fluid_mutex_unlock(_m)

#elif defined(WIN32)
typedef HANDLE fluid_mutex_t;
#define fluid_mutex_init(_m)      { (_m) = CreateMutex(NULL, 0, NULL); }
#define fluid_mutex_destroy(_m)   if (_m) { CloseHandle(_m); }
#define fluid_mutex_lock(_m)      WaitForSingleObject(_m, INFINITE)
#define fluid_mutex_unlock(_m)    ReleaseMutex(_m)

#else
typedef pthread_mutex_t fluid_mutex_t;
#define fluid_mutex_init(_m)      pthread_mutex_init(&(_m), NULL)
#define fluid_mutex_destroy(_m)   pthread_mutex_destroy(&(_m))
#define fluid_mutex_lock(_m)      pthread_mutex_lock(&(_m))
#define fluid_mutex_unlock(_m)    pthread_mutex_unlock(&(_m))
#endif


/**
     Threads

*/

typedef struct _fluid_thread_t fluid_thread_t;
typedef void (*fluid_thread_func_t)(void* data);

/** When detached, 'join' does not work and the thread destroys itself
    when finished. */
fluid_thread_t* new_fluid_thread(fluid_thread_func_t func, void* data, int detach);
int delete_fluid_thread(fluid_thread_t* thread);
int fluid_thread_join(fluid_thread_t* thread);


/**
     Sockets

*/


/** The function should return 0 if no error occured, non-zero
    otherwise. If the function return non-zero, the socket will be
    closed by the server. */
typedef int (*fluid_server_func_t)(void* data, fluid_socket_t client_socket, char* addr);


fluid_server_socket_t* new_fluid_server_socket(int port, fluid_server_func_t func, void* data);
int delete_fluid_server_socket(fluid_server_socket_t* sock);
int fluid_server_socket_join(fluid_server_socket_t* sock);


/** Create a new client socket. */
fluid_socket_t new_fluid_client_socket(char* host, int port);

/** Delete the client socket. This function should only be called on
    sockets create with 'new_fluid_client_socket'. */
void delete_fluid_client_socket(fluid_socket_t sock);


void fluid_socket_close(fluid_socket_t sock);
fluid_istream_t fluid_socket_get_istream(fluid_socket_t sock);
fluid_ostream_t fluid_socket_get_ostream(fluid_socket_t sock);



/**

    Profiling
 */


/**
    Profile numbers. List all the pieces of code you want to profile
    here. Be sure to add an entry in the fluid_profile_data table in
    fluid_sys.c
*/
enum {
  FLUID_PROF_WRITE_S16,
  FLUID_PROF_ONE_BLOCK,
  FLUID_PROF_ONE_BLOCK_CLEAR,
  FLUID_PROF_ONE_BLOCK_VOICE,
  FLUID_PROF_ONE_BLOCK_VOICES,
  FLUID_PROF_ONE_BLOCK_REVERB,
  FLUID_PROF_ONE_BLOCK_CHORUS,
  FLUID_PROF_VOICE_NOTE,
  FLUID_PROF_VOICE_RELEASE,
  FLUID_PROF_LAST
};


#if WITH_PROFILING

void fluid_profiling_print(void);


/** Profiling data. Keep track of min/avg/max values to execute a
    piece of code. */
typedef struct _fluid_profile_data_t {
  int num;
  char* description;
  double min, max, total;
  unsigned int count;
} fluid_profile_data_t;

extern fluid_profile_data_t fluid_profile_data[];

/** Macro to obtain a time refence used for the profiling */
#define fluid_profile_ref() fluid_utime()

/** Macro to calculate the min/avg/max. Needs a time refence and a
    profile number. */
#define fluid_profile(_num,_ref) { \
  double _now = fluid_utime(); \
  double _delta = _now - _ref; \
  fluid_profile_data[_num].min = _delta < fluid_profile_data[_num].min ? _delta : fluid_profile_data[_num].min; \
  fluid_profile_data[_num].max = _delta > fluid_profile_data[_num].max ? _delta : fluid_profile_data[_num].max; \
  fluid_profile_data[_num].total += _delta; \
  fluid_profile_data[_num].count++; \
  _ref = _now; \
}


#else

/* No profiling */
#define fluid_profiling_print()
#define fluid_profile_ref()  0
#define fluid_profile(_num,_ref)

#endif



/**

    Memory locking

    Memory locking is used to avoid swapping of the large block of
    sample data.
 */

#if HAVE_SYS_MMAN_H
#define fluid_mlock(_p,_n)      mlock(_p, _n)
#define fluid_munlock(_p,_n)    munlock(_p,_n)
#else
#define fluid_mlock(_p,_n)      0
#define fluid_munlock(_p,_n)
#endif


/**

    Floating point exceptions

    fluid_check_fpe() checks for "unnormalized numbers" and other
    exceptions of the floating point processsor.
*/
#ifdef FPE_CHECK
#define fluid_check_fpe(expl) fluid_check_fpe_i386(expl)
#define fluid_clear_fpe() fluid_clear_fpe_i386()
#else
#define fluid_check_fpe(expl)
#define fluid_clear_fpe()
#endif

unsigned int fluid_check_fpe_i386(char * explanation_in_case_of_fpe);
void fluid_clear_fpe_i386(void);

#endif /* _FLUID_SYS_H */
