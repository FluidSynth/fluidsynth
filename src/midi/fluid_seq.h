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

#ifndef _FLUID_SEQ_H
#define _FLUID_SEQ_H

#include "fluidsynth.h"
#include "fluid_event.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void fluid_sequencer_invalidate_note(fluid_sequencer_t *seq, fluid_seq_id_t dest, fluid_note_id_t id);

#ifdef __cplusplus
}
#endif

#endif /* _FLUID_SEQ_H */
