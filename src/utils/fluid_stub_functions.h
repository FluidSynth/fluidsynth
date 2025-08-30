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
 * @file fluid_stub_functions.h
 *
 * This header provides macros to create inline stub functions.
 */

#ifndef _FLUID_STUB_FUNCTIONS_H
#define _FLUID_STUB_FUNCTIONS_H

#include "fluidsynth_priv.h"


#define STUB_FUNCTION_VOID(function, args) \
    static FLUID_INLINE void \
    function args \
    { \
        FLUID_LOG(FLUID_ERR, "function " # function " is a stub"); \
    }

#define STUB_FUNCTION(function, type, result, args) \
    static FLUID_INLINE type \
    function args \
    { \
        FLUID_LOG(FLUID_ERR, "function " # function " is a stub, always returning " # result); \
        return result; \
    }

#define STUB_FUNCTION_VOID_SILENT(function, args) \
    static FLUID_INLINE void \
    function args \
    { \
    }

#define STUB_FUNCTION_SILENT(function, type, result, args) \
    static FLUID_INLINE type \
    function args \
    { \
        return result; \
    }

#endif /* _FLUID_STUB_FUNCTIONS_H */
