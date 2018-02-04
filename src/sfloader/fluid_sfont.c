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

#include "fluid_sfont.h"
#include "fluid_sys.h"

void * default_fopen(const char * path)
{
    return FLUID_FOPEN(path, "rb");
}

int default_fclose(void * handle)
{
    return FLUID_FCLOSE((FILE *)handle) == 0 ? FLUID_OK : FLUID_FAILED;
}

long default_ftell(void * handle)
{
    return FLUID_FTELL((FILE *)handle);
}

int safe_fread (void *buf, int count, void * fd)
{
  if (FLUID_FREAD(buf, count, 1, (FILE *)fd) != 1)
    {
      if (feof ((FILE *)fd))
	FLUID_LOG (FLUID_ERR, _("EOF while attemping to read %d bytes"), count);
      else
	FLUID_LOG (FLUID_ERR, _("File read failed"));
  
      return FLUID_FAILED;
    }
  return FLUID_OK;
}

int safe_fseek (void * fd, long ofs, int whence)
{
  if (FLUID_FSEEK((FILE *)fd, ofs, whence) != 0) {
    FLUID_LOG (FLUID_ERR, _("File seek failed with offset = %ld and whence = %d"), ofs, whence);
    return FLUID_FAILED;
  }
  return FLUID_OK;
}

/**
 * Creates a new SoundFont loader.
 * 
 * @param load Pointer to a function that provides a #fluid_sfont_t (see #fluid_sfloader_load_t).
 * @param free Pointer to a function that destroys this instance (see #fluid_sfloader_free_t).
 * Unless any private data needs to be freed it is sufficient to set this to delete_fluid_sfloader().
 * 
 * @return the SoundFont loader instance on success, NULL otherwise.
 */
fluid_sfloader_t* new_fluid_sfloader(fluid_sfloader_load_t load, fluid_sfloader_free_t free)
{
    fluid_sfloader_t *loader;
    
    fluid_return_val_if_fail(load != NULL, NULL);
    fluid_return_val_if_fail(free != NULL, NULL);
    
    loader = FLUID_NEW(fluid_sfloader_t);
    if (loader == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    FLUID_MEMSET(loader, 0, sizeof(*loader));
    
    loader->load = load;
    loader->free = free;
    fluid_sfloader_set_callbacks(loader,
                                 default_fopen,
                                 safe_fread,
                                 safe_fseek,
                                 default_ftell,
                                 default_fclose);
    
    return loader;
}

/**
 * Frees a SoundFont loader created with new_fluid_sfloader().
 * 
 * @param loader The SoundFont loader instance to free.
 */
void delete_fluid_sfloader(fluid_sfloader_t* loader)
{
    fluid_return_if_fail(loader != NULL);
    
    FLUID_FREE(loader);
}

/**
 * Specify private data to be used by #fluid_sfloader_load_t.
 * 
 * @param loader The SoundFont loader instance.
 * @param data The private data to store.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_sfloader_set_data(fluid_sfloader_t* loader, void* data)
{
    fluid_return_val_if_fail(loader != NULL, FLUID_FAILED);
    
    loader->data = data;
    return FLUID_OK;
}

/**
 * Obtain private data previously set with fluid_sfloader_set_data().
 * 
 * @param loader The SoundFont loader instance.
 * @return The private data or NULL if none explicitly set before.
 */
void* fluid_sfloader_get_data(fluid_sfloader_t* loader)
{
    fluid_return_val_if_fail(loader != NULL, NULL);
    
    return loader->data;
}

/**
 * Set custom callbacks to be used upon soundfont loading.
 * 
 * Useful for loading a soundfont from memory, see \a doc/fluidsynth_sfload_mem.c as an example.
 * 
 * @param loader The SoundFont loader instance.
 * @param open A function implementing #fluid_sfloader_callback_open_t.
 * @param read A function implementing #fluid_sfloader_callback_read_t.
 * @param seek A function implementing #fluid_sfloader_callback_seek_t.
 * @param tell A function implementing #fluid_sfloader_callback_tell_t.
 * @param close A function implementing #fluid_sfloader_callback_close_t.
 * @return #FLUID_OK if the callbacks have been successfully set, #FLUID_FAILED otherwise.
 */
int fluid_sfloader_set_callbacks(fluid_sfloader_t* loader,
                                  fluid_sfloader_callback_open_t open,
                                  fluid_sfloader_callback_read_t read,
                                  fluid_sfloader_callback_seek_t seek,
                                  fluid_sfloader_callback_tell_t tell,
                                  fluid_sfloader_callback_close_t close)
{
    fluid_file_callbacks_t *cb;
    
    fluid_return_val_if_fail(loader != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(open != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(read != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(seek != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(tell != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(close != NULL, FLUID_FAILED);
    
    cb = &loader->file_callbacks;
    
    cb->fopen = open;
    cb->fread = read;
    cb->fseek = seek;
    cb->ftell = tell;
    cb->fclose = close;
    
    return FLUID_OK;
}

/**
 * Creates a new virtual SoundFont instance structure.
 * @param get_name A function implementing #fluid_sfont_get_name_t.
 * @param get_preset A function implementing #fluid_sfont_get_preset_t.
 * @param free A function implementing #fluid_sfont_free_t.
 * @return The soundfont instance on success or NULL otherwise.
 */
fluid_sfont_t* new_fluid_sfont(fluid_sfont_get_name_t get_name,
                               fluid_sfont_get_preset_t get_preset,
                               fluid_sfont_free_t free)
{
    fluid_sfont_t* sfont;
    
    fluid_return_val_if_fail(get_name != NULL, NULL);
    fluid_return_val_if_fail(get_preset != NULL, NULL);
    fluid_return_val_if_fail(free != NULL, NULL);
    
    sfont = FLUID_NEW(fluid_sfont_t);
    if (sfont == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    FLUID_MEMSET(sfont, 0, sizeof(*sfont));

    sfont->get_name = get_name;
    sfont->get_preset = get_preset;
    sfont->free = free;
    
    return sfont;
}

/**
 * Set private data to use with a SoundFont instance.
 * 
 * @param sfont The SoundFont instance.
 * @param data The private data to store.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_sfont_set_data(fluid_sfont_t* sfont, void* data)
{
    fluid_return_val_if_fail(sfont != NULL, FLUID_FAILED);
    
    sfont->data = data;
    return FLUID_OK;
}

/**
 * Retrieve the private data of a SoundFont instance.
 * 
 * @param sfont The SoundFont instance.
 * @return The private data or NULL if none explicitly set before.
 */
void* fluid_sfont_get_data(fluid_sfont_t* sfont)
{
    fluid_return_val_if_fail(sfont != NULL, NULL);
    
    return sfont->data;
}

/**
 * @internal KISS! No need to expose this to public API currently.
 */
void fluid_sfont_set_iteration_start(fluid_sfont_t* sfont, fluid_sfont_iteration_start_t iter_start)
{
    sfont->iteration_start = iter_start;
}

/**
 * @internal KISS! No need to expose this to public API currently.
 */
void fluid_sfont_set_iteration_next(fluid_sfont_t* sfont, fluid_sfont_iteration_next_t iter_next)
{
    sfont->iteration_next = iter_next;
}

/**
 * Destroys a SoundFont instance created with new_fluid_sfont().
 * 
 * Implements #fluid_sfont_free_t.
 * 
 * @param sfont The SoundFont instance to destroy.
 * @return Always returns 0.
 */
int delete_fluid_sfont(fluid_sfont_t* sfont)
{
    fluid_return_val_if_fail(sfont != NULL, 0);
    
    FLUID_FREE(sfont);
    return 0;
}

/**
 * Create a virtual SoundFont preset instance.
 * 
 * @param parent_sfont The SoundFont instance this preset shall belong to
 * @param get_name A function implementing #fluid_preset_get_name_t
 * @param get_bank A function implementing #fluid_preset_get_banknum_t
 * @param get_num A function implementing #fluid_preset_get_num_t
 * @param noteon A function implementing #fluid_preset_noteon_t
 * @param free A function implementing #fluid_preset_free_t
 * @return The preset instance on success, NULL otherwise.
 */
fluid_preset_t* new_fluid_preset(fluid_sfont_t* parent_sfont,
                                 fluid_preset_get_name_t get_name,
                                 fluid_preset_get_banknum_t get_bank,
                                 fluid_preset_get_num_t get_num,
                                 fluid_preset_noteon_t noteon,
                                 fluid_preset_free_t free)
{
    fluid_preset_t* preset;
 
    fluid_return_val_if_fail(parent_sfont != NULL, NULL);
    fluid_return_val_if_fail(get_name != NULL, NULL);
    fluid_return_val_if_fail(get_bank != NULL, NULL);
    fluid_return_val_if_fail(get_num != NULL, NULL);
    fluid_return_val_if_fail(noteon != NULL, NULL);
    fluid_return_val_if_fail(free != NULL, NULL);
    
    preset = FLUID_NEW(fluid_preset_t);
    if (preset == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    FLUID_MEMSET(preset, 0, sizeof(*preset));
    
    preset->sfont = parent_sfont;
    preset->get_name = get_name;
    preset->get_banknum = get_bank;
    preset->get_num = get_num;
    preset->noteon = noteon;
    preset->free = free;
    
    return preset;
}

/**
 * Set private data to use with a SoundFont preset instance.
 * 
 * @param preset The SoundFont preset instance.
 * @param data The private data to store.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_preset_set_data(fluid_preset_t* preset, void* data)
{
    fluid_return_val_if_fail(preset != NULL, FLUID_FAILED);
    
    preset->data = data;
    return FLUID_OK;
}

/**
 * Retrieve the private data of a SoundFont preset instance.
 * 
 * @param sfont The SoundFont preset instance.
 * @return The private data or NULL if none explicitly set before.
 */
void* fluid_preset_get_data(fluid_preset_t* preset)
{
    fluid_return_val_if_fail(preset != NULL, NULL);
    
    return preset->data;
}

/**
 * Destroys a SoundFont preset instance created with new_fluid_preset().
 * 
 * Implements #fluid_preset_free_t.
 * 
 * @param preset The SoundFont preset instance to destroy.
 */
void delete_fluid_preset(fluid_preset_t* preset)
{
    fluid_return_if_fail(preset != NULL);
    
    FLUID_FREE(preset);
}

/**
 * Create a new sample instance.
 * @return  The sample on success, NULL otherwise.
 */
fluid_sample_t*
new_fluid_sample()
{
    fluid_sample_t* sample = NULL;

    sample = FLUID_NEW(fluid_sample_t);
    if (sample == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    FLUID_MEMSET(sample, 0, sizeof(*sample));

    return sample;
}

/**
 * Destroy a sample instance previously created with new_fluid_sample().
 * @param sample The sample to destroy.
 */
void
delete_fluid_sample(fluid_sample_t* sample)
{
    fluid_return_if_fail(sample != NULL);
        
    if (sample->auto_free)
    {
        FLUID_FREE(sample->data);
        FLUID_FREE(sample->data24);
    }

    FLUID_FREE(sample);
}

/**
 * Returns the size of the fluid_sample_t structure.
 * 
 * Useful in low latency scenarios e.g. to allocate a sample on the stack.
 */
size_t fluid_sample_sizeof()
{
    return sizeof(fluid_sample_t);
}

/**
 * Set the name of a SoundFont sample.
 * @param sample SoundFont sample
 * @param name Name to assign to sample (20 chars in length + zero terminator)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int fluid_sample_set_name(fluid_sample_t* sample, const char *name)
{
    fluid_return_val_if_fail(sample != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);
    
    FLUID_STRNCPY(sample->name, name, sizeof(sample->name));
    return FLUID_OK;
}

/**
 * Assign sample data to a SoundFont sample.
 * @param sample SoundFont sample
 * @param data Buffer containing 16 bit (mono-)audio sample data
 * @param data24 If not NULL, pointer to the least significant byte counterparts of each sample data point in order to create 24 bit audio samples
 * @param nbframes Number of samples in \a data
 * @param sample_rate Sampling rate of the sample data
 * @param copy_data TRUE to copy the sample data (and automatically free it upon delete_fluid_sample()), FALSE to use it directly (and not free it)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * @note If \a copy_data is FALSE, data should have 8 unused frames at start
 * and 8 unused frames at the end and \a nbframes should be >=48
 */
int
fluid_sample_set_sound_data (fluid_sample_t* sample,
                             short *data,
                             char *data24,
                             unsigned int nbframes,
                             unsigned int sample_rate,
                             short copy_data
                            )
{
    /* the number of samples before the start and after the end */
    #define SAMPLE_LOOP_MARGIN 8U

    fluid_return_val_if_fail(sample != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(data != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(nbframes == 0, FLUID_FAILED);
    
    /* in case we already have some data */
    if ((sample->data != NULL || sample->data24 != NULL) && sample->auto_free)
    {
        FLUID_FREE(sample->data);
        FLUID_FREE(sample->data24);
    }

    if (copy_data)
    {
        unsigned int storedNbFrames;

        /* nbframes should be >= 48 (SoundFont specs) */
        storedNbFrames = nbframes;
        if (storedNbFrames < 48) storedNbFrames = 48;
        
        storedNbFrames += 2*SAMPLE_LOOP_MARGIN;
        
        sample->data = FLUID_ARRAY(short, storedNbFrames);
        if (sample->data == NULL)
        {
            goto error_rec;
        }
        FLUID_MEMSET(sample->data, 0, storedNbFrames);
        FLUID_MEMCPY(sample->data + SAMPLE_LOOP_MARGIN, data, nbframes*sizeof(short));
        
        if(data24 != NULL)
        {
            sample->data24 = FLUID_ARRAY(char, storedNbFrames);
            if (sample->data24 == NULL)
            {
                goto error_rec;
            }
            FLUID_MEMSET(sample->data24, 0, storedNbFrames);
            FLUID_MEMCPY(sample->data24 + SAMPLE_LOOP_MARGIN, data24, nbframes*sizeof(char));
        }
        
        /* pointers */
        /* all from the start of data */
        sample->start = SAMPLE_LOOP_MARGIN;
        sample->end = SAMPLE_LOOP_MARGIN + storedNbFrames - 1;
    }
    else
    {
        /* we cannot assure the SAMPLE_LOOP_MARGIN */
        sample->data = data;
        sample->data24 = data24;
        sample->start = 0;
        sample->end = nbframes - 1;
    }

    sample->samplerate = sample_rate;
    sample->sampletype = FLUID_SAMPLETYPE_MONO;
    sample->valid = 1;
    sample->auto_free = copy_data;

    return FLUID_OK;
    
error_rec:
    FLUID_LOG(FLUID_ERR, "Out of memory");
    FLUID_FREE(sample->data);
    FLUID_FREE(sample->data24);
    return FLUID_FAILED;
    
#undef SAMPLE_LOOP_MARGIN
}

/**
 * Set the loop of a sample.
 * 
 * @param loop_start Start sample index of the loop.
 * @param loop_end End index of the loop (must be a valid sample as it marks the last sample to be played).
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_sample_set_loop(fluid_sample_t* sample, unsigned int loop_start, unsigned int loop_end)
{
    fluid_return_val_if_fail(sample != NULL, FLUID_FAILED);
    
    sample->loopstart = loop_start;
    sample->loopend = loop_end;
    
    return FLUID_OK;
}

/**
 * Set the pitch of a sample.
 * 
 * @param rootkey Root MIDI note of sample (0-127)
 * @param fine_tune Fine tune in cents
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_sample_set_pitch(fluid_sample_t* sample, int root_key, int fine_tune)
{
    fluid_return_val_if_fail(sample != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(0<=root_key && root_key<=127, FLUID_FAILED);
    
    sample->origpitch = root_key;
    sample->pitchadj = fine_tune;
    
    return FLUID_OK;
}
