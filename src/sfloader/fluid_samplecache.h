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


#ifndef _FLUID_SAMPLECACHE_H
#define _FLUID_SAMPLECACHE_H

#include "fluid_sfont.h"

int fluid_cached_sampledata_load(char *filename,
                                 unsigned int samplepos,
                                 unsigned int samplesize,
                                 short **sampledata,
                                 unsigned int sample24pos,
                                 unsigned int sample24size,
                                 char **sample24data,
                                 int try_mlock,
                                 const fluid_file_callbacks_t *fcbs);

int fluid_cached_sampledata_unload(const short *sampledata);

#endif /* _FLUID_SAMPLECACHE_H */
