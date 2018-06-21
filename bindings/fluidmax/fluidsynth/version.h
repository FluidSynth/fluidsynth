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

#ifndef _FLUIDSYNTH_VERSION_H
#define _FLUIDSYNTH_VERSION_H


#ifdef __cplusplus
extern "C" {
#endif

#define FLUIDSYNTH_VERSION "1.0.3"
#define FLUIDSYNTH_VERSION_MAJOR 1
#define FLUIDSYNTH_VERSION_MINOR 0
#define FLUIDSYNTH_VERSION_MICRO 3


FLUIDSYNTH_API void fluid_version(int *major, int *minor, int *micro);

FLUIDSYNTH_API char *fluid_version_str(void);


#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_VERSION_H */
