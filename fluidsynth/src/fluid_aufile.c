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

/* fluid_aufile.c
 *
 * Audio driver, outputs the audio to a file (non real-time)
 *
 */

#include "fluid_adriver.h"
#include "fluid_settings.h"
#include "fluid_sys.h"
#include <stdio.h>


/** fluid_file_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct {
	fluid_audio_driver_t driver;
	fluid_audio_func_t callback;
	void* data;
	int period_size;
	double sample_rate;
	FILE* file;
	fluid_timer_t* timer;
	float* left;
	float* right;
	short* buf;
	int buf_size;
	unsigned int samples;
} fluid_file_audio_driver_t;


fluid_audio_driver_t* new_fluid_file_audio_driver(fluid_settings_t* settings,
						  fluid_synth_t* synth);

int delete_fluid_file_audio_driver(fluid_audio_driver_t* p);
void fluid_file_audio_driver_settings(fluid_settings_t* settings);
static int fluid_file_audio_run_s16(void* d, unsigned int msec);

/**************************************************************
 *
 *        'file' audio driver
 *
 */

void fluid_file_audio_driver_settings(fluid_settings_t* settings)
{
	fluid_settings_register_str(settings, "audio.file.name", "fluidsynth.raw", 0, NULL, NULL);
}


fluid_audio_driver_t*
new_fluid_file_audio_driver(fluid_settings_t* settings,
			    fluid_synth_t* synth)
{
	fluid_file_audio_driver_t* dev;
	int err;
	char* filename;
	int msec;

	dev = FLUID_NEW(fluid_file_audio_driver_t);
	if (dev == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	FLUID_MEMSET(dev, 0, sizeof(fluid_file_audio_driver_t));

	fluid_settings_getint(settings, "audio.period-size", &dev->period_size);
	fluid_settings_getnum(settings, "synth.sample-rate", &dev->sample_rate);

	dev->data = synth;
	dev->callback = (fluid_audio_func_t) fluid_synth_process;
	dev->samples = 0;
	dev->left = FLUID_ARRAY(float, dev->period_size);
	dev->right = FLUID_ARRAY(float, dev->period_size);
	dev->buf = FLUID_ARRAY(short, 2 * dev->period_size);
	dev->buf_size = 2 * dev->period_size * sizeof(short);

	if (fluid_settings_getstr(settings, "audio.file.name", &filename) == 0) {
		FLUID_LOG(FLUID_ERR, "No file name specified");
		goto error_recovery;
	}

	dev->file = fopen(filename, "wb");
	if (dev->file == NULL) {
		FLUID_LOG(FLUID_ERR, "Failed to open the file '%s'", filename);
		goto error_recovery;
	}

	msec = (int) (0.5 + dev->period_size / dev->sample_rate * 1000.0);
	dev->timer = new_fluid_timer(msec, fluid_file_audio_run_s16, (void*) dev, 1, 0);
	if (dev->timer == NULL) {
		FLUID_LOG(FLUID_PANIC, "Couldn't create the audio thread.");
		goto error_recovery;
	}

	return (fluid_audio_driver_t*) dev;

 error_recovery:
	delete_fluid_file_audio_driver((fluid_audio_driver_t*) dev);
	return NULL;
}

int delete_fluid_file_audio_driver(fluid_audio_driver_t* p)
{
	fluid_file_audio_driver_t* dev = (fluid_file_audio_driver_t*) p;

	if (dev == NULL) {
		return FLUID_OK;
	}

	if (dev->timer != NULL) {
		delete_fluid_timer(dev->timer);
	}

	if (dev->file != NULL) {
		fclose(dev->file);
	}

	if (dev->left != NULL) {
		FLUID_FREE(dev->left);
	}

	if (dev->right != NULL) {
		FLUID_FREE(dev->right);
	}

	if (dev->buf != NULL) {
		FLUID_FREE(dev->buf);
	}

	FLUID_FREE(dev);
	return FLUID_OK;
}

static int fluid_file_audio_run_s16(void* d, unsigned int clock_time)
{
	fluid_file_audio_driver_t* dev = (fluid_file_audio_driver_t*) d;
	int n, offset;
	unsigned int sample_time;

	sample_time = (unsigned int) (dev->samples / dev->sample_rate * 1000.0);
	if (sample_time > clock_time) {
		return 1;
	}

	fluid_synth_write_s16(dev->data, dev->period_size, dev->buf, 0, 2, dev->buf, 1, 2);

	for (offset = 0; offset < dev->buf_size; offset += n) {

		n = fwrite((char*) dev->buf + offset, 1, dev->buf_size - offset, dev->file);
		if (n < 0) {
			FLUID_LOG(FLUID_ERR, "Audio output file write error: %s",
				  strerror (errno));
			return 0;
		}
	}

	dev->samples += dev->period_size;

	return 1;
}
