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

#ifndef _FLUIDSYNTH_SEQ_H
#define _FLUIDSYNTH_SEQ_H

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*fluid_event_callback_t)(unsigned int time, fluid_event_t* event, 
				      fluid_sequencer_t* seq, void* data);


/** Allocate a new sequencer structure */
FLUIDSYNTH_API fluid_sequencer_t* new_fluid_sequencer(void);

/** Free the sequencer structure */
FLUIDSYNTH_API void delete_fluid_sequencer(fluid_sequencer_t* seq);

/** clients can be sources or destinations of events. These functions ensure a unique ID for any
source or dest, for filtering purposes.
sources only dont need to register a callback.
*/

/** Register a client. The registration returns a unique client ID (-1 if error) */
FLUIDSYNTH_API 
short fluid_sequencer_register_client(fluid_sequencer_t* seq, char* name, 
				     fluid_event_callback_t callback, void* data);

/** Unregister a previously registered client. */
FLUIDSYNTH_API void fluid_sequencer_unregister_client(fluid_sequencer_t* seq, short id);

/** Returns the number of register clients. */
FLUIDSYNTH_API int fluid_sequencer_count_clients(fluid_sequencer_t* seq);

/** Returns the id of a registered client (-1 if non existing) */
FLUIDSYNTH_API short fluid_sequencer_get_client_id(fluid_sequencer_t* seq, int index);

/** Returns the name of a registered client, given its id. */
FLUIDSYNTH_API char* fluid_sequencer_get_client_name(fluid_sequencer_t* seq, int id);

/** Returns 1 if client is a destination (has a callback) */
FLUIDSYNTH_API int fluid_sequencer_client_is_dest(fluid_sequencer_t* seq, int id);



/** Sending an event immediately. */
FLUIDSYNTH_API void fluid_sequencer_send_now(fluid_sequencer_t* seq, fluid_event_t* evt);


/** Schedule an event for later sending. If absolute is 0, the time of
    the event will be offset with the current tick of the
    sequencer. If absolute is different from 0, the time will assumed
    to be absolute (starting from the creation of the sequencer). 
    MAKES A COPY */
FLUIDSYNTH_API 
int fluid_sequencer_send_at(fluid_sequencer_t* seq, fluid_event_t* evt, 
			   unsigned int time, int absolute);

/** Remove events from the event queue. The events can be filtered on
    the source, the destination, and the type of the event. To avoid
    filtering, set either source, dest, or type to -1.  */
FLUIDSYNTH_API 
void fluid_sequencer_remove_events(fluid_sequencer_t* seq, short source, short dest, int type);


/** Get the current tick */
FLUIDSYNTH_API unsigned int fluid_sequencer_get_tick(fluid_sequencer_t* seq);

/** Set the conversion from tick to absolute time. scale should be
    expressed as ticks per second. */
FLUIDSYNTH_API void fluid_sequencer_set_time_scale(fluid_sequencer_t* seq, double scale);

/** Set the conversion from tick to absolute time (ticks per
    second). */
FLUIDSYNTH_API double fluid_sequencer_get_time_scale(fluid_sequencer_t* seq);

// compile in internal traceing functions
#define FLUID_SEQ_WITH_TRACE 0

#if FLUID_SEQ_WITH_TRACE
FLUIDSYNTH_API char * fluid_seq_gettrace(fluid_sequencer_t* seq);
FLUIDSYNTH_API void fluid_seq_cleartrace(fluid_sequencer_t* seq);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_SEQ_H */
