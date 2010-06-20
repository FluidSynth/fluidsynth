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


#ifndef _FLUID_VOICE_H
#define _FLUID_VOICE_H

#include "fluid_phase.h"
#include "fluid_gen.h"
#include "fluid_mod.h"
#include "fluid_iir_filter.h"
#include "fluid_adsr_env.h"
#include "fluid_lfo.h"

#define NO_CHANNEL             0xff

enum fluid_voice_status
{
	FLUID_VOICE_CLEAN,
	FLUID_VOICE_ON,
	FLUID_VOICE_SUSTAINED,
	FLUID_VOICE_OFF
};


/*
 * fluid_voice_t
 */
struct _fluid_voice_t
{
	unsigned int id;                /* the id is incremented for every new noteon.
					   it's used for noteoff's  */
	unsigned char status;
	unsigned char chan;             /* the channel number, quick access for channel messages */
	unsigned char key;              /* the key, quick acces for noteoff */
	unsigned char vel;              /* the velocity */
	fluid_channel_t* channel;
	fluid_gen_t gen[GEN_LAST];
	fluid_mod_t mod[FLUID_NUM_MOD];
	int mod_count;
	int has_looped;                 /* Flag that is set as soon as the first loop is completed. */
	fluid_sample_t* sample;
	int check_sample_sanity_flag;   /* Flag that initiates, that sample-related parameters
					   have to be checked. */
#if 0
	/* Instead of keeping a pointer to a fluid_sample_t structure,
	 * I think it would be better to copy the sample data in the
	 * voice structure. SoundFont loader then do not have to
	 * allocate and maintain the fluid_sample_t structure. [PH]
	 *
	 * The notify callback may be used also for streaming samples.
	 */
	short* sample_data;             /* pointer to the sample data */
	int sample_data_offset;         /* the offset of data[0] in the whole sample */
	int sample_data_length;         /* the length of the data array */
	unsigned int sample_start;
	unsigned int sample_end;
	unsigned int sample_loopstart;
	unsigned int sample_loopend;
	unsigned int sample_rate;
	int sample_origpitch;
	int sample_pitchadj;
	int sample_type;
	int (*sample_notify)(fluid_voice_t* voice, int reason);
	void* sample_userdata;
#endif

	/* basic parameters */
	fluid_real_t output_rate;        /* the sample rate of the synthesizer */

	unsigned int start_time;
	unsigned int ticks;
	unsigned int noteoff_ticks;      /* Delay note-off until this tick */

	fluid_real_t amp;                /* current linear amplitude */
	fluid_phase_t phase;             /* the phase of the sample wave */

	/* Temporary variables used in fluid_voice_write() */

	fluid_real_t phase_incr;	/* the phase increment for the next 64 samples */
	fluid_real_t amp_incr;		/* amplitude increment value */
	fluid_real_t *dsp_buf;		/* buffer to store interpolated sample data to */
	int dsp_buf_count;              /* Number of audio samples in dsp_buf */

	/* End temporary variables */

	/* basic parameters */
	fluid_real_t pitch;              /* the pitch in midicents */
	fluid_real_t attenuation;        /* the attenuation in centibels */
	fluid_real_t min_attenuation_cB; /* Estimate on the smallest possible attenuation
					  * during the lifetime of the voice */
	fluid_real_t root_pitch, root_pitch_hz;
	

	/* sample and loop start and end points (offset in sample memory).  */
	int start;
	int end;
	int loopstart;
	int loopend;	/* Note: first point following the loop (superimposed on loopstart) */

	/* master gain */
	fluid_real_t synth_gain;

	/* vol env */
        fluid_adsr_env_t volenv;
	fluid_real_t amplitude_that_reaches_noise_floor_nonloop;
	fluid_real_t amplitude_that_reaches_noise_floor_loop;

	/* mod env */
        fluid_adsr_env_t modenv;
	fluid_real_t modenv_to_fc;
	fluid_real_t modenv_to_pitch;

	/* mod lfo */
        fluid_lfo_t modlfo;
	fluid_real_t modlfo_to_fc;
	fluid_real_t modlfo_to_pitch;
	fluid_real_t modlfo_to_vol;

	/* vib lfo */
        fluid_lfo_t viblfo;
	fluid_real_t viblfo_to_pitch;

	fluid_iir_filter_t resonant_filter; /* IIR resonance dsp filter */

	/* pan */
	fluid_real_t pan;
	fluid_real_t amp_left;
	fluid_real_t amp_right;

	/* reverb */
	fluid_real_t reverb_send;
	fluid_real_t amp_reverb;

	/* chorus */
	fluid_real_t chorus_send;
	fluid_real_t amp_chorus;

	/* interpolation method, as in fluid_interp in fluidsynth.h */
	int interp_method;

	/* for debugging */
	int debug;
	double ref;
};


fluid_voice_t* new_fluid_voice(fluid_real_t output_rate);
int delete_fluid_voice(fluid_voice_t* voice);

void fluid_voice_start(fluid_voice_t* voice);
void  fluid_voice_calculate_gen_pitch(fluid_voice_t* voice);

int fluid_voice_write (fluid_voice_t* voice, fluid_real_t *dsp_buf);

int fluid_voice_init(fluid_voice_t* voice, fluid_sample_t* sample,
		     fluid_channel_t* channel, int key, int vel,
		     unsigned int id, unsigned int time, fluid_real_t gain);

int fluid_voice_modulate(fluid_voice_t* voice, int cc, int ctrl);
int fluid_voice_modulate_all(fluid_voice_t* voice);

/** Set the NRPN value of a generator. */
int fluid_voice_set_param(fluid_voice_t* voice, int gen, fluid_real_t value, int abs);


/** Set the gain. */
int fluid_voice_set_gain(fluid_voice_t* voice, fluid_real_t gain);


/** Update all the synthesis parameters, which depend on generator
    'gen'. This is only necessary after changing a generator of an
    already operating voice.  Most applications will not need this
    function.*/
void fluid_voice_update_param(fluid_voice_t* voice, int gen);

int fluid_voice_noteoff(fluid_voice_t* voice);
int fluid_voice_off(fluid_voice_t* voice);
void fluid_voice_mix (fluid_voice_t *voice,
                      fluid_real_t* left_buf, fluid_real_t* right_buf,
                      fluid_real_t* reverb_buf, fluid_real_t* chorus_buf);
int fluid_voice_kill_excl(fluid_voice_t* voice);

#define fluid_voice_get_channel(voice)  ((voice)->channel)


#define fluid_voice_set_id(_voice, _id)  { (_voice)->id = (_id); }
#define fluid_voice_get_chan(_voice)     (_voice)->chan


#define _PLAYING(voice)  (((voice)->status == FLUID_VOICE_ON) || ((voice)->status == FLUID_VOICE_SUSTAINED))

/* A voice is 'ON', if it has not yet received a noteoff
 * event. Sending a noteoff event will advance the envelopes to
 * section 5 (release). */
#define _ON(voice)  ((voice)->status == FLUID_VOICE_ON && fluid_adsr_env_get_section(&voice->volenv) < FLUID_VOICE_ENVRELEASE)
#define _SUSTAINED(voice)  ((voice)->status == FLUID_VOICE_SUSTAINED)
#define _AVAILABLE(voice)  (((voice)->status == FLUID_VOICE_CLEAN) || ((voice)->status == FLUID_VOICE_OFF))
#define _RELEASED(voice)  ((voice)->chan == NO_CHANNEL)
#define _SAMPLEMODE(voice) ((int)(voice)->gen[GEN_SAMPLEMODE].val)


/* FIXME - This doesn't seem to be used anywhere - JG */
fluid_real_t fluid_voice_gen_value(fluid_voice_t* voice, int num);

#define fluid_voice_get_loudness(voice) (fluid_adsr_env_get_max_val(&voice->volenv))

#define _GEN(_voice, _n) \
  ((fluid_real_t)(_voice)->gen[_n].val \
   + (fluid_real_t)(_voice)->gen[_n].mod \
   + (fluid_real_t)(_voice)->gen[_n].nrpn)

#define FLUID_SAMPLESANITY_CHECK (1 << 0)
#define FLUID_SAMPLESANITY_STARTUP (1 << 1)


/* defined in fluid_dsp_float.c */

void fluid_dsp_float_config (void);
int fluid_dsp_float_interpolate_none (fluid_voice_t *voice);
int fluid_dsp_float_interpolate_linear (fluid_voice_t *voice);
int fluid_dsp_float_interpolate_4th_order (fluid_voice_t *voice);
int fluid_dsp_float_interpolate_7th_order (fluid_voice_t *voice);

#endif /* _FLUID_VOICE_H */
