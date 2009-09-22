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

#ifndef _FLUID_EVENT_QUEUE_H
#define _FLUID_EVENT_QUEUE_H

#include "fluid_sys.h"
#include "fluid_midi.h"

/**
 * Type of queued event.
 */
enum fluid_event_queue_elem
{
  FLUID_EVENT_QUEUE_ELEM_MIDI,          /**< MIDI event. Uses midi field of event value */
  FLUID_EVENT_QUEUE_ELEM_GAIN,          /**< Synth gain set or return event. Uses dval field of event value */
  FLUID_EVENT_QUEUE_ELEM_POLYPHONY,     /**< Synth polyphony event. Uses ival field of event value */
  FLUID_EVENT_QUEUE_ELEM_GEN,           /**< Generator event. Uses gen field of event value */
  FLUID_EVENT_QUEUE_ELEM_PRESET,        /**< Preset set event. Uses preset field of event value */
  FLUID_EVENT_QUEUE_ELEM_STOP_VOICES,   /**< Stop voices event. Uses ival field of event value */
  FLUID_EVENT_QUEUE_ELEM_REVERB,        /**< Reverb set or return event. Uses reverb field of event value */
  FLUID_EVENT_QUEUE_ELEM_CHORUS,        /**< Chorus set or return event. Uses chorus field of event value */
  FLUID_EVENT_QUEUE_ELEM_FREE_PRESET    /**< Free a preset return event.  Uses pval field of event value */
};

/**
 * SoundFont generator set event structure.
 */
typedef struct
{
  int channel;          /**< MIDI channel number */
  int param;            /**< FluidSynth generator ID */
  float value;          /**< Value for the generator (absolute or relative) */
  int absolute;         /**< 1 if value is absolute, 0 if relative */
} fluid_event_gen_t;

/**
 * Preset channel assignment event structure.
 */
typedef struct
{
  int channel;                  /**< MIDI channel number */
  fluid_preset_t *preset;       /**< Preset to assign (synth thread owns) */
} fluid_event_preset_t;

/**
 * Reverb assignment structure.
 */
typedef struct
{
  char set;           /**< Bit 0: roomsize, 1: damping, 2: width, 3: level */
  float roomsize;
  float damping;
  float width;
  float level;
} fluid_event_reverb_t;

/**
 * Chorus assignment structure.
 */
typedef struct
{
  char set;           /**< Bit 0: nr, 1: type, 2: level, 3: speed, 4: depth */
  char nr;
  char type;
  float level;
  float speed;
  float depth;
} fluid_event_chorus_t;


/**
 * Event queue element structure.
 */
typedef struct
{
  char type;            /**< #fluid_event_queue_elem */

  union
  {
    fluid_midi_event_t midi;    /**< If type == #FLUID_EVENT_QUEUE_ELEM_MIDI */
    fluid_event_gen_t gen;      /**< If type == #FLUID_EVENT_QUEUE_ELEM_GEN */
    fluid_event_preset_t preset;        /**< If type == #FLUID_EVENT_QUEUE_ELEM_PRESET */
    fluid_event_reverb_t reverb;        /**< If type == #FLUID_EVENT_QUEUE_ELEM_REVERB */
    fluid_event_chorus_t chorus;        /**< If type == #FLUID_EVENT_QUEUE_ELEM_CHORUS */
    double dval;                /**< A floating point payload value */
    int ival;                   /**< An integer payload value */
    void *pval;                 /**< A pointer payload value */
  };
} fluid_event_queue_elem_t;

/**
 * Lockless event queue instance.
 */
typedef struct
{
  fluid_event_queue_elem_t *array;  /**< Queue array of arbitrary size elements */
  int totalcount;       /**< Total count of elements in array */
  int count;            /**< Current count of elements */
  int in;               /**< Index in queue to store next pushed element */
  int out;              /**< Index in queue of next popped element */
  void *synth;          /**< Owning fluid_synth_t instance */
} fluid_event_queue_t;


fluid_event_queue_t *fluid_event_queue_new (int count);
void fluid_event_queue_free (fluid_event_queue_t *queue);
fluid_event_queue_elem_t *fluid_event_queue_get_inptr (fluid_event_queue_t *queue);
void fluid_event_queue_next_inptr (fluid_event_queue_t *queue);
fluid_event_queue_elem_t *fluid_event_queue_get_outptr (fluid_event_queue_t *queue);
void fluid_event_queue_next_outptr (fluid_event_queue_t *queue);

#endif /* _FLUID_EVENT_QUEUE_H */
