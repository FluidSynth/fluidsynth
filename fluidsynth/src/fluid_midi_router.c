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

#include "fluid_midi_router.h"
#include "fluid_midi.h"
#include "fluid_synth.h"
#include "fluid_io.h"

/**
 * Create a new midi router.
 * @param settings Settings used to configure MIDI router
 * @param handler MIDI event callback
 * @param event_handler_data Caller defined data pointer which gets passed to 'handler'
 * @return New MIDI router instance or NULL on error
 *
 * A midi handler connects to a midi input
 * device and forwards incoming midi events to the synthesizer.
 */
fluid_midi_router_t*
new_fluid_midi_router(fluid_settings_t* settings, handle_midi_event_func_t handler, void* event_handler_data)
{
  fluid_midi_router_t* router=NULL;
  fluid_midi_router_rule_t* rule=NULL;

  /* create the router */
  router = FLUID_NEW(fluid_midi_router_t); if (router == NULL){
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  };

  /* Clear the router, so that error_recovery can safely free all the rules.
   * Dump functions are also NULLed.
   */
  FLUID_MEMSET(router, 0, sizeof(fluid_midi_router_t));
  /* Retrieve the number of MIDI channels for range limiting */
  fluid_settings_getint(settings, "synth.midi-channels", &router->nr_midi_channels);

  fluid_mutex_init(router->ruletables_mutex);

  router->synth = (fluid_synth_t*) event_handler_data;
  router->event_handler=handler;
  router->event_handler_data=event_handler_data;

  /* Create the default routing rules
  * They accept events on any channel for any range of parameters,
  * and route them unchanged ("result=par*1.0+0") */

  if (fluid_midi_router_create_default_rules(router) != FLUID_OK) goto error_recovery;

  return router;

 error_recovery:
  FLUID_LOG(FLUID_ERR, "new_fluid_midi_router failed");
  fluid_midi_router_destroy_all_rules(router);
  FLUID_FREE(router);
  return NULL;
}

/**
 * Delete a MIDI router instance.
 * @param router MIDI router to delete
 * @return Always returns 0
 */
int
delete_fluid_midi_router(fluid_midi_router_t* router)
{
  if (router == NULL) {
    return FLUID_OK;
  }
  fluid_midi_router_destroy_all_rules(router);
  FLUID_FREE(router);
  return FLUID_OK;
}

/*
 * fluid_midi_router_destroy_all_rules(fluid_midi_router_t* router)
 * Purpose:
 * Frees the used memory. This is used only for shutdown!
 */
void fluid_midi_router_destroy_all_rules(fluid_midi_router_t* router){
  fluid_midi_router_rule_t* rules[6];
  fluid_midi_router_rule_t* current_rule;
  fluid_midi_router_rule_t* next_rule;
  int i;

  rules[0]=router->note_rules;
  rules[1]=router->cc_rules;
  rules[2]=router->progchange_rules;
  rules[3]=router->pitchbend_rules;
  rules[4]=router->channel_pressure_rules;
  rules[5]=router->key_pressure_rules;
  for (i=0; i < 6; i++){
    current_rule=rules[i];
    while (current_rule){
      next_rule=current_rule->next;
      FLUID_FREE(current_rule);
      current_rule=next_rule;
    };
  };
}

/*
 * new_fluid_midi_router_rule
 */
fluid_midi_router_rule_t* new_fluid_midi_router_rule(void)
{
  fluid_midi_router_rule_t* rule=FLUID_NEW(fluid_midi_router_rule_t);
  if (rule == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  FLUID_MEMSET(rule, 0, sizeof(fluid_midi_router_rule_t));
  return rule;
};

/*
 * delete_fluid_midi_router_rule
 */
int
delete_fluid_midi_router_rule(fluid_midi_router_rule_t* rule)
{
  FLUID_FREE(rule);
  return FLUID_OK;
}


int fluid_midi_router_create_default_rules(fluid_midi_router_t* router)
{
  fluid_midi_router_rule_t** rules[6];
  int i;

  rules[0]=&router->note_rules;
  rules[1]=&router->cc_rules;
  rules[2]=&router->progchange_rules;
  rules[3]=&router->pitchbend_rules;
  rules[4]=&router->channel_pressure_rules;
  rules[5]=&router->key_pressure_rules;

  for (i=0; i < 6; i++){
    /* Start a new rule. rules[i] is the destination. _end will insert
     * a new rule there into the linked list.
     */
    if (fluid_midi_router_begin(router, rules[i]) != FLUID_OK) goto error_recovery;
    if (fluid_midi_router_end(router) != FLUID_OK) goto error_recovery;
  };

  return FLUID_OK;
 error_recovery:
  FLUID_LOG(FLUID_ERR, "fluid_midi_router_create_default_rules failed");
  return FLUID_FAILED;
};

/* Purpose:
 * Resets the new-rule registers in 'router' to their default values.
 * Stores the destination (the linked list into which the new rule is inserted by _end)
 */
int fluid_midi_router_begin(fluid_midi_router_t* router, fluid_midi_router_rule_t** dest)
{
  if (!dest) goto error_recovery;
  router->dest=dest;

  /* In the days before the router, MIDI data went straight to the synth.
   * So there is no need to check for reasonable values here.
   */
  router->new_rule_chan_min=0;
  router->new_rule_chan_max=999999;
  router->new_rule_chan_mul=1.;
  router->new_rule_chan_add=0;
  router->new_rule_par1_min=0;
  router->new_rule_par1_max=999999;
  router->new_rule_par1_mul=1.;
  router->new_rule_par1_add=0;
  router->new_rule_par2_min=0;
  router->new_rule_par2_max=999999;
  router->new_rule_par2_mul=1.;
  router->new_rule_par2_add=0;

  return FLUID_OK;
 error_recovery:
  FLUID_LOG(FLUID_ERR, "fluid_midi_router_begin failed");
  return FLUID_FAILED;
};

/* Purpose:
 * Concludes a sequence of commands:
 * - router_start type
 * - router_chan
 * - router_par1
 * - router_par2
 * - router_end (this call).
 * Creates a new event from the given parameters and stores it in
 * the correct list.
 */

int fluid_midi_router_end(fluid_midi_router_t* router){
  fluid_midi_router_rule_t* rule=new_fluid_midi_router_rule(); if (rule == NULL) goto error_recovery;

  rule->chan_min=router->new_rule_chan_min;
  rule->chan_max=router->new_rule_chan_max;
  rule->chan_mul=router->new_rule_chan_mul;
  rule->chan_add=router->new_rule_chan_add;
  rule->par1_min=router->new_rule_par1_min;
  rule->par1_max=router->new_rule_par1_max;
  rule->par1_mul=router->new_rule_par1_mul;
  rule->par1_add=router->new_rule_par1_add;
  rule->par2_min=router->new_rule_par2_min;
  rule->par2_max=router->new_rule_par2_max;
  rule->par2_mul=router->new_rule_par2_mul;
  rule->par2_add=router->new_rule_par2_add;

  /* Now we modify the rules table. Lock it to make sure, that the RT
   * thread is not in the middle of an event. Also prevent, that some
   * other thread modifies the table at the same time.
   */

  fluid_mutex_lock(router->ruletables_mutex);

  rule->next= *(router->dest);
  *router->dest=rule;

  fluid_mutex_unlock(router->ruletables_mutex);

  return FLUID_OK;

 error_recovery:
  FLUID_LOG(FLUID_ERR, "fluid_midi_router_end failed");
  delete_fluid_midi_router_rule(rule);
  return FLUID_FAILED;
};

/**
 * Handle a MIDI event through a MIDI router instance.
 * @param data MIDI router instance #fluid_midi_router_t (DOCME why is it a void *?)
 * @param event MIDI event to handle
 * @return 0 on success, -1 otherwise
 *
 * Purpose: The midi router is called for each event, that is received
 * via the 'physical' midi input. Each event can trigger an arbitrary number
 * of generated events.
 *
 * In default mode, a noteon event is just forwarded to the synth's 'noteon' function,
 * a 'CC' event to the synth's 'CC' function and so on.
 *
 * The router can be used to
 * - filter messages (for example: Pass sustain pedal CCs only to selected channels),
 * - split the keyboard (noteon with notenr < x: to ch 1, >x to ch 2),
 * - layer sounds (for each noteon received on ch 1, create a noteon on ch1, ch2, ch3,...)
 * - velocity scaling (for each noteon event, scale the velocity by 1.27 to give DX7 users
 *   a chance)
 * - velocity switching ("v <=100: Angel Choir; V > 100: Hell's Bells")
 * - get rid of aftertouch
 * - ...
 */
int
fluid_midi_router_handle_midi_event(void* data, fluid_midi_event_t* event)
{
  fluid_midi_router_t* router=(fluid_midi_router_t*)data;
  fluid_synth_t* synth = router->synth;
  fluid_midi_router_rule_t* rule=NULL;
  fluid_midi_router_rule_t* next_rule=NULL;
  int event_has_par2=0; /* Flag, indicates that current event needs two parameters */
  int negative_event=0; /* Flag, indicates that current event releases a key / pedal */
  int par1_max=127;     /* Range limit for par1 */
  int par2_max=127;     /* Range limit for par2 */
  int ret_val=FLUID_OK;

  /* Some keyboards report noteoff through a noteon event with vel=0.
   * Convert those to noteoff to ease processing.*/
  if (event->type == NOTE_ON && event->param2 == 0) {
    /* Channel: Remains the same
     * Param 1: Note number, remains the same
     * Param 2: release velocity), set to max
     */
    event->type = NOTE_OFF;
    event->param2=127;
  };

  /* Lock the rules table, so that for example the shell thread doesn't
   * clear the rules we are just working with */
  fluid_mutex_lock(router->ruletables_mutex);

  /* Depending on the event type, choose the correct table of rules.
   * Also invoke the appropriate callback function for the event type
   * to notify external applications.
   * Note: An event type not listed here simply won't find any rules and
   * will be ignored.
   */
  switch (event->type) {
      case NOTE_ON:
	rule=router->note_rules;
	event_has_par2=1;
	break;
      case NOTE_OFF:
	rule=router->note_rules;
	event_has_par2=1;
	break;
      case CONTROL_CHANGE:
	rule=router->cc_rules;
	event_has_par2=1;
	break;
      case PROGRAM_CHANGE:
	rule=router->progchange_rules;
	break;
      case PITCH_BEND:
	rule=router->pitchbend_rules;
	par1_max=16383;
	break;
      case CHANNEL_PRESSURE:
	rule=router->channel_pressure_rules;
	break;
      case KEY_PRESSURE:
	rule=router->key_pressure_rules;
	event_has_par2=1;
	break;
      case MIDI_SYSTEM_RESET:
        return router->event_handler(router->event_handler_data,event);
      default:
	break;
  }

  /* At this point 'rule' contains the first rule in a linked list.
   * Check for all rules, whether channel and parameter ranges match.
   */

  while (rule){
    int chan; /* Channel of the generated event */
    int par1; /* par1 of the generated event */
    int par2=0;
    int event_par1=(int)event->param1;
    int event_par2=(int)event->param2;
    fluid_midi_event_t new_event;

    /* Store the pointer to the next rule right now. If the rule is later flagged for destruction,
     * it may not be accessed anymore.
     */
    next_rule=rule->next;

    /* Check, whether the rule is still active. Expired rules cannot be removed immediately,
     * because freeing memory in a realtime thread is no good idea. And the MIDI thread,
     * which calls us here, has raised priority.*/
    if (rule->state == MIDIRULE_DONE){
      goto do_next_rule;
    };

    /* Channel window */
    if (rule->chan_min > rule->chan_max){
      /* Inverted rule: Exclude everything between max and min (but not min/max) */
      if (event->channel > rule->chan_max && event->channel < rule->chan_min){
	goto do_next_rule;
      }
    } else {
      /* Normal rule: Exclude everything < max or > min (but not min/max) */
      if (event->channel > rule->chan_max || event->channel < rule->chan_min){
	goto do_next_rule;
      }
    };

    /* Par 1 window */
    if (rule->par1_min > rule->par1_max){
      /* Inverted rule: Exclude everything between max and min (but not min/max) */
      if (event_par1 > rule->par1_max && event_par1 < rule->par1_min){
	goto do_next_rule;
      }
    } else {
      /* Normal rule: Exclude everything < max or > min (but not min/max)*/
      if (event_par1 > rule->par1_max || event_par1 < rule->par1_min){
	goto do_next_rule;
      }
    };

    /* Par 2 window (only applies to event types, which have 2 pars)
     * For noteoff events, velocity switching doesn't make any sense.
     * Velocity scaling might be useful, though.
     */
    if (event_has_par2 && event->type != NOTE_OFF){
      if (rule->par2_min > rule->par2_max){
	/* Inverted rule: Exclude everything between max and min (but not min/max) */
	if (event_par2 > rule->par2_max && event_par2 < rule->par2_min){
	  goto do_next_rule;
	}
      } else {
	/* Normal rule: Exclude everything < max or > min (but not min/max)*/
	if (event_par2 > rule->par2_max || event_par2 < rule->par2_min){
	  goto do_next_rule;
	};
      };
    };

    /* Channel scaling / offset
     * Note: rule->chan_mul will probably be 0 or 1. If it's 0, input from all
     * input channels is mapped to the same synth channel.
     */
    chan=(int)((fluid_real_t)event->channel * (fluid_real_t)rule->chan_mul + (fluid_real_t)rule->chan_add + 0.5);

    /* Par 1 scaling / offset */
    par1=(int)((fluid_real_t)event_par1 * (fluid_real_t)rule->par1_mul + (fluid_real_t)rule->par1_add + 0.5);

    /* Par 2 scaling / offset, if applicable */
    if (event_has_par2){
      par2=(int)((fluid_real_t)event_par2 * (fluid_real_t)rule->par2_mul + (fluid_real_t)rule->par2_add + 0.5);
    };

    /* Channel range limiting */
    if (chan < 0){
      chan=0;
      /* Upper limit is hard to implement, because the number of MIDI channels can be changed via settings any time. */
    };

    /* Par1 range limiting */
    if (par1 < 0){
      par1=0;
    } else if (par1 > par1_max){
      par1=par1_max;
    };

    /* Par2 range limiting */
    if (event_has_par2){
      if (par2 < 0){
	par2=0;
      } else if (par2 > par2_max){
	par2=par2_max;
      };
    };

    /* At this point we have to create an event of event->type on 'chan' with par1 (maybe par2).
     * We keep track on the state of noteon and sustain pedal events. If the application tries
     * to delete a rule, it will only be fully removed, if pending noteoff / pedal off events have
     * arrived. In the meantime (MIDIRULE_WAITING) state, it will only let through 'negative' events
     * (noteoff or pedal up).
     */

    if (
      event->type == NOTE_ON
      || (event->type == CONTROL_CHANGE && par1 == SUSTAIN_SWITCH && par2 >= 64)
      ){
      /* Noteon or sustain pedal down event generated */
      if (rule->keys_cc[par1] == 0){
	rule->keys_cc[par1]=1;
	rule->pending_events++;
      };
    } else if (
      event->type == NOTE_OFF
      || (event->type == CONTROL_CHANGE && par1 == SUSTAIN_SWITCH && par2 < 64)
      ){
      /* Noteoff or sustain pedal up event generated */
      if (rule->keys_cc[par1] > 0){
	rule->keys_cc[par1]=0;
	rule->pending_events--;
	negative_event=1;
      };
    };

    if (rule->state == MIDIRULE_WAITING){
      if (negative_event){
	if (rule->pending_events == 0){
	  /* There are no more pending events in the rule - all keys are up.
	   * Change its state to 'unused', it will be cleared up at the next
	   * opportunity to run 'free' safely.
	   * Note: After this, the rule may disappear at any time
	   */
	    rule->state=MIDIRULE_DONE;
	};
      } else {

	/* This rule only exists because of some unfinished business:
	 * There is still a note or sustain pedal waiting to be
	 * released. But the event that came in does not release any.
	 * So we just ignore the event and go on with the next rule.
	 */
	goto do_next_rule;
      };
    };

    /* At this point it is decided, what is sent to the synth.
     * Create a new event and make the appropriate call
     */

    fluid_midi_event_set_type(&new_event, event->type);
    fluid_midi_event_set_channel(&new_event, chan);
/*     fluid_midi_event_set_param1(&new_event, par1); */
/*     fluid_midi_event_set_param2(&new_event, par2); */
    new_event.param1 = par1;
    new_event.param2 = par2;

    /* Don't know what to do with return values: What is, if
     * generating one event fails?  Current solution: Make all calls,
     * regardless of failures, but report failure to the caller, as
     * soon as only one call fails.  Strategy should be thought
     * through.
     */
    if (router->event_handler(router->event_handler_data, &new_event) != FLUID_OK) {
      ret_val=FLUID_FAILED;
    }

  do_next_rule:
    rule=next_rule;
  };

  /* We are finished with the rule tables. Allow the other threads to do their job. */
  fluid_mutex_unlock(router->ruletables_mutex);

  return ret_val;
}

int fluid_midi_router_handle_clear(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out)
{
  fluid_midi_router_t* router=synth->midi_router;

  if (ac != 0) {
    fluid_ostream_printf(out, "router_clear needs no arguments.\n");
    goto error_recovery;
  }

  /* Disable rules and mark for destruction */
  fluid_midi_router_disable_all_rules(router);

  /* Free unused rules */
  fluid_midi_router_free_unused_rules(router);

  return 0;
 error_recovery:
  return -1;
};

int fluid_midi_router_handle_default(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out)
{
  fluid_midi_router_t* router=synth->midi_router;

  if (ac != 0) {
    fluid_ostream_printf(out, "router_default needs no arguments.\n");
    return -1;
  }

  /* Disable rules and mark for destruction */
  fluid_midi_router_disable_all_rules(router);

  /* Create default rules */
  if (fluid_midi_router_create_default_rules(router) != FLUID_OK){
    FLUID_LOG(FLUID_ERR, "create_default_rules failed");
    goto error_recovery;
  };

  /* Free unused rules */
  fluid_midi_router_free_unused_rules(router);

  return 0;
 error_recovery:
  return -1;
};

int fluid_midi_router_handle_begin(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out)
{
  fluid_midi_router_t* router=synth->midi_router;
  fluid_midi_router_rule_t** dest=NULL;

  if (ac != 1) {
    fluid_ostream_printf(out, "router_begin needs no arguments.\n");
      goto error_recovery;
  }

  if (FLUID_STRCMP(av[0],"note") == 0){
    dest=& router->note_rules;
  } else if (FLUID_STRCMP(av[0],"cc") == 0){
    dest=& router->cc_rules;
  } else if (FLUID_STRCMP(av[0],"prog") == 0){
    dest=& router->progchange_rules;
  } else if (FLUID_STRCMP(av[0],"pbend") == 0){
    dest=& router->pitchbend_rules;
  } else if (FLUID_STRCMP(av[0],"cpress") == 0){
    dest=& router->channel_pressure_rules;
  } else if (FLUID_STRCMP(av[0],"kpress") == 0){
    dest=& router->key_pressure_rules;
  };

  if (dest == NULL){
      fluid_ostream_printf(out, "router_begin args: note, cc, prog, pbend, cpress, kpress\n");
      goto error_recovery;
  };

  if (fluid_midi_router_begin(router, dest) != FLUID_OK){
    goto error_recovery;
  };

  /* Free unused rules (give it a try) */
  fluid_midi_router_free_unused_rules(router);

  return 0;

 error_recovery:
  return -1;
};

int fluid_midi_router_handle_end(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out)
{
  fluid_midi_router_t* router=synth->midi_router;

  if (ac != 0) {
    fluid_ostream_printf(out, "router_end needs no arguments.");
    goto error_recovery;
  }

  if (fluid_midi_router_end(router) != FLUID_OK){
    FLUID_LOG(FLUID_ERR, "midi_router_end failed");
    goto error_recovery;
  };

  /* Free unused rules (give it a try) */
  fluid_midi_router_free_unused_rules(router);

  return 0;

 error_recovery:
  return -1;
};

int fluid_midi_router_handle_chan(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out)
{
  fluid_midi_router_t* router=synth->midi_router;

  if (ac != 4) {
    fluid_ostream_printf(out, "router_chan needs four args: min, max, mul, add.");
    goto error_recovery;
  }

  router->new_rule_chan_min=atoi(av[0]);
  router->new_rule_chan_max=atoi(av[1]);
  router->new_rule_chan_mul=atoi(av[2]);
  router->new_rule_chan_add=atoi(av[3]);

  /* Free unused rules (give it a try) */
  fluid_midi_router_free_unused_rules(router);

  return 0;

 error_recovery:
  return -1;
};

int fluid_midi_router_handle_par1(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out)
{
  fluid_midi_router_t* router=synth->midi_router;

  if (ac != 4) {
    fluid_ostream_printf(out, "router_par1 needs four args: min, max, mul, add.");
    goto error_recovery;
  }

  router->new_rule_par1_min=atoi(av[0]);
  router->new_rule_par1_max=atoi(av[1]);
  router->new_rule_par1_mul=atoi(av[2]);
  router->new_rule_par1_add=atoi(av[3]);

  /* Free unused rules (give it a try) */
  fluid_midi_router_free_unused_rules(router);

  return 0;

 error_recovery:
  return -1;
};

int fluid_midi_router_handle_par2(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out)
{
  fluid_midi_router_t* router=synth->midi_router;

  if (ac != 4) {
    fluid_ostream_printf(out, "router_par2 needs four args: min, max, mul, add.");
    goto error_recovery;
  }

  router->new_rule_par2_min=atoi(av[0]);
  router->new_rule_par2_max=atoi(av[1]);
  router->new_rule_par2_mul=atoi(av[2]);
  router->new_rule_par2_add=atoi(av[3]);

  /* Free unused rules (give it a try) */
  fluid_midi_router_free_unused_rules(router);
  return 0;

 error_recovery:
  return -1;
};

void fluid_midi_router_disable_all_rules(fluid_midi_router_t* router)
{
  /* Go through all rules. If the rule has no outstanding events, then
   * flag it for destruction. Otherwise change it to wait mode. Then
   * it will handle the pending events, and flag itself for destruction
   * as soon as all events have arrived.
   */

  fluid_midi_router_rule_t* rules[6];
  fluid_midi_router_rule_t* current_rule;
  int i;
  rules[0]=router->note_rules;
  rules[1]=router->cc_rules;
  rules[2]=router->progchange_rules;
  rules[3]=router->pitchbend_rules;
  rules[4]=router->channel_pressure_rules;
  rules[5]=router->key_pressure_rules;

  /* Lock the rules table. We live on the assumption, that the rules
   * table does not change while we are chewing at it.
   * Changes between the processing of note / cc etc are permitted.
   */
  for (i=0; i < 6; i++){
    fluid_mutex_lock(router->ruletables_mutex);
    current_rule=rules[i];
    while (current_rule){
      if (current_rule->pending_events == 0){
	/* Flag for destruction */
	current_rule->state=MIDIRULE_DONE;
      } else {
	/* Wait mode */
	current_rule->state=MIDIRULE_WAITING;
      };
      current_rule=current_rule->next;
    };
    fluid_mutex_unlock(router->ruletables_mutex);
  };
};

void fluid_midi_router_free_unused_rules(fluid_midi_router_t* router)
{
  int i;

  for (i=0; i < 6; i++){
    fluid_midi_router_rule_t** p=NULL;

    /* We assume, that the table does not change while we are at it.
     * Between different types (note, cc etc) we can allow changes.
     */
    fluid_mutex_lock(router->ruletables_mutex);
    switch(i){
	case 0:
	  p=&router->note_rules;
	  break;
	case 1:
	  p=&router->cc_rules;
	  break;
	case 2:
	  p=&router->progchange_rules;
	  break;
	case 3:
	  p=&router->pitchbend_rules;
	  break;
	case 4:
	  p=&router->channel_pressure_rules;
	  break;
	case 5:
	  p=&router->key_pressure_rules;
	  break;
	default:
	  break;
    };

    while (*p){
      fluid_midi_router_rule_t* current_rule=*p;
      fluid_midi_router_rule_t* next_rule=current_rule->next;

      if (current_rule->state == MIDIRULE_DONE){
	/* p points to current_rule.
	 * current_rule->next points to next_rule.
	 * Unlink current_rule from the chain by setting the content
	 * of p to next_rule.
	 */

	*p=next_rule;

	/* Now the rule is not in the chain anymore. Destroy it. */
	delete_fluid_midi_router_rule(current_rule);

      } else {
	/* We have to keep the rule, there is still unfinished business. */
	p = &current_rule->next;
      };
    };
    fluid_mutex_unlock(router->ruletables_mutex);
  };
};

/**
 * MIDI event callback function to display event information to stdout
 * @param data MIDI router instance
 * @param event MIDI event data
 * @return 0 on success, -1 otherwise
 *
 * An implementation of the #handle_midi_event_func_t function type, used for
 * displaying MIDI event information between the MIDI driver and router to
 * stdout.  Useful for adding into a MIDI router chain for debugging MIDI events.
 */
int fluid_midi_dump_prerouter(void* data, fluid_midi_event_t* event)
{
  switch (event->type) {
      case NOTE_ON:
	fprintf(stdout, "event_pre_noteon %i %i %i\n",
		event->channel, event->param1, event->param2);
	break;
      case NOTE_OFF:
	fprintf(stdout, "event_pre_noteoff %i %i %i\n",
		event->channel, event->param1, event->param2);
	break;
      case CONTROL_CHANGE:
	fprintf(stdout, "event_pre_cc %i %i %i\n",
		event->channel, event->param1, event->param2);
	break;
      case PROGRAM_CHANGE:
	fprintf(stdout, "event_pre_prog %i %i\n", event->channel, event->param1);
	break;
      case PITCH_BEND:
        fprintf(stdout, "event_pre_pitch %i %i\n", event->channel, event->param1);
	break;
      case CHANNEL_PRESSURE:
	fprintf(stdout, "event_pre_cpress %i %i\n", event->channel, event->param1);
	break;
      case KEY_PRESSURE:
	fprintf(stdout, "event_pre_kpress %i %i %i\n",
		event->channel, event->param1, event->param2);
	break;
      default:
	break;
  };
  return fluid_midi_router_handle_midi_event((fluid_midi_router_t*) data, event);
};

/**
 * MIDI event callback function to display event information to stdout
 * @param data MIDI router instance
 * @param event MIDI event data
 * @return 0 on success, -1 otherwise
 *
 * An implementation of the #handle_midi_event_func_t function type, used for
 * displaying MIDI event information between the MIDI driver and router to
 * stdout.  Useful for adding into a MIDI router chain for debugging MIDI events.
 */
int fluid_midi_dump_postrouter(void* data, fluid_midi_event_t* event)
{
  switch (event->type) {
      case NOTE_ON:
	fprintf(stdout, "event_post_noteon %i %i %i\n",
		event->channel, event->param1, event->param2);
	break;
      case NOTE_OFF:
	fprintf(stdout, "event_post_noteoff %i %i %i\n",
		event->channel, event->param1, event->param2);
	break;
      case CONTROL_CHANGE:
	fprintf(stdout, "event_post_cc %i %i %i\n",
		event->channel, event->param1, event->param2);
	break;
      case PROGRAM_CHANGE:
	fprintf(stdout, "event_post_prog %i %i\n", event->channel, event->param1);
	break;
      case PITCH_BEND:
	fprintf(stdout, "event_post_pitch %i %i\n", event->channel, event->param1);
	break;
      case CHANNEL_PRESSURE:
	fprintf(stdout, "event_post_cpress %i %i\n", event->channel, event->param1);
	break;
      case KEY_PRESSURE:
	fprintf(stdout, "event_post_kpress %i %i %i\n",
		event->channel, event->param1, event->param2);
	break;
      default:
	break;
  };
  return fluid_synth_handle_midi_event((fluid_synth_t*) data, event);
};
