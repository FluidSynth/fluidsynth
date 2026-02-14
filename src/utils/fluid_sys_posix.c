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

#include <pthread.h>

#include <utils/fluid_sys.h>
#include <utils/fluid_sys_posix.h>

#ifdef HAVE_STDATOMIC_H

#include <stdatomic.h>

// we want these in .c file, because otherwise it could be used by C++ code

#define exchange_acquire(ptr, _new) \
  atomic_exchange_explicit((atomic_uint *) (ptr), (_new), __ATOMIC_ACQUIRE)

#define exchange_release(ptr, _new) \
  atomic_exchange_explicit((atomic_uint *) (ptr), (_new), __ATOMIC_RELEASE)

#else

#define exchange_acquire(ptr, _new) \
  __atomic_exchange_4((ptr), (_new), __ATOMIC_ACQUIRE)

#define exchange_release(ptr, _new) \
  __atomic_exchange_4((ptr), (_new), __ATOMIC_RELEASE)

#endif


static void* set_prio_and_continue(void* arg) {
    fluid_thread_info_t* tinfo;
    tinfo = (fluid_thread_info_t*)arg;
    fluid_thread_self_set_prio(tinfo->prio_level);
    tinfo->func(tinfo->data);
    FLUID_FREE(tinfo);
    return 0;
}

FLUID_INLINE fluid_thread_t *
new_fluid_thread(const char *name, fluid_thread_func_t func, void *data, int prio_level, int detach)
{
    int create_result;
    void* actual_data;
    fluid_thread_info_t* tinfo = 0;
    pthread_attr_t attr;
    pthread_t *thread;

    thread = FLUID_NEW(pthread_t);
    if (thread) {
        fluid_thread_func_t actual_func = func;
        actual_data = data;
        if (prio_level > 0) {
            tinfo = FLUID_NEW(fluid_thread_info_t);
            if (tinfo == 0) {
                FLUID_LOG(FLUID_ERR, "out of memory");
                free(thread);
                return 0;
            }
            tinfo->prio_level = prio_level;
            tinfo->func = func;
            tinfo->data = data;
            actual_func = set_prio_and_continue;
            actual_data = tinfo;
        }
        if (pthread_attr_init(&attr) == 0) {
#ifdef HAVE_PTHREAD_ATTR_SETINHERITSCHED
            pthread_attr_setinheritsched (&attr, PTHREAD_INHERIT_SCHED);
#endif
            create_result = pthread_create(thread, &attr, actual_func, actual_data);

            if (detach) {
                pthread_detach(*thread);
            }

            pthread_attr_destroy(&attr);

            if (create_result == 0) {
                return thread;
            }
            FLUID_LOG(FLUID_ERR, "pthread_create failed");
        }
        else {
            FLUID_LOG(FLUID_ERR, "pthread_attr_init failed");
        }

        free(tinfo);
        free(thread);
    }
    return 0;
}


/* futex-related code adapted from glib */

#if defined(HAVE_FUTEX) || defined(HAVE_FUTEX_TIME64)
static inline void slow_path_lock(fluid_mutex_t* m) { fluid_futex_simple(&m->state, (size_t) FUTEX_WAIT_PRIVATE, FLUID_MUTEX_STATE_CONTENDED, NULL); }
static inline void slow_path_unlock(fluid_mutex_t* m) { fluid_futex_simple(&m->state, (size_t) FUTEX_WAKE_PRIVATE, (size_t) 1, NULL); }
#else
static inline void slow_path_lock(fluid_mutex_t* m) { pthread_mutex_lock(&m->slow_path_mutex); }
static inline void slow_path_unlock(fluid_mutex_t* m) { pthread_mutex_unlock(&m->slow_path_mutex); }
#endif

void _fluid_light_mutex_lock_slowpath(fluid_mutex_t* m) {
  /* Set to contended.  If it was empty before then we
   * just acquired the lock.
   *
   * Otherwise, sleep for as long as the contended state remains...
   */
  while (exchange_acquire (&m->state, FLUID_MUTEX_STATE_CONTENDED) != FLUID_MUTEX_STATE_EMPTY)
    {
      slow_path_lock(m);
    }
}

void _fluid_light_mutex_unlock_slowpath(fluid_mutex_t* m, unsigned int prev)
{
  /* We seem to get better code for the uncontended case by splitting
   * this out...
   */
  if FLUID_UNLIKELY(prev == FLUID_MUTEX_STATE_EMPTY)
    {
      FLUID_LOG(FLUID_PANIC, "Attempt to unlock mutex that was not locked\n");
      abort();
    }

  slow_path_unlock(m);
}

void _fluid_mutex_unlock_by_pointer(fluid_mutex_t* m)
{
  unsigned int prev;

  prev = exchange_release (&m->state, FLUID_MUTEX_STATE_EMPTY);

  /* 1->0 and we're done.  Anything else and we need to signal... */
  if FLUID_UNLIKELY(prev != FLUID_MUTEX_STATE_OWNED)
    _fluid_light_mutex_unlock_slowpath(m, prev);
}
