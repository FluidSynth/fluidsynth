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
static int dynamic_samples_preset_notify(fluid_preset_t *preset, int reason, int chan);
static int dynamic_samples_sample_notify(fluid_sample_t *sample, int reason);
static int fluid_preset_zone_create_voice_zones(fluid_preset_zone_t *preset_zone);
static fluid_inst_t *find_inst_by_idx(fluid_defsfont_t *defsfont, int idx);


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
fluid_sfloader_t *new_fluid_defsfloader(fluid_settings_t *settings)
{
    fluid_sfloader_t *loader;
    fluid_return_val_if_fail(settings != NULL, NULL);

    loader = new_fluid_sfloader(fluid_defsfloader_load, delete_fluid_sfloader);

    if(loader == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    fluid_sfloader_set_data(loader, settings);

    return loader;
}

fluid_sfont_t *fluid_defsfloader_load(fluid_sfloader_t *loader, const char *filename)
{
    fluid_defsfont_t *defsfont;
    fluid_sfont_t *sfont;

    defsfont = new_fluid_defsfont(fluid_sfloader_get_data(loader));

    if(defsfont == NULL)
    {
        return NULL;
    }

    sfont = new_fluid_sfont(fluid_defsfont_sfont_get_name,
                            fluid_defsfont_sfont_get_preset,
                            fluid_defsfont_sfont_iteration_start,
                            fluid_defsfont_sfont_iteration_next,
                            fluid_defsfont_sfont_delete);

    if(sfont == NULL)
    {
        delete_fluid_defsfont(defsfont);
        return NULL;
    }

    fluid_sfont_set_data(sfont, defsfont);

    defsfont->sfont = sfont;

    if(fluid_defsfont_load(defsfont, &loader->file_callbacks, filename) == FLUID_FAILED)
    {
        fluid_defsfont_sfont_delete(sfont);
        return NULL;
    }

    return sfont;
}



/***************************************************************
 *
 *                           PUBLIC INTERFACE
 */

int fluid_defsfont_sfont_delete(fluid_sfont_t *sfont)
{
    if(delete_fluid_defsfont(fluid_sfont_get_data(sfont)) != FLUID_OK)
    {
        return -1;
    }

    delete_fluid_sfont(sfont);
    return 0;
}

const char *fluid_defsfont_sfont_get_name(fluid_sfont_t *sfont)
{
    return fluid_defsfont_get_name(fluid_sfont_get_data(sfont));
}

fluid_preset_t *
fluid_defsfont_sfont_get_preset(fluid_sfont_t *sfont, int bank, int prenum)
{
    return fluid_defsfont_get_preset(fluid_sfont_get_data(sfont), bank, prenum);
}

void fluid_defsfont_sfont_iteration_start(fluid_sfont_t *sfont)
{
    fluid_defsfont_iteration_start(fluid_sfont_get_data(sfont));
}

fluid_preset_t *fluid_defsfont_sfont_iteration_next(fluid_sfont_t *sfont)
{
    return fluid_defsfont_iteration_next(fluid_sfont_get_data(sfont));
}

void fluid_defpreset_preset_delete(fluid_preset_t *preset)
{
    fluid_defsfont_t *defsfont;
    fluid_defpreset_t *defpreset;

    defsfont = fluid_sfont_get_data(preset->sfont);
    defpreset = fluid_preset_get_data(preset);

    if(defsfont)
    {
        defsfont->preset = fluid_list_remove(defsfont->preset, defpreset);
    }

    delete_fluid_defpreset(defpreset);
    delete_fluid_preset(preset);
}

const char *fluid_defpreset_preset_get_name(fluid_preset_t *preset)
{
    return fluid_defpreset_get_name(fluid_preset_get_data(preset));
}

int fluid_defpreset_preset_get_banknum(fluid_preset_t *preset)
{
    return fluid_defpreset_get_banknum(fluid_preset_get_data(preset));
}

int fluid_defpreset_preset_get_num(fluid_preset_t *preset)
{
    return fluid_defpreset_get_num(fluid_preset_get_data(preset));
}

int fluid_defpreset_preset_noteon(fluid_preset_t *preset, fluid_synth_t *synth,
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
fluid_defsfont_t *new_fluid_defsfont(fluid_settings_t *settings)
{
    fluid_defsfont_t *defsfont;

    defsfont = FLUID_NEW(fluid_defsfont_t);

    if(defsfont == NULL)
    {
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
int delete_fluid_defsfont(fluid_defsfont_t *defsfont)
{
    fluid_list_t *list;
    fluid_preset_t *preset;
    fluid_sample_t *sample;

    fluid_return_val_if_fail(defsfont != NULL, FLUID_OK);

    /* Check that no samples are currently used */
    for(list = defsfont->sample; list; list = fluid_list_next(list))
    {
        sample = (fluid_sample_t *) fluid_list_get(list);

        if(sample->refcount != 0)
        {
            return FLUID_FAILED;
        }
    }

    if(defsfont->filename != NULL)
    {
        FLUID_FREE(defsfont->filename);
    }

    for(list = defsfont->sample; list; list = fluid_list_next(list))
    {
        sample = (fluid_sample_t *) fluid_list_get(list);

        /* If the sample data pointer is different to the sampledata chunk of
         * the soundfont, then the sample has been loaded individually (SF3)
         * and needs to be unloaded explicitly. This is safe even if using
         * dynamic sample loading, as the sample_unload mechanism sets
         * sample->data to NULL after unload. */
        if ((sample->data != NULL) && (sample->data != defsfont->sampledata))
        {
            fluid_samplecache_unload(sample->data);
        }
        delete_fluid_sample(sample);
    }

    if(defsfont->sample)
    {
        delete_fluid_list(defsfont->sample);
    }

    if(defsfont->sampledata != NULL)
    {
        fluid_samplecache_unload(defsfont->sampledata);
    }

    for(list = defsfont->preset; list; list = fluid_list_next(list))
    {
        preset = (fluid_preset_t *)fluid_list_get(list);
        fluid_defpreset_preset_delete(preset);
    }

    delete_fluid_list(defsfont->preset);

    for(list = defsfont->inst; list; list = fluid_list_next(list))
    {
        delete_fluid_inst(fluid_list_get(list));
    }

    delete_fluid_list(defsfont->inst);

    FLUID_FREE(defsfont);
    return FLUID_OK;
}

/*
 * fluid_defsfont_get_name
 */
const char *fluid_defsfont_get_name(fluid_defsfont_t *defsfont)
{
    return defsfont->filename;
}

/* Load sample data for a single sample from the Soundfont file.
 * Returns FLUID_OK on error, otherwise FLUID_FAILED
 */
int fluid_defsfont_load_sampledata(fluid_defsfont_t *defsfont, SFData *sfdata, fluid_sample_t *sample)
{
    int num_samples;
    unsigned int source_end = sample->source_end;

    /* For uncompressed samples we want to include the 46 zero sample word area following each sample
     * in the Soundfont. Otherwise samples with loopend > end, which we have decided not to correct, would
     * be corrected after all in fluid_sample_sanitize_loop */
    if(!(sample->sampletype & FLUID_SAMPLETYPE_OGG_VORBIS))
    {
        source_end += 46;  /* Length of zero sample word after each sample, according to SF specs */

        /* Safeguard against Soundfonts that are not quite valid and don't include 46 sample words after the
         * last sample */
        if(source_end >= (defsfont->samplesize  / sizeof(short)))
        {
            source_end = defsfont->samplesize  / sizeof(short);
        }
    }

    num_samples = fluid_samplecache_load(
                      sfdata, sample->source_start, source_end, sample->sampletype,
                      defsfont->mlock, &sample->data, &sample->data24);

    if(num_samples < 0)
    {
        return FLUID_FAILED;
    }

    if(num_samples == 0)
    {
        sample->start = sample->end = 0;
        sample->loopstart = sample->loopend = 0;
        return FLUID_OK;
    }

    /* Ogg Vorbis samples already have loop pointers relative to the invididual decompressed sample,
     * but SF2 samples are relative to sample chunk start, so they need to be adjusted */
    if(!(sample->sampletype & FLUID_SAMPLETYPE_OGG_VORBIS))
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
    if(!sf3_file)
    {
        int read_samples;
        int num_samples = sfdata->samplesize / sizeof(short);

        read_samples = fluid_samplecache_load(sfdata, 0, num_samples - 1, 0, defsfont->mlock,
                                              &defsfont->sampledata, &defsfont->sample24data);

        if(read_samples != num_samples)
        {
            FLUID_LOG(FLUID_ERR, "Attempted to read %d words of sample data, but got %d instead",
                      num_samples, read_samples);
            return FLUID_FAILED;
        }
    }

    for(list = defsfont->sample; list; list = fluid_list_next(list))
    {
        sample = fluid_list_get(list);

        if(sf3_file)
        {
            /* SF3 samples get loaded individually, as most (or all) of them are in Ogg Vorbis format
             * anyway */
            if(fluid_defsfont_load_sampledata(defsfont, sfdata, sample) == FLUID_FAILED)
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
int fluid_defsfont_load(fluid_defsfont_t *defsfont, const fluid_file_callbacks_t *fcbs, const char *file)
{
    SFData *sfdata;
    fluid_list_t *p;
    SFPreset *sfpreset;
    SFSample *sfsample;
    fluid_sample_t *sample;
    fluid_defpreset_t *defpreset = NULL;

    defsfont->filename = FLUID_STRDUP(file);

    if(defsfont->filename == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return FLUID_FAILED;
    }

    defsfont->fcbs = fcbs;

    /* The actual loading is done in the sfont and sffile files */
    sfdata = fluid_sffile_open(file, fcbs);

    if(sfdata == NULL)
    {
        /* error message already printed */
        return FLUID_FAILED;
    }

    if(fluid_sffile_parse_presets(sfdata) == FLUID_FAILED)
    {
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

    while(p != NULL)
    {
        sfsample = (SFSample *)fluid_list_get(p);

        sample = new_fluid_sample();

        if(sample == NULL)
        {
            goto err_exit;
        }

        if(fluid_sample_import_sfont(sample, sfsample, defsfont) == FLUID_OK)
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
    if(!defsfont->dynamic_samples)
    {
        if(fluid_defsfont_load_all_sampledata(defsfont, sfdata) == FLUID_FAILED)
        {
            FLUID_LOG(FLUID_ERR, "Unable to load all sample data");
            goto err_exit;
        }
    }

    /* Load all the presets */
    p = sfdata->preset;

    while(p != NULL)
    {
        sfpreset = (SFPreset *)fluid_list_get(p);
        defpreset = new_fluid_defpreset();

        if(defpreset == NULL)
        {
            goto err_exit;
        }

        if(fluid_defpreset_import_sfont(defpreset, sfpreset, defsfont) != FLUID_OK)
        {
            goto err_exit;
        }

        if(fluid_defsfont_add_preset(defsfont, defpreset) == FLUID_FAILED)
        {
            goto err_exit;
        }

        p = fluid_list_next(p);
    }

    fluid_sffile_close(sfdata);

    return FLUID_OK;

err_exit:
    fluid_sffile_close(sfdata);
    delete_fluid_defpreset(defpreset);
    return FLUID_FAILED;
}

/* fluid_defsfont_add_sample
 *
 * Add a sample to the SoundFont
 */
int fluid_defsfont_add_sample(fluid_defsfont_t *defsfont, fluid_sample_t *sample)
{
    defsfont->sample = fluid_list_append(defsfont->sample, sample);
    return FLUID_OK;
}

/* fluid_defsfont_add_preset
 *
 * Add a preset to the SoundFont
 */
int fluid_defsfont_add_preset(fluid_defsfont_t *defsfont, fluid_defpreset_t *defpreset)
{
    fluid_preset_t *preset;

    preset = new_fluid_preset(defsfont->sfont,
                              fluid_defpreset_preset_get_name,
                              fluid_defpreset_preset_get_banknum,
                              fluid_defpreset_preset_get_num,
                              fluid_defpreset_preset_noteon,
                              fluid_defpreset_preset_delete);

    if(defsfont->dynamic_samples)
    {
        preset->notify = dynamic_samples_preset_notify;
    }

    if(preset == NULL)
    {
        return FLUID_FAILED;
    }

    fluid_preset_set_data(preset, defpreset);

    defsfont->preset = fluid_list_append(defsfont->preset, preset);

    return FLUID_OK;
}

/*
 * fluid_defsfont_get_preset
 */
fluid_preset_t *fluid_defsfont_get_preset(fluid_defsfont_t *defsfont, int bank, int num)
{
    fluid_preset_t *preset;
    fluid_list_t *list;

    for(list = defsfont->preset; list != NULL; list = fluid_list_next(list))
    {
        preset = (fluid_preset_t *)fluid_list_get(list);

        if((fluid_preset_get_banknum(preset) == bank) && (fluid_preset_get_num(preset) == num))
        {
            return preset;
        }
    }

    return NULL;
}

/*
 * fluid_defsfont_iteration_start
 */
void fluid_defsfont_iteration_start(fluid_defsfont_t *defsfont)
{
    defsfont->preset_iter_cur = defsfont->preset;
}

/*
 * fluid_defsfont_iteration_next
 */
fluid_preset_t *fluid_defsfont_iteration_next(fluid_defsfont_t *defsfont)
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
fluid_defpreset_t *
new_fluid_defpreset(void)
{
    fluid_defpreset_t *defpreset = FLUID_NEW(fluid_defpreset_t);

    if(defpreset == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    defpreset->next = NULL;
    defpreset->name[0] = 0;
    defpreset->bank = 0;
    defpreset->num = 0;
    defpreset->global_zone = NULL;
    defpreset->zone = NULL;
    return defpreset;
}

/*
 * delete_fluid_defpreset
 */
void
delete_fluid_defpreset(fluid_defpreset_t *defpreset)
{
    fluid_preset_zone_t *zone;

    fluid_return_if_fail(defpreset != NULL);

    delete_fluid_preset_zone(defpreset->global_zone);
    defpreset->global_zone = NULL;

    zone = defpreset->zone;

    while(zone != NULL)
    {
        defpreset->zone = zone->next;
        delete_fluid_preset_zone(zone);
        zone = defpreset->zone;
    }

    FLUID_FREE(defpreset);
}

int
fluid_defpreset_get_banknum(fluid_defpreset_t *defpreset)
{
    return defpreset->bank;
}

int
fluid_defpreset_get_num(fluid_defpreset_t *defpreset)
{
    return defpreset->num;
}

const char *
fluid_defpreset_get_name(fluid_defpreset_t *defpreset)
{
    return defpreset->name;
}

/*
 * fluid_defpreset_next
 */
fluid_defpreset_t *
fluid_defpreset_next(fluid_defpreset_t *defpreset)
{
    return defpreset->next;
}

#ifdef DEBUG
/*
 Prints all members of a modulator.
*/
static void fluid_dump_linked_mod(fluid_mod_t *mod, int offset)
{
	int i, num = fluid_get_num_mod(mod);
	
	printf("modulator count:%d\n", num);
	for (i = 0; i < num; i++)
	{
        printf("mod%02d ", i + offset);
		fluid_dump_modulator(mod);
		mod = mod->next;
	}
}

/*
 Prints all the voice modulators of an instrument zone.
 Filter parameters allow to display modulators of the instrument zone and 
 preset zone corresponding to filter names.

 @param voice voice of the instrument zone.
 @param preset_zone_name, actual preset_zone name.
 @param filter_preset_zone_name, filter name of preset_zone to display. 
 @param inst_zone_name, actual instrument zone name.
 @param filter_inst_zone_name, filter name of intrument zone to display. 
*/
static void fluid_print_voice_mod(fluid_voice_t  *voice, 
                        char *preset_zone_name,
                        char *filter_preset_zone_name,
                        char *inst_zone_name,
                        char *filter_inst_zone_name)
{
    int i;
    fluid_mod_t *mod;

    if(strcmp(preset_zone_name, filter_preset_zone_name)
      || strcmp(inst_zone_name, filter_inst_zone_name))
    {
        return;
	}
    FLUID_LOG(FLUID_INFO, "\"%s\" \"%s\" voice modulators ---------------------------------", preset_zone_name,inst_zone_name);
    for(i = 0; i < voice->mod_count; i+= fluid_get_num_mod(mod))
    {
        mod = &voice->mod[i];
        fluid_dump_linked_mod (mod, i);
    }
}
#endif

/*
 * Adds global and local linked modulators list to the voice. This is done in 2 steps:
 * - Step 1: Local modulators replace identic global modulators.
 * - Step 2: global + local modulators are added to the voice using mode.
 *
 * Instrument zone list (local/global) must be added using FLUID_VOICE_DEFAULT
 * Preset zone list (local/global) must be added using FLUID_VOICE_ADD.
 *
 * @param voice voice instance.
 * @param global_mod global list of linked modulators.
 * @param local_mod local list of linked modulators.
 * @param mode Determines how to handle an existing identical complex modulator.
 *   # FLUID_VOICE_DEFAULT add the modulator at the end of voice table.
 *   # FLUID_VOICE_ADD to add (offset) the modulator amounts to existing
 *     complex linked modulator.
*/
static void
fluid_defpreset_noteon_add_linked_mod_to_voice(fluid_voice_t *voice, 
                                               fluid_mod_t *global_mod,
                                               fluid_mod_t *local_mod,int mode)
{
    fluid_mod_t *mod;
    /* list for 'sorting' global/local modulators */
    fluid_mod_t *mod_list[FLUID_NUM_MOD];
    int mod_list_count, i;

    /* identity_limit_count is the modulator upper limit number to handle with 
     * existing identical modulators.
     * When identity_limit_count is below the actual number of modulators, this 
     * will restrict identity check to this upper limit,
     * This is useful when we know by advance that there is no duplicate with
     * modulators at index above this limit. This avoid wasting cpu cycles at
     * noteon.
     */
     int identity_limit_count; 

    /* Step 1: Local modulators replace identic global modulators. */

    /* local (instrument zone/preset zone), modulators: Put them all into a list.
       Only the first member of a complex linked modulator is putted in the list.
     */

    mod_list_count = 0;
    while(local_mod)
    {
        /* As modulators number in local_mod list was limited to FLUID_NUM_MOD at
           soundfont loading time (fluid_limit_mod_list()), here we don't need
           to check if mod_list is full.
         */
        mod_list[mod_list_count++] = local_mod;
        local_mod = fluid_get_next_mod(local_mod); /* next complex modulator */
    }

    /* global (instrument zone/preset zone), complex modulators.
     * Replace modulators with the same definition in the global list:
     *
     * mod_list contains local complex modulators. Now we know that there
     * is no global complex modulator identic to another global complex modulator
     * (this has been checked at soundfont loading time). So global complex
     * modulators are only checked against local complex modulators number.
     */

    /* Restrict identity check to the number of local modulators */
    identity_limit_count = mod_list_count;

    while(global_mod)
    {
        /* 'Identical' global modulators are ignored.*/

        for(i = 0; i < identity_limit_count; i++)
        {
            if(fluid_linked_mod_test_identity(global_mod,0, mod_list[i],
                                             FLUID_LINKED_MOD_TEST_ONLY))
            {
                break;
            }
        }

        /* Finally add the new modulator to the list. */
        if(i >= identity_limit_count)
        {
            /* local_mod and global_mod lists was limited to FLUID_NUM_MOD at
               soundfont loading time. Any complex modulator has at least 2
               members, so local + global modulators never will exceed
               FLUID_NUM_MOD. So, no need to check if mod_list_count exceed
               FLUID_NUM_MOD.
             */

            mod_list[mod_list_count++] = global_mod;
        }
        global_mod = fluid_get_next_mod(global_mod); /* next complex modulator */
    }

    /* Step 2: global + local modulators are added to the voice using mode. */

    /*
     * mod_list contains local and global modulators, we know that:
     * - there is no global modulator identic to another global modulator,
     * - there is no local modulator identic to another local modulator,
     * So these local/global modulators are only checked against
     * actual number of voice modulators.
     */

    /* Restrict identity check to the actual number of voice linked modulators */
    /* Acual number of voice linked modulators: instruments */
    identity_limit_count = voice->mod_count; 

    for(i = 0; i < mod_list_count; i++)
    {

        mod = mod_list[i];
        /* Instrument linked modulators are added in mode: FLUID_VOICE_DEFAULT */
        /* Preset linked modulators are added in mode: FLUID_VOICE_ADD */
        /* identity_limit_count is effective only in mode FLUID_VOICE_ADD */
        fluid_voice_add_mod_local(voice, mod, mode, identity_limit_count);
    }
}

/*
 * Adds global and local modulators list to the voice. This is done in 2 steps:
 * - Step 1: Local modulators replace identic global modulators.
 * - Step 2: global + local modulators are added to the voice using mode.
 *
 * Instrument zone list (local/global) must be added using FLUID_VOICE_OVERWRITE.
 * Preset zone list (local/global) must be added using FLUID_VOICE_ADD.
 *
 * @param voice voice instance.
 * @param global_mod global list of modulators.
 * @param local_mod local list of modulators.
 * @param mode Determines how to handle an existing identical modulator.
 *   #FLUID_VOICE_ADD to add (offset) the modulator amounts,
 *   #FLUID_VOICE_OVERWRITE to replace the modulator,
*/
static void
fluid_defpreset_noteon_add_mod_to_voice(fluid_voice_t *voice,
                                        fluid_mod_t *global_mod, fluid_mod_t *local_mod,
                                        int mode)
{
    fluid_mod_t *mod;
    /* list for 'sorting' global/local modulators */
    fluid_mod_t *mod_list[FLUID_NUM_MOD];
    int mod_list_count, i;

    /* identity_limit_count is the modulator upper limit number to handle with
     * existing identical modulators.
     * When identity_limit_count is below the actual number of modulators, this
     * will restrict identity check to this upper limit,
     * This is useful when we know by advance that there is no duplicate with
     * modulators at index above this limit. This avoid wasting cpu cycles at
     * noteon.
     */
    int identity_limit_count;

    /* Step 1: Local modulators replace identic global modulators. */

    /* local (instrument zone/preset zone), modulators: Put them all into a list. */
    mod_list_count = 0;

    while(local_mod)
    {
        /* As modulators number in local_mod list was limited to FLUID_NUM_MOD at
           soundfont loading time (fluid_limit_mod_list()), here we don't need
           to check if mod_list is full.
         */
        mod_list[mod_list_count++] = local_mod;
        local_mod = local_mod->next;
    }

    /* global (instrument zone/preset zone), modulators.
     * Replace modulators with the same definition in the global list:
     * (Instrument zone: SF 2.01 page 69, 'bullet' 8)
     * (Preset zone:     SF 2.01 page 69, second-last bullet).
     *
     * mod_list contains local modulators. Now we know that there
     * is no global modulator identic to another global modulator (this has
     * been checked at soundfont loading time). So global modulators
     * are only checked against local modulators number.
     */

    /* Restrict identity check to the number of local modulators */
    identity_limit_count = mod_list_count;

    while(global_mod)
    {
        /* 'Identical' global modulators are ignored.
         *  SF2.01 section 9.5.1
         *  page 69, 'bullet' 3 defines 'identical'.  */

        for(i = 0; i < identity_limit_count; i++)
        {
            if(fluid_mod_test_identity(global_mod, mod_list[i]))
            {
                break;
            }
        }

        /* Finally add the new modulator to the list. */
        if(i >= identity_limit_count)
        {
            /* Although local_mod and global_mod lists was limited to
               FLUID_NUM_MOD at soundfont loading time, it is possible that
               local + global modulators exceeds FLUID_NUM_MOD.
               So, checks if mod_list_count reachs the limit.
            */
            if(mod_list_count >= FLUID_NUM_MOD)
            {
                /* mod_list is full, we silently forget this modulator and
                   next global modulators. When mod_list will be added to the
                   voice, a warning will be displayed if the voice list is full.
                   (see fluid_voice_add_mod_local()).
                */
                break;
            }

            mod_list[mod_list_count++] = global_mod;
        }

        global_mod = global_mod->next;
    }

    /* Step 2: global + local modulators are added to the voice using mode. */

    /*
     * mod_list contains local and global modulators, we know that:
     * - there is no global modulator identic to another global modulator,
     * - there is no local modulator identic to another local modulator,
     * So these local/global modulators are only checked against
     * actual number of voice modulators.
     */

    /* Restrict identity check to the actual number of voice modulators */
    /* Acual number of voice modulators : defaults + [instruments] */
    identity_limit_count = voice->mod_count;

    for(i = 0; i < mod_list_count; i++)
    {

        mod = mod_list[i];
        /* in mode FLUID_VOICE_OVERWRITE disabled instruments modulators CANNOT be skipped. */
        /* in mode FLUID_VOICE_ADD disabled preset modulators can be skipped. */

        if((mode == FLUID_VOICE_OVERWRITE) || (mod->amount != 0))
        {
            /* Instrument modulators -supersede- existing (default) modulators.
               SF 2.01 page 69, 'bullet' 6 */

            /* Preset modulators -add- to existing instrument modulators.
               SF2.01 page 70 first bullet on page */
            fluid_voice_add_mod_local(voice, mod, mode, identity_limit_count);
        }
    }
}

/*
 * fluid_defpreset_noteon
 */
int
fluid_defpreset_noteon(fluid_defpreset_t *defpreset, fluid_synth_t *synth, int chan, int key, int vel)
{
    fluid_preset_zone_t *preset_zone, *global_preset_zone;
    fluid_inst_t *inst;
    fluid_inst_zone_t *inst_zone, *global_inst_zone;
    fluid_voice_zone_t *voice_zone;
    fluid_list_t *list;
    fluid_voice_t *voice;
    int i;

    global_preset_zone = fluid_defpreset_get_global_zone(defpreset);

    /* run thru all the zones of this preset */
    preset_zone = fluid_defpreset_get_zone(defpreset);

    while(preset_zone != NULL)
    {

        /* check if the note falls into the key and velocity range of this
           preset */
        if(fluid_zone_inside_range(&preset_zone->range, key, vel))
        {

            inst = fluid_preset_zone_get_inst(preset_zone);
            global_inst_zone = fluid_inst_get_global_zone(inst);

            /* run thru all the zones of this instrument that could start a voice */
            for(list = preset_zone->voice_zone; list != NULL; list = fluid_list_next(list))
            {
                voice_zone = fluid_list_get(list);

                /* check if the instrument zone is ignored and the note falls into
                   the key and velocity range of this  instrument zone.
                   An instrument zone must be ignored when its voice is already running
                   played by a legato passage (see fluid_synth_noteon_monopoly_legato()) */
                if(fluid_zone_inside_range(&voice_zone->range, key, vel))
                {

                    inst_zone = voice_zone->inst_zone;

                    /* this is a good zone. allocate a new synthesis process and initialize it */
                    voice = fluid_synth_alloc_voice_LOCAL(synth, inst_zone->sample, chan, key, vel, &voice_zone->range);

                    if(voice == NULL)
                    {
                        return FLUID_FAILED;
                    }


                    /* Instrument level, generators */

                    for(i = 0; i < GEN_LAST; i++)
                    {

                        /* SF 2.01 section 9.4 'bullet' 4:
                         *
                         * A generator in a local instrument zone supersedes a
                         * global instrument zone generator.  Both cases supersede
                         * the default generator -> voice_gen_set */

                        if(inst_zone->gen[i].flags)
                        {
                            fluid_voice_gen_set(voice, i, inst_zone->gen[i].val);

                        }
                        else if((global_inst_zone != NULL) && (global_inst_zone->gen[i].flags))
                        {
                            fluid_voice_gen_set(voice, i, global_inst_zone->gen[i].val);

                        }
                        else
                        {
                            /* The generator has not been defined in this instrument.
                             * Do nothing, leave it at the default.
                             */
                        }

                    } /* for all generators */

                    /* Adds instrument zone modulators (global and local) to the voice.*/
                    fluid_defpreset_noteon_add_mod_to_voice(voice,
                                                            /* global instrument modulators */
                                                            global_inst_zone ? global_inst_zone->mod : NULL,
                                                            inst_zone->mod, /* local instrument modulators */
                                                            FLUID_VOICE_OVERWRITE); /* mode */

                    /* Preset level, generators */

                    for(i = 0; i < GEN_LAST; i++)
                    {

                        /* SF 2.01 section 8.5 page 58: If some generators are
                         encountered at preset level, they should be ignored.
                         However this check is not necessary when the soundfont
                         loader has ignored invalid preset generators.
                         Actually load_pgen()has ignored these invalid preset
                         generators:
                           GEN_STARTADDROFS,      GEN_ENDADDROFS,
                           GEN_STARTLOOPADDROFS,  GEN_ENDLOOPADDROFS,
                           GEN_STARTADDRCOARSEOFS,GEN_ENDADDRCOARSEOFS,
                           GEN_STARTLOOPADDRCOARSEOFS,
                           GEN_KEYNUM, GEN_VELOCITY,
                           GEN_ENDLOOPADDRCOARSEOFS,
                           GEN_SAMPLEMODE, GEN_EXCLUSIVECLASS,GEN_OVERRIDEROOTKEY
                        */

                        /* SF 2.01 section 9.4 'bullet' 9: A generator in a
                         * local preset zone supersedes a global preset zone
                         * generator.  The effect is -added- to the destination
                         * summing node -> voice_gen_incr */

                        if(preset_zone->gen[i].flags)
                        {
                            fluid_voice_gen_incr(voice, i, preset_zone->gen[i].val);
                        }
                        else if((global_preset_zone != NULL) && global_preset_zone->gen[i].flags)
                        {
                            fluid_voice_gen_incr(voice, i, global_preset_zone->gen[i].val);
                        }
                        else
                        {
                            /* The generator has not been defined in this preset
                             * Do nothing, leave it unchanged.
                             */
                        }
                    } /* for all generators */

                    /* Adds preset zone modulators (global and local) to the voice.*/
                    fluid_defpreset_noteon_add_mod_to_voice(voice,
                                                            /* global preset modulators */
                                                            global_preset_zone ? global_preset_zone->mod : NULL,
                                                            preset_zone->mod, /* local preset modulators */
                                                            FLUID_VOICE_ADD); /* mode */

                    /* instrument zone linked modulators */
                    fluid_defpreset_noteon_add_linked_mod_to_voice(voice, 
                                                            global_inst_zone ? global_inst_zone->linked_mod : NULL,
                                                            inst_zone->linked_mod,
                                                            FLUID_VOICE_DEFAULT);
                    /* preset zone linked modulators */
                    fluid_defpreset_noteon_add_linked_mod_to_voice(voice, 
                                                            global_preset_zone ? global_preset_zone->linked_mod : NULL,
                                                            preset_zone->linked_mod,
                                                            FLUID_VOICE_ADD);

                    /* add the synthesis process to the synthesis loop. */
                    fluid_synth_start_voice(synth, voice);

                    /* Store the ID of the first voice that was created by this noteon event.
                     * Exclusive class may only terminate older voices.
                     * That avoids killing voices, which have just been created.
                     * (a noteon event can create several voice processes with the same exclusive
                     * class - for example when using stereo samples)
                     */
                }
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
fluid_defpreset_set_global_zone(fluid_defpreset_t *defpreset, fluid_preset_zone_t *zone)
{
    defpreset->global_zone = zone;
    return FLUID_OK;
}

/*
 * fluid_defpreset_import_sfont
 */
int
fluid_defpreset_import_sfont(fluid_defpreset_t *defpreset,
                             SFPreset *sfpreset,
                             fluid_defsfont_t *defsfont)
{
    fluid_list_t *p;
    SFZone *sfzone;
    fluid_preset_zone_t *zone;
    int count;
    char zone_name[256];

    if(FLUID_STRLEN(sfpreset->name) > 0)
    {
        FLUID_STRCPY(defpreset->name, sfpreset->name);
    }
    else
    {
        FLUID_SNPRINTF(defpreset->name, sizeof(defpreset->name), "Bank%d,Pre%d", sfpreset->bank, sfpreset->prenum);
    }

    defpreset->bank = sfpreset->bank;
    defpreset->num = sfpreset->prenum;
    p = sfpreset->zone;
    count = 0;

    while(p != NULL)
    {
        sfzone = (SFZone *)fluid_list_get(p);
        FLUID_SNPRINTF(zone_name, sizeof(zone_name), "pz:%s/%d", defpreset->name, count);
        zone = new_fluid_preset_zone(zone_name);

        if(zone == NULL)
        {
            return FLUID_FAILED;
        }

        if(fluid_preset_zone_import_sfont(zone, sfzone, defsfont) != FLUID_OK)
        {
            delete_fluid_preset_zone(zone);
            return FLUID_FAILED;
        }

        if((count == 0) && (fluid_preset_zone_get_inst(zone) == NULL))
        {
            fluid_defpreset_set_global_zone(defpreset, zone);
        }
        else if(fluid_defpreset_add_zone(defpreset, zone) != FLUID_OK)
        {
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
fluid_defpreset_add_zone(fluid_defpreset_t *defpreset, fluid_preset_zone_t *zone)
{
    if(defpreset->zone == NULL)
    {
        zone->next = NULL;
        defpreset->zone = zone;
    }
    else
    {
        zone->next = defpreset->zone;
        defpreset->zone = zone;
    }

    return FLUID_OK;
}

/*
 * fluid_defpreset_get_zone
 */
fluid_preset_zone_t *
fluid_defpreset_get_zone(fluid_defpreset_t *defpreset)
{
    return defpreset->zone;
}

/*
 * fluid_defpreset_get_global_zone
 */
fluid_preset_zone_t *
fluid_defpreset_get_global_zone(fluid_defpreset_t *defpreset)
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
fluid_preset_zone_t *
fluid_preset_zone_next(fluid_preset_zone_t *zone)
{
    return zone->next;
}

/*
 * new_fluid_preset_zone
 */
fluid_preset_zone_t *
new_fluid_preset_zone(char *name)
{
    fluid_preset_zone_t *zone = NULL;
    zone = FLUID_NEW(fluid_preset_zone_t);

    if(zone == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    zone->next = NULL;
    zone->voice_zone = NULL;
    zone->name = FLUID_STRDUP(name);

    if(zone->name == NULL)
    {
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
    fluid_gen_init(&zone->gen[0], NULL);
    zone->mod = NULL; /* list of modulators */
    zone->linked_mod = NULL; /* List of linked modulators */
    return zone;
}

/*
 * delete list of modulators.
 */
void delete_fluid_list_mod(fluid_mod_t *mod)
{
    fluid_mod_t *tmp;

    while(mod)	/* delete the modulators */
    {
        tmp = mod;
        mod = mod->next;
        delete_fluid_mod(tmp);
    }
}

/*
 * delete_fluid_preset_zone
 */
void
delete_fluid_preset_zone(fluid_preset_zone_t *zone)
{
    fluid_list_t *list;

    fluid_return_if_fail(zone != NULL);

    delete_fluid_list_mod(zone->mod);       /* unlinked modulators */
    delete_fluid_list_mod(zone->linked_mod);/* linked modulators */

    for(list = zone->voice_zone; list != NULL; list = fluid_list_next(list))
    {
        FLUID_FREE(fluid_list_get(list));
    }

    delete_fluid_list(zone->voice_zone);

    FLUID_FREE(zone->name);
    FLUID_FREE(zone);
}

static int fluid_preset_zone_create_voice_zones(fluid_preset_zone_t *preset_zone)
{
    fluid_inst_zone_t *inst_zone;
    fluid_sample_t *sample;
    fluid_voice_zone_t *voice_zone;
    fluid_zone_range_t *irange;
    fluid_zone_range_t *prange = &preset_zone->range;

    fluid_return_val_if_fail(preset_zone->inst != NULL, FLUID_FAILED);

    inst_zone = fluid_inst_get_zone(preset_zone->inst);

    while(inst_zone != NULL)
    {

        /* We only create voice ranges for zones that could actually start a voice,
         * i.e. that have a sample and don't point to ROM */
        sample = fluid_inst_zone_get_sample(inst_zone);

        if((sample == NULL) || fluid_sample_in_rom(sample))
        {
            inst_zone = fluid_inst_zone_next(inst_zone);
            continue;
        }

        voice_zone = FLUID_NEW(fluid_voice_zone_t);

        if(voice_zone == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return FLUID_FAILED;
        }

        voice_zone->inst_zone = inst_zone;

        irange = &inst_zone->range;

        voice_zone->range.keylo = (prange->keylo > irange->keylo) ? prange->keylo : irange->keylo;
        voice_zone->range.keyhi = (prange->keyhi < irange->keyhi) ? prange->keyhi : irange->keyhi;
        voice_zone->range.vello = (prange->vello > irange->vello) ? prange->vello : irange->vello;
        voice_zone->range.velhi = (prange->velhi < irange->velhi) ? prange->velhi : irange->velhi;
        voice_zone->range.ignore = FALSE;

        preset_zone->voice_zone = fluid_list_append(preset_zone->voice_zone, voice_zone);

        inst_zone = fluid_inst_zone_next(inst_zone);
    }

    return FLUID_OK;
}

/**
 * Checks if modulator mod is identic to another modulator in the list
 * (specs SF 2.0X  7.4, 7.8).
 * @param mod, modulator list.
 * @param name, if not NULL, pointer on a string displayed as warning.
 * @return TRUE if mod is identic to another modulator, FALSE otherwise.
 */
static int
fluid_zone_is_mod_identic(const fluid_mod_t *mod, char *name)
{
    fluid_mod_t *next = mod->next;

    while(next)
    {
        /* is mod identic to next ? */
        if(fluid_mod_test_identity(mod, next))
        {
            if(name)
            {
                FLUID_LOG(FLUID_WARN, "Ignoring identic modulator %s", name);
            }

            return TRUE;
        }

        next = next->next;
    }

    return FALSE;
}

/*
 * returns the number of modulators inside a list.
 * @param mod, pointer on modulator list.
 * @return  number of modulators.
 */
int fluid_get_count_mod(const fluid_mod_t *mod)
{
    int count =0;
    while(mod)
    {
        count++;
        mod = mod->next;
    }
    return count;
}

/* bit FLUID_PATH_VALID set to 1 indicates that the modulator belongs to
 a complete valid linked path already discovered */
#define FLUID_PATH_VALID  1 << 0
/* bit FLUID_PATH_CURENT set to 1 indicates that the modulator belongs to
 the current linked path . It allows detection of circular path */
#define FLUID_PATH_CUR  1 << 1

/*
 * Check linked modulator paths without destination and circular linked modulator
 * paths (specif SF 2.0  7.4, 7.8  and 9.5.4).
 *
 * Warning: This function must be called before calling
 * fluid_list_copy_linked_mod().
 *
 * Any linked modulator path from the start to the end are checked and returned
 * in path table.
 *
 * Let a linked path     CC-->m2-->m6-->m3-->gen
 *
 * - A linked path begins from a modulator with source scr1 not linked and
 *   destination linked to a modulator (e.g m2).
 * - A linked path ends on a modulator with source scr1 linked and destination
 *   connected to a generator (e.g m3).
 *
 * - Path without destination:
 *   When a destination cannot be reached inside a path, this path is said to be
 *   "without destination". The following message displays this situation:
 *      fluidsynth: warning: path without destination zone-name/mod2.
 *   with, mod2 being the modulator at the beginning of the path.
 *	 This case occurs when a modulator doesn't exist at m6 destination index
 *	 for example (CC->m2-->m6-->?).
 *	 This case occurs also if a modulator exists at m6 destination index
 *	 (e.g CC->m2-->m6-->m3->...) and this modulator (e.g m3) have source src1 not
 *   linked. Two messages are displayed to show the later case:
 *      fluidsynth: warning: invalid destination zone-name/mod3.
 *      fluidsynth: warning: path without destination zone-name/mod2.
 *   First message indicates that m3 is invalid (because source src1 isn't linked
 *   or mod is invalid or amount is 0).
 *   When a path is without destination, all modulators from the beginning to the one
 *   without destination are marked invalid (FLUID_PATH_VALID  = 0, amount = 0)
 *   (e.g  m2,m6).
 *
 * - Circular path:
 *   When a destination is a modulator already encountered this is a circular path
 *   (e.g: CC-->m2-->m6-->m3-->m8-->m6). Two messages are displayed:
 *      fluidsynth: warning: invalid circular path zone-name/mod6.
 *      fluidsynth: warning: path without destination zone-name/mod2.
 *   First message indicates that m6 is a modulator already encountered.
 *   Second message indicates the modulator at the beginning of the path (e.g m2).
 *   When a path is circular, all modulators from the beginning to the one
 *   already encontered are marked invalid (FLUID_PATH_VALID  = 0, amount = 0)
 *   (e.g  m2,m6,m3,m8).
 *
 * Other incomplete linked modulator paths are isolated.
 * Isolated path begins with modulator mx having source src1 linked, with no
 * others modulators connected to mx.
 * These isolated modulator paths are still in list_mod but not in path table.
 * They should be marked invalid later.
 *
 * The function searchs all linked path starting from the beginning of the path
 * (ie. a modulator with source not linked) forward to the endind of the path
 * (ie. a modulator connected to a generator).
 * Search direction is the reverse that the one done in fluid_list_copy_linked_mod().
 * The function is recursive and intended to be called the first time to
 * start the search from the beginning of any path (see dest_idx, path_idx).
 *
 * @param zone_name, zone's name used to prefix messages displayed.
 * @param list_mod, pointer on modulators list. amount value to 0 indicates that
 *  modulator are invalid.
 * @param dest_idx, index of the destination linked modulator to search.
 * Should be - 1 at first call.
 *   if < 0, search first modulator (i.e first linked modulateur).
 *   if >= 0 index of the destination linked modulator to search.
 * @param path, pointer on table for path registering.
 * On input, FLUID_PATH_CUR , FLUID_PATH_VALID must be initialized to 0.
 * On return, path table contains:
 * - no path (FLUID_PATH_CUR set to 0, FLUID_PATH_VALID set to 0) or
 * - valid complete paths (FLUID_PATH_VALID set to 1) or
 * - invalid incomplete paths (FLUID_PATH_CUR set to 1, FLUID_PATH_VALID set to 0).
 *
 * @return  TRUE if at least one complete valid linked modulators path exists,
 *          FALSE  otherwise.
*/
static int
fluid_check_linked_mod_path(char *zone_name, fluid_mod_t *list_mod,
                            int dest_idx, 
                            unsigned char path[])
{
    int result = FALSE;
    int mod_idx = 0; /* index of current mod in list */
    fluid_mod_t *mod = list_mod; /* first modulator in list_mod */
    while(mod)
    {
        /* is request for search first linked modulator of a path ? */
        if (dest_idx < 0)
        {
            /* checks if mod source isn't linked and mod destination is linked */
            if (!fluid_mod_has_linked_src1(mod) && (mod->dest & FLUID_MOD_LINK_DEST)
                 && (mod->amount != 0))
            {
                /* memorizes mod state: in current linked path */
                path[mod_idx] |= FLUID_PATH_CUR;

                /* search and check the full path to the end. */
                if(! fluid_check_linked_mod_path(zone_name, list_mod, mod->dest,
                                                 path))
                {   /* no final destination found for mod */
                    mod->amount = 0; /* mod marked invalid */
                    /* current path is without destination */
                    FLUID_LOG(FLUID_WARN, "path without destination %s/mod%d",
                              zone_name, mod_idx);
                }
                else
                {
                    path[mod_idx] |= FLUID_PATH_VALID; /* current path is valid */
                    result = TRUE;
                }
            }
        }
        /* request to search next modulator in the current path */
        else if((mod_idx | FLUID_MOD_LINK_DEST) == dest_idx) /* is mod a destination ? */
        { 
            /* mod is destination of a previous modulator in path */
            /* is this modulator destination valid ? */
            if (!fluid_mod_has_linked_src1(mod) || (mod->amount ==0))
            {
                /* warning: path without destination */
                FLUID_LOG(FLUID_WARN, "invalid destination %s/mod%d", 
                          zone_name, mod_idx);
                return FALSE; /* current path is invalid */
            }
 
            /* mod is a valid destination modulator */
            /* Checks if mod belongs to a path already discovered */
            if (path[mod_idx] & FLUID_PATH_VALID)
            {
                return TRUE; /* current path is valid */
            }

            /* Checks if mod belongs to current path */
            if (path[mod_idx] & FLUID_PATH_CUR)
            {
                /* warning: invalid circular path */
                FLUID_LOG(FLUID_WARN, "invalid circular path %s/mod%d", 
                          zone_name, mod_idx);
                return FALSE; /* current path is invalid */
            }

            /* memorizes mod state: in current linked path */
            path[mod_idx] |= FLUID_PATH_CUR;

            /* does mod destination linked ? */
            if((mod->dest & FLUID_MOD_LINK_DEST) &&
               ! fluid_check_linked_mod_path(zone_name, list_mod, mod->dest,
                                             path))
            {
                mod->amount = 0; /* mod marked invalid */
                return FALSE;    /* current path is invalid */
            }
            path[mod_idx] |= FLUID_PATH_VALID; /* current path is valid */
            return TRUE;
        }
        mod = mod->next;
        mod_idx++;
    }
    return result;
}

/*
 * Valid linked modulators are searched and cloned from mod_list list to 
 * linked_mod list.
 * When finished, modulators in linked_mod are grouped in complex modulator.
 * (cm0,cm1,cm2..).
 * The first member of any complex modulator is the ending modulator connected
 * to a generator. Other members are linked to each other to reach the last
 * member. The destination index of modulator member following the first is
 * relative (0 based) to the first member index.
 *
 * Warning: fluid_check_linked_mod_path() must be called before calling this
 * function.
 * 
 * The function searchs all linked path starting from the end of the path 
 * (i.e modulator connected to a generator) backward to the beginning of 
 * the path (ie. a modulator with source not linked).
 * Search direction is the reverse that the one done in fluid_check_linked_mod_path().
 * The function is recursive and intended to be called the first time to
 * start the search from ending linked modulator (see dest_idx, new_idx).
 *
 * @param list_mod, modulators list. amount value to 0 indicates that
 *  modulator are invalid.
 * @param dest_idx, initial index of linked destination modulator to search.
 *  Must be set to -1 at first call.
 *  -1, to search ending linked modulator.
 *  >= 0, to search a modulator with linked destination equal to dest_idx index.
 *
 * @param new_idx, index (1 based) of the most recent modulator at the end
 *  of linked_mod. Must be set to 0 at first call.
 *
 * @param linked_mod, address of pointer on linked modulators list returned
 *  if any linked modulators exist.
 *
 * @return  
 *   0, linked_mod list is empty.
 *   > 0 , linked_mod list contains linked modulators.
 *   FLUID_FAILED otherwise.
*/
static int 
fluid_list_copy_linked_mod(const fluid_mod_t *list_mod, int dest_idx, int new_idx,
                           fluid_mod_t **linked_mod)
{
    int linked_count = new_idx; /* Last added modulator index in linked_mod */
    int mod_idx = 0; /* first modulator index in list mod*/
    const fluid_mod_t *mod = list_mod;
    while(mod)
    {
        if (mod->amount != 0) /* ignores invalid modulators */
        {
            /* is_src_linked is true when modulator mod's input are linked */
            int is_src1_linked = fluid_mod_has_linked_src1(mod);

            /* is_mod_dst_only is true when mod is a linked ending modulator */
            int is_mod_dst_only = (dest_idx < 0) && is_src1_linked &&
                                  !(mod->dest & FLUID_MOD_LINK_DEST);

            /* is_mod_src is true when mod linked destination is equal to dest_idx */
            int is_mod_src = ((dest_idx >= 0) && (dest_idx == mod->dest));

            /* is mod any linked modulator of interest ? */
            if (is_mod_dst_only || is_mod_src)
            {
                /* Make a copy of this modulator */
                fluid_mod_t *mod_cpy = new_fluid_mod(); /* next field is set to NULL */
                if(mod_cpy == NULL)
                { 
                    delete_fluid_list_mod(*linked_mod); /* freeing */
                    *linked_mod = NULL;
                    return FLUID_FAILED;
                }
                fluid_mod_clone(mod_cpy, mod);
                
                /* updates destination field of mod_cpy (but ending modulator) */
                if (is_mod_src)
                {
                    /* new destination field must be an index 0 based. */
                    mod_cpy->dest = FLUID_MOD_LINK_DEST | (new_idx - 1);
                }

                /* adding mod_cpy in linked_mod */
                if (linked_count == 0)
                {   
                    /* puts mod_cpy at the begin of linked list */
                    *linked_mod = mod_cpy; 
                } 
                else 
                {   
                    /* puts mod_cpy at the end of linked list */
                    fluid_mod_t * last_mod = *linked_mod;

                    /* Find the end of the list */
                    while (last_mod->next != NULL){ last_mod = last_mod->next; }
                    last_mod->next = mod_cpy;
                }
                /* force index of ending modulator to 0 */
                if (is_mod_dst_only)
                {
                    linked_count = 0;
                }
                linked_count++; /* updates count of linked mod */

                /* is mod's source src1 linked ? */
                if(is_src1_linked) 
                {	/* search a modulator with output linked to mod */
                    linked_count = fluid_list_copy_linked_mod(list_mod,
                                                 mod_idx | FLUID_MOD_LINK_DEST,
                                                 linked_count, linked_mod);
                    if(linked_count == FLUID_FAILED)
                    {
                        return FLUID_FAILED;
                    }
                }
            }
        }
        mod = mod->next;
        mod_idx++;
    }
    return linked_count;
}
/**
 * Checks all modulators from a zone modulator list and optionally clone
 * valid linked modulators from mod_list list to linked_mod list.
 * - check valid sources.
 * - check identic modulator.
 * - check linked modulators path.
 * - clone valid linked modulators to linked_mod.
 * The function does the same job that fluid_zone_check_mod() except that
 * that modulators aren't removed from mod_list and the list length isn't
 * reduced. The function is appropriate to be
 * called by API fluid_voice_add_mod(),fluid_synth_add_default_mod().
 *
 * @param list_name, list's name used to prefix messages displayed.
 * @param list_mod, pointer on modulators list.
 *  On input, the list may contains any unlinked or linked modulators.
 *  On output, invalid modulators are marked invalid with amount value forced
 *  to 0.
 *
 * @param linked_mod, if not NULL, address of pointer on linked modulators
 *  list returned. NULL is returned in this pointer if linked
 *  modulators doesn't exist in list_mod.
 * @return
 *  - TRUE if any valid linked path exists.
 *  - FALSE if no linked path exists.
 *  - FLUID_FAILED if failed (memory error).
 */
int
fluid_list_check_linked_mod(char *list_name, fluid_mod_t *list_mod,
                            fluid_mod_t **linked_mod)
{
    int result;
    /* path is a flags table state to register valid modulators and
       valid complete linked modulator path */
    unsigned char *path;
    fluid_mod_t *mod;
    int count; /* number of modulators in list_mod. */

    /* count number of modulators in list_mod */
    count = fluid_get_count_mod(list_mod);
    if(!count)
    { /* There are no modulators, no need to go further */
        return FALSE;
    }

    /* path allocation */
    path = FLUID_MALLOC (sizeof(*path) * count);
    if(path == NULL)
    {
        return FLUID_FAILED;
    }
    /* initialise path: reset bits FLUID_PATH_VALID, FLUID_PATH_CUR */
    FLUID_MEMSET(path, 0, count);

    /* checks valid modulator sources (specs SF 2.01  7.4, 7.8, 8.2.1).*/
    /* checks identic modulator in the list (specs SF 2.01  7.4, 7.8). */
    mod = list_mod; /* first modulator in list_mod */
    count = 0;
    while(mod)
    {
        char zone_mod_name[256];

        /* prepare modulator name: zonename/#modulator */
        FLUID_SNPRINTF(zone_mod_name, sizeof(zone_mod_name),"%s/mod%d", list_name, count);

        /* has mod invalid sources ? */
        if(!fluid_mod_check_sources (mod,  zone_mod_name)
        /* or is mod identic to any following modulator ? */
           ||fluid_zone_is_mod_identic(mod, zone_mod_name))
        {   /* marks this modulator invalid for future checks */
            mod->amount = 0;
        }

        count++;
        mod = mod->next;
    }


    /* Now check linked modulator path */
    result = fluid_check_linked_mod_path(list_name, list_mod, -1, path);

    /* Now path contains complete discovered modulators paths (or not).
       Other incomplete linked modulators path (isolated) are still in list_mod but
       not in path. These should now indicated invalid.
       (specifications SF 2.01  7.4, 7.8) */
    count = 0; /* number of modulators in list_mod. */
    mod = list_mod; /* first modulator in list_mod */
    while(mod)
    {
        if( /* Check linked mod only not in discovered paths */
            fluid_mod_is_linked(mod)
            /* Check if mod isn't in discovered paths */
            && !(path[count] & FLUID_PATH_CUR) )
        {
            mod->amount = 0; /* marked invalid */
            FLUID_LOG(FLUID_WARN, "invalid isolated path %s/mod%d", list_name, count);
        }
        mod = mod->next;
        count++;
    }

    /* clone of linked modulators if requested */
    if(linked_mod)
    {
        *linked_mod = NULL; /* Initialize linked modulator list to NULL */
        /* does one or more valid linked modulators exists ? */
        if(result)
        {
            /* one or more linked modulators paths exists */
            /* clone valid linked modulator paths from list_mod to linked_mod.*/
            result = fluid_list_copy_linked_mod(list_mod, -1, 0, linked_mod);
        }
    }

    /* free path */
    FLUID_FREE(path);

    return result;
}

/**
 * Checks and remove invalid modulators from a zone modulators list.
 * - remove linked modulators.
 * - remove modulators with invalid sources (specs SF 2.01  7.4, 7.8, 8.2.1).
 * - remove identic modulator in the list (specs SF 2.01  7.4, 7.8).
 * On output, the list contains only valid unlinked modulators.
 *
 * @param list_mod, address of pointer on modulator list.
 */
static void fluid_zone_check_remove_mod(fluid_mod_t **list_mod)
{
    fluid_mod_t *prev_mod = NULL; /* previous modulator in list_mod */
    fluid_mod_t *mod = *list_mod; /* first modulator in list_mod */
    while(mod)
    {	
        fluid_mod_t *next = mod->next;
        if(   /* Is mod a linked modulator ? */
              fluid_mod_is_linked(mod)
              /* or has mod invalid sources ? */
              || !fluid_mod_check_sources(mod, NULL)
              /* or is mod identic to any following modulator ? */
              || fluid_zone_is_mod_identic(mod, NULL))
        {  
            /* the modulator is useless so we remove it */
            if (prev_mod)
            {
                prev_mod->next =next;
			}
            else
            {
                *list_mod = next;
			}

            delete_fluid_mod(mod); /* freeing */
        }
        else 
        {
            prev_mod = mod; 
        }
        mod = next;
    }
}

/**
 * Limits the number of modulators in a modulator list.
 * This is appropriate to internal synthesizer modulators tables
 * which have a fixed size (FLUID_NUM_MOD).
 *
 * @param zone_name, zone name used to prefix messages displayed.
 * @param list_mod, address of pointer on modulator list.
 */
static void fluid_limit_mod_list(char *zone_name, fluid_mod_t **list_mod)
{
    int mod_idx = 0; /* modulator index */
    fluid_mod_t *prev_mod = NULL; /* previous modulator in list_mod */
    fluid_mod_t *mod = *list_mod; /* first modulator in list_mod */

    while(mod)
    {
        if((mod_idx + fluid_get_num_mod(mod)) > FLUID_NUM_MOD )
        {
            /* truncation of list_mod */
            if(mod_idx)
            {
                prev_mod->next = NULL;
            }
            else
            {
                *list_mod = NULL;
            }

            delete_fluid_list_mod(mod);
            FLUID_LOG(FLUID_WARN, "%s, modulators count limited to %d", zone_name,
                      FLUID_NUM_MOD);
            break;
        }

        mod_idx++;
        prev_mod = mod;
        mod = mod->next;
    }
}

/**
 * Checks and remove invalid modulators from a zone modulators list.
 * - checks valid modulator sources (specs SF 2.01  7.4, 7.8, 8.2.1).
 * - checks identic modulators in the list (specs SF 2.01  7.4, 7.8).
 * - checks linked modulators path from a zone modulators list.
 * - extracts valid linked modulators to linked_mod.
 * - removing all invalid modulators.
 * - limiting size of modulators list.
 * The function does the same job that fluid_list_check_linked_mod() except that
 * input list_mod keeps only valid unlinked modulators. The function is
 * appropriate to be called by soundfont loader.
 *
 * @param zone_name, zone name used to prefix messages displayed.
 * @param list_mod, address of pointer on modulators list.
 *  On input, the list may contains any unlinked or linked modulators.
 *  On output, the list contains only valid unlinked modulators.
 * @param linked_mod, address of pointer on linked modulators list returned
 *  if any linked modulators exist (Only if FLUID_OK). NULL is returned
 *  in this pointer if linked modulators doesn't exist in list_mod.
 * @return FLUID_OK if success, FLUID_FAILED otherwise (memory error).
 */
int
fluid_zone_check_mod(char *zone_name, fluid_mod_t **list_mod,
                     fluid_mod_t **linked_mod)
{

    /* Checks linked modulators paths from a zone modulators list */
    if(fluid_list_check_linked_mod(zone_name, *list_mod, linked_mod) == FLUID_FAILED)
    {
        return FLUID_FAILED;
    }

    /* removing all invalid modulators */
    fluid_zone_check_remove_mod(list_mod);

    /* limits the size of modulators list */
    fluid_limit_mod_list(zone_name, list_mod);

    /* limits the size of linked modulators list */
    fluid_limit_mod_list(zone_name, linked_mod);

    return FLUID_OK;
}

/*
 * fluid_zone_gen_import_sfont
 * Imports generators from sfzone to gen and range.
 * @param gen, pointer on destination generators table.
 * @param range, pointer on destination range generators.
 * @param sfzone, pointer on soundfont zone generators.
 */
static void
fluid_zone_gen_import_sfont(fluid_gen_t *gen, fluid_zone_range_t *range, SFZone *sfzone)
{
    fluid_list_t *r;
    SFGen *sfgen;

    for(r = sfzone->gen; r != NULL;)
    {
        sfgen = (SFGen *)fluid_list_get(r);

        switch(sfgen->id)
        {
        case GEN_KEYRANGE:
            range->keylo = sfgen->amount.range.lo;
            range->keyhi = sfgen->amount.range.hi;
            break;

        case GEN_VELRANGE:
            range->vello = sfgen->amount.range.lo;
            range->velhi = sfgen->amount.range.hi;
            break;

        case GEN_ATTENUATION:
            /* EMU8k/10k hardware applies a scale factor to initial attenuation generator values set at
             * preset and instrument level */
            gen[sfgen->id].val = (fluid_real_t) sfgen->amount.sword * EMU_ATTENUATION_FACTOR;
            gen[sfgen->id].flags = GEN_SET;
            break;

        default:
            /* FIXME: some generators have an unsigne word amount value but i don't know which ones */
            gen[sfgen->id].val = (fluid_real_t) sfgen->amount.sword;
            gen[sfgen->id].flags = GEN_SET;
            break;
        }

        r = fluid_list_next(r);
    }
}

/*
 * fluid_zone_mod_source_import_sfont
 * Imports source information from sf_source to src and flags.
 * @param src, pointer on destination modulator source.
 * @param flags, pointer on destination modulator flags.
 * @param sf_source, soundfont modulator source.
 * @return return TRUE if success, FALSE if source type is unknow.
 */
static int
fluid_zone_mod_source_import_sfont(unsigned char *src, unsigned char *flags, unsigned short sf_source)
{
    int type;
    unsigned char flags_dest; /* destination flags */

    /* sources */
    *src = sf_source & 127; /* index of source, seven-bit value, SF2.01 section 8.2, page 50 */

    /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
    flags_dest = 0;

    if(sf_source & (1 << 7))
    {
        flags_dest |= FLUID_MOD_CC;
    }
    else
    {
        flags_dest |= FLUID_MOD_GC;
    }

    /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
    if(sf_source & (1 << 8))
    {
        flags_dest |= FLUID_MOD_NEGATIVE;
    }
    else
    {
        flags_dest |= FLUID_MOD_POSITIVE;
    }

    /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
    if(sf_source & (1 << 9))
    {
        flags_dest |= FLUID_MOD_BIPOLAR;
    }
    else
    {
        flags_dest |= FLUID_MOD_UNIPOLAR;
    }

    /* modulator source types: SF2.01 section 8.2.1 page 52 */
    type = sf_source >> 10;
    type &= 63; /* type is a 6-bit value */

    if(type == 0)
    {
        flags_dest |= FLUID_MOD_LINEAR;
    }
    else if(type == 1)
    {
        flags_dest |= FLUID_MOD_CONCAVE;
    }
    else if(type == 2)
    {
        flags_dest |= FLUID_MOD_CONVEX;
    }
    else if(type == 3)
    {
        flags_dest |= FLUID_MOD_SWITCH;
    }
    else
    {
        *flags = flags_dest;
        /* This shouldn't happen - unknown type! */
        return FALSE;
    }

    *flags = flags_dest;
    return TRUE;
}

/*
 * fluid_zone_mod_import_sfont
 * Imports modulators from sfzone to modulators list mod.
 * @param zone_name, zone name.
 * @param mod, address of pointer on modulators list to return.
 * @param linked_mod, address of pointer on linked modulators list returned
 *  if any linked modulators exist. NULL is returned in this pointer if linked
 *  modulators doesn't exist in mod.
 * @param sfzone, pointer on soundfont zone.
 * @return FLUID_OK if success, FLUID_FAILED otherwise.
 */
static int
fluid_zone_mod_import_sfont(char *zone_name, fluid_mod_t **mod, 
                            fluid_mod_t **linked_mod, SFZone *sfzone)
{
    /* bit link of destination in soundfont modulators */
    static const unsigned int FLUID_SFMOD_LINK_DEST = (1 << 15);   /* Link is bit 15 of destination */

    fluid_list_t *r;
    int count;

    /* Import the modulators (only SF2.1 and higher) */
    for(count = 0, r = sfzone->mod; r != NULL; count++)
    {

        SFMod *mod_src = (SFMod *)fluid_list_get(r);
        fluid_mod_t *mod_dest = new_fluid_mod(); /* next field is set to NULL */

        if(mod_dest == NULL)
        {
            return FLUID_FAILED;
        }

        /* *** Amount *** */
        mod_dest->amount = mod_src->amount;

        /* *** Source *** */
        if(!fluid_zone_mod_source_import_sfont(&mod_dest->src1, &mod_dest->flags1, mod_src->src))
        {
            /* This shouldn't happen - unknown type!
             * Deactivate the modulator by setting the amount to 0. */
            mod_dest->amount = 0;
        }  

        /* *** Dest *** */
        mod_dest->dest = mod_src->dest; /* index of controlled generator */

        /* import bit link of destination field */
        if(mod_src->dest & FLUID_SFMOD_LINK_DEST)
        {
            mod_dest->dest |= FLUID_MOD_LINK_DEST; /* set link bit of dest */
        }

        /* *** Amount source *** */
        if(!fluid_zone_mod_source_import_sfont(&mod_dest->src2, &mod_dest->flags2, mod_src->amtsrc))
        {
            /* This shouldn't happen - unknown type!
             * Deactivate the modulator by setting the amount to 0. */
            mod_dest->amount = 0;
        }  

        /* *** Transform *** */
        /* SF2.01 only uses the 'linear' transform (0).
         * Deactivate the modulator by setting the amount to 0 in any other case.
         */
        if(mod_src->trans != 0)
        {
            mod_dest->amount = 0;
        }

        /* Store the new modulator in the zone The order of modulators
         * will make a difference, at least in an instrument context: The
         * second modulator overwrites the first one, if they only differ
         * in amount. */
        if(count == 0)
        {
            *mod = mod_dest;
        }
        else
        {
            fluid_mod_t *last_mod = *mod;

            /* Find the end of the list */
            while(last_mod->next != NULL)
            {
                last_mod = last_mod->next;
            }

            last_mod->next = mod_dest;
        }

        r = fluid_list_next(r);
    } /* foreach modulator */

    /* checks and removes invalid modulators in modulators list*/
    return fluid_zone_check_mod(zone_name, mod, linked_mod);
}

/*
 * fluid_preset_zone_import_sfont
 */
int
fluid_preset_zone_import_sfont(fluid_preset_zone_t *zone, SFZone *sfzone, fluid_defsfont_t *defsfont)
{
    /* import the generators */
    fluid_zone_gen_import_sfont(zone->gen, &zone->range, sfzone);

    if((sfzone->instsamp != NULL) && (sfzone->instsamp->data != NULL))
    {
        SFInst *sfinst = sfzone->instsamp->data;

        zone->inst = find_inst_by_idx(defsfont, sfinst->idx);

        if(zone->inst == NULL)
        {
            zone->inst = fluid_inst_import_sfont(sfinst, defsfont);
        }

        if(zone->inst == NULL)
        {
            return FLUID_FAILED;
        }

        if(fluid_preset_zone_create_voice_zones(zone) == FLUID_FAILED)
        {
            return FLUID_FAILED;
        }
    }

    /* Import the modulators (only SF2.1 and higher) */
    return fluid_zone_mod_import_sfont(zone->name, &zone->mod,
                                       &zone->linked_mod, sfzone);
}

/*
 * fluid_preset_zone_get_inst
 */
fluid_inst_t *
fluid_preset_zone_get_inst(fluid_preset_zone_t *zone)
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
fluid_inst_t *
new_fluid_inst()
{
    fluid_inst_t *inst = FLUID_NEW(fluid_inst_t);

    if(inst == NULL)
    {
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
delete_fluid_inst(fluid_inst_t *inst)
{
    fluid_inst_zone_t *zone;

    fluid_return_if_fail(inst != NULL);

    delete_fluid_inst_zone(inst->global_zone);
    inst->global_zone = NULL;

    zone = inst->zone;

    while(zone != NULL)
    {
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
fluid_inst_set_global_zone(fluid_inst_t *inst, fluid_inst_zone_t *zone)
{
    inst->global_zone = zone;
    return FLUID_OK;
}

/*
 * fluid_inst_import_sfont
 */
fluid_inst_t *
fluid_inst_import_sfont(SFInst *sfinst, fluid_defsfont_t *defsfont)
{
    fluid_list_t *p;
    fluid_inst_t *inst;
    SFZone *sfzone;
    fluid_inst_zone_t *inst_zone;
    char zone_name[256];
    int count;

    inst = (fluid_inst_t *) new_fluid_inst();

    if(inst == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    inst->source_idx = sfinst->idx;

    p = sfinst->zone;

    if(FLUID_STRLEN(sfinst->name) > 0)
    {
        FLUID_STRCPY(inst->name, sfinst->name);
    }
    else
    {
        FLUID_STRCPY(inst->name, "<untitled>");
    }

    count = 0;

    while(p != NULL)
    {

        sfzone = (SFZone *)fluid_list_get(p);
        /* instrument zone name */
        FLUID_SNPRINTF(zone_name, sizeof(zone_name), "iz:%s/%d", inst->name, count);

        inst_zone = new_fluid_inst_zone(zone_name);

        if(inst_zone == NULL)
        {
            return NULL;
        }

        if(fluid_inst_zone_import_sfont(inst_zone, sfzone, defsfont) != FLUID_OK)
        {
            delete_fluid_inst_zone(inst_zone);
            return NULL;
        }

        if((count == 0) && (fluid_inst_zone_get_sample(inst_zone) == NULL))
        {
            fluid_inst_set_global_zone(inst, inst_zone);

        }
        else if(fluid_inst_add_zone(inst, inst_zone) != FLUID_OK)
        {
            return NULL;
        }

        p = fluid_list_next(p);
        count++;
    }

    defsfont->inst = fluid_list_append(defsfont->inst, inst);
    return inst;
}

/*
 * fluid_inst_add_zone
 */
int
fluid_inst_add_zone(fluid_inst_t *inst, fluid_inst_zone_t *zone)
{
    if(inst->zone == NULL)
    {
        zone->next = NULL;
        inst->zone = zone;
    }
    else
    {
        zone->next = inst->zone;
        inst->zone = zone;
    }

    return FLUID_OK;
}

/*
 * fluid_inst_get_zone
 */
fluid_inst_zone_t *
fluid_inst_get_zone(fluid_inst_t *inst)
{
    return inst->zone;
}

/*
 * fluid_inst_get_global_zone
 */
fluid_inst_zone_t *
fluid_inst_get_global_zone(fluid_inst_t *inst)
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
fluid_inst_zone_t *
new_fluid_inst_zone(char *name)
{
    fluid_inst_zone_t *zone = NULL;
    zone = FLUID_NEW(fluid_inst_zone_t);

    if(zone == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    zone->next = NULL;
    zone->name = FLUID_STRDUP(name);

    if(zone->name == NULL)
    {
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
    fluid_gen_init(&zone->gen[0], NULL);
    zone->mod = NULL; /* list of modulators */
    zone->linked_mod = NULL; /* List of linked modulators */
    return zone;
}

/*
 * delete_fluid_inst_zone
 */
void
delete_fluid_inst_zone(fluid_inst_zone_t *zone)
{
    fluid_return_if_fail(zone != NULL);

    delete_fluid_list_mod(zone->mod);       /* unlinked modulators */
    delete_fluid_list_mod(zone->linked_mod);/* linked modulators */

    FLUID_FREE(zone->name);
    FLUID_FREE(zone);
}

/*
 * fluid_inst_zone_next
 */
fluid_inst_zone_t *
fluid_inst_zone_next(fluid_inst_zone_t *zone)
{
    return zone->next;
}

/*
 * fluid_inst_zone_import_sfont
 */
int
fluid_inst_zone_import_sfont(fluid_inst_zone_t *inst_zone, SFZone *sfzone, fluid_defsfont_t *defsfont)
{
    /* import the generators */
    fluid_zone_gen_import_sfont(inst_zone->gen, &inst_zone->range, sfzone);

    /* FIXME */
    /*    if (zone->gen[GEN_EXCLUSIVECLASS].flags == GEN_SET) { */
    /*      FLUID_LOG(FLUID_DBG, "ExclusiveClass=%d\n", (int) zone->gen[GEN_EXCLUSIVECLASS].val); */
    /*    } */

    /* fixup sample pointer */
    if((sfzone->instsamp != NULL) && (sfzone->instsamp->data != NULL))
    {
        inst_zone->sample = ((SFSample *)(sfzone->instsamp->data))->fluid_sample;
    }

    /* Import the modulators (only SF2.1 and higher) */
    return fluid_zone_mod_import_sfont(inst_zone->name, &inst_zone->mod,
                                       &inst_zone->linked_mod, sfzone);
}

/*
 * fluid_inst_zone_get_sample
 */
fluid_sample_t *
fluid_inst_zone_get_sample(fluid_inst_zone_t *zone)
{
    return zone->sample;
}


int
fluid_zone_inside_range(fluid_zone_range_t *range, int key, int vel)
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
fluid_sample_in_rom(fluid_sample_t *sample)
{
    return (sample->sampletype & FLUID_SAMPLETYPE_ROM);
}


/*
 * fluid_sample_import_sfont
 */
int
fluid_sample_import_sfont(fluid_sample_t *sample, SFSample *sfsample, fluid_defsfont_t *defsfont)
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

    if(defsfont->dynamic_samples)
    {
        sample->notify = dynamic_samples_sample_notify;
    }

    if(fluid_sample_validate(sample, defsfont->samplesize) == FLUID_FAILED)
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
    if(reason == FLUID_SAMPLE_DONE && sample->preset_count == 0)
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

    if(reason == FLUID_PRESET_SELECTED)
    {
        FLUID_LOG(FLUID_DBG, "Selected preset '%s' on channel %d", fluid_preset_get_name(preset), chan);
        defsfont = fluid_sfont_get_data(preset->sfont);
        load_preset_samples(defsfont, preset);
    }
    else if(reason == FLUID_PRESET_UNSELECTED)
    {
        FLUID_LOG(FLUID_DBG, "Deselected preset '%s' from channel %d", fluid_preset_get_name(preset), chan);
        defsfont = fluid_sfont_get_data(preset->sfont);
        unload_preset_samples(defsfont, preset);
    }

    return FLUID_OK;
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

    while(preset_zone != NULL)
    {
        inst = fluid_preset_zone_get_inst(preset_zone);
        inst_zone = fluid_inst_get_zone(inst);

        while(inst_zone != NULL)
        {
            sample = fluid_inst_zone_get_sample(inst_zone);

            if((sample != NULL) && (sample->start != sample->end))
            {
                sample->preset_count++;

                /* If this is the first time this sample has been selected,
                 * load the sampledata */
                if(sample->preset_count == 1)
                {
                    /* Make sure we have an open Soundfont file. Do this here
                     * to avoid having to open the file if no loading is necessary
                     * for a preset */
                    if(sffile == NULL)
                    {
                        sffile = fluid_sffile_open(defsfont->filename, defsfont->fcbs);

                        if(sffile == NULL)
                        {
                            FLUID_LOG(FLUID_ERR, "Unable to open Soundfont file");
                            return FLUID_FAILED;
                        }
                    }

                    if(fluid_defsfont_load_sampledata(defsfont, sffile, sample) == FLUID_OK)
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

    if(sffile != NULL)
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

    while(preset_zone != NULL)
    {
        inst = fluid_preset_zone_get_inst(preset_zone);
        inst_zone = fluid_inst_get_zone(inst);

        while(inst_zone != NULL)
        {
            sample = fluid_inst_zone_get_sample(inst_zone);

            if((sample != NULL) && (sample->preset_count > 0))
            {
                sample->preset_count--;

                /* If the sample is not used by any preset or used by a
                 * sounding voice, unload it from the sample cache. If it's
                 * still in use by a voice, dynamic_samples_sample_notify will
                 * take care of unloading the sample as soon as the voice is
                 * finished with it (but only on the next API call). */
                if(sample->preset_count == 0 && sample->refcount == 0)
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

    if(fluid_samplecache_unload(sample->data) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Unable to unload sample '%s'", sample->name);
    }
    else
    {
        sample->data = NULL;
        sample->data24 = NULL;
    }
}

static fluid_inst_t *find_inst_by_idx(fluid_defsfont_t *defsfont, int idx)
{
    fluid_list_t *list;
    fluid_inst_t *inst;

    for(list = defsfont->inst; list != NULL; list = fluid_list_next(list))
    {
        inst = fluid_list_get(list);

        if(inst->source_idx == idx)
        {
            return inst;
        }
    }

    return NULL;
}
