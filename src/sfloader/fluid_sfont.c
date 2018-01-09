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
    
    return loader;
}

/**
 * Frees a SoundFont loader created with new_fluid_sfloader().
 */
void delete_fluid_sfloader(fluid_sfloader_t* loader)
{
    fluid_return_if_fail(loader != NULL);
    
    FLUID_FREE(loader);
}

/**
 * Specify private data to be used by #fluid_sfloader_load_t.
 */
void fluid_sfloader_set_data(fluid_sfloader_t* loader, void* data)
{
    loader->data = data;
}

void* fluid_sfloader_get_data(fluid_sfloader_t* loader)
{
    return loader->data;
}

void fluid_sfloader_set_callbacks(fluid_sfloader_t* loader,
                                  fluid_sfloader_callback_open_t open,
                                  fluid_sfloader_callback_read_t read,
                                  fluid_sfloader_callback_seek_t seek,
                                  fluid_sfloader_callback_tell_t tell,
                                  fluid_sfloader_callback_close_t close)
{
    fluid_file_callbacks_t *cb = &loader->file_callbacks;
    
    cb->fopen = open;
    cb->fread = read;
    cb->fseek = seek;
    cb->ftell = tell;
    cb->fclose = close;
}




/*
 * new_fluid_sample
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

/*
 * delete_fluid_sample
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
 * @param data Buffer containing 16 bit audio sample data
 * @param data24 If not NULL, pointer to the least significant byte counterparts of each sample data point in order to create 24 bit audio samples
 * @param nbframes Number of samples in \a data
 * @param copy_data TRUE to copy the sample data (and automatically free it upon delete_fluid_sample()), FALSE to use it directly (and not free it)
 * @param rootkey Root MIDI note of sample (0-127)
 * @param sample_rate Sampling rate of the sample data
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
                             short copy_data,
                             int rootkey,
                             unsigned int sample_rate
                            )
{
    /* the number of samples before the start and after the end */
    #define SAMPLE_LOOP_MARGIN 8

    fluid_return_val_if_fail(sample != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(data != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(0<=rootkey && rootkey<=127, FLUID_FAILED);
    
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
        sample->end = SAMPLE_LOOP_MARGIN + storedNbFrames;
    }
    else
    {
        /* we cannot assure the SAMPLE_LOOP_MARGIN */
        sample->data = data;
        sample->data24 = data24;
        sample->start = 0;
        sample->end = nbframes;
    }

    /* only used as markers for the LOOP generators : set them on the first real frame */
    sample->loopstart = sample->start;
    sample->loopend = sample->end;

    sample->samplerate = sample_rate;
    sample->origpitch = rootkey;
    sample->pitchadj = 0;
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


