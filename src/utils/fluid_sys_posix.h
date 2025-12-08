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


#ifndef _FLUID_SYS_PTHREAD_H
#define _FLUID_SYS_PTHREAD_H

#include "fluidsynth_priv.h"

#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/stat.h>

/* Futex-related code is adapted from glib */

#if defined(HAVE_FUTEX) || defined(HAVE_FUTEX_TIME64)
#include <errno.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef FUTEX_WAIT_PRIVATE
#define FUTEX_WAIT_PRIVATE FUTEX_WAIT
#define FUTEX_WAKE_PRIVATE FUTEX_WAKE
#endif
#endif /* defined(HAVE_FUTEX) || defined(HAVE_FUTEX_TIME64) */

#define FALSE (0)
#define TRUE (!FALSE)

#ifdef LADSPA
#error "LADSPA is not supported with posix OSAL abstraction"
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
#define fluid_strerror          strerror
#define fluid_setenv            setenv

/* Time functions */

static FLUID_INLINE void fluid_msleep(unsigned int msecs) {
    usleep((msecs)*1000);
}

static FLUID_INLINE double fluid_utime(void) {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
    return ((double)tp.tv_sec)*1000000.0 + ((double)tp.tv_nsec)/1000.0;
}

/* Atomic operations, adapted from glib */

#if __has_attribute(__unused__)
#define _UNUSED_ATTRIBUTE __attribute__ ((__unused__))
#else
#define _UNUSED_ATTRIBUTE
#endif

#define _BASIC_STATIC_ASSERT(expr) typedef char _GStaticAssertCompileTimeAssertion_ ## __LINE__ [(expr) ? 1 : -1] _UNUSED_ATTRIBUTE
typedef void* _basic_pointer_t;
typedef unsigned long unsigned_integer_of_pointer_size_t;

_BASIC_STATIC_ASSERT(sizeof(_basic_pointer_t) == sizeof(unsigned_integer_of_pointer_size_t));

/* We prefer the new C11-style atomic extension of GCC if available */
#if defined(__ATOMIC_SEQ_CST)

#define fluid_atomic_int_get(atomic) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    int gaig_temp;                                                          \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                  \
    __atomic_load ((int *)(atomic), &gaig_temp, __ATOMIC_SEQ_CST);          \
    (int) gaig_temp;                                                        \
  }))
#define fluid_atomic_int_set(atomic, newval) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    int gais_temp = (int) (newval);                                        \
    (void) (0 ? *(atomic) ^ (newval) : 1);                                   \
    __atomic_store ((int *)(atomic), &gais_temp, __ATOMIC_SEQ_CST);         \
  }))

#define fluid_atomic_pointer_get(atomic)                                       \
  (({                                                     \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));               \
    __typeof__ (*(atomic)) gapg_temp_newval;                              \
    __typeof__ ((atomic)) gapg_temp_atomic = (atomic);                    \
    __atomic_load (gapg_temp_atomic, &gapg_temp_newval, __ATOMIC_SEQ_CST); \
    gapg_temp_newval;                                                      \
  }))
#define fluid_atomic_pointer_set(atomic, newval)                                \
  (({                                                      \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                \
    __typeof__ ((atomic)) gaps_temp_atomic = (atomic);                     \
    __typeof__ (*(atomic)) gaps_temp_newval = (newval);                    \
    (void) (0 ? (_basic_pointer_t) * (atomic) : NULL);                              \
    __atomic_store (gaps_temp_atomic, &gaps_temp_newval, __ATOMIC_SEQ_CST); \
  }))

#define fluid_atomic_int_inc(atomic) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                  \
    (void) __atomic_fetch_add ((atomic), 1, __ATOMIC_SEQ_CST);               \
  }))
#define fluid_atomic_int_dec_and_test(atomic) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                  \
    __atomic_fetch_sub ((atomic), 1, __ATOMIC_SEQ_CST) == 1;                 \
  }))
/* See comments below about equivalent fluid_atomic_pointer_compare_and_exchange()
 * shenanigans for type-safety when compiling in C++ mode. */
#define fluid_atomic_int_compare_and_exchange(atomic, oldval, newval) \
  (({                                                       \
    __typeof__ (*(atomic)) gaicae_oldval = (oldval);                        \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (newval) ^ (oldval) : 1);                        \
    __atomic_compare_exchange_n ((atomic), &gaicae_oldval, (newval), FALSE, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ? TRUE : FALSE; \
  }))
#define fluid_atomic_int_compare_and_exchange_full(atomic, oldval, newval, preval) \
  (({                                                         \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                       \
    _BASIC_STATIC_ASSERT (sizeof *(preval) == sizeof (int));                       \
    (void) (0 ? *(atomic) ^ (newval) ^ (oldval) ^ *(preval) : 1);              \
    *(preval) = (oldval);                                                      \
    __atomic_compare_exchange_n ((atomic), (preval), (newval), FALSE,          \
                                 __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)           \
                                 ? TRUE : FALSE;                               \
  }))
#define fluid_atomic_int_exchange(atomic, newval) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (newval) : 1);                                   \
    (int) __atomic_exchange_n ((atomic), (newval), __ATOMIC_SEQ_CST);       \
  }))
#define fluid_atomic_int_add(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (val) : 1);                                      \
    (int) __atomic_fetch_add ((atomic), (val), __ATOMIC_SEQ_CST);           \
  }))
#define fluid_atomic_int_and(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (val) : 1);                                      \
    (unsigned int) __atomic_fetch_and ((atomic), (val), __ATOMIC_SEQ_CST);          \
  }))
#define fluid_atomic_int_or(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (val) : 1);                                      \
    (unsigned int) __atomic_fetch_or ((atomic), (val), __ATOMIC_SEQ_CST);           \
  }))
#define fluid_atomic_int_xor(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (val) : 1);                                      \
    (unsigned int) __atomic_fetch_xor ((atomic), (val), __ATOMIC_SEQ_CST);          \
  }))
#define fluid_atomic_pointer_compare_and_exchange(atomic, oldval, newval) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof (oldval) == sizeof (_basic_pointer_t));                  \
    _basic_pointer_t gapcae_oldval = (_basic_pointer_t)(oldval);                             \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    __atomic_compare_exchange_n ((atomic), (void *) (&(gapcae_oldval)), (newval), FALSE, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ? TRUE : FALSE; \
  }))
#define fluid_atomic_pointer_compare_and_exchange_full(atomic, oldval, newval, preval) \
  (({                                                             \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                       \
    _BASIC_STATIC_ASSERT (sizeof *(preval) == sizeof (_basic_pointer_t));                       \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                      \
    (void) (0 ? (_basic_pointer_t) *(preval) : NULL);                                      \
    *(preval) = (oldval);                                                          \
    __atomic_compare_exchange_n ((atomic), (preval), (newval), FALSE,              \
                                 __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ?             \
                                 TRUE : FALSE;                                     \
  }))
#define fluid_atomic_pointer_exchange(atomic, newval) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    (_basic_pointer_t) __atomic_exchange_n ((atomic), (newval), __ATOMIC_SEQ_CST);   \
  }))
#define fluid_atomic_pointer_add(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    (void) (0 ? (val) ^ (val) : 1);                                          \
    (gintptr) __atomic_fetch_add ((atomic), (val), __ATOMIC_SEQ_CST);        \
  }))
#define fluid_atomic_pointer_and(atomic, val) \
  (({                                                       \
    unsigned_integer_of_pointer_size_t *gapa_atomic = (unsigned_integer_of_pointer_size_t *) (atomic);                           \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (unsigned_integer_of_pointer_size_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    (void) (0 ? (val) ^ (val) : 1);                                          \
    (unsigned_integer_of_pointer_size_t) __atomic_fetch_and (gapa_atomic, (val), __ATOMIC_SEQ_CST);    \
  }))
#define fluid_atomic_pointer_or(atomic, val) \
  (({                                                       \
    unsigned_integer_of_pointer_size_t *gapo_atomic = (unsigned_integer_of_pointer_size_t *) (atomic);                           \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (unsigned_integer_of_pointer_size_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    (void) (0 ? (val) ^ (val) : 1);                                          \
    (unsigned_integer_of_pointer_size_t) __atomic_fetch_or (gapo_atomic, (val), __ATOMIC_SEQ_CST);     \
  }))
#define fluid_atomic_pointer_xor(atomic, val) \
  (({                                                       \
    unsigned_integer_of_pointer_size_t *gapx_atomic = (unsigned_integer_of_pointer_size_t *) (atomic);                           \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (unsigned_integer_of_pointer_size_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    (void) (0 ? (val) ^ (val) : 1);                                          \
    (unsigned_integer_of_pointer_size_t) __atomic_fetch_xor (gapx_atomic, (val), __ATOMIC_SEQ_CST);    \
  }))

#else /* defined(__ATOMIC_SEQ_CST) */

/* We want to achieve __ATOMIC_SEQ_CST semantics here. See
 * https://en.cppreference.com/w/c/atomic/memory_order#Constants. For load
 * operations, that means performing an *acquire*:
 * > A load operation with this memory order performs the acquire operation on
 * > the affected memory location: no reads or writes in the current thread can
 * > be reordered before this load. All writes in other threads that release
 * > the same atomic variable are visible in the current thread.
 *
 * “no reads or writes in the current thread can be reordered before this load”
 * is implemented using a compiler barrier (a no-op `__asm__` section) to
 * prevent instruction reordering. Writes in other threads are synchronised
 * using `__sync_synchronize()`. It’s unclear from the GCC documentation whether
 * `__sync_synchronize()` acts as a compiler barrier, hence our explicit use of
 * one.
 *
 * For store operations, `__ATOMIC_SEQ_CST` means performing a *release*:
 * > A store operation with this memory order performs the release operation:
 * > no reads or writes in the current thread can be reordered after this store.
 * > All writes in the current thread are visible in other threads that acquire
 * > the same atomic variable (see Release-Acquire ordering below) and writes
 * > that carry a dependency into the atomic variable become visible in other
 * > threads that consume the same atomic (see Release-Consume ordering below).
 *
 * “no reads or writes in the current thread can be reordered after this store”
 * is implemented using a compiler barrier to prevent instruction reordering.
 * “All writes in the current thread are visible in other threads” is implemented
 * using `__sync_synchronize()`; similarly for “writes that carry a dependency”.
 */
#define fluid_atomic_int_get(atomic) \
  (({                                                       \
    int gaig_result;                                                        \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                  \
    gaig_result = (int) *(atomic);                                          \
    __sync_synchronize ();                                                   \
    __asm__ __volatile__ ("" : : : "memory");                                \
    gaig_result;                                                             \
  }))
#define fluid_atomic_int_set(atomic, newval) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (newval) : 1);                                   \
    __sync_synchronize ();                                                   \
    __asm__ __volatile__ ("" : : : "memory");                                \
    *(atomic) = (newval);                                                    \
  }))
#define fluid_atomic_pointer_get(atomic) \
  (({                                                       \
    _basic_pointer_t gapg_result;                                                    \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    gapg_result = (_basic_pointer_t) *(atomic);                                      \
    __sync_synchronize ();                                                   \
    __asm__ __volatile__ ("" : : : "memory");                                \
    gapg_result;                                                             \
  }))
#define fluid_atomic_pointer_set(atomic, newval) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    __sync_synchronize ();                                                   \
    __asm__ __volatile__ ("" : : : "memory");                                \
    *(atomic) = (__typeof__ (*(atomic))) (unsigned_integer_of_pointer_size_t) (newval);               \
  }))

#define fluid_atomic_int_inc(atomic) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                  \
    (void) __sync_fetch_and_add ((atomic), 1);                               \
  }))
#define fluid_atomic_int_dec_and_test(atomic) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                  \
    __sync_fetch_and_sub ((atomic), 1) == 1;                                 \
  }))
#define fluid_atomic_int_compare_and_exchange(atomic, oldval, newval) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (newval) ^ (oldval) : 1);                        \
    __sync_bool_compare_and_swap ((atomic), (oldval), (newval)) ? TRUE : FALSE; \
  }))
#define fluid_atomic_int_compare_and_exchange_full(atomic, oldval, newval, preval) \
  (({                                                         \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                       \
    _BASIC_STATIC_ASSERT (sizeof *(preval) == sizeof (int));                       \
    (void) (0 ? *(atomic) ^ (newval) ^ (oldval) ^ *(preval) : 1);              \
    *(preval) = __sync_val_compare_and_swap ((atomic), (oldval), (newval));    \
    (*(preval) == (oldval)) ? TRUE : FALSE;                                    \
  }))
#if defined(_GLIB_GCC_HAVE_SYNC_SWAP)
#define fluid_atomic_int_exchange(atomic, newval) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (newval) : 1);                                   \
    (int) __sync_swap ((atomic), (newval));                                 \
  }))
#else /* defined(_GLIB_GCC_HAVE_SYNC_SWAP) */
  #define fluid_atomic_int_exchange(atomic, newval) \
  (({                                                       \
    int oldval;                                                             \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (newval) : 1);                                   \
    do                                                                       \
      {                                                                      \
        oldval = *atomic;                                                    \
      } while (!__sync_bool_compare_and_swap (atomic, oldval, newval));      \
    oldval;                                                                  \
  }))
#endif /* defined(_GLIB_GCC_HAVE_SYNC_SWAP) */
#define fluid_atomic_int_add(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (val) : 1);                                      \
    (int) __sync_fetch_and_add ((atomic), (val));                           \
  }))
#define fluid_atomic_int_and(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (val) : 1);                                      \
    (unsigned int) __sync_fetch_and_and ((atomic), (val));                          \
  }))
#define fluid_atomic_int_or(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (val) : 1);                                      \
    (unsigned int) __sync_fetch_and_or ((atomic), (val));                           \
  }))
#define fluid_atomic_int_xor(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (val) : 1);                                      \
    (unsigned int) __sync_fetch_and_xor ((atomic), (val));                          \
  }))

#define fluid_atomic_pointer_compare_and_exchange(atomic, oldval, newval) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    __sync_bool_compare_and_swap ((atomic), (oldval), (newval)) ? TRUE : FALSE; \
  }))
#define fluid_atomic_pointer_compare_and_exchange_full(atomic, oldval, newval, preval) \
  (({                                                             \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                       \
    _BASIC_STATIC_ASSERT (sizeof *(preval) == sizeof (_basic_pointer_t));                       \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                      \
    (void) (0 ? (_basic_pointer_t) *(preval) : NULL);                                      \
    *(preval) = __sync_val_compare_and_swap ((atomic), (oldval), (newval));        \
    (*(preval) == (oldval)) ? TRUE : FALSE;                                        \
  }))
#if defined(_GLIB_GCC_HAVE_SYNC_SWAP)
#define fluid_atomic_pointer_exchange(atomic, newval) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    (_basic_pointer_t) __sync_swap ((atomic), (newval));                             \
  }))
#else
#define fluid_atomic_pointer_exchange(atomic, newval) \
  (({                                                       \
    _basic_pointer_t oldval;                                                         \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    do                                                                       \
      {                                                                      \
        oldval = (_basic_pointer_t) *atomic;                                         \
      } while (!__sync_bool_compare_and_swap (atomic, oldval, newval));      \
    oldval;                                                                  \
  }))
#endif /* defined(_GLIB_GCC_HAVE_SYNC_SWAP) */
#define fluid_atomic_pointer_add(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    (void) (0 ? (val) ^ (val) : 1);                                          \
    (gintptr) __sync_fetch_and_add ((atomic), (val));                        \
  }))
#define fluid_atomic_pointer_and(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    (void) (0 ? (val) ^ (val) : 1);                                          \
    (unsigned_integer_of_pointer_size_t) __sync_fetch_and_and ((atomic), (val));                       \
  }))
#define fluid_atomic_pointer_or(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    (void) (0 ? (val) ^ (val) : 1);                                          \
    (unsigned_integer_of_pointer_size_t) __sync_fetch_and_or ((atomic), (val));                        \
  }))
#define fluid_atomic_pointer_xor(atomic, val) \
  (({                                                       \
    _BASIC_STATIC_ASSERT (sizeof *(atomic) == sizeof (_basic_pointer_t));                 \
    (void) (0 ? (_basic_pointer_t) *(atomic) : NULL);                                \
    (void) (0 ? (val) ^ (val) : 1);                                          \
    (unsigned_integer_of_pointer_size_t) __sync_fetch_and_xor ((atomic), (val));                       \
  }))

#endif /* !defined(__ATOMIC_SEQ_CST) */

#define fluid_atomic_int_exchange_and_add(_pi, _add) fluid_atomic_int_add(_pi, _add)

/* Muteces */

typedef enum {
    FLUID_MUTEX_STATE_EMPTY,
    FLUID_MUTEX_STATE_OWNED,
    FLUID_MUTEX_STATE_CONTENDED
} _fulid_light_mutex_state_t;

#if defined(HAVE_FUTEX) || defined(HAVE_FUTEX_TIME64)

typedef struct {
    unsigned int state;
} fluid_mutex_t;

#define FLUID_MUTEX_INIT          { FLUID_MUTEX_STATE_EMPTY }
#define fluid_mutex_init(_m) do { (_m).state = FLUID_MUTEX_STATE_EMPTY; } while(0)
#define fluid_mutex_destroy(_m) /* no slow_path_mutex to destroy */

#else

typedef struct {
    unsigned int state;
    pthread_mutex_t slow_path_mutex;
} fluid_mutex_t;

#define FLUID_MUTEX_INIT          { FLUID_MUTEX_STATE_EMPTY, PTHREAD_MUTEX_INITIALIZER }
#define fluid_mutex_destroy(_m) pthread_mutex_destroy(&((_m).slow_path_mutex))
#define fluid_mutex_init(_m) do { (_m).state = FLUID_MUTEX_STATE_EMPTY; pthread_mutex_init(&((_m).slow_path_mutex), 0); } while(0)

#endif /* defined(HAVE_FUTEX) || defined(HAVE_FUTEX_TIME64) */

void _fluid_light_mutex_lock_slowpath(fluid_mutex_t*);

static FLUID_INLINE void
_fluid_mutex_lock_by_pointer(fluid_mutex_t* m) {
    if FLUID_UNLIKELY(!fluid_atomic_int_compare_and_exchange(&m->state, FLUID_MUTEX_STATE_EMPTY, FLUID_MUTEX_STATE_OWNED) ) {
        _fluid_light_mutex_lock_slowpath(m);
    }
}
void _fluid_light_mutex_unlock_slowpath(fluid_mutex_t*, unsigned int);

#define fluid_mutex_lock(_m) _fluid_mutex_lock_by_pointer(&(_m))

void _fluid_mutex_unlock_by_pointer(fluid_mutex_t* m);

#define fluid_mutex_unlock(_m) _fluid_mutex_unlock_by_pointer(&(_m))

#if defined(HAVE_FUTEX) || defined(HAVE_FUTEX_TIME64)

/* Futex-related code is adapted from glib */

/* Wrapper macro to call `futex_time64` and/or `futex` with simple
 * parameters and without returning the return value.
 *
 * We expect futex to sometimes return EAGAIN due to the race
 * between the caller checking the current value and deciding to
 * do the futex op. To avoid splattering errno on success, we
 * restore the original errno if EAGAIN is seen. See also:
 *   https://gitlab.gnome.org/GNOME/glib/-/issues/3034
 *
 * If the `futex_time64` syscall does not exist (`ENOSYS`), we retry again
 * with the normal `futex` syscall. This can happen if newer kernel headers
 * are used than the kernel that is actually running.
 *
 * The `futex_time64` syscall is also skipped in favour of `futex` if the
 * Android runtime’s API level is lower than 30, as it’s blocked by seccomp
 * there and using it will cause the app to be terminated:
 *   https://android-review.googlesource.com/c/platform/bionic/+/1094758
 *   https://github.com/aosp-mirror/platform_bionic/commit/ee7bc3002dc3127faac110167d28912eb0e86a20
 *
 * This must not be called with a timeout parameter as that differs
 * in size between the two syscall variants!
 */
#if defined(HAVE_FUTEX) && defined(HAVE_FUTEX_TIME64)
#if defined(__ANDROID__)
#define fluid_futex_simple(uaddr, futex_op, ...)                                     \
  do                                                                   \
  {                                                                              \
    int saved_errno = errno;                                                     \
    int res = 0;                                                                 \
    if (__builtin_available (android 30, *))                                     \
      {                                                                          \
        res = syscall (__NR_futex_time64, uaddr, (size_t) futex_op, __VA_ARGS__); \
        if (res < 0 && errno == ENOSYS)                                          \
          {                                                                      \
            errno = saved_errno;                                                 \
            res = syscall (__NR_futex, uaddr, (size_t) futex_op, __VA_ARGS__);    \
          }                                                                      \
      }                                                                          \
    else                                                                         \
      {                                                                          \
        res = syscall (__NR_futex, uaddr, (size_t) futex_op, __VA_ARGS__);        \
      }                                                                          \
    if (res < 0 && errno == EAGAIN)                                              \
      {                                                                          \
        errno = saved_errno;                                                     \
      }                                                                          \
  }                                                                              \
  while(0)
#else
#define fluid_futex_simple(uaddr, futex_op, ...)                                     \
  do                                                                   \
  {                                                                              \
    int saved_errno = errno;                                                     \
    int res = syscall (__NR_futex_time64, uaddr, (size_t) futex_op, __VA_ARGS__); \
    if (res < 0 && errno == ENOSYS)                                              \
      {                                                                          \
        errno = saved_errno;                                                     \
        res = syscall (__NR_futex, uaddr, (size_t) futex_op, __VA_ARGS__);        \
      }                                                                          \
    if (res < 0 && errno == EAGAIN)                                              \
      {                                                                          \
        errno = saved_errno;                                                     \
      }                                                                          \
  }                                                                              \
  while(0)
#endif /* defined(__ANDROID__) */
#elif defined(HAVE_FUTEX_TIME64)
#define fluid_futex_simple(uaddr, futex_op, ...)                                     \
  do                                                                   \
  {                                                                              \
    int saved_errno = errno;                                                     \
    int res = syscall (__NR_futex_time64, uaddr, (size_t) futex_op, __VA_ARGS__); \
    if (res < 0 && errno == EAGAIN)                                              \
      {                                                                          \
        errno = saved_errno;                                                     \
      }                                                                          \
  }                                                                              \
  while(0)
#elif defined(HAVE_FUTEX)
#define fluid_futex_simple(uaddr, futex_op, ...)                              \
  do                                                            \
  {                                                                       \
    int saved_errno = errno;                                              \
    int res = syscall (__NR_futex, uaddr, (size_t) futex_op, __VA_ARGS__); \
    if (res < 0 && errno == EAGAIN)                                       \
      {                                                                   \
        errno = saved_errno;                                              \
      }                                                                   \
  }                                                                       \
  while(0)
#else /* !defined(HAVE_FUTEX) && !defined(HAVE_FUTEX_TIME64) */
#error "Neither __NR_futex nor __NR_futex_time64 are available"
#endif /* defined(HAVE_FUTEX) && defined(HAVE_FUTEX_TIME64) */

/* Dynamically allocated mutex suitable for fluid_cond_t use */
typedef fluid_mutex_t fluid_cond_mutex_t;
#define fluid_cond_mutex_lock(_m)        _fluid_mutex_lock_by_pointer(_m)
#define fluid_cond_mutex_unlock(_m)      _fluid_mutex_unlock_by_pointer(_m)

static FLUID_INLINE fluid_cond_mutex_t *
new_fluid_cond_mutex(void)
{
    fluid_cond_mutex_t *mutex;
    mutex = FLUID_NEW(fluid_cond_mutex_t);
    if (mutex) {
        mutex->state = FLUID_MUTEX_STATE_EMPTY;
    }
    return mutex;
}

static FLUID_INLINE void
delete_fluid_cond_mutex(fluid_cond_mutex_t *m)
{
    fluid_return_if_fail(m != NULL);
    FLUID_FREE(m);
}

/* Thread condition signaling */

typedef struct {
    unsigned int counter;
} fluid_cond_t;

#define fluid_cond_signal(_cond) do { \
    fluid_atomic_int_inc(&(_cond)->counter); \
    fluid_futex_simple(&(_cond)->counter, (size_t) FUTEX_WAKE_PRIVATE, (size_t) 1, NULL); \
  } while(0)

#define fluid_cond_broadcast(_cond) do { \
    fluid_atomic_int_inc(&(_cond)->counter); \
    fluid_futex_simple(&(_cond)->counter, (size_t) FUTEX_WAKE_PRIVATE, (size_t) INT_MAX, NULL); \
  } while(0)

#define fluid_cond_wait(_cond, _mutex) do { \
    unsigned int sampled = (unsigned int) fluid_atomic_int_get(&(_cond)->counter); \
    _fluid_mutex_unlock_by_pointer(_mutex); \
    fluid_futex_simple(&(_cond)->counter, (size_t) FUTEX_WAIT_PRIVATE, (size_t) sampled, NULL); \
    _fluid_mutex_lock_by_pointer(_mutex); \
  } while(0)

static FLUID_INLINE fluid_cond_t *
new_fluid_cond(void)
{
    fluid_cond_t *cond;
    cond = FLUID_NEW(fluid_cond_t);
    if (cond) {
        cond->counter = 0;
    }
    return (cond);
}

static FLUID_INLINE void
delete_fluid_cond(fluid_cond_t *cond)
{
    fluid_return_if_fail(cond != NULL);
    FLUID_FREE(cond);
}

#else /* defined(HAVE_FUTEX) || defined(HAVE_FUTEX_TIME64) */

/* Dynamically allocated mutex suitable for fluid_cond_t use */

typedef pthread_mutex_t fluid_cond_mutex_t;
#define fluid_cond_mutex_lock(_m)        pthread_mutex_lock(_m)
#define fluid_cond_mutex_unlock(_m)      pthread_mutex_unlock(_m)

static FLUID_INLINE fluid_cond_mutex_t *
new_fluid_cond_mutex(void)
{
    fluid_cond_mutex_t *mutex;
    mutex = FLUID_NEW(fluid_cond_mutex_t);
    if (mutex) {
       pthread_mutex_init(mutex, 0);
    }
    return mutex;
}

static FLUID_INLINE void
delete_fluid_cond_mutex(fluid_cond_mutex_t *m)
{
    fluid_return_if_fail(m != NULL);
    pthread_mutex_destroy(m);
    FLUID_FREE(m);
}

/* Thread condition signaling */
typedef pthread_cond_t fluid_cond_t;
#define fluid_cond_signal(_cond)         pthread_cond_signal(_cond)
#define fluid_cond_broadcast(_cond)      pthread_cond_broadcast(_cond)
#define fluid_cond_wait(_cond, _mutex)    pthread_cond_wait((_cond), (_mutex))

static FLUID_INLINE fluid_cond_t *
new_fluid_cond(void)
{
    fluid_cond_t *cond;
    cond = FLUID_NEW(fluid_cond_t);
    if (cond) {
        pthread_cond_init(cond, 0);
    }
    return (cond);
}
 
static FLUID_INLINE void
delete_fluid_cond(fluid_cond_t *cond)
{
    fluid_return_if_fail(cond != NULL);
    pthread_cond_destroy(cond);
    FLUID_FREE(cond);
}

#endif /* (else) defined(HAVE_FUTEX) || defined(HAVE_FUTEX_TIME64) */

/* Recursive lock capable mutex */
typedef pthread_mutex_t fluid_rec_mutex_t;
static FLUID_INLINE void
_fluid_rec_mutex_init(pthread_mutex_t* mutex) {
    pthread_mutexattr_t attr;
    int res = -1;
    if(pthread_mutexattr_init(&attr) == 0) {
        if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0) {
            res = pthread_mutex_init(mutex, &attr);
        }
        pthread_mutexattr_destroy(&attr);
    }
    if (res != 0) {
        FLUID_LOG(FLUID_ERR, "failed to create recursive mutex");
    }
}
#define fluid_rec_mutex_init(_m)      _fluid_rec_mutex_init(&(_m))
#define fluid_rec_mutex_destroy(_m)   pthread_mutex_destroy(&(_m))
#define fluid_rec_mutex_lock(_m)      pthread_mutex_lock(&(_m))
#define fluid_rec_mutex_unlock(_m)    pthread_mutex_unlock(&(_m))

/* Thread private data */

typedef pthread_key_t fluid_private_t;
#define fluid_private_init(_priv)                  pthread_key_create(&(_priv), 0)
#define fluid_private_free(_priv)                  pthread_key_delete(_priv)
#define fluid_private_get(_priv)                   pthread_getspecific(_priv)
#define fluid_private_set(_priv, _data)            pthread_setspecific((_priv), (_data))

/* Threads */

/* other thread implementations might change this for their needs */
typedef void *fluid_thread_return_t;
typedef fluid_thread_return_t (*fluid_thread_func_t)(void *data);

/* static return value for thread functions which requires a return value */
#define FLUID_THREAD_RETURN_VALUE (NULL)

typedef pthread_t fluid_thread_t;

/* whether or not the implementation can be thread safe at all */
#define FLUID_THREAD_SAFE_CAPABLE 1

static FLUID_INLINE void delete_fluid_thread(fluid_thread_t *thread) { FLUID_FREE(thread); }

static FLUID_INLINE int
fluid_thread_join(fluid_thread_t *thread) {
    if (pthread_join(*thread, 0) == 0) {
        return FLUID_OK;
    }
    return FLUID_FAILED;
}

/* File access */
#define FLUID_FILE_TEST_EXISTS      1
#define FLUID_FILE_TEST_IS_REGULAR  2

typedef struct {
    #undef st_mtime
    int st_mtime;
} fluid_stat_buf_t;

static FLUID_INLINE bool fluid_file_test(const char *path, int flags) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return false;
    }
    if ((flags & FLUID_FILE_TEST_EXISTS) != 0) {
        return true;
    }
    if ((flags & FLUID_FILE_TEST_IS_REGULAR) != 0) {
        return (st.st_mode & S_IFREG) != 0;
    }
    return false;
}

static FLUID_INLINE int fluid_stat(const char *path, fluid_stat_buf_t *buffer)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        buffer->st_mtime = 0;
        return FLUID_FAILED;
    }
#if defined(APPLE) || defined(BSD)
    buffer->st_mtime = st.st_mtimespec.tv_sec;
#else
    buffer->st_mtime = st.st_mtim.tv_sec;
#endif
    return FLUID_OK;
}

/* Debug functions */
#define fluid_assert assert

#ifdef __cplusplus
}
#endif
#endif /* _FLUID_SYS_PTHREAD_H */
