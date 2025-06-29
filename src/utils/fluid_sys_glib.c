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

#include "fluid_sys.h"


/**
 * Suspend the execution of the current thread for the specified amount of time.
 * @param milliseconds to wait.
 */
void fluid_msleep(unsigned int msecs)
{
    g_usleep(msecs * 1000);
}


/**
 * Get time in microseconds to be used in relative timing operations.
 * @return time in microseconds.
 * Note: When used for profiling we need high precision clock given
 * by g_get_monotonic_time()if available (glib version >= 2.53.3).
 * If glib version is too old and in the case of Windows the function
 * uses high precision performance counter instead of g_getmonotic_time().
 */
double
fluid_utime(void)
{
    double utime;

#if GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION >= 28
    /* use high precision monotonic clock if available (g_monotonic_time().
     * For Windows, if this clock is actually implemented as low prec. clock
     * (i.e. in case glib is too old), high precision performance counter are
     * used instead.
     * see: https://bugzilla.gnome.org/show_bug.cgi?id=783340
     */
#if defined(WITH_PROFILING) &&  defined(_WIN32) &&\
	/* glib < 2.53.3 */\
	(GLIB_MINOR_VERSION <= 53 && (GLIB_MINOR_VERSION < 53 || GLIB_MICRO_VERSION < 3))
    /* use high precision performance counter. */
    static LARGE_INTEGER freq_cache = {0, 0};	/* Performance Frequency */
    LARGE_INTEGER perf_cpt;

    if(! freq_cache.QuadPart)
    {
        QueryPerformanceFrequency(&freq_cache);  /* Frequency value */
    }

    QueryPerformanceCounter(&perf_cpt); /* Counter value */
    utime = perf_cpt.QuadPart * 1000000.0 / freq_cache.QuadPart; /* time in micros */
#else
    utime = g_get_monotonic_time();
#endif
#else
    /* fallback to less precise clock */
    GTimeVal timeval;
    g_get_current_time(&timeval);
    utime = (timeval.tv_sec * 1000000.0 + timeval.tv_usec);
#endif

    return utime;
}


/***************************************************************
 *
 *               Threads
 *
 */

#if OLD_GLIB_THREAD_API

/* Rather than inline this one, we just declare it as a function, to prevent
 * GCC warning about inline failure. */
fluid_cond_t *
new_fluid_cond(void)
{
    if(!g_thread_supported())
    {
        g_thread_init(NULL);
    }

    return g_cond_new();
}

#endif


/**
 * Create a new thread.
 * @param func Function to execute in new thread context
 * @param data User defined data to pass to func
 * @param prio_level Priority level.  If greater than 0 then high priority scheduling will
 *   be used, with the given priority level (used by pthreads only).  0 uses normal scheduling.
 * @param detach If TRUE, 'join' does not work and the thread destroys itself when finished.
 * @return New thread pointer or NULL on error
 */
fluid_thread_t *
new_fluid_thread(const char *name, fluid_thread_func_t func, void *data, int prio_level, int detach)
{
    GThread *thread;
    fluid_thread_info_t *info = NULL;
    GError *err = NULL;

    g_return_val_if_fail(func != NULL, NULL);

#if OLD_GLIB_THREAD_API

    /* Make sure g_thread_init has been called.
     * Probably not a good idea in a shared library,
     * but what can we do *and* remain backwards compatible? */
    if(!g_thread_supported())
    {
        g_thread_init(NULL);
    }

#endif

    if(prio_level > 0)
    {
        info = FLUID_NEW(fluid_thread_info_t);

        if(!info)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return NULL;
        }

        info->func = func;
        info->data = data;
        info->prio_level = prio_level;
#if NEW_GLIB_THREAD_API
        thread = g_thread_try_new(name, fluid_thread_high_prio, info, &err);
#else
        thread = g_thread_create(fluid_thread_high_prio, info, detach == FALSE, &err);
#endif
    }

    else
    {
#if NEW_GLIB_THREAD_API
        thread = g_thread_try_new(name, (GThreadFunc)func, data, &err);
#else
        thread = g_thread_create((GThreadFunc)func, data, detach == FALSE, &err);
#endif
    }

    if(!thread)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create the thread: %s",
                  fluid_gerror_message(err));
        g_clear_error(&err);
        FLUID_FREE(info);
        return NULL;
    }

#if NEW_GLIB_THREAD_API

    if(detach)
    {
        g_thread_unref(thread);    // Release thread reference, if caller wants to detach
    }

#endif

    return thread;
}

/**
 * Frees data associated with a thread (does not actually stop thread).
 * @param thread Thread to free
 */
void
delete_fluid_thread(fluid_thread_t *thread)
{
    /* Threads free themselves when they quit, nothing to do */
}

/**
 * Join a thread (wait for it to terminate).
 * @param thread Thread to join
 * @return FLUID_OK
 */
int
fluid_thread_join(fluid_thread_t *thread)
{
    g_thread_join(thread);
    return FLUID_OK;
}
