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


/*
 * @file fluid_sys_cpp11.h
 *
 * This header contains the C++11 based OS abstraction.
 */

#ifndef _FLUID_SYS_CPP11_H
#define _FLUID_SYS_CPP11_H

#include "fluidsynth_priv.h"
#include "fluid_stub_functions.h"

#include <stdbool.h>

#define FALSE (0)
#define TRUE (!FALSE)

#ifdef LADSPA
#error "LADSPA is not yet supported with the C++11 OS abstraction"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Endian detection */
#define FLUID_IS_BIG_ENDIAN       false

#define FLUID_LE32TOH(x)          (x)
#define FLUID_LE16TOH(x)          (x)

/*
 * Utility functions
 */

#define fluid_shell_parse_argv  fluid_shell_parse_argv_internal
#define fluid_strfreev          fluid_strfreev_internal

STUB_FUNCTION(fluid_strerror, const char *, "stub", (int error))
STUB_FUNCTION(fluid_setenv, int, -1, (const char *name, const char *value, int overwrite))


/* Time functions */

void fluid_msleep(unsigned int msecs);
double fluid_utime(void);


/* Muteces */

typedef void *fluid_mutex_t;

#define FLUID_MUTEX_INIT        NULL
#define fluid_mutex_init(mutex) _fluid_mutex_init(&(mutex))
void _fluid_mutex_init(fluid_mutex_t *mutex);
void fluid_mutex_destroy(fluid_mutex_t mutex);
#define fluid_mutex_lock(mutex) _fluid_mutex_lock(&(mutex))
void _fluid_mutex_lock(fluid_mutex_t *mutex);
void fluid_mutex_unlock(fluid_mutex_t mutex);

/* Recursive lock capable mutex */
typedef void *fluid_rec_mutex_t;

#define fluid_rec_mutex_init(mutex) _fluid_rec_mutex_init(&(mutex))
void _fluid_rec_mutex_init(fluid_rec_mutex_t *mutex);
void fluid_rec_mutex_destroy(fluid_rec_mutex_t mutex);
void fluid_rec_mutex_lock(fluid_rec_mutex_t mutex);
void fluid_rec_mutex_unlock(fluid_rec_mutex_t mutex);

/* Dynamically allocated mutex suitable for fluid_cond_t use */
typedef void fluid_cond_mutex_t;

void fluid_cond_mutex_lock(fluid_cond_mutex_t *mutex);
void fluid_cond_mutex_unlock(fluid_cond_mutex_t *mutex);
fluid_cond_mutex_t *new_fluid_cond_mutex(void);
void delete_fluid_cond_mutex(fluid_cond_mutex_t *mutex);

/* Thread condition signaling */
typedef void *fluid_cond_t;

void fluid_cond_signal(fluid_cond_t cond);
void fluid_cond_broadcast(fluid_cond_t cond);
void fluid_cond_wait(fluid_cond_t cond, fluid_cond_mutex_t *mutex);
fluid_cond_t new_fluid_cond(void);
void delete_fluid_cond(fluid_cond_t cond);

/* Thread private data */
typedef void *fluid_private_t;

#define fluid_private_init(priv) _fluid_private_init(&(priv))
void _fluid_private_init(fluid_private_t *priv);
void fluid_private_free(fluid_private_t priv);
void *fluid_private_get(fluid_private_t priv);
void fluid_private_set(fluid_private_t priv, void *value);


/* Atomic operations */

#include "fluid_locked_atomics.h"


/* Threads */

typedef void *fluid_pointer_t;

fluid_pointer_t fluid_thread_high_prio(fluid_pointer_t data);

/* other thread implementations might change this for their needs */
typedef void *fluid_thread_return_t;
typedef fluid_thread_return_t (*fluid_thread_func_t)(void *data);

/* static return value for thread functions which requires a return value */
#define FLUID_THREAD_RETURN_VALUE (NULL)

typedef void fluid_thread_t;

/* whether or not the implementation can be thread safe at all */
#define FLUID_THREAD_SAFE_CAPABLE 1

/* File access */
#define FLUID_FILE_TEST_EXISTS      1
#define FLUID_FILE_TEST_IS_REGULAR  2

typedef struct {
    #undef st_mtime
    int st_mtime;
} fluid_stat_buf_t;

bool fluid_file_test(const char *path, int flags);
int fluid_stat(const char *path, fluid_stat_buf_t *buffer);


/* Debug functions */
#define fluid_assert assert

#ifdef __cplusplus
}
#endif
#endif /* _FLUID_SYS_CPP11_H */
