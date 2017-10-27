
#ifndef _FLUID_ATOMIC_H
#define _FLUID_ATOMIC_H

#include <stdbool.h>

#ifndef STATIC_ASSERT
#if __STDC_VERSION__ >= 201112L
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT(expr, msg) typedef char __static_assertion_ ## __COUNTER__[(expr) ? 1 : -1]
#endif
#endif //STATIC_ASSERT

#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4

#define fluid_atomic_int_add(atomic, val) __extension__ ({            \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int),     \
                          "Atomic must be the size of an int");       \
            __sync_fetch_and_add(&(atomic)->value, (val));})

#define fluid_atomic_int_get(atomic) __extension__ ({             \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int), \
                          "Atomic must be the size of an int");   \
            __sync_synchronize();                                 \
            (atomic)->value;})

#define fluid_atomic_int_set(atomic, newval) __extension__ ({         \
                STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int), \
                              "Atomic must be the size of an int");   \
                (atomic)->value = (newval);                           \
                __sync_synchronize();})

#define fluid_atomic_int_inc(atomic) __extension__ ({             \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int), \
                          "Atomic must be the size of an int");   \
            __sync_synchronize();                                 \
            __sync_fetch_and_add(&(atomic)->value, 1);})

#define fluid_atomic_int_compare_and_exchange(atomic, oldval, newval) __extension__ ({ \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int),       \
                          "Atomic must be the size of an int");         \
            __sync_bool_compare_and_swap(&(atomic)->value, (oldval), (newval));})

#define fluid_atomic_int_exchange_and_add(atomic, add) __extension__ ({   \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int),         \
            "Atomic must be the size of an int");                         \
            __sync_synchronize();                                         \
            __sync_fetch_and_add(&(atomic)->value, add);})
            
#define fluid_atomic_float_get(atomic) __extension__ ({   \
  STATIC_ASSERT(sizeof((atomic)->value) == sizeof(float), \
                "Atomic must be the size of a float");    \
  __sync_synchronize();                                   \
  (atomic)->value;})

#define fluid_atomic_float_set(atomic, val) __extension__ ({  \
      STATIC_ASSERT(sizeof((atomic)->value) == sizeof(float), \
                    "Atomic must be the size of a float");    \
      (atomic)->value = (val);                                \
      __sync_synchronize();})

#else

#error "GCC builtins missings for atomic operations"

#endif

#endif /* _FLUID_ATOMIC_H */
