#pragma once

/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2021 Tom Moebert and others.
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

#ifdef __cplusplus
extern "C" {
#endif

#include "fluid_sys.h"

#ifdef __cplusplus
}
#endif

#include <sstream>

template<typename T>
auto guardedCall (T&& func, decltype(func()) errorReturnValue) -> decltype(func())
{
    std::stringstream ss;
    try
    {
        return func();
    }
    catch( const std::bad_alloc& e )
    {
        FLUID_LOG(FLUID_PANIC, "Out of memory\n");
    }
    catch( const std::exception& e )
    {
        ss << e.what();
        FLUID_LOG(FLUID_ERR, "%s\n", ss.str().c_str());
    }
    catch(...)
    {
        FLUID_LOG(FLUID_ERR, "Unknown exception occurred\n");
    }

    return std::move(errorReturnValue);
}
