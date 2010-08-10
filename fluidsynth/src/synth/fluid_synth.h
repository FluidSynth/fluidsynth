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


#ifndef _FLUID_SYNTH_H
#define _FLUID_SYNTH_H


/***************************************************************
 *
 *                         INCLUDES
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include "fluidsynth_priv.h"
#include "fluid_event_queue.h"
#include "fluid_list.h"
#include "fluid_rev.h"
#include "fluid_voice.h"
#include "fluid_chorus.h"
#include "fluid_ladspa.h"
#include "fluid_midi_router.h"
#include "fluid_sys.h"
#include "fluid_rvoice_event.h"

/***************************************************************
 *
 *                         DEFINES
 */
#define FLUID_NUM_PROGRAMS      128
#define DRUM_INST_BANK		128

#define FLUID_UNSET_PROGRAM     128     /* Program number used to unset a preset */

#if defined(WITH_FLOAT)
#define FLUID_SAMPLE_FORMAT     FLUID_SAMPLE_FLOAT
#else
#define FLUID_SAMPLE_FORMAT     FLUID_SAMPLE_DOUBLE
#endif


/***************************************************************
 *
 *                         ENUM
 */
/*enum fluid_loop {
  FLUID_UNLOOPED = 0,
  FLUID_LOOP_DURING_RELEASE = 1,
  FLUID_NOTUSED = 2,
  FLUID_LOOP_UNTIL_RELEASE = 3
};*/

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

/**
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
 * Mutual exclusion notes:
 *
 * Set only once on init:
 * ----------------------
 * verbose
 * dump
 * sample_rate (will be runtime change-able in the future)
 * min_note_length_ticks
 * midi_channels
 * audio_channels
 * audio_groups
 * effects_channels
 * start
 * channel[] (Contents change)
 * nvoice
 * voice[] (Contents change)
 * nbuf
 * left_buf[], right_buf[] (Contents change)
 * fx_left_buf[], fx_right_buf[] (Contents change)
 * LADSPA_FxUnit (Contents change)
 * cores
 * core_threads[]
 * bank_select (FIXME: pending implementation of SYSEX midi mode changes)
 *
 * Single thread use only (modify only prior to synthesis):
 * loaders<>
 * midi_router
 *
 * Mutex protected:
 * settings{} (has its own mutex)
 * sfont_info<>
 * tuning
 * sfont_id
 * reverb_roomsize, reverb_damping, reverb_width, reverb_level
 * chorus_nr, chorus_level, chorus_speed, chorus_depth, chorus_type
 *
 * Atomic operations:
 * ----------------------
 * with_reverb
 * with_chorus
 * state
 * gain
 * cpu_load
 * noteid
 * storeid
 * outbuf
 * sample_timers
 *
 * Only synth thread changes (atomic operations for non-synth thread reads)
 * -------------------------
 * ticks
 * reverb{}
 * chorus{}
 * cur
 * dither_index
 * polyphony
 * active_voice_count
 */

struct _fluid_synth_t
{
#if 0
  fluid_thread_id_t synth_thread_id; /**< ID of the synthesis thread or FLUID_THREAD_ID_NULL if not yet set */
  fluid_private_t thread_queues;     /**< Thread private data for event queues for each non-synthesis thread queuing events */
  fluid_event_queue_t *queues[FLUID_MAX_EVENT_QUEUES];   /**< Thread event queues (NULL for unused elements) */
#endif 

  fluid_rec_mutex_t mutex;           /**< Lock for multi-thread sensitive variables (not used by synthesis process) */
  int use_mutex;                     /**< Use mutex for all public API functions? */
  int public_api_count;            /**< How many times the mutex is currently locked */
#if 0  
  fluid_list_t *queue_pool;          /**< List of event queues whose threads have been destroyed and which can be re-used */
  fluid_event_queue_t *return_queue; /**< Event queue for events from synthesis thread to non-synthesis threads (memory frees, etc) */
  fluid_thread_t *return_queue_thread;  /**< Return event queue processing thread */
  fluid_cond_mutex_t *return_queue_mutex;       /**< Mutex for return queue condition */
  fluid_cond_t *return_queue_cond;   /**< Return queue thread synchronization condition */
#endif
  fluid_settings_t* settings;        /**< the synthesizer settings */
  int device_id;                     /**< Device ID used for SYSEX messages */
  int polyphony;                     /**< Maximum polyphony */
  int shadow_polyphony;              /**< Maximum polyphony shadow value (for non-synth threads) */
  int with_reverb;                  /**< Should the synth use the built-in reverb unit? */
  int with_chorus;                  /**< Should the synth use the built-in chorus unit? */
  int verbose;                      /**< Turn verbose mode on? */
  int dump;                         /**< Dump events to stdout to hook up a user interface? */
  double sample_rate;                /**< The sample rate */
  int midi_channels;                 /**< the number of MIDI channels (>= 16) */
  int bank_select;                   /**< the style of Bank Select MIDI messages */
  int audio_channels;                /**< the number of audio channels (1 channel=left+right) */
  int audio_groups;                  /**< the number of (stereo) 'sub'groups from the synth.
					  Typically equal to audio_channels. */
  int effects_channels;              /**< the number of effects channels (>= 2) */
  int state;                         /**< the synthesizer state */
  unsigned int ticks_since_start;    /**< the number of audio samples since the start */
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
//  int nbuf;                          /**< How many audio buffers are used? (depends on nr of audio channels / groups)*/
  fluid_rvoice_eventhandler_t* eventhandler;
/*
  fluid_real_t** left_buf;
  fluid_real_t** right_buf;
  fluid_real_t** fx_left_buf;
  fluid_real_t** fx_right_buf;

  fluid_revmodel_t* reverb;
  fluid_chorus_t* chorus;
*/
  float reverb_roomsize;             /**< Shadow of reverb roomsize */
  float reverb_damping;              /**< Shadow of reverb damping */
  float reverb_width;                /**< Shadow of reverb width */
  float reverb_level;                /**< Shadow of reverb level */

  int chorus_nr;                     /**< Shadow of chorus number */
  float chorus_level;                /**< Shadow of chorus level */
  float chorus_speed;                /**< Shadow of chorus speed */
  float chorus_depth;                /**< Shadow of chorus depth */
  int chorus_type;                   /**< Shadow of chorus type */

  int cur;                           /**< the current sample in the audio buffers to be output */
  int curmax;                        /**< current amount of samples present in the audio buffers */
  int dither_index;		     /**< current index in random dither value buffer: fluid_synth_(write_s16|dither_s16) */

  char outbuf[256];                  /**< buffer for message output */
  float cpu_load;                    /**< CPU load in percent (CPU time required / audio synthesized time * 100) */

  fluid_tuning_t*** tuning;          /**< 128 banks of 128 programs for the tunings */
  fluid_private_t tuning_iter;       /**< Tuning iterators per each thread */

  fluid_midi_router_t* midi_router;  /**< The midi router. Could be done nicer. */
  fluid_sample_timer_t* sample_timers; /**< List of timers triggered before a block is processed */
  unsigned int min_note_length_ticks; /**< If note-offs are triggered just after a note-on, they will be delayed */

  int cores;                         /**< Number of CPU cores (1 by default) */
#if 0
  fluid_thread_t **core_threads;     /**< Array of core threads (cores - 1 in length) */
  unsigned char cores_active;        /**< TRUE if core slave threads should remain active, FALSE to terminate them */

  /* Multi-core variables (protected by core_mutex) */
  fluid_cond_mutex_t *core_mutex;    /**< Mutex to protect all core_ variables and use with core_cond and core_wait_last_cond */
  fluid_cond_t *core_cond;           /**< Thread condition for signaling core slave threads */
  int core_work;                     /**< Boolean: TRUE if there is work, FALSE otherwise */

  /* Used in a lockless atomic fashion */
  int core_voice_index;              /**< Next voice index to process */
  fluid_voice_t **core_voice_processed;  /**< Array for processed voices */
  fluid_real_t *core_bufs;           /**< Block containing audio buffers for each voice (FLUID_BUFSIZE in length each) */
  int core_inprogress;               /**< Count of secondary core threads in progress */
  int core_waiting_for_last;         /**< Boolean: Set to TRUE if primary synthesis thread is waiting for last slave thread to finish */
  fluid_cond_t *core_wait_last_cond; /**< Thread condition for signaling primary synthesis thread when last slave thread finishes */
#endif

#ifdef LADSPA
  fluid_LADSPA_FxUnit_t* LADSPA_FxUnit; /**< Effects unit for LADSPA support */
#endif
};

int fluid_synth_setstr(fluid_synth_t* synth, const char* name, const char* str);
int fluid_synth_dupstr(fluid_synth_t* synth, const char* name, char** str);
int fluid_synth_setnum(fluid_synth_t* synth, const char* name, double val);
int fluid_synth_getnum(fluid_synth_t* synth, const char* name, double* val);
int fluid_synth_setint(fluid_synth_t* synth, const char* name, int val);
int fluid_synth_getint(fluid_synth_t* synth, const char* name, int* val);

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

int fluid_synth_set_reverb_preset(fluid_synth_t* synth, int num);
int fluid_synth_set_reverb_full(fluid_synth_t* synth, int set, double roomsize,
                                double damping, double width, double level);

int fluid_synth_set_chorus_full(fluid_synth_t* synth, int set, int nr, double level,
                                double speed, double depth_ms, int type);

fluid_sample_timer_t* new_fluid_sample_timer(fluid_synth_t* synth, fluid_timer_callback_t callback, void* data);
int delete_fluid_sample_timer(fluid_synth_t* synth, fluid_sample_timer_t* timer);

void fluid_synth_api_enter(fluid_synth_t* synth);
void fluid_synth_api_exit(fluid_synth_t* synth);

/*
 * misc
 */

void fluid_synth_settings(fluid_settings_t* settings);

#endif  /* _FLUID_SYNTH_H */
