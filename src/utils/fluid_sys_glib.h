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
 * @file fluid_sys_glib.h
 *
 * This header contains the GLib based OS abstraction.
 */

#ifndef _FLUID_SYS_GLIB_H
#define _FLUID_SYS_GLIB_H


/** Integer types  */
#if !HAVE_STDINT_H

/* Assume GLIB types */
typedef gint8    int8_t;
typedef guint8   uint8_t;
typedef gint16   int16_t;
typedef guint16  uint16_t;
typedef gint32   int32_t;
typedef guint32  uint32_t;
typedef gint64   int64_t;
typedef guint64  uint64_t;
typedef guintptr uintptr_t;
typedef gintptr  intptr_t;

#endif

#ifdef LADSPA
#include <gmodule.h>
#endif

#include <glib/gstdio.h>

#include "fluid_file.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Endian detection */
#define FLUID_IS_BIG_ENDIAN       (G_BYTE_ORDER == G_BIG_ENDIAN)

#define FLUID_LE32TOH(x)          GINT32_FROM_LE(x)
#define FLUID_LE16TOH(x)          GINT16_FROM_LE(x)

/*
 * Utility functions
 */

#define fluid_shell_parse_argv(command_line, argcp, argvp) g_shell_parse_argv(command_line, argcp, argvp, NULL)
#define fluid_strfreev g_strfreev

#define fluid_strerror g_strerror
#define fluid_setenv g_setenv


// Macros to use for pre-processor if statements to test which Glib thread API we have (pre or post 2.32)
#define NEW_GLIB_THREAD_API   GLIB_CHECK_VERSION(2,32,0)
#define OLD_GLIB_THREAD_API  !GLIB_CHECK_VERSION(2,32,0)

/* Muteces */

#if NEW_GLIB_THREAD_API

/* glib 2.32 and newer */

/* Regular mutex */
typedef GMutex fluid_mutex_t;
#define FLUID_MUTEX_INIT          { 0 }
#define fluid_mutex_init(_m)      g_mutex_init (&(_m))
#define fluid_mutex_destroy(_m)   g_mutex_clear (&(_m))
#define fluid_mutex_lock(_m)      g_mutex_lock(&(_m))
#define fluid_mutex_unlock(_m)    g_mutex_unlock(&(_m))

/* Recursive lock capable mutex */
typedef GRecMutex fluid_rec_mutex_t;
#define fluid_rec_mutex_init(_m)      g_rec_mutex_init(&(_m))
#define fluid_rec_mutex_destroy(_m)   g_rec_mutex_clear(&(_m))
#define fluid_rec_mutex_lock(_m)      g_rec_mutex_lock(&(_m))
#define fluid_rec_mutex_unlock(_m)    g_rec_mutex_unlock(&(_m))

/* Dynamically allocated mutex suitable for fluid_cond_t use */
typedef GMutex    fluid_cond_mutex_t;
#define fluid_cond_mutex_lock(m)        g_mutex_lock(m)
#define fluid_cond_mutex_unlock(m)      g_mutex_unlock(m)

static FLUID_INLINE fluid_cond_mutex_t *
new_fluid_cond_mutex(void)
{
    GMutex *mutex;
    mutex = g_new(GMutex, 1);
    g_mutex_init(mutex);
    return (mutex);
}

static FLUID_INLINE void
delete_fluid_cond_mutex(fluid_cond_mutex_t *m)
{
    fluid_return_if_fail(m != NULL);
    g_mutex_clear(m);
    g_free(m);
}

/* Thread condition signaling */
typedef GCond fluid_cond_t;
#define fluid_cond_signal(cond)         g_cond_signal(cond)
#define fluid_cond_broadcast(cond)      g_cond_broadcast(cond)
#define fluid_cond_wait(cond, mutex)    g_cond_wait(cond, mutex)

static FLUID_INLINE fluid_cond_t *
new_fluid_cond(void)
{
    GCond *cond;
    cond = g_new(GCond, 1);
    g_cond_init(cond);
    return (cond);
}

static FLUID_INLINE void
delete_fluid_cond(fluid_cond_t *cond)
{
    fluid_return_if_fail(cond != NULL);
    g_cond_clear(cond);
    g_free(cond);
}

/* Thread private data */

typedef GPrivate fluid_private_t;
#define fluid_private_init(_priv)                  memset (&_priv, 0, sizeof (_priv))
#define fluid_private_free(_priv)
#define fluid_private_get(_priv)                   g_private_get(&(_priv))
#define fluid_private_set(_priv, _data)            g_private_set(&(_priv), _data)

#else

/* glib prior to 2.32 */

/* Regular mutex */
typedef GStaticMutex fluid_mutex_t;
#define FLUID_MUTEX_INIT          G_STATIC_MUTEX_INIT
#define fluid_mutex_destroy(_m)   g_static_mutex_free(&(_m))
#define fluid_mutex_lock(_m)      g_static_mutex_lock(&(_m))
#define fluid_mutex_unlock(_m)    g_static_mutex_unlock(&(_m))

#define fluid_mutex_init(_m)      do { \
  if (!g_thread_supported ()) g_thread_init (NULL); \
  g_static_mutex_init (&(_m)); \
} while(0)

/* Recursive lock capable mutex */
typedef GStaticRecMutex fluid_rec_mutex_t;
#define fluid_rec_mutex_destroy(_m)   g_static_rec_mutex_free(&(_m))
#define fluid_rec_mutex_lock(_m)      g_static_rec_mutex_lock(&(_m))
#define fluid_rec_mutex_unlock(_m)    g_static_rec_mutex_unlock(&(_m))

#define fluid_rec_mutex_init(_m)      do { \
  if (!g_thread_supported ()) g_thread_init (NULL); \
  g_static_rec_mutex_init (&(_m)); \
} while(0)

/* Dynamically allocated mutex suitable for fluid_cond_t use */
typedef GMutex    fluid_cond_mutex_t;
#define delete_fluid_cond_mutex(m)      g_mutex_free(m)
#define fluid_cond_mutex_lock(m)        g_mutex_lock(m)
#define fluid_cond_mutex_unlock(m)      g_mutex_unlock(m)

static FLUID_INLINE fluid_cond_mutex_t *
new_fluid_cond_mutex(void)
{
    if(!g_thread_supported())
    {
        g_thread_init(NULL);
    }

    return g_mutex_new();
}

/* Thread condition signaling */
typedef GCond fluid_cond_t;
fluid_cond_t *new_fluid_cond(void);
#define delete_fluid_cond(cond)         g_cond_free(cond)
#define fluid_cond_signal(cond)         g_cond_signal(cond)
#define fluid_cond_broadcast(cond)      g_cond_broadcast(cond)
#define fluid_cond_wait(cond, mutex)    g_cond_wait(cond, mutex)

/* Thread private data */
typedef GStaticPrivate fluid_private_t;
#define fluid_private_get(_priv)                   g_static_private_get(&(_priv))
#define fluid_private_set(_priv, _data)            g_static_private_set(&(_priv), _data, NULL)
#define fluid_private_free(_priv)                  g_static_private_free(&(_priv))

#define fluid_private_init(_priv)                  do { \
  if (!g_thread_supported ()) g_thread_init (NULL); \
  g_static_private_init (&(_priv)); \
} while(0)

#endif


/* Atomic operations */

#define fluid_atomic_int_inc(_pi) g_atomic_int_inc(_pi)
#define fluid_atomic_int_get(_pi) g_atomic_int_get(_pi)
#define fluid_atomic_int_set(_pi, _val) g_atomic_int_set(_pi, _val)
#define fluid_atomic_int_dec_and_test(_pi) g_atomic_int_dec_and_test(_pi)
#define fluid_atomic_int_compare_and_exchange(_pi, _old, _new) \
  g_atomic_int_compare_and_exchange(_pi, _old, _new)

#if GLIB_MAJOR_VERSION > 2 || (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION >= 30)
#define fluid_atomic_int_exchange_and_add(_pi, _add) \
  g_atomic_int_add(_pi, _add)
#define fluid_atomic_int_add(_pi, _add) \
  g_atomic_int_add(_pi, _add)
#else
#define fluid_atomic_int_exchange_and_add(_pi, _add) \
  g_atomic_int_exchange_and_add(_pi, _add)
#define fluid_atomic_int_add(_pi, _add) \
  g_atomic_int_exchange_and_add(_pi, _add)
#endif

#define fluid_atomic_pointer_get(_pp)           g_atomic_pointer_get(_pp)
#define fluid_atomic_pointer_set(_pp, val)      g_atomic_pointer_set(_pp, val)
#define fluid_atomic_pointer_compare_and_exchange(_pp, _old, _new) \
  g_atomic_pointer_compare_and_exchange(_pp, _old, _new)


/* Threads */

typedef gpointer fluid_pointer_t;

fluid_pointer_t fluid_thread_high_prio(fluid_pointer_t data);

/* other thread implementations might change this for their needs */
typedef void *fluid_thread_return_t;
typedef fluid_thread_return_t (*fluid_thread_func_t)(void *data);

/* static return value for thread functions which requires a return value */
#define FLUID_THREAD_RETURN_VALUE (NULL)

typedef GThread fluid_thread_t;

#define FLUID_THREAD_ID_NULL            NULL                    /* A NULL "ID" value */
#define fluid_thread_id_t               GThread *               /* Data type for a thread ID */
#define fluid_thread_get_id()           g_thread_self()         /* Get unique "ID" for current thread */

/* whether or not the implementation can be thread safe at all */
#define FLUID_THREAD_SAFE_CAPABLE 1


/* Dynamic Module Loading, currently only used by LADSPA subsystem */
#ifdef LADSPA

typedef GModule fluid_module_t;

#define fluid_module_open(_name)        g_module_open((_name), G_MODULE_BIND_LOCAL)
#define fluid_module_close(_mod)        g_module_close(_mod)
#define fluid_module_error()            g_module_error()
#define fluid_module_name(_mod)         g_module_name(_mod)
#define fluid_module_symbol(_mod, _name, _ptr) g_module_symbol((_mod), (_name), (_ptr))

#endif /* LADSPA */

/* File access */
#define fluid_stat(_filename, _statbuf)   g_stat((_filename), (_statbuf))
#if !GLIB_CHECK_VERSION(2, 26, 0)
    /* GStatBuf has not been introduced yet, manually typedef to what they had at that time:
     * https://github.com/GNOME/glib/blob/e7763678b56e3be073cc55d707a6e92fc2055ee0/glib/gstdio.h#L98-L115
     */
    #if defined(_WIN32) || HAVE_WINDOWS_H // somehow reliably mock G_OS_WIN32??
        // Any effort from our side to reliably mock GStatBuf on Windows is in vain. E.g. glib-2.16 is broken as it uses struct stat rather than struct _stat32 on Win x86.
        // Disable it (the user has been warned by cmake).
        #undef fluid_stat
        #define fluid_stat(_filename, _statbuf)  (-1)
        typedef struct _fluid_stat_buf_t{int st_mtime;} fluid_stat_buf_t;
    #else
        /* posix, OS/2, etc. */
        typedef struct stat fluid_stat_buf_t;
    #endif
#else
typedef GStatBuf fluid_stat_buf_t;
#endif

/* Debug functions */
#define fluid_assert g_assert

#ifdef __cplusplus
}
#endif
#endif /* _FLUID_SYS_GLIB_H */
