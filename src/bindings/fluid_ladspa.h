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

#ifndef _FLUID_LADSPA_H
#define _FLUID_LADSPA_H

#include "fluid_sys.h"

#ifdef __cplusplus
extern "C" {
#endif

fluid_ladspa_fx_t *new_fluid_ladspa_fx(fluid_real_t sample_rate, int buffer_size);
void delete_fluid_ladspa_fx(fluid_ladspa_fx_t *fx);

int fluid_ladspa_set_sample_rate(fluid_ladspa_fx_t *fx, fluid_real_t sample_rate);

void fluid_ladspa_run(fluid_ladspa_fx_t *fx, int block_count, int block_size);

int fluid_ladspa_add_host_ports(fluid_ladspa_fx_t *fx, const char *prefix,
                                int num_buffers, fluid_real_t buffers[], int buf_stride);

#ifdef __cplusplus
}
#endif

#endif /* _FLUID_LADSPA_H */
