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

struct _fluid_file_renderer_t {
	FILE* file;
	fluid_synth_t* synth;
	short* buf;
	int period_size;
	int buf_size;
};

void delete_fluid_file_renderer(fluid_file_renderer_t* dev)
{
	if (dev == NULL) {
		return;
	}

	if (dev->file != NULL) {
		fclose(dev->file);
	}

	if (dev->buf != NULL) {
		FLUID_FREE(dev->buf);
	}

	FLUID_FREE(dev);
	return;
}


/* 
 * Create a new file renderer object and open the file. 
 */ 

fluid_file_renderer_t* new_fluid_file_renderer(fluid_synth_t* synth, char* filename, int period_size)
{
	fluid_file_renderer_t* dev;

	dev = FLUID_NEW(fluid_file_renderer_t);
	if (dev == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	FLUID_MEMSET(dev, 0, sizeof(fluid_file_renderer_t));

	dev->synth = synth;
	dev->period_size = period_size;
	dev->buf_size = 2 * dev->period_size * sizeof(short);

	dev->buf = FLUID_ARRAY(short, 2 * dev->period_size);
	if (dev->buf == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		goto error_recovery;
	}

	if (filename == NULL) {
		FLUID_LOG(FLUID_ERR, "No file name specified");
		goto error_recovery;
	}

	dev->file = fopen(filename, "wb");
	if (dev->file == NULL) {
		FLUID_LOG(FLUID_ERR, "Failed to open the file '%s'", filename);
		goto error_recovery;
	}

	return dev;

 error_recovery:
	delete_fluid_file_renderer(dev);
	return NULL;
	
}

/*
 * Write period_size samples to file.
 */
int fluid_file_renderer_process_block(fluid_file_renderer_t* dev)
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

struct _fluid_file_renderer_t {
	FILE* file;
	fluid_synth_t* synth;
	short* buf;
	int period_size;
	int buf_size;
};

void delete_fluid_file_renderer(fluid_file_renderer_t* dev)
{
	if (dev == NULL) {
		return;
	}

	if (dev->file != NULL) {
		fclose(dev->file);
	}

	if (dev->buf != NULL) {
		FLUID_FREE(dev->buf);
	}

	FLUID_FREE(dev);
	return;
}


/* 
 * Create a new file renderer object and open the file. 
 */ 

fluid_file_renderer_t* new_fluid_file_renderer(fluid_synth_t* synth, char* filename, int period_size)
{
	fluid_file_renderer_t* dev;

	dev = FLUID_NEW(fluid_file_renderer_t);
	if (dev == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	FLUID_MEMSET(dev, 0, sizeof(fluid_file_renderer_t));

	dev->synth = synth;
	dev->period_size = period_size;
	dev->buf_size = 2 * dev->period_size * sizeof(short);

	dev->buf = FLUID_ARRAY(short, 2 * dev->period_size);
	if (dev->buf == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		goto error_recovery;
	}

	if (filename == NULL) {
		FLUID_LOG(FLUID_ERR, "No file name specified");
		goto error_recovery;
	}

	dev->file = fopen(filename, "wb");
	if (dev->file == NULL) {
		FLUID_LOG(FLUID_ERR, "Failed to open the file '%s'", filename);
		goto error_recovery;
	}

	return dev;

 error_recovery:
	delete_fluid_file_renderer(dev);
	return NULL;
	
}

/*
 * Write period_size samples to file.
 */
int fluid_file_renderer_process_block(fluid_file_renderer_t* dev)
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

struct _fluid_file_renderer_t {
	FILE* file;
	fluid_synth_t* synth;
	short* buf;
	int period_size;
	int buf_size;
};

void delete_fluid_file_renderer(fluid_file_renderer_t* dev)
{
	if (dev == NULL) {
		return;
	}

	if (dev->file != NULL) {
		fclose(dev->file);
	}

	if (dev->buf != NULL) {
		FLUID_FREE(dev->buf);
	}

	FLUID_FREE(dev);
	return;
}


/* 
 * Create a new file renderer object and open the file. 
 */ 

fluid_file_renderer_t* new_fluid_file_renderer(fluid_synth_t* synth, char* filename, int period_size)
{
	fluid_file_renderer_t* dev;

	dev = FLUID_NEW(fluid_file_renderer_t);
	if (dev == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	FLUID_MEMSET(dev, 0, sizeof(fluid_file_renderer_t));

	dev->synth = synth;
	dev->period_size = period_size;
	dev->buf_size = 2 * dev->period_size * sizeof(short);

	dev->buf = FLUID_ARRAY(short, 2 * dev->period_size);
	if (dev->buf == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		goto error_recovery;
	}

	if (filename == NULL) {
		FLUID_LOG(FLUID_ERR, "No file name specified");
		goto error_recovery;
	}

	dev->file = fopen(filename, "wb");
	if (dev->file == NULL) {
		FLUID_LOG(FLUID_ERR, "Failed to open the file '%s'", filename);
		goto error_recovery;
	}

	return dev;

 error_recovery:
	delete_fluid_file_renderer(dev);
	return NULL;
	
}

/*
 * Write period_size samples to file.
 */
int fluid_file_renderer_process_block(fluid_file_renderer_t* dev)
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

