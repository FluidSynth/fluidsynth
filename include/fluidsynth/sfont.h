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

#ifndef _FLUIDSYNTH_SFONT_H
#define _FLUIDSYNTH_SFONT_H

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @file sfont.h
 * @brief SoundFont plugins
 *
 * It is possible to add new SoundFont loaders to the
 * synthesizer. The API uses a couple of "interfaces" (structures
 * with callback functions): #fluid_sfloader_t, #fluid_sfont_t, and
 * #fluid_preset_t.  This API allows for virtual SoundFont files to be loaded
 * and synthesized, which may not actually be SoundFont files, as long as they
 * can be represented by the SoundFont synthesis model.
 *
 * To add a new SoundFont loader to the synthesizer, call
 * fluid_synth_add_sfloader() and pass a pointer to an
 * fluid_sfloader_t structure. The important callback function in
 * this structure is "load", which should try to load a file and
 * returns a #fluid_sfont_t structure, or NULL if it fails.
 *
 * The #fluid_sfont_t structure contains a callback to obtain the
 * name of the SoundFont. It contains two functions to iterate
 * though the contained presets, and one function to obtain a
 * preset corresponding to a bank and preset number. This
 * function should return a #fluid_preset_t structure.
 *
 * The #fluid_preset_t structure contains some functions to obtain
 * information from the preset (name, bank, number). The most
 * important callback is the noteon function. The noteon function
 * should call fluid_synth_alloc_voice() for every sample that has
 * to be played. fluid_synth_alloc_voice() expects a pointer to a
 * #fluid_sample_t structure and returns a pointer to the opaque
 * #fluid_voice_t structure. To set or increment the values of a
 * generator, use fluid_voice_gen_set() or fluid_voice_gen_incr(). When you are
 * finished initializing the voice call fluid_voice_start() to
 * start playing the synthesis voice.
 */

/**
 * Some notification enums for presets and samples.
 */
enum {
  FLUID_PRESET_SELECTED,                /**< Preset selected notify */
  FLUID_PRESET_UNSELECTED,              /**< Preset unselected notify */
  FLUID_SAMPLE_DONE                     /**< Sample no longer needed notify */
};

/**
 * Indicates the type of a sample used by the _fluid_sample_t::sampletype field.
 */
enum fluid_sample_type
{
    FLUID_SAMPLETYPE_MONO = 0x1, /**< Used for mono samples */
    FLUID_SAMPLETYPE_RIGHT = 0x2, /**< Used for right samples of a stereo pair */
    FLUID_SAMPLETYPE_LEFT = 0x4, /**< Used for left samples of a stereo pair */
    FLUID_SAMPLETYPE_LINKED = 0x8, /**< Currently not used */
    FLUID_SAMPLETYPE_OGG_VORBIS = 0x10, /**< Used for Ogg Vorbis compressed samples @since 1.1.7 */
    FLUID_SAMPLETYPE_ROM = 0x8000 /**< Indicates ROM samples, causes sample to be ignored */
};


/**
 * Method to load an instrument file (does not actually need to be a real file name,
 * could be another type of string identifier that the \a loader understands).
 * @param loader SoundFont loader
 * @param filename File name or other string identifier
 * @return The loaded instrument file (SoundFont) or NULL if an error occured.
 */
typedef fluid_sfont_t* (*fluid_sfloader_load_t)(fluid_sfloader_t* loader, const char* filename);

/**
 * The free method should free the memory allocated for a fluid_sfloader_t instance in
 * addition to any private data. Any custom user provided cleanup function must ultimately call
 * delete_fluid_sfloader() to ensure proper cleanup of the #fluid_sfloader_t struct. If no private data
 * needs to be freed, setting this to delete_fluid_sfloader() is sufficient.
 * @param loader SoundFont loader
 */
typedef void (*fluid_sfloader_free_t)(fluid_sfloader_t* loader);


FLUIDSYNTH_API fluid_sfloader_t* new_fluid_sfloader(fluid_sfloader_load_t load, fluid_sfloader_free_t free);
FLUIDSYNTH_API void delete_fluid_sfloader(fluid_sfloader_t* loader);


/**
 * Opens the file or memory indicated by \c filename in binary read mode.
 * \c filename matches the one provided during the fluid_synth_sfload() call.
 *
 * @return returns a file handle on success, NULL otherwise
 */
typedef void * (* fluid_sfloader_callback_open )(const char * filename);

/**
 * Reads \c count bytes to the specified buffer \c buf.
 * 
 * @return returns #FLUID_OK if exactly \c count bytes were successfully read, else #FLUID_FAILED
 */
typedef int (* fluid_sfloader_callback_read )(void *buf, int count, void * handle);

/**
 * Same purpose and behaviour as fseek.
 * 
 * @param origin either \c SEEK_SET, \c SEEK_CUR or \c SEEK_END
 * 
 * @return returns #FLUID_OK if the seek was successfully performed while not seeking beyond a buffer or file, #FLUID_FAILED otherwise
 */
typedef int (* fluid_sfloader_callback_seek )(void * handle, long offset, int origin);

/** 
 * Closes the handle returned by #fluid_sfloader_callback_open and frees used ressources.
 * 
 * @return returns #FLUID_OK on success, #FLUID_FAILED on error
 */
typedef int (* fluid_sfloader_callback_close )(void * handle);

/** @return returns current file offset or #FLUID_FAILED on error */
typedef long (* fluid_sfloader_callback_tell )(void * handle);


FLUIDSYNTH_API void fluid_sfloader_set_callbacks(fluid_sfloader_t* loader,
                                  fluid_sfloader_callback_open open,
                                  fluid_sfloader_callback_read read,
                                  fluid_sfloader_callback_seek seek,
                                  fluid_sfloader_callback_tell tell,
                                  fluid_sfloader_callback_close close);

FLUIDSYNTH_API void fluid_sfloader_set_data(fluid_sfloader_t* loader, void* data);


FLUIDSYNTH_API fluid_sfloader_t* new_fluid_defsfloader(fluid_settings_t* settings);

/**
 * Virtual SoundFont instance structure.
 */
struct _fluid_sfont_t {
  void* data;           /**< User defined data */
  unsigned int id;      /**< SoundFont ID */

  /**
   * Method to free a virtual SoundFont bank.
   * @param sfont Virtual SoundFont to free.
   * @return Should return 0 when it was able to free all resources or non-zero
   *   if some of the samples could not be freed because they are still in use,
   *   in which case the free will be tried again later, until success.
   */
  int (*free)(fluid_sfont_t* sfont);

  /**
   * Method to return the name of a virtual SoundFont.
   * @param sfont Virtual SoundFont
   * @return The name of the virtual SoundFont.
   */
  const char* (*get_name)(fluid_sfont_t* sfont);

  /**
   * Get a virtual SoundFont preset by bank and program numbers.
   * @param sfont Virtual SoundFont
   * @param bank MIDI bank number (0-16384)
   * @param prenum MIDI preset number (0-127)
   * @return Should return an allocated virtual preset or NULL if it could not
   *   be found.
   */
  fluid_preset_t* (*get_preset)(fluid_sfont_t* sfont, unsigned int bank, unsigned int prenum);

  /**
   * Start virtual SoundFont preset iteration method.
   * @param sfont Virtual SoundFont
   *
   * Starts/re-starts virtual preset iteration in a SoundFont.
   */
  void (*iteration_start)(fluid_sfont_t* sfont);

  /**
   * Virtual SoundFont preset iteration function.
   * @param sfont Virtual SoundFont
   * @param preset Caller supplied preset to fill in with current preset information
   * @return 0 when no more presets are available, 1 otherwise
   *
   * Should store preset information to the caller supplied \a preset structure
   * and advance the internal iteration state to the next preset for subsequent
   * calls.
   */
  int (*iteration_next)(fluid_sfont_t* sfont, fluid_preset_t* preset);
};


/**
 * Virtual SoundFont preset.
 */
struct _fluid_preset_t {
  void* data;                                   /**< User supplied data */
  fluid_sfont_t* sfont;                         /**< Parent virtual SoundFont */

  /**
   * Method to free a virtual SoundFont preset.
   * @param preset Virtual SoundFont preset
   * @return Should return 0
   */
  int (*free)(fluid_preset_t* preset);

  /**
   * Method to get a virtual SoundFont preset name.
   * @param preset Virtual SoundFont preset
   * @return Should return the name of the preset.  The returned string must be
   *   valid for the duration of the virtual preset (or the duration of the
   *   SoundFont, in the case of preset iteration).
   */
  const char* (*get_name)(fluid_preset_t* preset);

  /**
   * Method to get a virtual SoundFont preset MIDI bank number.
   * @param preset Virtual SoundFont preset
   * @param return The bank number of the preset
   */
  int (*get_banknum)(fluid_preset_t* preset);

  /**
   * Method to get a virtual SoundFont preset MIDI program number.
   * @param preset Virtual SoundFont preset
   * @param return The program number of the preset
   */
  int (*get_num)(fluid_preset_t* preset);

  /**
   * Method to handle a noteon event (synthesize the instrument).
   * @param preset Virtual SoundFont preset
   * @param synth Synthesizer instance
   * @param chan MIDI channel number of the note on event
   * @param key MIDI note number (0-127)
   * @param vel MIDI velocity (0-127)
   * @return #FLUID_OK on success (0) or #FLUID_FAILED (-1) otherwise
   *
   * This method may be called from within synthesis context and therefore
   * should be as efficient as possible and not perform any operations considered
   * bad for realtime audio output (memory allocations and other OS calls).
   *
   * Call fluid_synth_alloc_voice() for every sample that has
   * to be played. fluid_synth_alloc_voice() expects a pointer to a
   * #fluid_sample_t structure and returns a pointer to the opaque
   * #fluid_voice_t structure. To set or increment the values of a
   * generator, use fluid_voice_gen_set() or fluid_voice_gen_incr(). When you are
   * finished initializing the voice call fluid_voice_start() to
   * start playing the synthesis voice.  Starting with FluidSynth 1.1.0 all voices
   * created will be started at the same time.
   */
  int (*noteon)(fluid_preset_t* preset, fluid_synth_t* synth, int chan, int key, int vel);

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


FLUIDSYNTH_API fluid_sample_t* new_fluid_sample(void);
FLUIDSYNTH_API void delete_fluid_sample(fluid_sample_t* sample);

FLUIDSYNTH_API int fluid_sample_set_name(fluid_sample_t* sample, const char *name);
FLUIDSYNTH_API int fluid_sample_set_sound_data (fluid_sample_t* sample,
                                                short *data,
                                                char *data24,
                                                unsigned int nbframes,
                                                short copy_data,
                                                int rootkey,
                                                unsigned int sample_rate);

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_SFONT_H */
