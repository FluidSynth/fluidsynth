/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *  
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */


#ifndef _FLUID_GEN_H
#define _FLUID_GEN_H

#include "fluidsynth_priv.h"

#define fluid_gen_set_mod(_gen, _val)  { (_gen)->mod = (double) (_val); }
#define fluid_gen_set_nrpn(_gen, _val) { (_gen)->nrpn = (double) (_val); }

fluid_real_t fluid_gen_map_nrpn(int gen, int nrpn);

int fluid_gen_init(fluid_gen_t* gen, fluid_channel_t* channel);

#endif /* _FLUID_GEN_H */
