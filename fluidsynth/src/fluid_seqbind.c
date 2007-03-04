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



/*
 2002 : API design by Peter Hanappe and Antoine Schmitt
 August 2002 : Implementation by Antoine Schmitt as@gratin.org
               as part of the infiniteCD author project
               http://www.infiniteCD.org/
*/

#include "fluidsynth_priv.h"

 /***************************************************************
 *
 *                           SEQUENCER BINDING
 */

void fluid_seq_fluidsynth_callback(unsigned int time, fluid_event_t* event, fluid_sequencer_t* seq, void* data);

/* registering the synth */
short fluid_sequencer_register_fluidsynth(fluid_sequencer_t* seq, fluid_synth_t* synth)
{
	/* register fluidsynth itself */
	return fluid_sequencer_register_client(seq, "fluidsynth", fluid_seq_fluidsynth_callback, (void *)synth);
}

/* the callback itself */
void fluid_seq_fluidsynth_callback(unsigned int time, fluid_event_t* evt, fluid_sequencer_t* seq, void* data)
{
	fluid_synth_t* synth = (fluid_synth_t *)data;

  switch (fluid_event_get_type(evt)) {

  case FLUID_SEQ_NOTEON:
  	fluid_synth_noteon(synth, fluid_event_get_channel(evt), fluid_event_get_key(evt), fluid_event_get_velocity(evt));
  	break;

  case FLUID_SEQ_NOTEOFF:
  	fluid_synth_noteoff(synth, fluid_event_get_channel(evt), fluid_event_get_key(evt));
  	break;

  case FLUID_SEQ_NOTE:
	  {
	  	unsigned int dur;
	  	fluid_synth_noteon(synth, fluid_event_get_channel(evt), fluid_event_get_key(evt), fluid_event_get_velocity(evt));
	  	dur = fluid_event_get_duration(evt);
	  	fluid_event_noteoff(evt, fluid_event_get_channel(evt), fluid_event_get_key(evt));
	  	fluid_sequencer_send_at(seq, evt, dur, 0);
	  }
  	break;

	case FLUID_SEQ_ALLSOUNDSOFF:
		/* NYI */
  	break;

  case FLUID_SEQ_ALLNOTESOFF:
  	fluid_synth_cc(synth, fluid_event_get_channel(evt), 0x7B, 0);
  	break;

  case FLUID_SEQ_BANKSELECT:
  	fluid_synth_bank_select(synth, fluid_event_get_channel(evt), fluid_event_get_bank(evt));
  	break;

  case FLUID_SEQ_PROGRAMCHANGE:
  	fluid_synth_program_change(synth, fluid_event_get_channel(evt), fluid_event_get_program(evt));
  	break;

  case FLUID_SEQ_PROGRAMSELECT:
  	fluid_synth_program_select(synth, fluid_event_get_channel(evt), fluid_event_get_sfont_id(evt),
		fluid_event_get_bank(evt), fluid_event_get_program(evt));
  	break;

  case FLUID_SEQ_ANYCONTROLCHANGE:
  	/* nothing = only used by remove_events */
  	break;

  case FLUID_SEQ_PITCHBEND:
  	fluid_synth_pitch_bend(synth, fluid_event_get_channel(evt), fluid_event_get_pitch(evt));
  	break;

  case FLUID_SEQ_PITCHWHHELSENS:
  	fluid_synth_pitch_wheel_sens(synth, fluid_event_get_channel(evt), fluid_event_get_value(evt));
  	break;

  case FLUID_SEQ_CONTROLCHANGE:
	 fluid_synth_cc(synth, fluid_event_get_channel(evt), fluid_event_get_control(evt), fluid_event_get_value(evt));
  	break;

  case FLUID_SEQ_MODULATION:
	  {
	  	short ctrl = 0x01;	// MODULATION_MSB
	  	fluid_synth_cc(synth, fluid_event_get_channel(evt), ctrl, fluid_event_get_value(evt));
	  }
  	break;

  case FLUID_SEQ_SUSTAIN:
	  {
	  	short ctrl = 0x40;	// SUSTAIN_SWITCH
	  	fluid_synth_cc(synth, fluid_event_get_channel(evt), ctrl, fluid_event_get_value(evt));
	  }
  	break;

  case FLUID_SEQ_PAN:
	  {
	  	short ctrl = 0x0A;	// PAN_MSB
	  	fluid_synth_cc(synth, fluid_event_get_channel(evt), ctrl, fluid_event_get_value(evt));
	  }
  	break;

  case FLUID_SEQ_VOLUME:
	  {
	  	short ctrl = 0x07;	// VOLUME_MSB
	  	fluid_synth_cc(synth, fluid_event_get_channel(evt), ctrl, fluid_event_get_value(evt));
	  }
  	break;

  case FLUID_SEQ_REVERBSEND:
	  {
	  	short ctrl = 0x5B;	// EFFECTS_DEPTH1
	  	fluid_synth_cc(synth, fluid_event_get_channel(evt), ctrl, fluid_event_get_value(evt));
	  }
  	break;

  case FLUID_SEQ_CHORUSSEND:
	  {
	  	short ctrl = 0x5D;	// EFFECTS_DEPTH3
	  	fluid_synth_cc(synth, fluid_event_get_channel(evt), ctrl, fluid_event_get_value(evt));
	  }
  	break;

  case FLUID_SEQ_TIMER:
	  /* nothing in fluidsynth */
  	break;

	default:
  	break;
	}
}


