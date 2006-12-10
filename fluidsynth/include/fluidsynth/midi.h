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

#ifndef _FLUIDSYNTH_MIDI_H
#define _FLUIDSYNTH_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file midi.h
 * @brief Functions for MIDI events, drivers and MIDI file playback.
 */

FLUIDSYNTH_API fluid_midi_event_t* new_fluid_midi_event(void);
FLUIDSYNTH_API int delete_fluid_midi_event(fluid_midi_event_t* event);

FLUIDSYNTH_API int fluid_midi_event_set_type(fluid_midi_event_t* evt, int type);
FLUIDSYNTH_API int fluid_midi_event_get_type(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_channel(fluid_midi_event_t* evt, int chan);
FLUIDSYNTH_API int fluid_midi_event_get_channel(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_get_key(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_key(fluid_midi_event_t* evt, int key);
FLUIDSYNTH_API int fluid_midi_event_get_velocity(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_velocity(fluid_midi_event_t* evt, int vel);
FLUIDSYNTH_API int fluid_midi_event_get_control(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_control(fluid_midi_event_t* evt, int ctrl);
FLUIDSYNTH_API int fluid_midi_event_get_value(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_value(fluid_midi_event_t* evt, int val);
FLUIDSYNTH_API int fluid_midi_event_get_program(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_program(fluid_midi_event_t* evt, int val);
FLUIDSYNTH_API int fluid_midi_event_get_pitch(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_pitch(fluid_midi_event_t* evt, int val);


/**
 * Generic callback function for MIDI events.
 * @param data User defined data pointer
 * @param event The MIDI event
 * @return DOCME
 *
 * Will be used between
 * - MIDI driver and MIDI router
 * - MIDI router and synth
 * to communicate events.
 * In the not-so-far future...
 */
typedef int (*handle_midi_event_func_t)(void* data, fluid_midi_event_t* event);

/*
 *  MIDI router
 *
 *  The MIDI handler forwards incoming MIDI events to the synthesizer
 */

FLUIDSYNTH_API fluid_midi_router_t* new_fluid_midi_router(fluid_settings_t* settings,
						       handle_midi_event_func_t handler, 
						       void* event_handler_data); 

FLUIDSYNTH_API int delete_fluid_midi_router(fluid_midi_router_t* handler); 
FLUIDSYNTH_API int fluid_midi_router_handle_midi_event(void* data, fluid_midi_event_t* event);
FLUIDSYNTH_API int fluid_midi_dump_prerouter(void* data, fluid_midi_event_t* event); 
FLUIDSYNTH_API int fluid_midi_dump_postrouter(void* data, fluid_midi_event_t* event); 

/*
 *  MIDI driver
 *
 *  The MIDI handler forwards incoming MIDI events to the synthesizer
 */

FLUIDSYNTH_API 
fluid_midi_driver_t* new_fluid_midi_driver(fluid_settings_t* settings, 
					 handle_midi_event_func_t handler, 
					 void* event_handler_data);

FLUIDSYNTH_API void delete_fluid_midi_driver(fluid_midi_driver_t* driver);



/*
 *  MIDI file player
 *
 *  The MIDI player allows you to play MIDI files with the FLUID Synth
 */

FLUIDSYNTH_API fluid_player_t* new_fluid_player(fluid_synth_t* synth);
FLUIDSYNTH_API int delete_fluid_player(fluid_player_t* player);
FLUIDSYNTH_API int fluid_player_add(fluid_player_t* player, char* midifile);
FLUIDSYNTH_API int fluid_player_play(fluid_player_t* player);
FLUIDSYNTH_API int fluid_player_stop(fluid_player_t* player);
FLUIDSYNTH_API int fluid_player_join(fluid_player_t* player);
FLUIDSYNTH_API int fluid_player_set_loop(fluid_player_t* player, int loop);
FLUIDSYNTH_API int fluid_player_set_midi_tempo(fluid_player_t* player, int tempo);
FLUIDSYNTH_API int fluid_player_set_bpm(fluid_player_t* player, int bpm);

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_MIDI_H */
