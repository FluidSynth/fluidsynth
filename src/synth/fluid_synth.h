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


#ifndef _FLUID_SYNTH_H
#define _FLUID_SYNTH_H


/***************************************************************
 *
 *                         INCLUDES
 */

#include "fluid_sys.h"
#include "fluid_list.h"
#include "fluid_rev.h"
#include "fluid_voice.h"
#include "fluid_chorus.h"
#include "fluid_ladspa.h"
#include "fluid_midi_router.h"
#include "fluid_rvoice_event.h"

/***************************************************************
 *
 *                         DEFINES
 */
#define FLUID_NUM_PROGRAMS      128
#define DRUM_INST_BANK		128

#define FLUID_UNSET_PROGRAM     128     /* Program number used to unset a preset */


/***************************************************************
 *
 *                         ENUM
 */

/**
 * Bank Select MIDI message styles. Default style is GS.
 */
enum fluid_midi_bank_select
{
    FLUID_BANK_STYLE_GM,  /**< GM style, bank = 0 always (CC0/MSB and CC32/LSB ignored) */
    FLUID_BANK_STYLE_GS,  /**< GS style, bank = CC0/MSB (CC32/LSB ignored) */
    FLUID_BANK_STYLE_XG,  /**< XG style, bank = CC32/LSB (CC0/MSB ignored) */
    FLUID_BANK_STYLE_MMA  /**< MMA style bank = 128*MSB+LSB */
};

enum fluid_synth_status
{
  FLUID_SYNTH_CLEAN,
  FLUID_SYNTH_PLAYING,
  FLUID_SYNTH_QUIET,
  FLUID_SYNTH_STOPPED
};

#define SYNTH_REVERB_CHANNEL 0
#define SYNTH_CHORUS_CHANNEL 1

/*
 * Structure used for sfont_info field in #fluid_synth_t for each loaded
 * SoundFont with the SoundFont instance and additional fields.
 */
typedef struct _fluid_sfont_info_t {
  fluid_sfont_t *sfont; /**< Loaded SoundFont */
  fluid_synth_t *synth; /**< Parent synth */
  int refcount;         /**< SoundFont reference count (0 if no presets referencing it) */
  int bankofs;          /**< Bank offset */
} fluid_sfont_info_t;

/*
 * fluid_synth_t
 *
 * Mutual exclusion notes (as of 1.1.2):
 *
 * All variables are considered belongning to the "public API" thread,
 * which processes all MIDI, except for:
 *
 * ticks_since_start - atomic, set by rendering thread only
 * cpu_load - atomic, set by rendering thread only
 * cur, curmax, dither_index - used by rendering thread only
 * ladspa_fx - same instance copied in rendering thread. Synchronising handled internally.
 *
 */

struct _fluid_synth_t
{
  fluid_rec_mutex_t mutex;           /**< Lock for public API */
  int use_mutex;                     /**< Use mutex for all public API functions? */
  int public_api_count;              /**< How many times the mutex is currently locked */

  fluid_settings_t* settings;        /**< the synthesizer settings */
  int device_id;                     /**< Device ID used for SYSEX messages */
  int polyphony;                     /**< Maximum polyphony */
  int with_reverb;                   /**< Should the synth use the built-in reverb unit? */
  int with_chorus;                   /**< Should the synth use the built-in chorus unit? */
  int verbose;                       /**< Turn verbose mode on? */
  double sample_rate;                /**< The sample rate */
  int midi_channels;                 /**< the number of MIDI channels (>= 16) */
  int bank_select;                   /**< the style of Bank Select MIDI messages */
  int audio_channels;                /**< the number of audio channels (1 channel=left+right) */
  int audio_groups;                  /**< the number of (stereo) 'sub'groups from the synth.
					  Typically equal to audio_channels. */
  int effects_channels;              /**< the number of effects channels (>= 2) */
  int state;                         /**< the synthesizer state */
  fluid_atomic_uint_t ticks_since_start;    /**< the number of audio samples since the start */
  unsigned int start;                /**< the start in msec, as returned by system clock */
  fluid_overflow_prio_t overflow;    /**< parameters for overflow priority (aka voice-stealing) */

  fluid_list_t *loaders;             /**< the SoundFont loaders */
  fluid_list_t *sfont_info;          /**< List of fluid_sfont_info_t for each loaded SoundFont (remains until SoundFont is unloaded) */
  fluid_hashtable_t *sfont_hash;     /**< Hash of fluid_sfont_t->fluid_sfont_info_t (remains until SoundFont is deleted) */
  unsigned int sfont_id;             /**< Incrementing ID assigned to each loaded SoundFont */

  float gain;                        /**< master gain */
  fluid_channel_t** channel;         /**< the channels */
  int nvoice;                        /**< the length of the synthesis process array (max polyphony allowed) */
  fluid_voice_t** voice;             /**< the synthesis voices */
  int active_voice_count;            /**< count of active voices */
  unsigned int noteid;               /**< the id is incremented for every new note. it's used for noteoff's  */
  unsigned int storeid;
  fluid_rvoice_eventhandler_t* eventhandler;

  double reverb_roomsize;             /**< Shadow of reverb roomsize */
  double reverb_damping;              /**< Shadow of reverb damping */
  double reverb_width;                /**< Shadow of reverb width */
  double reverb_level;                /**< Shadow of reverb level */

  int chorus_nr;                     /**< Shadow of chorus number */
  double chorus_level;                /**< Shadow of chorus level */
  double chorus_speed;                /**< Shadow of chorus speed */
  double chorus_depth;                /**< Shadow of chorus depth */
  int chorus_type;                   /**< Shadow of chorus type */

  int cur;                           /**< the current sample in the audio buffers to be output */
  int curmax;                        /**< current amount of samples present in the audio buffers */
  int dither_index;		     /**< current index in random dither value buffer: fluid_synth_(write_s16|dither_s16) */

  fluid_atomic_float_t cpu_load;                    /**< CPU load in percent (CPU time required / audio synthesized time * 100) */

  fluid_tuning_t*** tuning;          /**< 128 banks of 128 programs for the tunings */
  fluid_private_t tuning_iter;       /**< Tuning iterators per each thread */

  fluid_sample_timer_t* sample_timers; /**< List of timers triggered before a block is processed */
  unsigned int min_note_length_ticks; /**< If note-offs are triggered just after a note-on, they will be delayed */

  int cores;                         /**< Number of CPU cores (1 by default) */

  fluid_mod_t* default_mod;          /**< the (dynamic) list of default modulators */

  fluid_ladspa_fx_t* ladspa_fx;      /**< Effects unit for LADSPA support */
};

fluid_preset_t* fluid_synth_find_preset(fluid_synth_t* synth,
				      unsigned int banknum,
				      unsigned int prognum);
void fluid_synth_sfont_unref (fluid_synth_t *synth, fluid_sfont_t *sfont);
				      

int fluid_synth_all_notes_off(fluid_synth_t* synth, int chan);
int fluid_synth_all_sounds_off(fluid_synth_t* synth, int chan);
int fluid_synth_kill_voice(fluid_synth_t* synth, fluid_voice_t * voice);

void fluid_synth_print_voice(fluid_synth_t* synth);

void fluid_synth_dither_s16(int *dither_index, int len, float* lin, float* rin,
			    void* lout, int loff, int lincr,
			    void* rout, int roff, int rincr);

int fluid_synth_reset_reverb(fluid_synth_t* synth);
int fluid_synth_set_reverb_preset(fluid_synth_t* synth, unsigned int num);
int fluid_synth_set_reverb_full(fluid_synth_t* synth, int set, double roomsize,
                                double damping, double width, double level);

int fluid_synth_reset_chorus(fluid_synth_t* synth);
int fluid_synth_set_chorus_full(fluid_synth_t* synth, int set, int nr, double level,
                                double speed, double depth_ms, int type);

fluid_sample_timer_t* new_fluid_sample_timer(fluid_synth_t* synth, fluid_timer_callback_t callback, void* data);
void delete_fluid_sample_timer(fluid_synth_t* synth, fluid_sample_timer_t* timer);

void fluid_synth_api_enter(fluid_synth_t* synth);
void fluid_synth_api_exit(fluid_synth_t* synth);

void fluid_synth_process_event_queue(fluid_synth_t* synth);

int fluid_synth_set_gen2 (fluid_synth_t* synth, int chan,
                                         int param, float value,
                                         int absolute, int normalized);
/*
 * misc
 */

void fluid_synth_settings(fluid_settings_t* settings);

#endif  /* _FLUID_SYNTH_H */
