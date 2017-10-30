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

/* Author: Markus Nentwig, nentwig@users.sourceforge.net
 */

#ifndef _FLUID_LADSPA_H
#define _FLUID_LADSPA_H

#include "fluid_sys.h"
#include "fluidsynth_priv.h"

#ifdef LADSPA

typedef struct _fluid_ladspa_fx_t fluid_ladspa_fx_t;

fluid_ladspa_fx_t *new_fluid_ladspa_fx(fluid_real_t sample_rate, int buffer_size);
void delete_fluid_ladspa_fx(fluid_ladspa_fx_t *fx);

int fluid_ladspa_set_sample_rate(fluid_ladspa_fx_t *fx, fluid_real_t sample_rate);

void fluid_ladspa_run(fluid_ladspa_fx_t *fx, int block_count, int block_size);

int fluid_ladspa_is_active(fluid_ladspa_fx_t *fx);
int fluid_ladspa_activate(fluid_ladspa_fx_t *fx);
int fluid_ladspa_deactivate(fluid_ladspa_fx_t *fx);
int fluid_ladspa_reset(fluid_ladspa_fx_t *fx);
int fluid_ladspa_check(fluid_ladspa_fx_t *fx, char *err, int err_size);

int fluid_ladspa_add_host_ports(fluid_ladspa_fx_t *fx, const char *prefix,
        int buffer_count, int buffer_size, fluid_real_t *left[], fluid_real_t *right[]);
int fluid_ladspa_host_port_exists(fluid_ladspa_fx_t *fx, const char *name);

int fluid_ladspa_add_buffer(fluid_ladspa_fx_t *fx, const char *name);
int fluid_ladspa_buffer_exists(fluid_ladspa_fx_t *fx, const char *name);

int fluid_ladspa_add_effect(fluid_ladspa_fx_t *fx, const char *effect_name,
        const char *lib_name, const char *plugin_name);
int fluid_ladspa_effect_can_mix(fluid_ladspa_fx_t *fx, const char *name);
int fluid_ladspa_effect_set_mix(fluid_ladspa_fx_t *fx, const char *name, int mix, float gain);
int fluid_ladspa_effect_port_exists(fluid_ladspa_fx_t *fx, const char *effect_name, const char *port_name);
int fluid_ladspa_effect_set_control(fluid_ladspa_fx_t *fx, const char *effect_name,
        const char *port_name, float val);
int fluid_ladspa_effect_link(fluid_ladspa_fx_t *fx, const char *effect_name,
        const char *port_name, const char *name);


#endif /* LADSPA */
#endif /* _FLUID_LADSPA_H */
