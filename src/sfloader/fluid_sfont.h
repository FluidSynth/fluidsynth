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


#ifndef _PRIV_FLUID_SFONT_H
#define _PRIV_FLUID_SFONT_H

#include "fluidsynth.h"

/*
 * Utility macros to access soundfonts, presets, and samples
 */

#define fluid_sfloader_delete(_loader) { if ((_loader) && (_loader)->free) (*(_loader)->free)(_loader); }
#define fluid_sfloader_load(_loader, _filename) (*(_loader)->load)(_loader, _filename)


#define fluid_sfont_delete_internal(_sf)   ( ((_sf) && (_sf)->free)? (*(_sf)->free)(_sf) : 0)

#define fluid_sfont_get_id(_sf) ((_sf)->id)
#define fluid_sfont_get_name(_sf) (*(_sf)->get_name)(_sf)
#define fluid_sfont_get_preset(_sf,_bank,_prenum) (*(_sf)->get_preset)(_sf,_bank,_prenum)
#define fluid_sfont_iteration_start(_sf) { if((_sf) && (_sf)->iteration_start) (*(_sf)->iteration_start)(_sf); }
#define fluid_sfont_iteration_next(_sf,_pr) (((_sf) && (_sf)->iteration_start) ? (*(_sf)->iteration_next)(_sf,_pr) : 0)


#define fluid_preset_delete_internal(_preset) \
  { if ((_preset) && (_preset)->free) { (*(_preset)->free)(_preset); }}

#define fluid_preset_get_name(_preset) (*(_preset)->get_name)(_preset)
#define fluid_preset_get_banknum(_preset) (*(_preset)->get_banknum)(_preset)
#define fluid_preset_get_num(_preset) (*(_preset)->get_num)(_preset)

#define fluid_preset_noteon(_preset,_synth,_ch,_key,_vel) \
  (*(_preset)->noteon)(_preset,_synth,_ch,_key,_vel)

#define fluid_preset_notify(_preset,_reason,_chan) \
  { if ((_preset) && (_preset)->notify) { (*(_preset)->notify)(_preset,_reason,_chan); }}


#define fluid_sample_incr_ref(_sample) { (_sample)->refcount++; }

#define fluid_sample_decr_ref(_sample) \
  (_sample)->refcount--; \
  if (((_sample)->refcount == 0) && ((_sample)->notify)) \
    (*(_sample)->notify)(_sample, FLUID_SAMPLE_DONE);



/**
 * File callback structure to enable custom soundfont loading (e.g. from memory).
 */
struct _fluid_file_callbacks_t
{
  fluid_sfloader_callback_open_t  fopen;
  fluid_sfloader_callback_read_t  fread;
  fluid_sfloader_callback_seek_t  fseek;
  fluid_sfloader_callback_close_t fclose;
  fluid_sfloader_callback_tell_t  ftell;
};

/**
 * SoundFont loader structure.
 */
struct _fluid_sfloader_t {
  void* data;           /**< User defined data pointer used by _fluid_sfloader_t::load() */

  /** Callback structure specifying file operations used during soundfont loading to allow custom loading, such as from memory */
  fluid_file_callbacks_t file_callbacks;

  fluid_sfloader_free_t free;

  fluid_sfloader_load_t load;
};

/**
 * Start virtual SoundFont preset iteration method.
 * @param sfont Virtual SoundFont
 *
 * Starts/re-starts virtual preset iteration in a SoundFont.
 */
typedef void (*fluid_sfont_iteration_start_t)(fluid_sfont_t* sfont);

/**
 * Virtual SoundFont preset iteration function.
 * @param sfont Virtual SoundFont
 * @param preset Caller supplied uninitialized buffer to fill in with current preset information
 * @return 0 when no more presets are available, 1 otherwise
 *
 * Should store preset information to the caller supplied \a preset structure
 * and advance the internal iteration state to the next preset for subsequent
 * calls.
 */
typedef int (*fluid_sfont_iteration_next_t)(fluid_sfont_t* sfont, fluid_preset_t* preset);

void fluid_sfont_set_iteration_start(fluid_sfont_t* sfont, fluid_sfont_iteration_start_t iter_start);
void fluid_sfont_set_iteration_next(fluid_sfont_t* sfont, fluid_sfont_iteration_next_t iter_next);

/**
 * Virtual SoundFont instance structure.
 */
struct _fluid_sfont_t {
  void* data;           /**< User defined data */
  unsigned int id;      /**< SoundFont ID */

  fluid_sfont_free_t free;

  fluid_sfont_get_name_t get_name;

  fluid_sfont_get_preset_t get_preset;

  fluid_sfont_iteration_start_t iteration_start;

  fluid_sfont_iteration_next_t iteration_next;
};

/**
 * Virtual SoundFont preset.
 */
struct _fluid_preset_t {
  void* data;                                   /**< User supplied data */
  fluid_sfont_t* sfont;                         /**< Parent virtual SoundFont */

  fluid_preset_free_t free;

  fluid_preset_get_name_t get_name;

  fluid_preset_get_banknum_t get_banknum;

  fluid_preset_get_num_t get_num;

  fluid_preset_noteon_t noteon;

  /**
   * Virtual SoundFont preset notify method.
   * @param preset Virtual SoundFont preset
   * @param reason #FLUID_PRESET_SELECTED or #FLUID_PRESET_UNSELECTED
   * @param chan MIDI channel number
   * @return Should return #FLUID_OK
   *
   * Implement this optional method if the preset needs to be notified about
   * preset select and unselect events.
   *
   * This method may be called from within synthesis context and therefore
   * should be as efficient as possible and not perform any operations considered
   * bad for realtime audio output (memory allocations and other OS calls).
   */
  int (*notify)(fluid_preset_t* preset, int reason, int chan);
};

/**
 * Virtual SoundFont sample.
 */
struct _fluid_sample_t
{
  char name[21];                /**< Sample name */
  unsigned int start;           /**< Start index */
  unsigned int end;	        /**< End index, index of last valid sample point (contrary to SF spec) */
  unsigned int loopstart;       /**< Loop start index */
  unsigned int loopend;         /**< Loop end index, first point following the loop (superimposed on loopstart) */
  unsigned int samplerate;      /**< Sample rate */
  int origpitch;                /**< Original pitch (MIDI note number, 0-127) */
  int pitchadj;                 /**< Fine pitch adjustment (+/- 99 cents) */
  int sampletype;               /**< Specifies the type of this sample as indicated by the #fluid_sample_type enum */
  int valid;                    /**< Should be TRUE if sample data is valid, FALSE otherwise (in which case it will not be synthesized) */
  int auto_free;                /**< TRUE if _fluid_sample_t::data and _fluid_sample_t::data24 should be freed upon sample destruction */
  short* data;                  /**< Pointer to the sample's 16 bit PCM data */
  char* data24;                 /**< If not NULL, pointer to the least significant byte counterparts of each sample data point in order to create 24 bit audio samples */

  int amplitude_that_reaches_noise_floor_is_valid;      /**< Indicates if \a amplitude_that_reaches_noise_floor is valid (TRUE), set to FALSE initially to calculate. */
  double amplitude_that_reaches_noise_floor;            /**< The amplitude at which the sample's loop will be below the noise floor.  For voice off optimization, calculated automatically. */

  unsigned int refcount;        /**< Count of voices using this sample */

  /**
   * Implement this function to receive notification when sample is no longer used.
   * @param sample Virtual SoundFont sample
   * @param reason #FLUID_SAMPLE_DONE only currently
   * @return Should return #FLUID_OK
   */
  int (*notify)(fluid_sample_t* sample, int reason);

  void* userdata;       /**< User defined data */
};


#endif /* _PRIV_FLUID_SFONT_H */
