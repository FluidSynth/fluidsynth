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

#ifndef _FLUID_MOD_H
#define _FLUID_MOD_H

#include "fluidsynth_priv.h"
#include "fluid_conv.h"

/*
 * Modulator structure.  See SoundFont 2.04 PDF section 8.2.
 */
struct _fluid_mod_t
{
    unsigned char dest;           /**< Destination generator to control */
    unsigned char src1;           /**< Source controller 1 */
    unsigned char flags1;         /**< Source controller 1 flags */
    unsigned char src2;           /**< Source controller 2 */
    unsigned char flags2;         /**< Source controller 2 flags */
    double amount;                /**< Multiplier amount */
    double link;                  /**< src1 link input node */
    /* The 'next' field allows to link modulators into a list.  It is
     * not used in fluid_voice.c, there each voice allocates memory for a
     * fixed number of modulators.  Since there may be a huge number of
     * different zones, this is more efficient.
     * However 'next' is used for identity test of complex linked modulator,
     * And moving to index of last member of a complex modulator.
     */
    fluid_mod_t *next;
};

/* bit link of destination in soundfont modulators */
#define FLUID_SFMOD_LINK_DEST  (1 << 15)   /* Link is bit 15 of destination */
/* bit link of destination in fluidsynth modulators */
#define FLUID_MOD_LINK_DEST  (1 << 7)     /* Link is bit 7 of destination */

unsigned char fluid_get_num_mod(fluid_mod_t *mod);
fluid_mod_t *fluid_get_next_mod(fluid_mod_t *mod);
int fluid_mod_has_linked_src1 (fluid_mod_t * mod);
int fluid_linked_mod_test_identity(fluid_mod_t *cm0,unsigned char cm0_idx,
								   fluid_mod_t *cm1, 
                                   unsigned char add_amount);

fluid_real_t fluid_mod_get_value(fluid_mod_t *mod, fluid_voice_t *voice);
int fluid_mod_check_sources(const fluid_mod_t *mod, char *name);

#ifdef DEBUG
void fluid_dump_modulator(fluid_mod_t *mod);
#endif


#endif /* _FLUID_MOD_H */
