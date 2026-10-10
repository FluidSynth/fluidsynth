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
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */


/*
 * @file fluid_sys_embedded.h
 *
 * This header contains the stubbed OS abstraction for pure embedded use.
 * It does not provide any threading support nor are the atomic operations
 * actually atomic. Time functions are stubs and always return 0.
 */

#ifndef _FLUID_SYS_EMBEDDED_H
#define _FLUID_SYS_EMBEDDED_H

#include "fluidsynth_priv.h"
#include "fluid_stub_functions.h"
#include "fluid_file.h"

#include <assert.h>
#include <stdbool.h>

#define FALSE (0)
#define TRUE (!FALSE)

#ifdef LADSPA
#error "LADSPA is not supported with the embedded OS abstraction"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void *fluid_pointer_t;

/* Endian detection */
#ifdef WORDS_BIGENDIAN
#define FLUID_IS_BIG_ENDIAN       true

#define FLUID_LE32TOH(x)          (((0xFF000000 & (x)) >> 24) | ((0x00FF0000 & (x)) >> 8) | ((0x0000FF00 & (x)) << 8) | ((0x000000FF & (x)) << 24));
#define FLUID_LE16TOH(x)          (((0xFF00 & (x)) >> 8) | ((0x00FF & (x)) << 8))
#else
#define FLUID_IS_BIG_ENDIAN       false

#define FLUID_LE32TOH(x)          (x)
#define FLUID_LE16TOH(x)          (x)
#endif

/*
 * Utility functions
 */

#define fluid_shell_parse_argv  fluid_shell_parse_argv_internal
#define fluid_strfreev          fluid_strfreev_internal

STUB_FUNCTION(fluid_strerror, const char *, "stub", (int error))
STUB_FUNCTION(fluid_setenv, int, -1, (const char *name, const char *value, int overwrite))


/* Time functions */

STUB_FUNCTION_VOID(fluid_msleep, (unsigned int msecs))
STUB_FUNCTION_SILENT(fluid_utime, double, 0, (void))


/* Muteces */

typedef bool fluid_mutex_t;
#define FLUID_MUTEX_INIT          { false }
#define fluid_mutex_init(_m)      (_m = true)
#define fluid_mutex_destroy(_m)   (_m = false)
#define fluid_mutex_lock(_m)      (_m = true)
#define fluid_mutex_unlock(_m)    (_m = false)

/* Recursive lock capable mutex */
typedef int fluid_rec_mutex_t;
#define fluid_rec_mutex_init(_m)      (_m = 0)
#define fluid_rec_mutex_destroy(_m)   (_m = 0)
#define fluid_rec_mutex_lock(_m)      (_m++)
#define fluid_rec_mutex_unlock(_m)    (_m--)

/* Dynamically allocated mutex suitable for fluid_cond_t use */
typedef bool fluid_cond_mutex_t;
#define fluid_cond_mutex_lock(m)        fluid_mutex_lock(*(m))
#define fluid_cond_mutex_unlock(m)      fluid_mutex_unlock(*(m))

static FLUID_INLINE fluid_cond_mutex_t *
new_fluid_cond_mutex(void)
{
    fluid_cond_mutex_t *mutex;
    mutex = FLUID_NEW(fluid_cond_mutex_t);
    fluid_mutex_init(*mutex);
    return (mutex);
}

static FLUID_INLINE void
delete_fluid_cond_mutex(fluid_cond_mutex_t *m)
{
    fluid_return_if_fail(m != NULL);
    fluid_mutex_destroy(*m);
    fluid_free(m);
}

/* Thread condition signaling */
typedef int fluid_cond_t;
#define fluid_cond_signal(cond)         /* nothing */
#define fluid_cond_broadcast(cond)      /* nothing */
#define fluid_cond_wait(cond, mutex)    /* nothing */

static FLUID_INLINE fluid_cond_t *
new_fluid_cond(void)
{
    fluid_cond_t *cond;
    cond = FLUID_NEW(fluid_cond_t);
    return (cond);
}

static FLUID_INLINE void
delete_fluid_cond(fluid_cond_t *cond)
{
    fluid_return_if_fail(cond != NULL);
    fluid_free(cond);
}

/* Thread private data */

typedef void *fluid_private_t;
#define fluid_private_init(_priv)                  memset(&_priv, 0, sizeof (_priv))
#define fluid_private_free(_priv)
#define fluid_private_get(_priv)                   (_priv)
#define fluid_private_set(_priv, _data)            (_priv) = _data


/* Atomic operations */

#define fluid_atomic_int_inc(_pi) (*(_pi))++
#define fluid_atomic_int_get(_pi) *(_pi)
#define fluid_atomic_int_set(_pi, _val) (*(_pi) = _val)

#define fluid_atomic_int_dec_and_test(_pi) \
    _fluid_atomic_int_dec_and_test((fluid_atomic_int_t *)_pi)
#define fluid_atomic_int_compare_and_exchange(_pi, _old, _new) \
    _fluid_atomic_int_compare_and_exchange((fluid_atomic_int_t *)_pi, _old, _new)
#define fluid_atomic_int_add(_pi, _add) \
    _fluid_atomic_int_add((fluid_atomic_int_t *)_pi, _add)
#define fluid_atomic_int_exchange_and_add fluid_atomic_int_add

#define fluid_atomic_pointer_get(_pp)           *(_pp)
#define fluid_atomic_pointer_set(_pp, val)      (*(_pp) = val)

static FLUID_INLINE bool
_fluid_atomic_int_dec_and_test(fluid_atomic_int_t *pi)
{
    return --(*pi) == 0;
}

static FLUID_INLINE bool
_fluid_atomic_int_compare_and_exchange(fluid_atomic_int_t *pi, int old, int _new)
{
    if (*pi != old)
        return false;

    *pi = _new;
    return true;
}

static FLUID_INLINE int
_fluid_atomic_int_add(fluid_atomic_int_t *pi, int add)
{
    int previous = *pi;
    *pi += add;
    return previous;
}

static FLUID_INLINE bool
fluid_atomic_pointer_compare_and_exchange(void **pp, void *old, void *_new)
{
    if (*pp != old)
        return false;

    *pp = _new;
    return true;
}


/* Threads */

/* other thread implementations might change this for their needs */
typedef void *fluid_thread_return_t;
typedef fluid_thread_return_t (*fluid_thread_func_t)(void *data);

/* static return value for thread functions which requires a return value */
#define FLUID_THREAD_RETURN_VALUE (NULL)

typedef int fluid_thread_t;

#define FLUID_THREAD_ID_NULL            NULL                    /* A NULL "ID" value */
#define fluid_thread_id_t               void *                  /* Data type for a thread ID */
#define fluid_thread_get_id()           NULL                    /* Get unique "ID" for current thread */

/* whether or not the implementation can be thread safe at all */
#define FLUID_THREAD_SAFE_CAPABLE 0

STUB_FUNCTION(new_fluid_thread, fluid_thread_t *, NULL, (const char *name, fluid_thread_func_t func, void *data, int prio_level, int detach))
STUB_FUNCTION_VOID_SILENT(delete_fluid_thread, (fluid_thread_t *thread))
STUB_FUNCTION_SILENT(fluid_thread_join, int, FLUID_OK, (fluid_thread_t *thread))
STUB_FUNCTION_VOID_SILENT(fluid_thread_self_set_prio, (int prio_level))


/* File access */
typedef struct {
    #undef st_mtime
    int st_mtime;
} fluid_stat_buf_t;

STUB_FUNCTION(fluid_file_test, bool, true, (const char *path, int flags))
STUB_FUNCTION(fluid_stat, int, -1, (const char *path, fluid_stat_buf_t *buffer))


/* Debug functions */
#define fluid_assert assert

#ifdef __cplusplus
}
#endif
#endif /* _FLUID_SYS_EMBEDDED_H */
