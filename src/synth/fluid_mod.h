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
    unsigned char dest;           /**< Destination generator to control or, if FLUID_MOD_LINK_DEST
                                       is set, index of the modulator this modulator is linked to. */
    unsigned char src1;           /**< Source controller 1 */
    unsigned char flags1;         /**< Source controller 1 flags */
    unsigned char src2;           /**< Source controller 2 */
    unsigned char flags2;         /**< Source controller 2 flags */
    double amount;                /**< Multiplier amount */
    double link;                  /**< Summation of modulator nodes linked to this modulator */
    /* The 'next' field allows to link modulators into a list.  It is
     * not used in fluid_voice.c, there each voice allocates memory for a
     * fixed number of modulators.  Since there may be a huge number of
     * different zones, this is more efficient.
     * However 'next' is used for identity test of complex linked modulator,
     * And moving to index of last member of a complex modulator.
     */
    fluid_mod_t *next;
};

/* bit link of destination in fluidsynth modulators */
#define FLUID_MOD_LINK_DEST  (1 << 7)     /* Link is bit 7 of destination */

int fluid_mod_get_linked_count(const fluid_mod_t *mod);
int fluid_mod_get_list_count(const fluid_mod_t *mod);

fluid_mod_t *fluid_mod_get_next(fluid_mod_t *mod);
int fluid_mod_has_linked_src1 (const fluid_mod_t * mod);
int fluid_mod_is_linked (const fluid_mod_t * mod);

/* this enum is used for test_mode parameter when calling
   fluid_linked_mod_test_identity()
   Note, for efficiency:
   FLUID_LINKED_MOD_TEST_OVERWRITE must be equal to FLUID_VOICE_OVERWRITE.
   FLUID_LINKED_MOD_TEST_ADD must be equal to FLUID_VOICE_ADD.
*/
enum fluid_linked_mod_test_identity
{
    /**< test identity and overwrite modulator amounts */
    FLUID_LINKED_MOD_TEST_OVERWRITE = FLUID_VOICE_OVERWRITE,

    /**< test identity and add (sum) modulator amounts */
    FLUID_LINKED_MOD_TEST_ADD = FLUID_VOICE_ADD,

    /**< test identity only */
    FLUID_LINKED_MOD_TEST_ONLY
};

int fluid_linked_mod_test_identity(fluid_mod_t *cm0, fluid_mod_t *cm1,
                                   unsigned char test_mode);

fluid_real_t fluid_mod_get_value(fluid_mod_t *mod, fluid_voice_t *voice);
int fluid_mod_check_sources(const fluid_mod_t *mod, char *name);

void fluid_zone_check_remove_mod(fluid_mod_t **list_mod);

int fluid_mod_check_linked_mod(char *list_name,
                            fluid_mod_t *list_mod, int mod_count,
                            fluid_mod_t **linked_mod, int linked_count);

void delete_fluid_mod_list(fluid_mod_t *mod);

void fluid_dump_modulator(fluid_mod_t *mod);
void fluid_dump_linked_mod(fluid_mod_t *mod, int mod_idx, int offset);

#endif /* _FLUID_MOD_H */
