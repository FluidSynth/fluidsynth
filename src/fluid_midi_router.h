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

/* Author: Markus Nentwig, nentwig@users.sourceforge.net
 */

#ifndef _FLUID_MIDIROUTER_H
#define _FLUID_MIDIROUTER_H

#include "fluidsynth_priv.h"
#include "fluid_midi.h"
#include "fluid_sys.h"



void fluid_midi_router_destroy_all_rules(fluid_midi_router_t* router);
fluid_midi_router_rule_t* new_fluid_midi_router_rule(void);

int fluid_midi_router_handle_clear(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);
int fluid_midi_router_handle_default(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);
int fluid_midi_router_handle_begin(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);
int fluid_midi_router_handle_chan(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);
int fluid_midi_router_handle_par1(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);
int fluid_midi_router_handle_par2(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);
int fluid_midi_router_handle_end(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);

int fluid_midi_router_begin(fluid_midi_router_t* router, fluid_midi_router_rule_t** dest);
int fluid_midi_router_end(fluid_midi_router_t* router);
int fluid_midi_router_create_default_rules(fluid_midi_router_t* router);
void fluid_midi_router_disable_all_rules(fluid_midi_router_t* router);
void fluid_midi_router_free_unused_rules(fluid_midi_router_t* router);

/*
 * fluid_midi_router
 */
struct _fluid_midi_router_t {
  fluid_synth_t* synth;

  fluid_midi_router_rule_t* note_rules;
  fluid_midi_router_rule_t* cc_rules;
  fluid_midi_router_rule_t* progchange_rules;
  fluid_midi_router_rule_t* pitchbend_rules;
  fluid_midi_router_rule_t* channel_pressure_rules;
  fluid_midi_router_rule_t* key_pressure_rules;

  int new_rule_chan_min;
  int new_rule_chan_max;
  double new_rule_chan_mul;
  int new_rule_chan_add;
  int new_rule_par1_min;
  int new_rule_par1_max;
  double new_rule_par1_mul;
  int new_rule_par1_add;
  int new_rule_par2_min;
  int new_rule_par2_max;
  double new_rule_par2_mul;
  int new_rule_par2_add;

  fluid_midi_router_rule_t** dest;

  handle_midi_event_func_t event_handler;    /* Callback function for generated events */
  void* event_handler_data;                  /* One arg for the callback */

  int nr_midi_channels;                      /* For clipping the midi channel */
  fluid_mutex_t ruletables_mutex;
};

struct _fluid_midi_router_rule_t {
    int chan_min;                             /* Channel window, for which this rule is valid */
    int chan_max;
    fluid_real_t chan_mul;                     /* Channel multiplier (usually 0 or 1) */
    int chan_add;                             /* Channel offset */

    int par1_min;                             /* Parameter 1 window, for which this rule is valid */
    int par1_max;
    fluid_real_t par1_mul;
    int par1_add;

    int par2_min;                              /* Parameter 2 window, for which this rule is valid */
    int par2_max;
    fluid_real_t par2_mul;
    int par2_add;

    int pending_events;                        /* In case of noteon: How many keys are still down? */
    char keys_cc[128];                         /* Flags, whether a key is down / controller is set (sustain) */
    fluid_midi_router_rule_t* next;             /* next entry */
    int state;                                 /* Does this rule expire, as soon as (for example) all keys are up? */
};

enum fluid_midirule_state {
    MIDIRULE_ACTIVE = 0x00,
    MIDIRULE_WAITING,
    MIDIRULE_DONE
};
#endif
