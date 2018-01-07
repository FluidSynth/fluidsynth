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

void fluid_sfloader_set_callbacks(fluid_sfloader_t* loader,
                                  fluid_sfloader_callback_open open,
                                  fluid_sfloader_callback_read read,
                                  fluid_sfloader_callback_seek seek,
                                  fluid_sfloader_callback_tell tell,
                                  fluid_sfloader_callback_close close)
{
    fluid_file_callbacks_t *cb = &loader->file_callbacks;
    
    cb->fopen = open;
    cb->fread = read;
    cb->fseek = seek;
    cb->ftell = tell;
    cb->fclose = close;
}

