/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * SoundFont file loading code borrowed from Smurf SoundFont Editor
 * Copyright (C) 1999-2001 Josh Green
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


#include "fluid_defsfont.h"
#include "fluid_sfont.h"
#include "fluid_sys.h"
#include "fluid_synth.h"
#include "fluid_samplecache.h"

/* EMU8k/10k hardware applies this factor to initial attenuation generator values set at preset and
 * instrument level in a soundfont. We apply this factor when loading the generator values to stay
 * compatible as most existing soundfonts expect exactly this (strange, non-standard) behaviour. */
#define EMU_ATTENUATION_FACTOR (0.4f)

/* Dynamic sample loading functions */
static int load_preset_samples(fluid_defsfont_t *defsfont, fluid_preset_t *preset);
static int unload_preset_samples(fluid_defsfont_t *defsfont, fluid_preset_t *preset);
static void unload_sample(fluid_sample_t *sample);
static int dynamic_samples_sample_notify(fluid_sample_t *sample, int reason);

static int dynamic_samples_preset_notify(fluid_preset_t *preset, int reason, int chan);
static int dynamic_samples_preset_load(fluid_preset_t *preset);
static int dynamic_samples_preset_unload(fluid_preset_t *preset);
static int dynamic_samples_preset_is_loaded(fluid_preset_t *preset);


/***************************************************************
 *
 *                           SFONT LOADER
 */

/**
 * Creates a default soundfont2 loader that can be used with fluid_synth_add_sfloader().
 * By default every synth instance has an initial default soundfont loader instance.
 * Calling this function is usually only necessary to load a soundfont from memory, by providing custom callback functions via fluid_sfloader_set_callbacks().
 * 
 * @param settings A settings instance obtained by new_fluid_settings()
 * @return A default soundfont2 loader struct
 */
fluid_sfloader_t* new_fluid_defsfloader(fluid_settings_t* settings)
{
  fluid_sfloader_t* loader;
  fluid_return_val_if_fail(settings != NULL, NULL);

  loader = new_fluid_sfloader(fluid_defsfloader_load, delete_fluid_sfloader);
  
  if (loader == NULL)
  {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  fluid_sfloader_set_data(loader, settings);

  return loader;
}

fluid_sfont_t* fluid_defsfloader_load(fluid_sfloader_t* loader, const char* filename)
{
  fluid_defsfont_t* defsfont;
  fluid_sfont_t* sfont;

  defsfont = new_fluid_defsfont(fluid_sfloader_get_data(loader));

  if (defsfont == NULL) {
    return NULL;
  }

  sfont = new_fluid_sfont(fluid_defsfont_sfont_get_name,
                          fluid_defsfont_sfont_get_preset,
                          fluid_defsfont_sfont_delete);
  if (sfont == NULL)
  {
    delete_fluid_defsfont(defsfont);
    return NULL;
  }
  
  fluid_sfont_set_iteration_start(sfont, fluid_defsfont_sfont_iteration_start);
  fluid_sfont_set_iteration_next(sfont, fluid_defsfont_sfont_iteration_next);
  fluid_sfont_set_data(sfont, defsfont);

  defsfont->sfont = sfont;
  if (fluid_defsfont_load(defsfont, &loader->file_callbacks, filename) == FLUID_FAILED) {
    fluid_sfont_delete_internal(sfont);
    return NULL;
  }

  return sfont;
}



/***************************************************************
 *
 *                           PUBLIC INTERFACE
 */

int fluid_defsfont_sfont_delete(fluid_sfont_t* sfont)
{
  if (delete_fluid_defsfont(fluid_sfont_get_data(sfont)) != FLUID_OK) {
    return -1;
  }
  delete_fluid_sfont(sfont);
  return 0;
}

const char* fluid_defsfont_sfont_get_name(fluid_sfont_t* sfont)
{
  return fluid_defsfont_get_name(fluid_sfont_get_data(sfont));
}

fluid_preset_t*
fluid_defsfont_sfont_get_preset(fluid_sfont_t* sfont, unsigned int bank, unsigned int prenum)
{
  return fluid_defsfont_get_preset(fluid_sfont_get_data(sfont), bank, prenum);
}

void fluid_defsfont_sfont_iteration_start(fluid_sfont_t* sfont)
{
  fluid_defsfont_iteration_start(fluid_sfont_get_data(sfont));
}

fluid_preset_t *fluid_defsfont_sfont_iteration_next(fluid_sfont_t* sfont)
{
  return fluid_defsfont_iteration_next(fluid_sfont_get_data(sfont));
}

void fluid_defpreset_preset_delete(fluid_preset_t* preset)
{
  fluid_defsfont_t* defsfont;
  fluid_defpreset_t* defpreset;

  defsfont = fluid_sfont_get_data(preset->sfont);
  defpreset = fluid_preset_get_data(preset);

  if (defsfont)
  {
      defsfont->preset = fluid_list_remove(defsfont->preset, defpreset);
  }

  delete_fluid_defpreset(defpreset);
  delete_fluid_preset(preset);
}

const char* fluid_defpreset_preset_get_name(fluid_preset_t* preset)
{
  return fluid_defpreset_get_name(fluid_preset_get_data(preset));
}

int fluid_defpreset_preset_get_banknum(fluid_preset_t* preset)
{
  return fluid_defpreset_get_banknum(fluid_preset_get_data(preset));
}

int fluid_defpreset_preset_get_num(fluid_preset_t* preset)
{
  return fluid_defpreset_get_num(fluid_preset_get_data(preset));
}

int fluid_defpreset_preset_noteon(fluid_preset_t* preset, fluid_synth_t* synth,
				 int chan, int key, int vel)
{
  return fluid_defpreset_noteon(fluid_preset_get_data(preset), synth, chan, key, vel);
}


/***************************************************************
 *
 *                           SFONT
 */

/*
 * new_fluid_defsfont
 */
fluid_defsfont_t* new_fluid_defsfont(fluid_settings_t* settings)
{
  fluid_defsfont_t* defsfont;

  defsfont = FLUID_NEW(fluid_defsfont_t);
  if (defsfont == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  FLUID_MEMSET(defsfont, 0, sizeof(*defsfont));
  
  fluid_settings_getint(settings, "synth.lock-memory", &defsfont->mlock);
  fluid_settings_getint(settings, "synth.dynamic-sample-loading", &defsfont->dynamic_samples);

  return defsfont;
}

/*
 * delete_fluid_defsfont
 */
int delete_fluid_defsfont(fluid_defsfont_t* defsfont)
{
  fluid_list_t *list;
  fluid_preset_t* preset;
  fluid_sample_t* sample;

  fluid_return_val_if_fail(defsfont != NULL, FLUID_OK);
  
  /* Check that no samples are currently used */
  for (list = defsfont->sample; list; list = fluid_list_next(list)) {
    sample = (fluid_sample_t*) fluid_list_get(list);
    if (sample->refcount != 0) {
      return FLUID_FAILED;
    }
  }

  /* Unload any manually loaded presets */
  if (defsfont->dynamic_samples)
  {
    for (list = defsfont->preset; list; list = fluid_list_next(list)) {
        preset = fluid_list_get(list);
        if (fluid_preset_is_loaded(preset))
        {
            fluid_preset_unload(preset);
        }
    }
  }

  if (defsfont->filename != NULL) {
    FLUID_FREE(defsfont->filename);
  }

  for (list = defsfont->sample; list; list = fluid_list_next(list)) {
    delete_fluid_sample((fluid_sample_t*) fluid_list_get(list));
  }

  if (defsfont->sample) {
    delete_fluid_list(defsfont->sample);
  }

  if (defsfont->sampledata != NULL) {
    fluid_samplecache_unload(defsfont->sampledata);
  }

  for (list = defsfont->preset; list; list = fluid_list_next(list)) {
      preset = (fluid_preset_t *)fluid_list_get(list);
      fluid_defpreset_preset_delete(preset);
  }
  delete_fluid_list(defsfont->preset);

  FLUID_FREE(defsfont);
  return FLUID_OK;
}

/*
 * fluid_defsfont_get_name
 */
const char* fluid_defsfont_get_name(fluid_defsfont_t* defsfont)
{
  return defsfont->filename;
}

/* Load sample data for a single sample from the Soundfont file.
 * Returns FLUID_OK on error, otherwise FLUID_FAILED
 */
int fluid_defsfont_load_sampledata(fluid_defsfont_t *defsfont, SFData *sfdata, fluid_sample_t *sample)
{
    int num_samples;
    int source_end = sample->source_end;

    /* For uncompressed samples we want to include the 46 zero sample word area following each sample
     * in the Soundfont. Otherwise samples with loopend > end, which we have decided not to correct, would
     * be corrected after all in fluid_sample_sanitize_loop */
    if (!(sample->sampletype & FLUID_SAMPLETYPE_OGG_VORBIS))
    {
        source_end += 46;  /* Length of zero sample word after each sample, according to SF specs */

        /* Safeguard against Soundfonts that are not quite valid and don't include 46 sample words after the
         * last sample */
        if (source_end >= (defsfont->samplesize  / sizeof(short)))
        {
            source_end = defsfont->samplesize  / sizeof(short);
        }
    }

    num_samples = fluid_samplecache_load(
            sfdata, sample->source_start, source_end, sample->sampletype,
            defsfont->mlock, &sample->data, &sample->data24);

    if (num_samples < 0)
    {
        return FLUID_FAILED;
    }

    if (num_samples == 0)
    {
        sample->start = sample->end = 0;
        sample->loopstart= sample->loopend = 0;
        return FLUID_OK;
    }

    /* Ogg Vorbis samples already have loop pointers relative to the invididual decompressed sample,
     * but SF2 samples are relative to sample chunk start, so they need to be adjusted */
    if (!(sample->sampletype & FLUID_SAMPLETYPE_OGG_VORBIS))
    {
        sample->loopstart = sample->source_loopstart - sample->source_start;
        sample->loopend = sample->source_loopend - sample->source_start;
    }

    /* As we've just loaded an individual sample into it's own buffer, we need to adjust the start
     * and end pointers */
    sample->start = 0;
    sample->end = num_samples - 1;

    return FLUID_OK;
}

/* Loads the sample data for all samples from the Soundfont file. For SF2 files, it loads the data in
 * one large block. For SF3 files, each compressed sample gets loaded individually.
 * Returns FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_defsfont_load_all_sampledata(fluid_defsfont_t *defsfont, SFData *sfdata)
{
    fluid_list_t *list;
    fluid_sample_t *sample;
    int sf3_file = (sfdata->version.major == 3);

    /* For SF2 files, we load the sample data in one large block */
    if (!sf3_file)
    {
        int read_samples;
        int num_samples = sfdata->samplesize / sizeof(short);

        read_samples = fluid_samplecache_load(sfdata, 0, num_samples - 1, 0, defsfont->mlock,
                    &defsfont->sampledata, &defsfont->sample24data);
        if (read_samples != num_samples)
        {
            FLUID_LOG(FLUID_ERR, "Attempted to read %d words of sample data, but got %d instead",
                    num_samples, read_samples);
            return FLUID_FAILED;
        }
    }

    for (list = defsfont->sample; list; list = fluid_list_next(list))
    {
        sample = fluid_list_get(list);

        if (sf3_file)
        {
            /* SF3 samples get loaded individually, as most (or all) of them are in Ogg Vorbis format
             * anyway */
            if (fluid_defsfont_load_sampledata(defsfont, sfdata, sample) == FLUID_FAILED)
            {
                FLUID_LOG(FLUID_ERR, "Failed to load sample '%s'", sample->name);
                return FLUID_FAILED;
            }
            fluid_sample_sanitize_loop(sample, (sample->end + 1) * sizeof(short));
        }
        else
        {
            /* Data pointers of SF2 samples point to large sample data block loaded above */
            sample->data = defsfont->sampledata;
            sample->data24 = defsfont->sample24data;
            fluid_sample_sanitize_loop(sample, defsfont->samplesize);
        }

        fluid_voice_optimize_sample(sample);
    }

    return FLUID_OK;
}

/*
 * fluid_defsfont_load
 */
int fluid_defsfont_load(fluid_defsfont_t* defsfont, const fluid_file_callbacks_t* fcbs, const char* file)
{
  SFData* sfdata;
  fluid_list_t *p;
  SFPreset* sfpreset;
  SFSample* sfsample;
  fluid_sample_t* sample;
  fluid_defpreset_t* defpreset = NULL;

  defsfont->filename = FLUID_STRDUP(file);
  if (defsfont->filename == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return FLUID_FAILED;
  }

  defsfont->fcbs = fcbs;

  /* The actual loading is done in the sfont and sffile files */
  sfdata = fluid_sffile_open(file, fcbs);
  if (sfdata == NULL) {
    FLUID_LOG(FLUID_ERR, "Couldn't load soundfont file");
    return FLUID_FAILED;
  }

  if (fluid_sffile_parse_presets(sfdata) == FLUID_FAILED) {
    FLUID_LOG(FLUID_ERR, "Couldn't parse presets from soundfont file");
    goto err_exit;
  }

  /* Keep track of the position and size of the sample data because
     it's loaded separately (and might be unoaded/reloaded in future) */
  defsfont->samplepos = sfdata->samplepos;
  defsfont->samplesize = sfdata->samplesize;
  defsfont->sample24pos = sfdata->sample24pos;
  defsfont->sample24size = sfdata->sample24size;

  /* Create all samples from sample headers */
  p = sfdata->sample;
  while (p != NULL) {
    sfsample = (SFSample *)fluid_list_get(p);

    sample = new_fluid_sample();
    if (sample == NULL) goto err_exit;

    if (fluid_sample_import_sfont(sample, sfsample, defsfont) == FLUID_OK)
    {
        fluid_defsfont_add_sample(defsfont, sample);
    }
    else
    {
        delete_fluid_sample(sample);
        sample = NULL;
    }

    /* Store reference to FluidSynth sample in SFSample for later IZone fixups */
    sfsample->fluid_sample = sample;

    p = fluid_list_next(p);
  }

  /* If dynamic sample loading is disabled, load all samples in the Soundfont */
  if (!defsfont->dynamic_samples)
  {
    if (fluid_defsfont_load_all_sampledata(defsfont, sfdata) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Unable to load all sample data");
        goto err_exit;
    }
  }

  /* Load all the presets */
  p = sfdata->preset;
  while (p != NULL) {
    sfpreset = (SFPreset *)fluid_list_get(p);
    defpreset = new_fluid_defpreset(defsfont);
    if (defpreset == NULL) goto err_exit;

    if (fluid_defpreset_import_sfont(defpreset, sfpreset, defsfont) != FLUID_OK)
      goto err_exit;

    if (fluid_defsfont_add_preset(defsfont, defpreset) == FLUID_FAILED)
    {
        goto err_exit;
    }
    p = fluid_list_next(p);
  }
  fluid_sffile_close (sfdata);

  return FLUID_OK;

err_exit:
  fluid_sffile_close (sfdata);
  delete_fluid_defpreset(defpreset);
  return FLUID_FAILED;
}

/* fluid_defsfont_add_sample
 *
 * Add a sample to the SoundFont
 */
int fluid_defsfont_add_sample(fluid_defsfont_t* defsfont, fluid_sample_t* sample)
{
  defsfont->sample = fluid_list_append(defsfont->sample, sample);
  return FLUID_OK;
}

/* fluid_defsfont_add_preset
 *
 * Add a preset to the SoundFont
 */
int fluid_defsfont_add_preset(fluid_defsfont_t* defsfont, fluid_defpreset_t* defpreset)
{
    fluid_preset_t *preset;

    preset = new_fluid_preset(defsfont->sfont,
                              fluid_defpreset_preset_get_name,
                              fluid_defpreset_preset_get_banknum,
                              fluid_defpreset_preset_get_num,
                              fluid_defpreset_preset_noteon,
                              fluid_defpreset_preset_delete);

    if (defsfont->dynamic_samples)
    {
        preset->notify = dynamic_samples_preset_notify;
        preset->load = dynamic_samples_preset_load;
        preset->unload = dynamic_samples_preset_unload;
        preset->is_loaded = dynamic_samples_preset_is_loaded;
    }

    if (preset == NULL) {
        return FLUID_FAILED;
    }

    fluid_preset_set_data(preset, defpreset);

    defsfont->preset = fluid_list_append(defsfont->preset, preset);

    return FLUID_OK;
}

/*
 * fluid_defsfont_get_preset
 */
fluid_preset_t* fluid_defsfont_get_preset(fluid_defsfont_t* defsfont, unsigned int bank, unsigned int num)
{
    fluid_preset_t *preset;
    fluid_list_t *list;

    for(list = defsfont->preset; list != NULL; list = fluid_list_next(list))
    {
        preset = (fluid_preset_t *)fluid_list_get(list);

        if ((fluid_preset_get_banknum(preset) == bank) && (fluid_preset_get_num(preset) == num))
        {
            return preset;
        }
    }

    return NULL;
}

/*
 * fluid_defsfont_iteration_start
 */
void fluid_defsfont_iteration_start(fluid_defsfont_t* defsfont)
{
  defsfont->preset_iter_cur = defsfont->preset;
}

/*
 * fluid_defsfont_iteration_next
 */
fluid_preset_t *fluid_defsfont_iteration_next(fluid_defsfont_t* defsfont)
{
    fluid_preset_t *preset = (fluid_preset_t *)fluid_list_get(defsfont->preset_iter_cur);

    defsfont->preset_iter_cur = fluid_list_next(defsfont->preset_iter_cur);

    return preset;
}

/***************************************************************
 *
 *                           PRESET
 */

/*
 * new_fluid_defpreset
 */
fluid_defpreset_t*
new_fluid_defpreset(fluid_defsfont_t* defsfont)
{
  fluid_defpreset_t* defpreset = FLUID_NEW(fluid_defpreset_t);
  if (defpreset == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  defpreset->next = NULL;
  defpreset->defsfont = defsfont;
  defpreset->name[0] = 0;
  defpreset->bank = 0;
  defpreset->num = 0;
  defpreset->global_zone = NULL;
  defpreset->zone = NULL;
  defpreset->loaded_manually = FALSE;
  return defpreset;
}

/*
 * delete_fluid_defpreset
 */
void
delete_fluid_defpreset(fluid_defpreset_t* defpreset)
{
  fluid_preset_zone_t* zone;
  
  fluid_return_if_fail(defpreset != NULL);
  
    delete_fluid_preset_zone(defpreset->global_zone);
    defpreset->global_zone = NULL;
  
  zone = defpreset->zone;
  while (zone != NULL) {
    defpreset->zone = zone->next;
    delete_fluid_preset_zone(zone);
    zone = defpreset->zone;
  }
  FLUID_FREE(defpreset);
}

int
fluid_defpreset_get_banknum(fluid_defpreset_t* defpreset)
{
  return defpreset->bank;
}

int
fluid_defpreset_get_num(fluid_defpreset_t* defpreset)
{
  return defpreset->num;
}

const char*
fluid_defpreset_get_name(fluid_defpreset_t* defpreset)
{
  return defpreset->name;
}

/*
 * fluid_defpreset_next
 */
fluid_defpreset_t*
fluid_defpreset_next(fluid_defpreset_t* defpreset)
{
  return defpreset->next;
}


/*
 * fluid_defpreset_noteon
 */
int
fluid_defpreset_noteon(fluid_defpreset_t* defpreset, fluid_synth_t* synth, int chan, int key, int vel)
{
  fluid_preset_zone_t *preset_zone, *global_preset_zone;
  fluid_inst_t* inst;
  fluid_inst_zone_t *inst_zone, *global_inst_zone;
  fluid_sample_t* sample;
  fluid_voice_t* voice;
  fluid_mod_t * mod;
  fluid_mod_t * mod_list[FLUID_NUM_MOD]; /* list for 'sorting' preset modulators */
  int mod_list_count;
  int i;

  global_preset_zone = fluid_defpreset_get_global_zone(defpreset);

  /* run thru all the zones of this preset */
  preset_zone = fluid_defpreset_get_zone(defpreset);
  while (preset_zone != NULL) {

    /* check if the note falls into the key and velocity range of this
       preset */
    if (fluid_zone_inside_range(&preset_zone->range, key, vel)) {

      inst = fluid_preset_zone_get_inst(preset_zone);
      global_inst_zone = fluid_inst_get_global_zone(inst);

      /* run thru all the zones of this instrument */
      inst_zone = fluid_inst_get_zone(inst);
	  while (inst_zone != NULL) {
	/* make sure this instrument zone has a valid sample */
	sample = fluid_inst_zone_get_sample(inst_zone);
	if ((sample == NULL) || fluid_sample_in_rom(sample)) {
	  inst_zone = fluid_inst_zone_next(inst_zone);
	  continue;
	}

	/* check if the instrument zone is ignored and the note falls into
	   the key and velocity range of this  instrument zone.
	   An instrument zone must be ignored when its voice is already running
	   played by a legato passage (see fluid_synth_noteon_monopoly_legato()) */
	if (fluid_zone_inside_range(&inst_zone->range, key, vel)) {

	  /* this is a good zone. allocate a new synthesis process and initialize it */
	  voice = fluid_synth_alloc_voice_LOCAL(synth, sample, chan, key, vel, &inst_zone->range);
	  if (voice == NULL) {
	    return FLUID_FAILED;
	  }


	  /* Instrument level, generators */

	  for (i = 0; i < GEN_LAST; i++) {

	    /* SF 2.01 section 9.4 'bullet' 4:
	     *
	     * A generator in a local instrument zone supersedes a
	     * global instrument zone generator.  Both cases supersede
	     * the default generator -> voice_gen_set */

	    if (inst_zone->gen[i].flags){
	      fluid_voice_gen_set(voice, i, inst_zone->gen[i].val);

	    } else if ((global_inst_zone != NULL) && (global_inst_zone->gen[i].flags)) {
	      fluid_voice_gen_set(voice, i, global_inst_zone->gen[i].val);

	    } else {
	      /* The generator has not been defined in this instrument.
	       * Do nothing, leave it at the default.
	       */
	    }

	  } /* for all generators */

	  /* global instrument zone, modulators: Put them all into a
	   * list. */

	  mod_list_count = 0;

	  if (global_inst_zone){
	    mod = global_inst_zone->mod;
	    while (mod){
	      mod_list[mod_list_count++] = mod;
	      mod = mod->next;
	    }
	  }

	  /* local instrument zone, modulators.
	   * Replace modulators with the same definition in the list:
	   * SF 2.01 page 69, 'bullet' 8
	   */
	  mod = inst_zone->mod;

	  while (mod){

	    /* 'Identical' modulators will be deleted by setting their
	     *  list entry to NULL.  The list length is known, NULL
	     *  entries will be ignored later.  SF2.01 section 9.5.1
	     *  page 69, 'bullet' 3 defines 'identical'.  */

	    for (i = 0; i < mod_list_count; i++){
	      if (mod_list[i] && fluid_mod_test_identity(mod,mod_list[i])){
		mod_list[i] = NULL;
	      }
	    }

	    /* Finally add the new modulator to to the list. */
	    mod_list[mod_list_count++] = mod;
	    mod = mod->next;
	  }

	  /* Add instrument modulators (global / local) to the voice. */
	  for (i = 0; i < mod_list_count; i++){

	    mod = mod_list[i];

	    if (mod != NULL){ /* disabled modulators CANNOT be skipped. */

	      /* Instrument modulators -supersede- existing (default)
	       * modulators.  SF 2.01 page 69, 'bullet' 6 */
	      fluid_voice_add_mod(voice, mod, FLUID_VOICE_OVERWRITE);
	    }
	  }

	  /* Preset level, generators */

	  for (i = 0; i < GEN_LAST; i++) {

	    /* SF 2.01 section 8.5 page 58: If some generators are
	     * encountered at preset level, they should be ignored */
	    if ((i != GEN_STARTADDROFS)
		&& (i != GEN_ENDADDROFS)
		&& (i != GEN_STARTLOOPADDROFS)
		&& (i != GEN_ENDLOOPADDROFS)
		&& (i != GEN_STARTADDRCOARSEOFS)
		&& (i != GEN_ENDADDRCOARSEOFS)
		&& (i != GEN_STARTLOOPADDRCOARSEOFS)
		&& (i != GEN_KEYNUM)
		&& (i != GEN_VELOCITY)
		&& (i != GEN_ENDLOOPADDRCOARSEOFS)
		&& (i != GEN_SAMPLEMODE)
		&& (i != GEN_EXCLUSIVECLASS)
		&& (i != GEN_OVERRIDEROOTKEY)) {

	      /* SF 2.01 section 9.4 'bullet' 9: A generator in a
	       * local preset zone supersedes a global preset zone
	       * generator.  The effect is -added- to the destination
	       * summing node -> voice_gen_incr */

	      if (preset_zone->gen[i].flags) {
		fluid_voice_gen_incr(voice, i, preset_zone->gen[i].val);
	      } else if ((global_preset_zone != NULL) && global_preset_zone->gen[i].flags) {
		fluid_voice_gen_incr(voice, i, global_preset_zone->gen[i].val);
	      } else {
		/* The generator has not been defined in this preset
		 * Do nothing, leave it unchanged.
		 */
	      }
	    } /* if available at preset level */
	  } /* for all generators */


	  /* Global preset zone, modulators: put them all into a
	   * list. */
	  mod_list_count = 0;
	  if (global_preset_zone){
	    mod = global_preset_zone->mod;
	    while (mod){
	      mod_list[mod_list_count++] = mod;
	      mod = mod->next;
	    }
	  }

	  /* Process the modulators of the local preset zone.  Kick
	   * out all identical modulators from the global preset zone
	   * (SF 2.01 page 69, second-last bullet) */

	  mod = preset_zone->mod;
	  while (mod){
	    for (i = 0; i < mod_list_count; i++){
	      if (mod_list[i] && fluid_mod_test_identity(mod,mod_list[i])){
		mod_list[i] = NULL;
	      }
	    }

	    /* Finally add the new modulator to the list. */
	    mod_list[mod_list_count++] = mod;
	    mod = mod->next;
	  }

	  /* Add preset modulators (global / local) to the voice. */
	  for (i = 0; i < mod_list_count; i++){
	    mod = mod_list[i];
	    if ((mod != NULL) && (mod->amount != 0)) { /* disabled modulators can be skipped. */

	      /* Preset modulators -add- to existing instrument /
	       * default modulators.  SF2.01 page 70 first bullet on
	       * page */
	      fluid_voice_add_mod(voice, mod, FLUID_VOICE_ADD);
	    }
	  }

	  /* add the synthesis process to the synthesis loop. */
	  fluid_synth_start_voice(synth, voice);

	  /* Store the ID of the first voice that was created by this noteon event.
	   * Exclusive class may only terminate older voices.
	   * That avoids killing voices, which have just been created.
	   * (a noteon event can create several voice processes with the same exclusive
	   * class - for example when using stereo samples)
	   */
	}

	inst_zone = fluid_inst_zone_next(inst_zone);
      }
	}
    preset_zone = fluid_preset_zone_next(preset_zone);
  }

  return FLUID_OK;
}

/*
 * fluid_defpreset_set_global_zone
 */
int
fluid_defpreset_set_global_zone(fluid_defpreset_t* defpreset, fluid_preset_zone_t* zone)
{
  defpreset->global_zone = zone;
  return FLUID_OK;
}

/*
 * fluid_defpreset_import_sfont
 */
int
fluid_defpreset_import_sfont(fluid_defpreset_t* defpreset,
			     SFPreset* sfpreset,
			     fluid_defsfont_t* defsfont)
{
  fluid_list_t *p;
  SFZone* sfzone;
  fluid_preset_zone_t* zone;
  int count;
  char zone_name[256];
  if (FLUID_STRLEN(sfpreset->name) > 0) {
    FLUID_STRCPY(defpreset->name, sfpreset->name);
  } else {
    FLUID_SNPRINTF(defpreset->name, sizeof(defpreset->name), "Bank%d,Pre%d", sfpreset->bank, sfpreset->prenum);
  }
  defpreset->bank = sfpreset->bank;
  defpreset->num = sfpreset->prenum;
  p = sfpreset->zone;
  count = 0;
  while (p != NULL) {
    sfzone = (SFZone *)fluid_list_get(p);
    FLUID_SNPRINTF(zone_name, sizeof(zone_name), "%s/%d", defpreset->name, count);
    zone = new_fluid_preset_zone(zone_name);
    if (zone == NULL) {
      return FLUID_FAILED;
    }
    if (fluid_preset_zone_import_sfont(zone, sfzone, defsfont) != FLUID_OK) {
      delete_fluid_preset_zone(zone);
      return FLUID_FAILED;
    }
    if ((count == 0) && (fluid_preset_zone_get_inst(zone) == NULL)) {
      fluid_defpreset_set_global_zone(defpreset, zone);
    } else if (fluid_defpreset_add_zone(defpreset, zone) != FLUID_OK) {
      return FLUID_FAILED;
    }
    p = fluid_list_next(p);
    count++;
  }
  return FLUID_OK;
}

/*
 * fluid_defpreset_add_zone
 */
int
fluid_defpreset_add_zone(fluid_defpreset_t* defpreset, fluid_preset_zone_t* zone)
{
  if (defpreset->zone == NULL) {
    zone->next = NULL;
    defpreset->zone = zone;
  } else {
    zone->next = defpreset->zone;
    defpreset->zone = zone;
  }
  return FLUID_OK;
}

/*
 * fluid_defpreset_get_zone
 */
fluid_preset_zone_t*
fluid_defpreset_get_zone(fluid_defpreset_t* defpreset)
{
  return defpreset->zone;
}

/*
 * fluid_defpreset_get_global_zone
 */
fluid_preset_zone_t*
fluid_defpreset_get_global_zone(fluid_defpreset_t* defpreset)
{
  return defpreset->global_zone;
}

/***************************************************************
 *
 *                           PRESET_ZONE
 */

/*
 * fluid_preset_zone_next
 */
fluid_preset_zone_t*
fluid_preset_zone_next(fluid_preset_zone_t* zone)
{
  return zone->next;
}

/*
 * new_fluid_preset_zone
 */
fluid_preset_zone_t*
new_fluid_preset_zone(char *name)
{
  fluid_preset_zone_t* zone = NULL;
  zone = FLUID_NEW(fluid_preset_zone_t);
  if (zone == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  zone->next = NULL;
  zone->name = FLUID_STRDUP(name);
  if (zone->name == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    FLUID_FREE(zone);
    return NULL;
  }
  zone->inst = NULL;
  zone->range.keylo = 0;
  zone->range.keyhi = 128;
  zone->range.vello = 0;
  zone->range.velhi = 128;
  zone->range.ignore = FALSE; 

  /* Flag all generators as unused (default, they will be set when they are found
   * in the sound font).
   * This also sets the generator values to default, but that is of no concern here.*/
  fluid_gen_set_default_values(&zone->gen[0]);
  zone->mod = NULL; /* list of modulators */
  return zone;
}

/*
 * delete_fluid_preset_zone
 */
void
delete_fluid_preset_zone(fluid_preset_zone_t* zone)
{
  fluid_mod_t *mod, *tmp;

  fluid_return_if_fail(zone != NULL);
  
  mod = zone->mod;
  while (mod)	/* delete the modulators */
    {
      tmp = mod;
      mod = mod->next;
      delete_fluid_mod (tmp);
    }

  FLUID_FREE (zone->name);
  delete_fluid_inst (zone->inst);
  FLUID_FREE(zone);
}

/*
 * fluid_preset_zone_import_sfont
 */
int
fluid_preset_zone_import_sfont(fluid_preset_zone_t* zone, SFZone *sfzone, fluid_defsfont_t* defsfont)
{
  fluid_list_t *r;
  SFGen* sfgen;
  int count;
  for (count = 0, r = sfzone->gen; r != NULL; count++) {
    sfgen = (SFGen *)fluid_list_get(r);
    switch (sfgen->id) {
    case GEN_KEYRANGE:
      zone->range.keylo = sfgen->amount.range.lo;
      zone->range.keyhi = sfgen->amount.range.hi;
      break;
    case GEN_VELRANGE:
      zone->range.vello = sfgen->amount.range.lo;
      zone->range.velhi = sfgen->amount.range.hi;
      break;
    case GEN_ATTENUATION:
      /* EMU8k/10k hardware applies a scale factor to initial attenuation generator values set at
       * preset and instrument level */
      zone->gen[sfgen->id].val = (fluid_real_t) sfgen->amount.sword * EMU_ATTENUATION_FACTOR;
      zone->gen[sfgen->id].flags = GEN_SET;
      break;
    default:
      /* FIXME: some generators have an unsigne word amount value but i don't know which ones */
      zone->gen[sfgen->id].val = (fluid_real_t) sfgen->amount.sword;
      zone->gen[sfgen->id].flags = GEN_SET;
      break;
    }
    r = fluid_list_next(r);
  }
  if ((sfzone->instsamp != NULL) && (sfzone->instsamp->data != NULL)) {
    zone->inst = (fluid_inst_t*) new_fluid_inst();
    if (zone->inst == NULL) {
      FLUID_LOG(FLUID_ERR, "Out of memory");
      return FLUID_FAILED;
    }
    if (fluid_inst_import_sfont(zone, zone->inst, 
							(SFInst *) sfzone->instsamp->data, defsfont) != FLUID_OK) {
      return FLUID_FAILED;
    }
  }

  /* Import the modulators (only SF2.1 and higher) */
  for (count = 0, r = sfzone->mod; r != NULL; count++) {

    SFMod* mod_src = (SFMod *)fluid_list_get(r);
    fluid_mod_t * mod_dest = new_fluid_mod();
    int type;

    if (mod_dest == NULL){
      return FLUID_FAILED;
    }
    mod_dest->next = NULL; /* pointer to next modulator, this is the end of the list now.*/

    /* *** Amount *** */
    mod_dest->amount = mod_src->amount;

    /* *** Source *** */
    mod_dest->src1 = mod_src->src & 127; /* index of source 1, seven-bit value, SF2.01 section 8.2, page 50 */
    mod_dest->flags1 = 0;

    /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
    if (mod_src->src & (1<<7)){
      mod_dest->flags1 |= FLUID_MOD_CC;
    } else {
      mod_dest->flags1 |= FLUID_MOD_GC;
    }

    /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
    if (mod_src->src & (1<<8)){
      mod_dest->flags1 |= FLUID_MOD_NEGATIVE;
    } else {
      mod_dest->flags1 |= FLUID_MOD_POSITIVE;
    }

    /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
    if (mod_src->src & (1<<9)){
      mod_dest->flags1 |= FLUID_MOD_BIPOLAR;
    } else {
      mod_dest->flags1 |= FLUID_MOD_UNIPOLAR;
    }

    /* modulator source types: SF2.01 section 8.2.1 page 52 */
    type=(mod_src->src) >> 10;
    type &= 63; /* type is a 6-bit value */
    if (type == 0){
      mod_dest->flags1 |= FLUID_MOD_LINEAR;
    } else if (type == 1){
      mod_dest->flags1 |= FLUID_MOD_CONCAVE;
    } else if (type == 2){
      mod_dest->flags1 |= FLUID_MOD_CONVEX;
    } else if (type == 3){
      mod_dest->flags1 |= FLUID_MOD_SWITCH;
    } else {
      /* This shouldn't happen - unknown type!
       * Deactivate the modulator by setting the amount to 0. */
      mod_dest->amount=0;
    }

    /* *** Dest *** */
    mod_dest->dest = mod_src->dest; /* index of controlled generator */

    /* *** Amount source *** */
    mod_dest->src2 = mod_src->amtsrc & 127; /* index of source 2, seven-bit value, SF2.01 section 8.2, p.50 */
    mod_dest->flags2 = 0;

    /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
    if (mod_src->amtsrc & (1<<7)){
      mod_dest->flags2 |= FLUID_MOD_CC;
    } else {
      mod_dest->flags2 |= FLUID_MOD_GC;
    }

    /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
    if (mod_src->amtsrc & (1<<8)){
      mod_dest->flags2 |= FLUID_MOD_NEGATIVE;
    } else {
      mod_dest->flags2 |= FLUID_MOD_POSITIVE;
    }

    /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
    if (mod_src->amtsrc & (1<<9)){
      mod_dest->flags2 |= FLUID_MOD_BIPOLAR;
    } else {
      mod_dest->flags2 |= FLUID_MOD_UNIPOLAR;
    }

    /* modulator source types: SF2.01 section 8.2.1 page 52 */
    type = (mod_src->amtsrc) >> 10;
    type &= 63; /* type is a 6-bit value */
    if (type == 0){
      mod_dest->flags2 |= FLUID_MOD_LINEAR;
    } else if (type == 1){
      mod_dest->flags2 |= FLUID_MOD_CONCAVE;
    } else if (type == 2){
      mod_dest->flags2 |= FLUID_MOD_CONVEX;
    } else if (type == 3){
      mod_dest->flags2 |= FLUID_MOD_SWITCH;
    } else {
      /* This shouldn't happen - unknown type!
       * Deactivate the modulator by setting the amount to 0. */
      mod_dest->amount=0;
    }

    /* *** Transform *** */
    /* SF2.01 only uses the 'linear' transform (0).
     * Deactivate the modulator by setting the amount to 0 in any other case.
     */
    if (mod_src->trans !=0){
      mod_dest->amount = 0;
    }

    /* Store the new modulator in the zone The order of modulators
     * will make a difference, at least in an instrument context: The
     * second modulator overwrites the first one, if they only differ
     * in amount. */
    if (count == 0){
      zone->mod = mod_dest;
    } else {
      fluid_mod_t * last_mod = zone->mod;

      /* Find the end of the list */
      while (last_mod->next != NULL){
	last_mod=last_mod->next;
      }

      last_mod->next = mod_dest;
    }

    r = fluid_list_next(r);
  } /* foreach modulator */

  return FLUID_OK;
}

/*
 * fluid_preset_zone_get_inst
 */
fluid_inst_t*
fluid_preset_zone_get_inst(fluid_preset_zone_t* zone)
{
  return zone->inst;
}


/***************************************************************
 *
 *                           INST
 */

/*
 * new_fluid_inst
 */
fluid_inst_t*
new_fluid_inst()
{
  fluid_inst_t* inst = FLUID_NEW(fluid_inst_t);
  if (inst == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  inst->name[0] = 0;
  inst->global_zone = NULL;
  inst->zone = NULL;
  return inst;
}

/*
 * delete_fluid_inst
 */
void
delete_fluid_inst(fluid_inst_t* inst)
{
  fluid_inst_zone_t* zone;
  
  fluid_return_if_fail(inst != NULL);
  
    delete_fluid_inst_zone(inst->global_zone);
    inst->global_zone = NULL;
    
  zone = inst->zone;
  while (zone != NULL) {
    inst->zone = zone->next;
    delete_fluid_inst_zone(zone);
    zone = inst->zone;
  }
  FLUID_FREE(inst);
}

/*
 * fluid_inst_set_global_zone
 */
int
fluid_inst_set_global_zone(fluid_inst_t* inst, fluid_inst_zone_t* zone)
{
  inst->global_zone = zone;
  return FLUID_OK;
}

/*
 * fluid_inst_import_sfont
 */
int
fluid_inst_import_sfont(fluid_preset_zone_t* preset_zone, fluid_inst_t* inst, 
						SFInst *sfinst, fluid_defsfont_t* defsfont)
{
  fluid_list_t *p;
  SFZone* sfzone;
  fluid_inst_zone_t* inst_zone;
  char zone_name[256];
  int count;

  p = sfinst->zone;
  if (FLUID_STRLEN(sfinst->name) > 0) {
    FLUID_STRCPY(inst->name, sfinst->name);
  } else {
    FLUID_STRCPY(inst->name, "<untitled>");
  }

  count = 0;
  while (p != NULL) {

    sfzone = (SFZone *)fluid_list_get(p);
    FLUID_SNPRINTF(zone_name, sizeof(zone_name), "%s/%d", inst->name, count);

    inst_zone = new_fluid_inst_zone(zone_name);
    if (inst_zone == NULL) {
      return FLUID_FAILED;
    }

    if (fluid_inst_zone_import_sfont(preset_zone, inst_zone, sfzone, defsfont) != FLUID_OK) {
      delete_fluid_inst_zone(inst_zone);
      return FLUID_FAILED;
    }

    if ((count == 0) && (fluid_inst_zone_get_sample(inst_zone) == NULL)) {
      fluid_inst_set_global_zone(inst, inst_zone);

    } else if (fluid_inst_add_zone(inst, inst_zone) != FLUID_OK) {
      return FLUID_FAILED;
    }

    p = fluid_list_next(p);
    count++;
  }
  return FLUID_OK;
}

/*
 * fluid_inst_add_zone
 */
int
fluid_inst_add_zone(fluid_inst_t* inst, fluid_inst_zone_t* zone)
{
  if (inst->zone == NULL) {
    zone->next = NULL;
    inst->zone = zone;
  } else {
    zone->next = inst->zone;
    inst->zone = zone;
  }
  return FLUID_OK;
}

/*
 * fluid_inst_get_zone
 */
fluid_inst_zone_t*
fluid_inst_get_zone(fluid_inst_t* inst)
{
  return inst->zone;
}

/*
 * fluid_inst_get_global_zone
 */
fluid_inst_zone_t*
fluid_inst_get_global_zone(fluid_inst_t* inst)
{
  return inst->global_zone;
}

/***************************************************************
 *
 *                           INST_ZONE
 */

/*
 * new_fluid_inst_zone
 */
fluid_inst_zone_t*
new_fluid_inst_zone(char* name)
{
  fluid_inst_zone_t* zone = NULL;
  zone = FLUID_NEW(fluid_inst_zone_t);
  if (zone == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  zone->next = NULL;
  zone->name = FLUID_STRDUP(name);
  if (zone->name == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    FLUID_FREE(zone);
    return NULL;
  }
  zone->sample = NULL;
  zone->range.keylo = 0;
  zone->range.keyhi = 128;
  zone->range.vello = 0;
  zone->range.velhi = 128;
  zone->range.ignore = FALSE;
  /* Flag the generators as unused.
   * This also sets the generator values to default, but they will be overwritten anyway, if used.*/
  fluid_gen_set_default_values(&zone->gen[0]);
  zone->mod=NULL; /* list of modulators */
  return zone;
}

/*
 * delete_fluid_inst_zone
 */
void
delete_fluid_inst_zone(fluid_inst_zone_t* zone)
{
  fluid_mod_t *mod, *tmp;

  fluid_return_if_fail(zone != NULL);
  
  mod = zone->mod;
  while (mod)	/* delete the modulators */
    {
      tmp = mod;
      mod = mod->next;
      delete_fluid_mod (tmp);
    }

  FLUID_FREE (zone->name);
  FLUID_FREE(zone);
}

/*
 * fluid_inst_zone_next
 */
fluid_inst_zone_t*
fluid_inst_zone_next(fluid_inst_zone_t* zone)
{
  return zone->next;
}

/*
 * fluid_inst_zone_import_sfont
 */
int
fluid_inst_zone_import_sfont(fluid_preset_zone_t* preset_zone, fluid_inst_zone_t* inst_zone,
                             SFZone *sfzone, fluid_defsfont_t* defsfont)
{
  fluid_list_t *r;
  SFGen* sfgen;
  int count;

  for (count = 0, r = sfzone->gen; r != NULL; count++) {
    sfgen = (SFGen *)fluid_list_get(r);
    switch (sfgen->id) {
    case GEN_KEYRANGE:
      inst_zone->range.keylo = sfgen->amount.range.lo;
      inst_zone->range.keyhi = sfgen->amount.range.hi;
      break;
    case GEN_VELRANGE:
      inst_zone->range.vello = sfgen->amount.range.lo;
      inst_zone->range.velhi = sfgen->amount.range.hi;
      break;
    case GEN_ATTENUATION:
      /* EMU8k/10k hardware applies a scale factor to initial attenuation generator values set at
       * preset and instrument level */
      inst_zone->gen[sfgen->id].val = (fluid_real_t) sfgen->amount.sword * EMU_ATTENUATION_FACTOR;
      inst_zone->gen[sfgen->id].flags = GEN_SET;
      break;
    default:
      /* FIXME: some generators have an unsigned word amount value but
	 i don't know which ones */
      inst_zone->gen[sfgen->id].val = (fluid_real_t) sfgen->amount.sword;
      inst_zone->gen[sfgen->id].flags = GEN_SET;
      break;
    }
    r = fluid_list_next(r);
  }
  
  /* adjust instrument zone keyrange to integrate preset zone keyrange */
  if (preset_zone->range.keylo > inst_zone->range.keylo) inst_zone->range.keylo = preset_zone->range.keylo;
  if (preset_zone->range.keyhi < inst_zone->range.keyhi) inst_zone->range.keyhi = preset_zone->range.keyhi;
  /* adjust instrument zone to integrate  preset zone velrange */
  if (preset_zone->range.vello > inst_zone->range.vello) inst_zone->range.vello = preset_zone->range.vello;
  if (preset_zone->range.velhi < inst_zone->range.velhi) inst_zone->range.velhi = preset_zone->range.velhi;

  /* FIXME */
/*    if (zone->gen[GEN_EXCLUSIVECLASS].flags == GEN_SET) { */
/*      FLUID_LOG(FLUID_DBG, "ExclusiveClass=%d\n", (int) zone->gen[GEN_EXCLUSIVECLASS].val); */
/*    } */

  /* fixup sample pointer */
  if ((sfzone->instsamp != NULL) && (sfzone->instsamp->data != NULL))
    inst_zone->sample = ((SFSample *)(sfzone->instsamp->data))->fluid_sample;

  /* Import the modulators (only SF2.1 and higher) */
  for (count = 0, r = sfzone->mod; r != NULL; count++) {
    SFMod* mod_src = (SFMod *)fluid_list_get(r);
    int type;
    fluid_mod_t* mod_dest;

    mod_dest = new_fluid_mod();
    if (mod_dest == NULL){
      return FLUID_FAILED;
    }

    mod_dest->next = NULL; /* pointer to next modulator, this is the end of the list now.*/

    /* *** Amount *** */
    mod_dest->amount = mod_src->amount;

    /* *** Source *** */
    mod_dest->src1 = mod_src->src & 127; /* index of source 1, seven-bit value, SF2.01 section 8.2, page 50 */
    mod_dest->flags1 = 0;

    /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
    if (mod_src->src & (1<<7)){
      mod_dest->flags1 |= FLUID_MOD_CC;
    } else {
      mod_dest->flags1 |= FLUID_MOD_GC;
    }

    /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
    if (mod_src->src & (1<<8)){
      mod_dest->flags1 |= FLUID_MOD_NEGATIVE;
    } else {
      mod_dest->flags1 |= FLUID_MOD_POSITIVE;
    }

    /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
    if (mod_src->src & (1<<9)){
      mod_dest->flags1 |= FLUID_MOD_BIPOLAR;
    } else {
      mod_dest->flags1 |= FLUID_MOD_UNIPOLAR;
    }

    /* modulator source types: SF2.01 section 8.2.1 page 52 */
    type = (mod_src->src) >> 10;
    type &= 63; /* type is a 6-bit value */
    if (type == 0){
      mod_dest->flags1 |= FLUID_MOD_LINEAR;
    } else if (type == 1){
      mod_dest->flags1 |= FLUID_MOD_CONCAVE;
    } else if (type == 2){
      mod_dest->flags1 |= FLUID_MOD_CONVEX;
    } else if (type == 3){
      mod_dest->flags1 |= FLUID_MOD_SWITCH;
    } else {
      /* This shouldn't happen - unknown type!
       * Deactivate the modulator by setting the amount to 0. */
      mod_dest->amount = 0;
    }

    /* *** Dest *** */
    mod_dest->dest=mod_src->dest; /* index of controlled generator */

    /* *** Amount source *** */
    mod_dest->src2=mod_src->amtsrc & 127; /* index of source 2, seven-bit value, SF2.01 section 8.2, page 50 */
    mod_dest->flags2 = 0;

    /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
    if (mod_src->amtsrc & (1<<7)){
      mod_dest->flags2 |= FLUID_MOD_CC;
    } else {
      mod_dest->flags2 |= FLUID_MOD_GC;
    }

    /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
    if (mod_src->amtsrc & (1<<8)){
      mod_dest->flags2 |= FLUID_MOD_NEGATIVE;
    } else {
      mod_dest->flags2 |= FLUID_MOD_POSITIVE;
    }

    /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
    if (mod_src->amtsrc & (1<<9)){
      mod_dest->flags2 |= FLUID_MOD_BIPOLAR;
    } else {
      mod_dest->flags2 |= FLUID_MOD_UNIPOLAR;
    }

    /* modulator source types: SF2.01 section 8.2.1 page 52 */
    type=(mod_src->amtsrc) >> 10;
    type &= 63; /* type is a 6-bit value */
    if (type == 0){
      mod_dest->flags2 |= FLUID_MOD_LINEAR;
    } else if (type == 1){
      mod_dest->flags2 |= FLUID_MOD_CONCAVE;
    } else if (type == 2){
      mod_dest->flags2 |= FLUID_MOD_CONVEX;
    } else if (type == 3){
      mod_dest->flags2 |= FLUID_MOD_SWITCH;
    } else {
      /* This shouldn't happen - unknown type!
       * Deactivate the modulator by setting the amount to 0. */
      mod_dest->amount = 0;
    }

    /* *** Transform *** */
    /* SF2.01 only uses the 'linear' transform (0).
     * Deactivate the modulator by setting the amount to 0 in any other case.
     */
    if (mod_src->trans !=0){
      mod_dest->amount = 0;
    }

    /* Store the new modulator in the zone
     * The order of modulators will make a difference, at least in an instrument context:
     * The second modulator overwrites the first one, if they only differ in amount. */
    if (count == 0){
      inst_zone->mod=mod_dest;
    } else {
      fluid_mod_t * last_mod=inst_zone->mod;
      /* Find the end of the list */
      while (last_mod->next != NULL){
	last_mod=last_mod->next;
      }
      last_mod->next=mod_dest;
    }

    r = fluid_list_next(r);
  } /* foreach modulator */
  return FLUID_OK;
}

/*
 * fluid_inst_zone_get_sample
 */
fluid_sample_t*
fluid_inst_zone_get_sample(fluid_inst_zone_t* zone)
{
  return zone->sample;
}


int
fluid_zone_inside_range(fluid_zone_range_t* range, int key, int vel)
{
    /* ignoreInstrumentZone is set in mono legato playing */
    int ignore_zone = range->ignore;
    
    /* Reset the 'ignore' request */
    range->ignore = FALSE;
    
  return !ignore_zone && ((range->keylo <= key) &&
	  (range->keyhi >= key) &&
	  (range->vello <= vel) &&
	  (range->velhi >= vel));
}

/***************************************************************
 *
 *                           SAMPLE
 */

/*
 * fluid_sample_in_rom
 */
int
fluid_sample_in_rom(fluid_sample_t* sample)
{
  return (sample->sampletype & FLUID_SAMPLETYPE_ROM);
}


/*
 * fluid_sample_import_sfont
 */
int
fluid_sample_import_sfont(fluid_sample_t* sample, SFSample* sfsample, fluid_defsfont_t* defsfont)
{
  FLUID_STRCPY(sample->name, sfsample->name);

  sample->source_start = sfsample->start;
  sample->source_end = (sfsample->end > 0) ? sfsample->end - 1 : 0; /* marks last sample, contrary to SF spec. */
  sample->source_loopstart = sfsample->loopstart;
  sample->source_loopend = sfsample->loopend;

  sample->start = sample->source_start;
  sample->end = sample->source_end;
  sample->loopstart = sample->source_loopstart;
  sample->loopend = sample->source_loopend;
  sample->samplerate = sfsample->samplerate;
  sample->origpitch = sfsample->origpitch;
  sample->pitchadj = sfsample->pitchadj;
  sample->sampletype = sfsample->sampletype;

  if (defsfont->dynamic_samples)
  {
    sample->notify = dynamic_samples_sample_notify;
  }

  if (fluid_sample_validate(sample, defsfont->samplesize) == FLUID_FAILED)
  {
      return FLUID_FAILED;
  }

  return FLUID_OK;
}

/* Called if a sample is no longer used by a voice. Used by dynamic sample loading
 * to unload a sample that is not used by any loaded presets anymore but couldn't
 * be unloaded straight away because it was still in use by a voice. */
static int dynamic_samples_sample_notify(fluid_sample_t *sample, int reason)
{
    if (reason == FLUID_SAMPLE_DONE && sample->preset_count == 0)
    {
        unload_sample(sample);
    }

    return FLUID_OK;
}

/* Called if a preset has been selected for or unselected from a channel. Used by
 * dynamic sample loading to load and unload samples on demand. */
static int dynamic_samples_preset_notify(fluid_preset_t *preset, int reason, int chan)
{
    fluid_defsfont_t *defsfont;

    if (reason == FLUID_PRESET_SELECTED)
    {
        FLUID_LOG(FLUID_DBG, "Selected preset '%s' on channel %d", fluid_preset_get_name(preset), chan);
        defsfont = fluid_sfont_get_data(preset->sfont);
        load_preset_samples(defsfont, preset);
    }
    else if (reason == FLUID_PRESET_UNSELECTED)
    {
        FLUID_LOG(FLUID_DBG, "Deselected preset '%s' from channel %d", fluid_preset_get_name(preset), chan);
        defsfont = fluid_sfont_get_data(preset->sfont);
        unload_preset_samples(defsfont, preset);
    }

    return FLUID_OK;
}

/* Manually loads all samples used by a preset. */
static int dynamic_samples_preset_load(fluid_preset_t *preset)
{
    fluid_defpreset_t *defpreset = fluid_preset_get_data(preset);
    fluid_return_val_if_fail(defpreset != NULL, FLUID_FAILED);

    if (fluid_preset_is_loaded(preset))
    {
        return FLUID_OK;
    }

    if (load_preset_samples(defpreset->defsfont, preset) == FLUID_FAILED)
    {
        return FLUID_FAILED;
    }

    defpreset->loaded_manually = TRUE;
    return FLUID_OK;
}

/* Manually unloads all samples used by a preset. */
static int dynamic_samples_preset_unload(fluid_preset_t *preset)
{
    fluid_defpreset_t *defpreset = fluid_preset_get_data(preset);
    fluid_return_val_if_fail(defpreset != NULL, FLUID_FAILED);

    if (!defpreset->loaded_manually)
    {
        return FLUID_FAILED;
    }

    if (unload_preset_samples(defpreset->defsfont, preset) == FLUID_FAILED)
    {
        return FLUID_FAILED;
    }

    defpreset->loaded_manually = FALSE;
    return FLUID_OK;
}

/* Checks if a preset has been manuall loaded. */
static int dynamic_samples_preset_is_loaded(fluid_preset_t *preset)
{
    fluid_defpreset_t *defpreset = fluid_preset_get_data(preset);
    fluid_return_val_if_fail(defpreset != NULL, FALSE);
    return defpreset->loaded_manually;
}


/* Walk through all samples used by the passed in preset and make sure that the
 * sample data is loaded for each sample. Used by dynamic sample loading. */
static int load_preset_samples(fluid_defsfont_t *defsfont, fluid_preset_t *preset)
{
    fluid_defpreset_t *defpreset;
    fluid_preset_zone_t *preset_zone;
    fluid_inst_t *inst;
    fluid_inst_zone_t *inst_zone;
    fluid_sample_t *sample;
    SFData *sffile = NULL;

    defpreset = fluid_preset_get_data(preset);
    preset_zone = fluid_defpreset_get_zone(defpreset);
    while (preset_zone != NULL)
    {
        inst = fluid_preset_zone_get_inst(preset_zone);
        inst_zone = fluid_inst_get_zone(inst);
        while (inst_zone != NULL) {
            sample = fluid_inst_zone_get_sample(inst_zone);

            if ((sample != NULL) && (sample->start != sample->end))
            {
                sample->preset_count++;

                /* If this is the first time this sample has been selected,
                 * load the sampledata */
                if (sample->preset_count == 1)
                {
                    /* Make sure we have an open Soundfont file. Do this here
                     * to avoid having to open the file if no loading is necessary
                     * for a preset */
                    if (sffile == NULL)
                    {
                        sffile = fluid_sffile_open(defsfont->filename, defsfont->fcbs);
                        if (sffile == NULL)
                        {
                            FLUID_LOG(FLUID_ERR, "Unable to open Soundfont file");
                            return FLUID_FAILED;
                        }
                    }

                    if (fluid_defsfont_load_sampledata(defsfont, sffile, sample) == FLUID_OK)
                    {
                        fluid_sample_sanitize_loop(sample, (sample->end + 1) * sizeof(short));
                        fluid_voice_optimize_sample(sample);
                    }
                    else
                    {
                        FLUID_LOG(FLUID_ERR, "Unable to load sample '%s', disabling", sample->name);
                        sample->start = sample->end = 0;
                    }
                }
            }

            inst_zone = fluid_inst_zone_next(inst_zone);
        }
        preset_zone = fluid_preset_zone_next(preset_zone);
    }

    if (sffile != NULL)
    {
        fluid_sffile_close(sffile);
    }

    return FLUID_OK;
}

/* Walk through all samples used by the passed in preset and unload the sample data
 * of each sample that is not used by any selected preset anymore. Used by dynamic
 * sample loading. */
static int unload_preset_samples(fluid_defsfont_t *defsfont, fluid_preset_t *preset)
{
    fluid_defpreset_t *defpreset;
    fluid_preset_zone_t *preset_zone;
    fluid_inst_t *inst;
    fluid_inst_zone_t *inst_zone;
    fluid_sample_t *sample;

    defpreset = fluid_preset_get_data(preset);
    preset_zone = fluid_defpreset_get_zone(defpreset);
    while (preset_zone != NULL)
    {
        inst = fluid_preset_zone_get_inst(preset_zone);
        inst_zone = fluid_inst_get_zone(inst);
        while (inst_zone != NULL) {
            sample = fluid_inst_zone_get_sample(inst_zone);

            if ((sample != NULL) && (sample->preset_count > 0))
            {
                sample->preset_count--;

                /* If the sample is not used by any preset or used by a
                 * sounding voice, unload it from the sample cache. If it's
                 * still in use by a voice, dynamic_samples_sample_notify will
                 * take care of unloading the sample as soon as the voice is
                 * finished with it (but only on the next API call). */
                if (sample->preset_count == 0 && sample->refcount == 0)
                {
                    unload_sample(sample);
                }
            }

            inst_zone = fluid_inst_zone_next(inst_zone);
        }
        preset_zone = fluid_preset_zone_next(preset_zone);
    }

    return FLUID_OK;
}

/* Unload an unused sample from the samplecache */
static void unload_sample(fluid_sample_t *sample)
{
    fluid_return_if_fail(sample != NULL);
    fluid_return_if_fail(sample->data != NULL);
    fluid_return_if_fail(sample->preset_count == 0);
    fluid_return_if_fail(sample->refcount == 0);

    FLUID_LOG(FLUID_DBG, "Unloading sample '%s'", sample->name);

    if (fluid_samplecache_unload(sample->data) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Unable to unload sample '%s'", sample->name);
    }
    else
    {
        sample->data = NULL;
        sample->data24 = NULL;
    }
}
