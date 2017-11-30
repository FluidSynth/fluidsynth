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


#ifndef _FLUID_SETTINGS_H
#define _FLUID_SETTINGS_H

int fluid_settings_add_option(fluid_settings_t* settings, const char* name, const char* s);
int fluid_settings_remove_option(fluid_settings_t* settings, const char* name, const char* s);

int fluid_settings_register_str(fluid_settings_t* settings, const char* name, const char* def, int hints);
int fluid_settings_register_num(fluid_settings_t* settings, const char* name, double def,
                                double min, double max, int hints);
int fluid_settings_register_int(fluid_settings_t* settings, const char* name, int def,
                                int min, int max, int hints);

typedef void (*fluid_settings_callback_t)(fluid_settings_t *settings, const char *name, void *data);
int fluid_settings_set_callback(fluid_settings_t* settings, const char *name,
                                fluid_settings_callback_t callback, void *data);

#endif /* _FLUID_SETTINGS_H */
