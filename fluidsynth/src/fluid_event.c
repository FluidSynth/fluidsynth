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
 Oct4.2002 : AS : corrected bug in heap allocation, that caused a crash during sequencer free.
*/

 
#include "fluid_event_priv.h"
#include "fluidsynth_priv.h"

/***************************************************************
 *
 *                           SEQUENCER EVENTS
 */

/* Event alloc/free */

fluid_event_t* 
new_fluid_event()
{
  fluid_event_t* evt;
  
  evt = FLUID_NEW(fluid_event_t);
  if (evt == NULL) {
    fluid_log(FLUID_PANIC, "event: Out of memory\n");
    return NULL;
  }

  FLUID_MEMSET(evt, 0, sizeof(fluid_event_t));
  
  // by default, no type
	evt->dest = -1;
	evt->src = -1;
	evt->type = -1;
	
	return(evt);
}

void 
delete_fluid_event(fluid_event_t* evt)
{

  if (evt == NULL) {
    return;
  }
  
  FLUID_FREE(evt);
}

/* Initializing events */
void
fluid_event_set_time(fluid_event_t* evt, unsigned int time)
{
	evt->time = time;
}

void
fluid_event_set_source(fluid_event_t* evt, short src)
{
	evt->src = src;
}

void
fluid_event_set_dest(fluid_event_t* evt, short dest)
{
	evt->dest = dest;
}

/* Timer events */
void
fluid_event_timer(fluid_event_t* evt, void* data)
{
	evt->type = FLUID_SEQ_TIMER;
	evt->data = data;
}


/* Note events */
void
fluid_event_noteon(fluid_event_t* evt, int channel, short key, short vel)
{
	evt->type = FLUID_SEQ_NOTEON;
	evt->channel = channel;
	evt->key = key;
	evt->vel = vel;
}

void
fluid_event_noteoff(fluid_event_t* evt, int channel, short key)
{
	evt->type = FLUID_SEQ_NOTEOFF;
	evt->channel = channel;
	evt->key = key;
}

void
fluid_event_note(fluid_event_t* evt, int channel, short key, short vel, unsigned int duration)
{
	evt->type = FLUID_SEQ_NOTE;
	evt->channel = channel;
	evt->key = key;
	evt->vel = vel;
	evt->duration = duration;
}

void
fluid_event_all_sounds_off(fluid_event_t* evt, int channel)
{
	evt->type = FLUID_SEQ_ALLSOUNDSOFF;
	evt->channel = channel;
}

void
fluid_event_all_notes_off(fluid_event_t* evt, int channel)
{
	evt->type = FLUID_SEQ_ALLNOTESOFF;
	evt->channel = channel;
}

void
fluid_event_bank_select(fluid_event_t* evt, int channel, short bank_num)
{
	evt->type = FLUID_SEQ_BANKSELECT;
	evt->channel = channel;
	evt->control = bank_num;
}

void
fluid_event_program_change(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_PROGRAMCHANGE;
	evt->channel = channel;
	evt->value = val;
}

void
fluid_event_program_select(fluid_event_t* evt, int channel,
						  unsigned int sfont_id, short bank_num, short preset_num)
{
	evt->type = FLUID_SEQ_PROGRAMSELECT;
	evt->channel = channel;
	evt->duration = sfont_id;
	evt->value = preset_num;
	evt->control = bank_num;
}

void
fluid_event_any_control_change(fluid_event_t* evt, int channel)
{
	evt->type = FLUID_SEQ_ANYCONTROLCHANGE;
	evt->channel = channel;
}

void
fluid_event_pitch_bend(fluid_event_t* evt, int channel, int pitch)
{
	evt->type = FLUID_SEQ_PITCHBEND;
	evt->channel = channel;
	if (pitch < 0) pitch = 0;
	if (pitch > 16383) pitch = 16383;
	evt->pitch = pitch;
}

void
fluid_event_pitch_wheelsens(fluid_event_t* evt, int channel, short value)
{
	evt->type = FLUID_SEQ_PITCHWHHELSENS;
	evt->channel = channel;
	evt->value = value;
}

void
fluid_event_modulation(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_MODULATION;
	evt->channel = channel;
	if (val < 0) val = 0;
	if (val > 127) val = 127;
	evt->value = val;
}

void
fluid_event_sustain(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_SUSTAIN;
	evt->channel = channel;
	if (val < 0) val = 0;
	if (val > 127) val = 127;
	evt->value = val;
}

void
fluid_event_control_change(fluid_event_t* evt, int channel, short control, short val)
{
	evt->type = FLUID_SEQ_CONTROLCHANGE;
	evt->channel = channel;
	evt->control = control;
	evt->value = val;
}

void
fluid_event_pan(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_PAN;
	evt->channel = channel;
	if (val < 0) val = 0;
	if (val > 127) val = 127;
	evt->value = val;
}

void
fluid_event_volume(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_VOLUME;
	evt->channel = channel;
	if (val < 0) val = 0;
	if (val > 127) val = 127;
	evt->value = val;
}

void
fluid_event_reverb_send(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_REVERBSEND;
	evt->channel = channel;
	if (val < 0) val = 0;
	if (val > 127) val = 127;
	evt->value = val;
}

void
fluid_event_chorus_send(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_CHORUSSEND;
	evt->channel = channel;
	if (val < 0) val = 0;
	if (val > 127) val = 127;
	evt->value = val;
}

/* Accessing event data */
int fluid_event_get_type(fluid_event_t* evt)
{
	return evt->type;
}

unsigned int fluid_event_get_time(fluid_event_t* evt)
{
	return evt->time;
}

short fluid_event_get_source(fluid_event_t* evt)
{
	return evt->src;
}

short fluid_event_get_dest(fluid_event_t* evt)
{
	return evt->dest;
}

int fluid_event_get_channel(fluid_event_t* evt)
{
	return evt->channel;
}

short fluid_event_get_key(fluid_event_t* evt)
{
	return evt->key;
}

short fluid_event_get_velocity(fluid_event_t* evt)

{
	return evt->vel;
}

short fluid_event_get_control(fluid_event_t* evt)
{
	return evt->control;
}

short fluid_event_get_value(fluid_event_t* evt)
{
	return evt->value;
}

void* fluid_event_get_data(fluid_event_t* evt)
{
	return evt->data;
}

unsigned int fluid_event_get_duration(fluid_event_t* evt)
{
	return evt->duration;
}
short fluid_event_get_bank(fluid_event_t* evt)
{
	return evt->control;
}
int fluid_event_get_pitch(fluid_event_t* evt)
{
	return evt->pitch;
}
short 
fluid_event_get_program(fluid_event_t* evt)
{
	return evt->value;
}
unsigned int 
fluid_event_get_sfont_id(fluid_event_t* evt)
{
	return evt->duration;
}



/********************/
/* heap management  */
/********************/

fluid_evt_heap_t*
_fluid_evt_heap_init(int nbEvents)
{
#ifdef HEAP_WITH_DYNALLOC

  int i;
  fluid_evt_heap_t* heap;
  fluid_evt_entry *tmp;

  heap = FLUID_NEW(fluid_evt_heap_t);
  if (heap == NULL) {
    fluid_log(FLUID_PANIC, "sequencer: Out of memory\n");
    return NULL;
  }

  heap->freelist = NULL;
  fluid_mutex_init(heap->mutex);

  /* LOCK */
  fluid_mutex_lock(heap->mutex);

  /* Allocate the event entries */
  for (i = 0; i < nbEvents; i++) {
    tmp = FLUID_NEW(fluid_evt_entry);
    tmp->next = heap->freelist;
    heap->freelist = tmp;
  }

  /* UNLOCK */
  fluid_mutex_unlock(heap->mutex);


#else
	int i;
	fluid_evt_heap_t* heap;
	int siz = 2*sizeof(fluid_evt_entry *) + sizeof(fluid_evt_entry)*nbEvents;
	
	heap = (fluid_evt_heap_t *)FLUID_MALLOC(siz);
  if (heap == NULL) {
    fluid_log(FLUID_PANIC, "sequencer: Out of memory\n");
    return NULL;
  }
  FLUID_MEMSET(heap, 0, siz);
  
  /* link all heap events */
  {
  	fluid_evt_entry *tmp = &(heap->pool);
	  for (i = 0 ; i < nbEvents - 1 ; i++)
 		 	tmp[i].next = &(tmp[i+1]);
 	 	tmp[nbEvents-1].next = NULL;
 	 	
 	 	/* set head & tail */
 	 	heap->tail = &(tmp[nbEvents-1]);
  	heap->head = &(heap->pool);
  }
#endif
  return (heap);
}

void
_fluid_evt_heap_free(fluid_evt_heap_t* heap)
{
#ifdef HEAP_WITH_DYNALLOC
  fluid_evt_entry *tmp, *next;

  /* LOCK */
  fluid_mutex_lock(heap->mutex);

  tmp = heap->freelist;
  while (tmp) {
    next = tmp->next;
    FLUID_FREE(tmp);
    tmp = next;
  }

  /* UNLOCK */
  fluid_mutex_unlock(heap->mutex);
  fluid_mutex_destroy(heap->mutex);

  FLUID_FREE(heap);
  
#else
	FLUID_FREE(heap);
#endif
}

fluid_evt_entry*
_fluid_seq_heap_get_free(fluid_evt_heap_t* heap)
{
#ifdef HEAP_WITH_DYNALLOC
  fluid_evt_entry* evt = NULL;

  /* LOCK */
  fluid_mutex_lock(heap->mutex);

#if !defined(MACOS9)
  if (heap->freelist == NULL) {
    heap->freelist = FLUID_NEW(fluid_evt_entry);
    if (heap->freelist != NULL) {
      heap->freelist->next = NULL;
    }
  }
#endif

  evt = heap->freelist;

  if (evt != NULL) {
    heap->freelist = heap->freelist->next;
    evt->next = NULL;
  }

  /* UNLOCK */
  fluid_mutex_unlock(heap->mutex);

  return evt;

#else
	fluid_evt_entry* evt;
	if (heap->head == NULL) return NULL;
	
	/* take from head of the heap */
	/* critical - should threadlock ? */
	evt = heap->head;
	heap->head = heap->head->next;
	
	return evt;
#endif
}

void
_fluid_seq_heap_set_free(fluid_evt_heap_t* heap, fluid_evt_entry* evt)
{
#ifdef HEAP_WITH_DYNALLOC

  /* LOCK */
  fluid_mutex_lock(heap->mutex);

  evt->next = heap->freelist;
  heap->freelist = evt;

  /* UNLOCK */
  fluid_mutex_unlock(heap->mutex);

#else
	/* append to the end of the heap */
	/* critical - should threadlock ? */
	heap->tail->next = evt;
	heap->tail = evt;
	evt->next = NULL;
#endif
}
