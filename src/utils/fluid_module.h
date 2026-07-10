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
 * @file fluid_mod.h
 *
 * Portable dynamic module (shared library) loading API.
 * Replaces the gmodule dependency previously used for LADSPA plugin loading.
 */

#ifndef _FLUID_MOD_H
#define _FLUID_MOD_H

#include "fluidsynth_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _fluid_module_t fluid_module_t;

/* Open a shared library by file name. Returns NULL on failure. */
fluid_module_t *fluid_module_open(const char *name);

/* Close a previously opened module. Returns non-zero on failure. */
int fluid_module_close(fluid_module_t *mod);

/* Retrieve a human-readable error string for the last module operation. */
const char *fluid_module_error(void);

/* Retrieve the file name of an opened module. */
const char *fluid_module_name(fluid_module_t *mod);

/* Look up a symbol in a module. Returns non-zero on success, zero on failure.
 * The resolved address is stored in *ptr. */
int fluid_module_symbol(fluid_module_t *mod, const char *name, void **ptr);

#ifdef __cplusplus
}
#endif

#endif /* _FLUID_MOD_H */
