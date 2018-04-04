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

#include "fluid_samplecache.h"
#include "fluidsynth.h"
#include "fluid_sys.h"

/***************************************************************
 *
 *                    CACHED SAMPLEDATA LOADER
 */

typedef struct _fluid_cached_sampledata_t {
  struct _fluid_cached_sampledata_t *next;

  char* filename;
  time_t modification_time;
  int num_references;
  int mlock;

  short* sampledata;
  unsigned int samplesize;
  
  char* sample24data;
  unsigned int sample24size;
} fluid_cached_sampledata_t;

static fluid_cached_sampledata_t* all_cached_sampledata = NULL;
static fluid_mutex_t cached_sampledata_mutex = FLUID_MUTEX_INIT;

static int fluid_get_file_modification_time(char *filename, time_t *modification_time)
{
#if defined(WIN32) || defined(__OS2__)
  *modification_time = 0;
  return FLUID_OK;
#else
  struct stat buf;

  if (stat(filename, &buf) == -1) {
    return FLUID_FAILED;
  }

  *modification_time = buf.st_mtime;
  return FLUID_OK;
#endif
}

int fluid_cached_sampledata_load(char *filename,
        unsigned int samplepos, unsigned int samplesize, short **sampledata,
        unsigned int sample24pos, unsigned int sample24size, char **sample24data,
        int try_mlock, const fluid_file_callbacks_t* fcbs)
{
  fluid_file fd = NULL;
  short *loaded_sampledata = NULL;
  char  *loaded_sample24data = NULL;
  fluid_cached_sampledata_t* cached_sampledata = NULL;
  time_t modification_time;

  fluid_mutex_lock(cached_sampledata_mutex);

  if (fluid_get_file_modification_time(filename, &modification_time) == FLUID_FAILED) {
    FLUID_LOG(FLUID_WARN, "Unable to read modificaton time of soundfont file.");
    modification_time = 0;
  }

  for (cached_sampledata = all_cached_sampledata; cached_sampledata; cached_sampledata = cached_sampledata->next) {
    if (FLUID_STRCMP(filename, cached_sampledata->filename))
      continue;
    if (cached_sampledata->modification_time != modification_time)
      continue;
    if (cached_sampledata->samplesize != samplesize || cached_sampledata->sample24size != sample24size) {
      FLUID_LOG(FLUID_ERR, "Cached size of soundfont doesn't match actual size of soundfont (cached: %u. actual: %u)",
        cached_sampledata->samplesize, samplesize);
      continue;
    }

    if (try_mlock && !cached_sampledata->mlock) {
      if (fluid_mlock(cached_sampledata->sampledata, samplesize) != 0)
        FLUID_LOG(FLUID_WARN, "Failed to pin the sample data to RAM; swapping is possible.");
      else
        cached_sampledata->mlock = try_mlock;
      
      if (cached_sampledata->sample24data != NULL)
          if(fluid_mlock(cached_sampledata->sample24data, sample24size) != 0)
            FLUID_LOG(FLUID_WARN, "Failed to pin the sample24 data to RAM; swapping is possible.");
    }

    cached_sampledata->num_references++;
    loaded_sampledata = cached_sampledata->sampledata;
    loaded_sample24data = cached_sampledata->sample24data;
    goto success_exit;
  }

  fd = fcbs->fopen(filename);
  if (fd == NULL) {
    FLUID_LOG(FLUID_ERR, "Can't open soundfont file");
    goto error_exit;
  }
  if (fcbs->fseek(fd, samplepos, SEEK_SET) == FLUID_FAILED) {
    perror("error");
    FLUID_LOG(FLUID_ERR, "Failed to seek position in data file");
    goto error_exit;
  }

  loaded_sampledata = (short*) FLUID_MALLOC(samplesize);
  if (loaded_sampledata == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    goto error_exit;
  }
  if (fcbs->fread(loaded_sampledata, samplesize, fd) == FLUID_FAILED) {
    FLUID_LOG(FLUID_ERR, "Failed to read sample data");
    goto error_exit;
  }

  if(sample24pos > 0)
  {
    if (fcbs->fseek(fd, sample24pos, SEEK_SET) == FLUID_FAILED) {
        perror("error");
        FLUID_LOG(FLUID_ERR, "Failed to seek position in data file");
        goto error_exit;
    }
    
    loaded_sample24data = (char*) FLUID_MALLOC(sample24size);
    if (loaded_sample24data == NULL) {
        FLUID_LOG(FLUID_ERR, "Out of memory when allocating 24bit sample, ignoring");
    }
    else if (fcbs->fread(loaded_sample24data, sample24size, fd) == FLUID_FAILED) {
        FLUID_LOG(FLUID_ERR, "Failed to read sample24 data");
        FLUID_FREE(loaded_sample24data);
        loaded_sample24data = NULL;
    }
  }
  
  fcbs->fclose(fd);
  fd = NULL;


  cached_sampledata = (fluid_cached_sampledata_t*) FLUID_MALLOC(sizeof(fluid_cached_sampledata_t));
  if (cached_sampledata == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory.");
    goto error_exit;
  }

  /* Lock the memory to disable paging. It's okay if this fails. It
     probably means that the user doesn't have the required permission.  */
  cached_sampledata->mlock = 0;
  if (try_mlock) {
    if (fluid_mlock(loaded_sampledata, samplesize) != 0)
      FLUID_LOG(FLUID_WARN, "Failed to pin the sample data to RAM; swapping is possible.");
    else
      cached_sampledata->mlock = try_mlock;
  }

  /* If this machine is big endian, the sample have to byte swapped  */
  if (FLUID_IS_BIG_ENDIAN) {
    unsigned char* cbuf;
    unsigned char hi, lo;
    unsigned int i, j;
    short s;
    cbuf = (unsigned char*)loaded_sampledata;
    for (i = 0, j = 0; j < samplesize; i++) {
      lo = cbuf[j++];
      hi = cbuf[j++];
      s = (hi << 8) | lo;
      loaded_sampledata[i] = s;
    }
  }

  cached_sampledata->filename = FLUID_STRDUP(filename);
  if (cached_sampledata->filename == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory.");
    goto error_exit;
  }

  cached_sampledata->modification_time = modification_time;
  cached_sampledata->num_references = 1;
  cached_sampledata->sampledata = loaded_sampledata;
  cached_sampledata->samplesize = samplesize;
  cached_sampledata->sample24data = loaded_sample24data;
  cached_sampledata->sample24size = sample24size;

  cached_sampledata->next = all_cached_sampledata;
  all_cached_sampledata = cached_sampledata;


 success_exit:
  fluid_mutex_unlock(cached_sampledata_mutex);
  *sampledata = loaded_sampledata;
  *sample24data = loaded_sample24data;
  return FLUID_OK;

 error_exit:
  if (fd != NULL) {
    fcbs->fclose(fd);
  }
  
  FLUID_FREE(loaded_sampledata);
  FLUID_FREE(loaded_sample24data);

  if (cached_sampledata != NULL) {
      FLUID_FREE(cached_sampledata->filename);
  }
    FLUID_FREE(cached_sampledata);

  fluid_mutex_unlock(cached_sampledata_mutex);
  *sampledata = NULL;
  *sample24data = NULL;
  return FLUID_FAILED;
}

int fluid_cached_sampledata_unload(const short *sampledata)
{
  fluid_cached_sampledata_t* prev = NULL;
  fluid_cached_sampledata_t* cached_sampledata;

  fluid_mutex_lock(cached_sampledata_mutex);
  cached_sampledata = all_cached_sampledata;

  while (cached_sampledata != NULL) {
    if (sampledata == cached_sampledata->sampledata) {

      cached_sampledata->num_references--;

      if (cached_sampledata->num_references == 0) {
        if (cached_sampledata->mlock)
        {
          fluid_munlock(cached_sampledata->sampledata, cached_sampledata->samplesize);
          fluid_munlock(cached_sampledata->sample24data, cached_sampledata->sample24size);
        }
        FLUID_FREE(cached_sampledata->sampledata);
        FLUID_FREE(cached_sampledata->sample24data);
        FLUID_FREE(cached_sampledata->filename);

        if (prev != NULL) {
          prev->next = cached_sampledata->next;
        } else {
          all_cached_sampledata = cached_sampledata->next;
        }

        FLUID_FREE(cached_sampledata);
      }

      goto success_exit;
    }

    prev = cached_sampledata;
    cached_sampledata = cached_sampledata->next;
  }

  FLUID_LOG(FLUID_ERR, "Trying to free sampledata not found in cache.");
  goto error_exit;
  
 success_exit:
  fluid_mutex_unlock(cached_sampledata_mutex);
  return FLUID_OK;

 error_exit:
  fluid_mutex_unlock(cached_sampledata_mutex);
  return FLUID_FAILED;
}
