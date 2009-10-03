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

 /* 
  * Low-level routines for file output.
  */

#include <stdio.h>
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include "fluid_sys.h"

#if LIBSNDFILE_SUPPORT
#include <sndfile.h>
#endif

struct _fluid_file_renderer_t {
	fluid_synth_t* synth;

#if LIBSNDFILE_SUPPORT
	SNDFILE* sndfile;
	float* buf;
#else
	FILE* file;
	short* buf;
#endif

	int period_size;
	int buf_size;
};

#if LIBSNDFILE_SUPPORT

/* Default file type used, if none specified and auto extension search fails */
#define FLUID_FILE_RENDERER_DEFAULT_FILE_TYPE   SF_FORMAT_WAV

/* File audio format names.
 * !! Keep in sync with format_ids[] */
const char *format_names[] = {
  "s8",
  "s16",
  "s24",
  "s32",
  "u8",
  "float",
  "double",
  NULL          /* Terminator */
};
  

/* File audio format IDs.
 * !! Keep in sync with format_names[] */
const int format_ids[] = {
  SF_FORMAT_PCM_S8,
  SF_FORMAT_PCM_16,
  SF_FORMAT_PCM_24,
  SF_FORMAT_PCM_32,
  SF_FORMAT_PCM_U8,
  SF_FORMAT_FLOAT,
  SF_FORMAT_DOUBLE
};

/* File endian byte order names.
 * !! Keep in sync with endian_ids[] */
const char *endian_names[] = {
  "auto",
  "little",
  "big",
  "cpu",
  NULL
};

/* File endian byte order ids.
 * !! Keep in sync with endian_names[] */
const int endian_ids[] = {
  SF_ENDIAN_FILE,
  SF_ENDIAN_LITTLE,
  SF_ENDIAN_BIG,
  SF_ENDIAN_CPU
};

static int fluid_file_renderer_parse_options (char *filetype, char *format,
                                              char *endian, char *filename, SF_INFO *info);
static int fluid_file_renderer_find_file_type (char *extension, int *type);


#else   /* No libsndfile support */


/* File type names. */
char *type_names[] = {
  "raw",
  NULL          /* Terminator */
};

/* File audio format names. */
char *format_names[] = {
  "s16",
  NULL          /* Terminator */
};

/* File endian byte order names. */
char *endian_names[] = {
  "cpu",
  NULL
};

#endif


/**
 * Create a new file renderer and open the file.
 * @param synth The synth that creates audio data.
 * @param filename Output filename 
 * @param type File type string or NULL for default "auto", which tries to
 *   determine format from filename extension or uses "wav".
 * @param format Audio format string (can be empty or NULL for default format "s16")
 * @param endian Endian specification or NULL for default "auto", which uses
 *   the file type's usual byte order.
 * @param period_size Sample count, amount of samples to write to the file at 
 * every call to fluid_file_renderer_process_block().
 * @return the new object, or NULL on failure
 * @since: 1.1.0
 */
fluid_file_renderer_t *
new_fluid_file_renderer(fluid_synth_t* synth, char* filename, char* type,
                        char* format, char* endian, int period_size)
{
	fluid_file_renderer_t* dev;
#if LIBSNDFILE_SUPPORT
	SF_INFO info;
	double samplerate;
#endif

	dev = FLUID_NEW(fluid_file_renderer_t);
	if (dev == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	FLUID_MEMSET(dev, 0, sizeof(fluid_file_renderer_t));

	dev->synth = synth;
	dev->period_size = period_size;

#if LIBSNDFILE_SUPPORT
	dev->buf_size = 2 * dev->period_size * sizeof (float);
	dev->buf = FLUID_ARRAY(float, 2 * dev->period_size);
#else
	dev->buf_size = 2 * dev->period_size * sizeof (short);
	dev->buf = FLUID_ARRAY(short, 2 * dev->period_size);
#endif

	if (dev->buf == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		goto error_recovery;
	}

	if (filename == NULL) {
		FLUID_LOG(FLUID_ERR, "No file name specified");
		goto error_recovery;
	}

#if LIBSNDFILE_SUPPORT
	memset (&info, 0, sizeof (info));

        info.format = FLUID_FILE_RENDERER_DEFAULT_FILE_TYPE | SF_FORMAT_PCM_16;

	if (!fluid_file_renderer_parse_options (type, format, endian, filename, &info))
		goto error_recovery;

	fluid_settings_getnum (synth->settings, "synth.sample-rate", &samplerate);
	info.samplerate = samplerate + 0.5;
	info.channels = 2;

	if (!sf_format_check (&info))
	{
	  FLUID_LOG(FLUID_ERR, "Invalid or unsupported audio file format settings");
	  goto error_recovery;
	}

	dev->sndfile = sf_open (filename, SFM_WRITE, &info);

	if (!dev->sndfile)
	{
	  FLUID_LOG(FLUID_ERR, "Failed to open audio file '%s' for writing", filename);
          goto error_recovery;
	}

	/* Turn on clipping and normalization of floats (-1.0 - 1.0) */
	sf_command (dev->sndfile, SFC_SET_CLIPPING, NULL, SF_TRUE);
	sf_command (dev->sndfile, SFC_SET_NORM_FLOAT, NULL, SF_TRUE);
#else
	dev->file = fopen(filename, "wb");
	if (dev->file == NULL) {
		FLUID_LOG(FLUID_ERR, "Failed to open the file '%s'", filename);
		goto error_recovery;
	}
#endif

	return dev;

 error_recovery:
	delete_fluid_file_renderer(dev);
	return NULL;
}

/**
 * Close file and destroy a file renderer object.
 * @param dev File renderer object.
 * @since: 1.1.0
 */
void delete_fluid_file_renderer(fluid_file_renderer_t* dev)
{
	if (dev == NULL) {
		return;
	}

#if LIBSNDFILE_SUPPORT
	if (dev->sndfile != NULL) {
		int retval = sf_close (dev->sndfile);
		if (retval != 0) FLUID_LOG (FLUID_WARN, "Error closing audio file: %s",
					    sf_error_number (retval));
        }
#else
	if (dev->file != NULL) {
		fclose(dev->file);
	}
#endif

	if (dev->buf != NULL) {
		FLUID_FREE(dev->buf);
	}

	FLUID_FREE(dev);
	return;
}

#if LIBSNDFILE_SUPPORT
/**
 * Write period_size samples to file.
 * @param dev File renderer instance
 * @return FLUID_OK or FLUID_FAILED if an error occurred
 * @since: 1.1.0
 */
int
fluid_file_renderer_process_block(fluid_file_renderer_t* dev)
{
	int n;

	fluid_synth_write_float(dev->synth, dev->period_size, dev->buf, 0, 2, dev->buf, 1, 2);

	n = sf_writef_float (dev->sndfile, dev->buf, dev->period_size);

	if (n != dev->period_size) {
		FLUID_LOG (FLUID_ERR, "Audio file write error: %s",
			   sf_strerror (dev->sndfile));
		return FLUID_FAILED;
	}
	return FLUID_OK;
}
#else   /* No libsndfile support */
int
fluid_file_renderer_process_block(fluid_file_renderer_t* dev)
{
	int n, offset;

	fluid_synth_write_s16(dev->synth, dev->period_size, dev->buf, 0, 2, dev->buf, 1, 2);

	for (offset = 0; offset < dev->buf_size; offset += n) {

		n = fwrite((char*) dev->buf + offset, 1, dev->buf_size - offset, dev->file);
		if (n < 0) {
			FLUID_LOG(FLUID_ERR, "Audio output file write error: %s",
				  strerror (errno));
			return FLUID_FAILED;
		}
	}
	return FLUID_OK;
}

#endif


#if LIBSNDFILE_SUPPORT
/**
 * Get NULL terminated list of supported audio file type names.
 * @return NULL terminated list of strings which is internal and should not be
 *   modified or freed.  NOTE: May return NULL if memory allocation fails.
 */
const char **
fluid_file_renderer_get_type_names (void)
{
  static fluid_mutex_t mutex = FLUID_MUTEX_INIT;
  static const char **type_names = NULL;
  SF_FORMAT_INFO finfo;
  int major_count;
  int i, i2, index;

  fluid_mutex_lock (mutex);

  if (!type_names)
  {
    sf_command (NULL, SFC_GET_FORMAT_MAJOR_COUNT, &major_count, sizeof (int));

    type_names = FLUID_ARRAY (const char *, major_count + 1);

    if (type_names)
    {
      for (i = 0, index = 0; i < major_count; i++)
      {
        finfo.format = i;
        sf_command (NULL, SFC_GET_FORMAT_MAJOR, &finfo, sizeof (finfo));

        /* Check for duplicates */
        for (i2 = 0; i2 < index; i2++)
          if (strcmp (type_names[i2], finfo.extension) == 0)
            break;

        if (i2 < index) continue;

        type_names[index++] = finfo.extension;  /* Add name to array */
      }

      type_names[index] = NULL;
    }
    else FLUID_LOG (FLUID_ERR, "Out of memory");
  }

  fluid_mutex_unlock (mutex);

  return (const char **)type_names;
}
#else
const char **
fluid_file_renderer_get_type_names (void)
{
  return type_names;
}
#endif


const char **
fluid_file_renderer_get_format_names (void)
{
  return format_names;
}

const char **
fluid_file_renderer_get_endian_names (void)
{
  return endian_names;
}


#if LIBSNDFILE_SUPPORT

/**
 * Parse a colon separated format string and configure an SF_INFO structure accordingly.
 * @param filetype File type string (NULL or "auto" to attempt to identify format
 *   by filename extension, with fallback to "wav")
 * @param format File audio format string or NULL to use "s16"
 * @param endian File endian string or NULL to use "auto" which uses the file type's
 *   default endian byte order.
 * @param filename File name (used by "auto" type to determine type, based on extension)
 * @param info Audio file info structure to configure
 * @return TRUE on success, FALSE otherwise
 */
static int
fluid_file_renderer_parse_options (char *filetype, char *format, char *endian,
                                   char *filename, SF_INFO *info)
{
  int type = -1;        /* -1 indicates "auto" type */
  char *s;
  int i;

  /* If "auto" type, then use extension to search for a match */
  if (!filetype || FLUID_STRCMP (filetype, "auto") == 0)
  {
    type = FLUID_FILE_RENDERER_DEFAULT_FILE_TYPE;
    s = FLUID_STRRCHR (filename, '.');

    if (s && s[1] != '\0')
    {
      if (!fluid_file_renderer_find_file_type (s + 1, &type))
        FLUID_LOG (FLUID_WARN, "Failed to determine audio file type from filename, defaulting to WAV");
    }
  }
  else if (!fluid_file_renderer_find_file_type (filetype, &type))
  {
    FLUID_LOG(FLUID_ERR, "Invalid or unsupported audio file type '%s'", filetype);
    return FALSE;
  }

  info->format = (info->format & ~SF_FORMAT_TYPEMASK) | type;

  /* Look for subtype */
  if (format)
  {
    for (i = 0; format_names[i]; i++)
      if (FLUID_STRCMP (format, format_names[i]) == 0)
        break;

    if (!format_names[i])
    {
      FLUID_LOG (FLUID_ERR, "Invalid or unsupported file audio format '%s'", format);
      return FALSE;
    }

    info->format = (info->format & ~SF_FORMAT_SUBMASK) | format_ids[i];
  }

  /* Look for endian */
  if (endian)
  {
    for (i = 0; endian_names[i]; i++)
      if (FLUID_STRCMP (endian, endian_names[i]) == 0)
        break;

    if (!endian_names[i])
    {
      FLUID_LOG (FLUID_ERR, "Invalid or unsupported endian byte order '%s'", endian);
      return FALSE;
    }

    info->format = (info->format & ~SF_FORMAT_ENDMASK) | endian_ids[i];
  }

  return TRUE;
}

/**
 * Searches for a supported libsndfile file type by extension.
 * @param extension The extension string
 * @param ext_len Length of the extension string
 * @param type Location to store the type (unmodified if not found)
 * @return TRUE if found, FALSE otherwise
 */
static int
fluid_file_renderer_find_file_type (char *extension, int *type)
{
  SF_FORMAT_INFO finfo;
  int major_count;
  int i;

  sf_command (NULL, SFC_GET_FORMAT_MAJOR_COUNT, &major_count, sizeof (int));

  for (i = 0; i < major_count; i++)
  {
    finfo.format = i;
    sf_command (NULL, SFC_GET_FORMAT_MAJOR, &finfo, sizeof (finfo));

    if (FLUID_STRCMP (extension, finfo.extension) == 0)
      break;
  }

  if (i < major_count)
  {
    *type = finfo.format;
    return TRUE;
  }

  return FALSE;
}

#endif
