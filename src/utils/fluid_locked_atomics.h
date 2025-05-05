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
 * @file fluid_locked_atomics.h
 *
 * This header contains generic mutex based atomic operations. The atomic
 * variables are protected by a central extern fluid_mutex_t _atomic_lock.
 */

#ifndef _FLUID_LOCKED_ATOMICS_H
#define _FLUID_LOCKED_ATOMICS_H

extern fluid_mutex_t _atomic_lock;

#define fluid_atomic_int_inc(_pi) fluid_atomic_int_add(_pi, 1)
#define fluid_atomic_int_get(_pi) fluid_atomic_int_add(_pi, 0)

#define fluid_atomic_int_set(_pi, _val) \
    _fluid_atomic_int_set((fluid_atomic_int_t *)_pi, _val)
#define fluid_atomic_int_dec_and_test(_pi) \
    _fluid_atomic_int_dec_and_test((fluid_atomic_int_t *)_pi)
#define fluid_atomic_int_compare_and_exchange(_pi, _old, _new) \
    _fluid_atomic_int_compare_and_exchange((fluid_atomic_int_t *)_pi, _old, _new)
#define fluid_atomic_int_add(_pi, _add) \
    _fluid_atomic_int_add((fluid_atomic_int_t *)_pi, _add)
#define fluid_atomic_int_exchange_and_add fluid_atomic_int_add

#define FLUID_ATOMIC_WRAPPER(op) \
    fluid_mutex_lock(_atomic_lock); \
    op; \
    fluid_mutex_unlock(_atomic_lock);

static FLUID_INLINE void
_fluid_atomic_int_set(fluid_atomic_int_t *pi, int val)
{
    FLUID_ATOMIC_WRAPPER(*pi = val)
}

static FLUID_INLINE bool
_fluid_atomic_int_dec_and_test(fluid_atomic_int_t *pi)
{
    bool result;
    FLUID_ATOMIC_WRAPPER(result = --(*pi) == 0)
    return result;
}

static FLUID_INLINE bool
_fluid_atomic_int_compare_and_exchange(fluid_atomic_int_t *pi, int old, int _new)
{
    bool result;
    FLUID_ATOMIC_WRAPPER(
        if (*pi != old)
        {
            result = false;
        }
        else
        {
            *pi = _new;
            result = true;
        }
    )

    return result;
}

static FLUID_INLINE int
_fluid_atomic_int_add(fluid_atomic_int_t *pi, int add)
{
    int previous;
    FLUID_ATOMIC_WRAPPER(
        previous = *pi;
        *pi += add;
    )

    return previous;
}

static FLUID_INLINE bool
fluid_atomic_pointer_compare_and_exchange(void *pp, void *old, void *_new)
{
    bool result;
    FLUID_ATOMIC_WRAPPER(
        if (*((void **)pp) != old)
        {
            result = false;
        }
        else
        {
            *((void **)pp) = _new;
            result = true;
        }
    )

    return result;
}

static FLUID_INLINE void *
fluid_atomic_pointer_get(void *pp)
{
    void *result;
    FLUID_ATOMIC_WRAPPER(result = *((void **)pp))
    return result;
}

static FLUID_INLINE void
fluid_atomic_pointer_set(void *pp, void *val)
{
    FLUID_ATOMIC_WRAPPER(*((void **)pp) = val);
}

#endif /* _FLUID_LOCKED_ATOMICS_H */
