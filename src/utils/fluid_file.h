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
 * fluid_file is a separate C++ module that contains a single function: fluid_file_test
 *
 * This function is required by libfluidsynth as well as by fluidsynth's executable.
 * When compiling with glib as OSAL, this function was defined as macro. When compiling with C++11 as OSAL,
 * we cannot define this function in fluid_sys* because it would not have linker visibility within libfluidsynth.
 * We could export this function by declaring it FLUIDSYNTH_API, however this resulted in the same linker error
 * for MinGW and Clang on Windows, presumably because __declspec(dllimport) was missing in the function's
 * declaration when this header is included into fluidsynth.c
 *
 * Because of that, define this function in its dedicated module and statically link this module to both,
 * the executable and libfluidsynth.
 */

#ifndef _FLUID_FILE_H
#define _FLUID_FILE_H

#include <stdbool.h>
#include "fluidsynth_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#if OSAL_glib

#define FLUID_FILE_TEST_EXISTS G_FILE_TEST_EXISTS
#define FLUID_FILE_TEST_IS_REGULAR G_FILE_TEST_IS_REGULAR

#else

#define FLUID_FILE_TEST_EXISTS      1
#define FLUID_FILE_TEST_IS_REGULAR  2

#endif

#if OSAL_glib || OSAL_cpp11
bool fluid_file_test(const char *path, int flags);
#endif

#ifdef __cplusplus
}
#endif
#endif /* _FLUID_FILE_H */
