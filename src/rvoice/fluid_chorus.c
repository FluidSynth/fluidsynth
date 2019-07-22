/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe, Markus Nentwig and others.
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

/*
  based on a chrous implementation made by Juergen Mueller And Sundry Contributors in 1998

  CHANGES

  - Adapted for fluidsynth, Peter Hanappe, March 2002

  - Variable delay line implementation using bandlimited
    interpolation, code reorganization: Markus Nentwig May 2002

 */


/*
 * 	Chorus effect.
 *
 * Flow diagram scheme for n delays ( 1 <= n <= MAX_CHORUS ):
 *
 *        * gain-in                                           ___
 * ibuff -----+--------------------------------------------->|   |
 *            |      _________                               |   |
 *            |     |         |                   * level 1  |   |
 *            +---->| delay 1 |----------------------------->|   |
 *            |     |_________|                              |   |
 *            |        /|\                                   |   |
 *            :         |                                    |   |
 *            : +-----------------+   +--------------+       | + |
 *            : | Delay control 1 |<--| mod. speed 1 |       |   |
 *            : +-----------------+   +--------------+       |   |
 *            |      _________                               |   |
 *            |     |         |                   * level n  |   |
 *            +---->| delay n |----------------------------->|   |
 *                  |_________|                              |   |
 *                     /|\                                   |___|
 *                      |                                      |
 *              +-----------------+   +--------------+         | * gain-out
 *              | Delay control n |<--| mod. speed n |         |
 *              +-----------------+   +--------------+         +----->obuff
 *
 *
 * The delay i is controlled by a sine or triangle modulation i ( 1 <= i <= n).
 *
 * The delay of each block is modulated between 0..depth ms
 *
 */


/* Variable delay line implementation
 * ==================================
 *
 * The modulated delay needs the value of the delayed signal between
 * samples.  A lowpass filter is used to obtain intermediate values
 * between samples (bandlimited interpolation).  The sample pulse
 * train is convoluted with the impulse response of the low pass
 * filter (sinc function).  To make it work with a small number of
 * samples, the sinc function is windowed (Hamming window).
 *
 */

#include "fluid_chorus.h"
#include "fluid_sys.h"

#define MAX_CHORUS	99
#define MAX_DELAY	100
#define MAX_DEPTH	10
#define MIN_SPEED_HZ	0.29
#define MAX_SPEED_HZ    5
/*-------------------------------------------------------------------------------------
Configuration macros:

NEW_MOD: when defined, uses new LFO modulators:
 - these modulators are computed on the fly, instead of using lfo lookup table.
 - The advantages are:
   - Avoiding a lost of 608272 memory bytes when lfo speed is low (0.3Hz).
   - Allows to diminish the lfo speed lower limit to 0.1Hz instead of 0.3Hz.
     A speed of 0.1 is interresting for chorus. Using a lookuptable for 0.1Hz
     would require too much memory (1824816 bytes).
   - Make use of first order all-pass interpolator instead of bandlimited interpolation.

NEW_MOD: when not defined:
   - use memory lfo lookup table.
   - use bandlimited interpolation with sinc lookup table.
--------------------------------------------------------------------------------------*/
#define NEW_MOD
#define PRINT // allows message to be printed on the console.

/*-------------------------------------------------------------------------------------
  Private
--------------------------------------------------------------------------------------*/
#ifdef NEW_MOD
#define MAX_SAMPLES 2048 /* delay lenght in sample (46.4 ms at sample rate: 44100Hz).*/
#define LOW_MOD_DEPTH 176             /* low mod_depth/2 in samples */
#define HIGH_MOD_DEPTH  MAX_SAMPLES/2 /* high mod_depth in sample */
#define RANGE_MOD_DEPTH (HIGH_MOD_DEPTH - LOW_MOD_DEPTH)

/* MOD_RATE acceptable for high speed (5Hz)*/
#define LOW_MOD_RATE 5  /* MOD_RATE acceptable for low modulation depth (8 ms) */
#define HIGH_MOD_RATE 4 /* MOD_RATE acceptable for high modulation depth (46.6 ms) */
#define RANGE_MOD_RATE (HIGH_MOD_RATE - LOW_MOD_RATE)

/* some chorus cpu_load measurement dependant of modulation rate: mod_rate
 mod_rate | chorus cpu load(%) | one voice cpu load (%)
 ----------------------------------------------------
 50       | 0.204              |
 5        | 0.256              |  0.169
 1        | 0.417              |
*/

/*
 Number of samples to add to the desired length of a delay line. This
 allow to take account of large modulation interpolation.
 1 is sufficient with DEPTH ms maximum (46.6 ms).
*/
//#define INTERP_SAMPLES_NBR 1
#define INTERP_SAMPLES_NBR 0

#else
/* chorus cpu load(%):0.343 */

/* Length of one delay line in samples:
 * Set through MAX_SAMPLES_LN2.
 * For example:
 * MAX_SAMPLES_LN2=12
 * => MAX_SAMPLES=pow(2,12-1)=2048
 * => MAX_SAMPLES_ANDMASK=2047
 */
#define MAX_SAMPLES_LN2 12

#define MAX_SAMPLES (1 << (MAX_SAMPLES_LN2-1))
#define MAX_SAMPLES_ANDMASK (MAX_SAMPLES-1)


/* Interpolate how many steps between samples? Must be power of two
   For example: 8 => use a resolution of 256 steps between any two
   samples
*/
#define INTERPOLATION_SUBSAMPLES_LN2 8
#define INTERPOLATION_SUBSAMPLES (1 << (INTERPOLATION_SUBSAMPLES_LN2-1))
#define INTERPOLATION_SUBSAMPLES_ANDMASK (INTERPOLATION_SUBSAMPLES-1)

/* Use how many samples for interpolation? Must be odd.  '7' sounds
   relatively clean, when listening to the modulated delay signal
   alone.  For a demo on aliasing try '1' With '3', the aliasing is
   still quite pronounced for some input frequencies
*/
#define INTERPOLATION_SAMPLES 5
#endif


#ifdef NEW_MOD
/*-----------------------------------------------------------------------------
 Sinusoidal modulator
-----------------------------------------------------------------------------*/
/* modulator */
typedef struct
{
    fluid_real_t   a1;          /* Coefficient: a1 = 2 * cos(w) */
    fluid_real_t   buffer1;     /* buffer1 */
    fluid_real_t   buffer2;     /* buffer2 */
    fluid_real_t   reset_buffer2;/* reset value of buffer2 */
} sinus_modulator;

/*-----------------------------------------------------------------------------
 Triangle modulator
-----------------------------------------------------------------------------*/
typedef struct
{
    fluid_real_t   freq;       /* Osc. Frequency (in Hertz) */
    fluid_real_t   val;         /* internal current value */
    fluid_real_t   inc;         /* increment value */
}triang_modulator;

/*-----------------------------------------------------------------------------
 modulator
-----------------------------------------------------------------------------*/
typedef struct
{
    /*-------------*/
    int line_out; /* current line out position for this modulator */
    /*-------------*/
    sinus_modulator sinus; /* sinus lfo */
    triang_modulator triang; /* triangle lfo */
    /*-------------------------*/
    /* first order All-Pass interpolator members */
    fluid_real_t  frac_pos_mod; /* fractional position part between samples */
    /* previous value used when interpolating using fractional */
    fluid_real_t  buffer;
}modulator;

#endif

/* Private data for SKEL file */
struct _fluid_chorus_t
{
    int type;
    fluid_real_t depth_ms;
    fluid_real_t level;
    fluid_real_t speed_Hz;
    int number_blocks;
    fluid_real_t sample_rate;

#ifdef NEW_MOD
    fluid_real_t *line; /* buffer line */
    int   size;    /* effective internal size (in samples) */
    /*-------------*/
    int line_in;  /* line in position */
//    int line_out; /* line out position */
    /*-------------------------*/
    /* center output position members */
    fluid_real_t  center_pos_mod; /* center output position modulated by modulator */
    int          mod_depth;   /* modulation depth (in samples) */
    /*-------------------------*/
    /* variable rate control of center output position */
    int index_rate;  /* index rate to know when to update center_pos_mod */
    int mod_rate;    /* rate at which center_pos_mod is updated */
    /*-------------------------*/
    /* modulator member */
    modulator mod[MAX_CHORUS]; /* sinus/triangle modulator */
#else
    fluid_real_t *chorusbuf;
    int counter;
    long phase[MAX_CHORUS];
    long modulation_period_samples;
    int *lookup_tab;
    /* sinc lookup table */
    fluid_real_t sinc_table[INTERPOLATION_SAMPLES][INTERPOLATION_SUBSAMPLES];
#endif
};

#ifdef NEW_MOD
/*-----------------------------------------------------------------------------
 Sets the frequency of sinus oscillator.

 @param mod pointer on modulator structure.
 @param freq frequency of the oscillator in Hz.
 @param sample_rate sample rate on audio output in Hz.
 @param phase initial phase of the oscillator in degree (0 to 360).
-----------------------------------------------------------------------------*/
static void set_sinus_frequency(sinus_modulator *mod,
                              float freq, float sample_rate, float phase)
{
    fluid_real_t w = 2 * FLUID_M_PI * freq / sample_rate; /* intial angle */
    fluid_real_t a;

    mod->a1 = 2 * FLUID_COS(w);

    a = (2 * FLUID_M_PI / 360) * phase;

    mod->buffer2 = FLUID_SIN(a - w); /* y(n-1) = sin(-intial angle) */
    mod->buffer1 = FLUID_SIN(a); /* y(n) = sin(initial phase) */
    mod->reset_buffer2 = FLUID_SIN(FLUID_M_PI / 2 - w); /* reset value for PI/2 */
}

/*-----------------------------------------------------------------------------
 Gets current value of sinus modulator:
   y(n) = a1 . y(n-1)  -  y(n-2)
   out = a1 . buffer1  -  buffer2

 @param pointer on modulator structure.
 @return current value of the modulator sine wave.
-----------------------------------------------------------------------------*/
static FLUID_INLINE fluid_real_t get_mod_sinus(sinus_modulator *mod)
{
    fluid_real_t out;
    out = mod->a1 * mod->buffer1 - mod->buffer2;
    mod->buffer2 = mod->buffer1;

    if(out >= 1.0f) /* reset in case of instability near PI/2 */
    {
        out = 1.0f; /* forces output to the right value */
        mod->buffer2 = mod->reset_buffer2;
    }

    if(out <= -1.0f) /* reset in case of instability near -PI/2 */
    {
        out = -1.0f; /* forces output to the right value */
        mod->buffer2 = - mod->reset_buffer2;
    }

    mod->buffer1 = out;
    return  out;
}

/*-----------------------------------------------------------------------------
 Set the frequency of triangular oscillator
 The frequency is converted in a slope value.
 The initial value is set according to frac_phase which is a position
 in the period relative to the beginning of the period.
 For example: 0 is the beginning of the period, 1/4 is at 1/4 of the period
 relative to the beginning.
-----------------------------------------------------------------------------*/
static void set_triangle_frequency(triang_modulator * mod, float freq,
                              float sample_rate, float frac_phase)
{
	fluid_real_t ns_period; /* period in numbers of sample */
	if (freq <= 0.0) freq = 0.5f;
	mod->freq = freq;
	/* period = ns_period * T
	   ns_period = period / T ; ns_period = sampleRate / Freq
	*/
	ns_period = sample_rate / freq;

	/* the slope of a triangular osc (0 up to +1 down to -1 up to 0....) is equivalent
	to the slope of a saw osc (0 -> +4) */
	mod->inc  = 4 / ns_period; /* positive slope */

	/* The initial value and the sign of the slope depend of initial the phase:
	  intial value = = (ns_period * frac_phase) * slope
	*/
	mod->val =  ns_period * frac_phase * mod->inc;
	if ( 1.0 <= mod->val && mod->val < 3.0)
	{
		mod->val = 2.0 - mod->val; /*  1.0 down to -1.0 */
		mod->inc = -mod->inc; /* negative slope */
	}
	else if (3.0 <= mod->val)
	{
		mod->val = mod->val - 4.0; /*  -1.0 up to +1.0. */
	}
	/* else val < 1.0 */
}

/*-----------------------------------------------------------------------------
   Get current value of triangular oscillator
       y(n) = y(n-1) + dy
-----------------------------------------------------------------------------*/
static FLUID_INLINE fluid_real_t get_mod_triang(triang_modulator * mod)
{
	mod->val = mod->val + mod->inc ;
	if( mod->val >= 1.0)
	{
		mod->inc = -mod->inc;
		return 1.0;
	}
	if( mod->val <= -1.0)
	{
		mod->inc = -mod->inc;
		return -1.0;
	}
    return  mod->val;
}
/*-----------------------------------------------------------------------------
 Reads the sample value out of the modulated delay line.
 @param mdl, pointer on modulated delay line.
 @return the sample value.
-----------------------------------------------------------------------------*/
static FLUID_INLINE fluid_real_t get_mod_delay(fluid_chorus_t *chorus,
                                               modulator *mod)
{
    fluid_real_t out_index;  /* new modulated index position */
    int int_out_index; /* integer part of out_index */
    fluid_real_t out; /* value to return */

    /* Checks if the modulator must be updated (every mod_rate samples). */
    /* Important: center_pos_mod must be used immediatly for the
       first sample. So, mdl->index_rate must be initialized
       to mdl->mod_rate (set_mod_delay_line())  */

//    if(++chorus->index_rate >= chorus->mod_rate)
    if(chorus->index_rate >= chorus->mod_rate)
    {
//        chorus->index_rate = 0;

        /* out_index = center position (center_pos_mod) + sinus waweform */
        if(chorus->type == FLUID_CHORUS_MOD_SINE)
        {
            out_index = chorus->center_pos_mod +
                        get_mod_sinus(&mod->sinus) * chorus->mod_depth;
        }
        else
        {
            out_index = chorus->center_pos_mod +
                        get_mod_triang(&mod->triang) * chorus->mod_depth;
        }
        /* extracts integer part in int_out_index */
        if(out_index >= 0.0f)
        {
            int_out_index = (int)out_index; /* current integer part */

            /* forces read index (line_out)  with integer modulation value  */
            /* Boundary check and circular motion as needed */
            if((mod->line_out = int_out_index) >= chorus->size)
            {
                mod->line_out -= chorus->size;
            }
        }
        else /* negative */
        {
            int_out_index = (int)(out_index - 1); /* previous integer part */
            /* forces read index (line_out) with integer modulation value  */
            /* circular motion as needed */
            mod->line_out   = int_out_index + chorus->size;
        }

        /* extracts fractionnal part. (it will be used when interpolating
          between line_out and line_out +1) and memorize it.
          Memorizing is necessary for modulation rate above 1 */
        mod->frac_pos_mod = out_index - int_out_index;
#if 0
        /* updates center position (center_pos_mod) to the next position
           specified by modulation rate */
        if((chorus->center_pos_mod += chorus->mod_rate) >= chorus->size)
        {
            chorus->center_pos_mod -= chorus->size;
        }
#endif
    }

    /*  First order all-pass interpolation ----------------------------------*/
    /* https://ccrma.stanford.edu/~jos/pasp/First_Order_Allpass_Interpolation.html */
    /*  begins interpolation: read current sample */
    out = chorus->line[mod->line_out];

    /* updates line_out to the next sample.
       Boundary check and circular motion as needed */
    if(++mod->line_out >= chorus->size)
    {
        mod->line_out -= chorus->size;
    }

    /* Fractional interpolation beetween next sample (at next position) and
       previous output added to current sample.
    */
    out += mod->frac_pos_mod * (chorus->line[mod->line_out] - mod->buffer);
    mod->buffer = out; /* memorizes current output */
    return out;
}

/*-----------------------------------------------------------------------------
 Push a sample val into the delay line
-----------------------------------------------------------------------------*/
#define push_in_delay_line(dl, val) \
{\
    dl->line[dl->line_in] = val;\
    /* Incrementation and circular motion if necessary */\
    if(++dl->line_in >= dl->size) dl->line_in -= dl->size;\
}\

/*-----------------------------------------------------------------------------
 Initializes the modulated center position (center_pos_mod) so that:
 the delay between center_pos_mod and line_in is:
          mod_depth + INTERP_SAMPLES_NBR.
-----------------------------------------------------------------------------*/
static void set_center_position(fluid_chorus_t *chorus)
{
    int center;
#if 1
	/* Sets the modulation rate. This rate defines how often
     the  center position (center_pos_mod ) is modulated .
     The value is expressed in samples. The default value is 1 that means that
     center_pos_mod is updated at every sample.
     For example with a value of 2, the center position position will be
     updated only one time every 2 samples only.
    */
    chorus->mod_rate = LOW_MOD_RATE; /* default modulation rate */
    /* compensate mod rate for high modulation depth */
    if(chorus->mod_depth > LOW_MOD_DEPTH)
	{
        int delta_mod_depth = (chorus->mod_depth - LOW_MOD_DEPTH);
		chorus->mod_rate += (delta_mod_depth * RANGE_MOD_RATE)/RANGE_MOD_DEPTH;
	}
#endif
    /* Initializes the modulated center position (center_pos_mod) so that:
        - the delay between center_pos_mod and line_in is:
          mod_depth + INTERP_SAMPLES_NBR.
    */
    center = chorus->line_in - (INTERP_SAMPLES_NBR + chorus->mod_depth);
    if(center < 0)
    {
        center += chorus->size;
    }
    chorus->center_pos_mod = (fluid_real_t)center;

    /* index rate to control when to update center_pos_mod */
    /* Important: must be set to get center_pos_mod immediatly used for the
       reading of first sample (see get_mod_delay()) */
    chorus->index_rate = chorus->mod_rate;
}

/*-----------------------------------------------------------------------------
 Modulated delay line initialization.

 Sets the length line ( alloc delay samples).
 Remark: the function sets the internal size accordling to the length delay_length.
 The size is augmented by INTERP_SAMPLES_NBR to take account of interpolation.

 @param chorus, pointer chorus unit.
 @param delay_length the length of the delay line in samples.
 @param mod_rate the rate of the modulation in samples.
 @return FLUID_OK if success , FLUID_FAILED if memory error.

 Return FLUID_OK if success, FLUID_FAILED if memory error.
-----------------------------------------------------------------------------*/
static int new_mod_delay_line(fluid_chorus_t *chorus,
                              int delay_length,
                              int mod_rate
                             )
{
    int i;
	/*-----------------------------------------------------------------------*/
    /* checks parameter */
    if(delay_length < 1)
    {
        return FLUID_FAILED;
    }
    chorus->mod_depth = 0;
    /*-----------------------------------------------------------------------
     allocates delay_line and initialize members: - line, size, line_in...
    */
    /* total size of the line:
    size = INTERP_SAMPLES_NBR + delay_length */
    chorus->size = delay_length + INTERP_SAMPLES_NBR;
    chorus->line = FLUID_ARRAY(fluid_real_t, chorus->size);

    if(! chorus->line)
    {
        return FLUID_FAILED;
    }

    /* clears the buffer:
     - delay line
     - interpolator member: buffer, frac_pos_mod
    */
    fluid_chorus_init(chorus);

    /* Initializes line_in to the start of the buffer */
    chorus->line_in = 0;
    /*------------------------------------------------------------------------
     Initializes modulation members:
     - modulation rate (the speed at which center_pos_mod is modulated: mod_rate
     - modulated center position: center_pos_mod
     - index rate to know when to update center_pos_mod:index_rate
     -------------------------------------------------------------------------*/
#if 0
	/* Sets the modulation rate. This rate defines how often
     the  center position (center_pos_mod ) is modulated .
     The value is expressed in samples. The default value is 1 that means that
     center_pos_mod is updated at every sample.
     For example with a value of 2, the center position position will be
     updated only one time every 2 samples only.
    */
    chorus->mod_rate = 1; /* default modulation rate: every one sample */

    if(mod_rate > chorus->size)
    {
        FLUID_LOG(FLUID_INFO,
                  "chorus: modulation rate is out of range");
    }
    else
    {
        chorus->mod_rate = mod_rate;
    }
#endif
    /* Initializes the modulated center position:
       mod_rate, center_pos_mod,  and index rate
    */
    set_center_position(chorus);

    return FLUID_OK;
}
#else // ! NEW_MOD
/* Purpose:
 *
 * Calculates a modulation waveform (sine) Its value ( modulo
 * MAXSAMPLES) varies between 0 and depth*INTERPOLATION_SUBSAMPLES.
 * Its period length is len.  The waveform data will be used modulo
 * MAXSAMPLES only.  Since MAXSAMPLES is substracted from the waveform
 * a couple of times here, the resulting (current position in
 * buffer)-(waveform sample) will always be positive.
 */
static void
fluid_chorus_sine(int *buf, int len, int depth)
{
    int i;
    double angle, incr, mult;

    /* Pre-calculate increment between angles. */
    incr = (2. * M_PI) / (double)len;

    /* Pre-calculate 'depth' multiplier. */
    mult = (double) depth / 2.0 * (double) INTERPOLATION_SUBSAMPLES;

    /* Initialize to zero degrees. */
    angle = 0.;

    /* Build sine modulation waveform */
    for(i = 0; i < len; i++)
    {
        buf[i] = (int)((1. + sin(angle)) * mult) - 3 * MAX_SAMPLES * INTERPOLATION_SUBSAMPLES;

        angle += incr;
    }
}

/* Purpose:
 * Calculates a modulation waveform (triangle)
 * See fluid_chorus_sine for comments.
 */
static void
fluid_chorus_triangle(int *buf, int len, int depth)
{
    int *il = buf;
    int *ir = buf + len - 1;
    int ival;
    double val, incr;

    /* Pre-calculate increment for the ramp. */
    incr = 2.0 / len * (double)depth * (double) INTERPOLATION_SUBSAMPLES;

    /* Initialize first value */
    val = 0. - 3. * MAX_SAMPLES * INTERPOLATION_SUBSAMPLES;

    /* Build triangular modulation waveform */
    while(il <= ir)
    {
        /* Assume 'val' to be always negative for rounding mode */
        ival = (int)(val - 0.5);

        *il++ = ival;
        *ir-- = ival;

        val += incr;
    }
}
#endif //NEW_MOD

/*-----------------------------------------------------------------------------
  API
------------------------------------------------------------------------------*/
fluid_chorus_t *
new_fluid_chorus(fluid_real_t sample_rate)
{
    int i;
    int ii;
    fluid_chorus_t *chorus;

    chorus = FLUID_NEW(fluid_chorus_t);

    if(chorus == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "chorus: Out of memory");
        return NULL;
    }

    FLUID_MEMSET(chorus, 0, sizeof(fluid_chorus_t));

    chorus->sample_rate = sample_rate;

#ifdef PRINT
    printf("fluid_chorus_t:%d bytes\n", sizeof(fluid_chorus_t));
    printf("fluid_real_t:%d bytes\n", sizeof(fluid_real_t));
#endif

#ifdef NEW_MOD
#ifdef PRINT
    printf("NEW_MOD\n");
#endif
    if(new_mod_delay_line(chorus, MAX_SAMPLES, LOW_MOD_RATE)== FLUID_FAILED)
    {
        goto error_recovery;
    }
#else //NEW_MOD
#ifdef PRINT
    printf("sinc_table:%d bytes\n", sizeof(chorus->sinc_table));
#endif
    /* Lookup table for the SI function (impulse response of an ideal low pass) */

    /* i: Offset in terms of whole samples */
    for(i = 0; i < INTERPOLATION_SAMPLES; i++)
    {

        /* ii: Offset in terms of fractional samples ('subsamples') */
        for(ii = 0; ii < INTERPOLATION_SUBSAMPLES; ii++)
        {
            /* Move the origin into the center of the table */
            double i_shifted = ((double) i - ((double) INTERPOLATION_SAMPLES) / 2.
                                + (double) ii / (double) INTERPOLATION_SUBSAMPLES);

            if(fabs(i_shifted) < 0.000001)
            {
                /* sinc(0) cannot be calculated straightforward (limit needed
                   for 0/0) */
                chorus->sinc_table[i][ii] = (fluid_real_t)1.;

            }
            else
            {
                chorus->sinc_table[i][ii] = (fluid_real_t)sin(i_shifted * M_PI) / (M_PI * i_shifted);
                /* Hamming window */
                chorus->sinc_table[i][ii] *= (fluid_real_t)0.5 * (1.0 + cos(2.0 * M_PI * i_shifted / (fluid_real_t)INTERPOLATION_SAMPLES));
            };
        };
    };
    /* allocate lookup tables */
    chorus->lookup_tab = FLUID_ARRAY(int, (int)(chorus->sample_rate / MIN_SPEED_HZ));

    if(chorus->lookup_tab == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "chorus: Out of memory");
        goto error_recovery;
    }
#ifdef PRINT
    printf("lookup_tab: %d bytes\n", (int)(chorus->sample_rate / MIN_SPEED_HZ)* sizeof(int));
#endif
    /* allocate sample buffer */

    chorus->chorusbuf = FLUID_ARRAY(fluid_real_t, MAX_SAMPLES);

    if(chorus->chorusbuf == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "chorus: Out of memory");
        goto error_recovery;
    }

    if(fluid_chorus_init(chorus) != FLUID_OK)
    {
        goto error_recovery;
    };
#endif //NEW_MOD

    return chorus;

error_recovery:
    delete_fluid_chorus(chorus);

    return NULL;
}

void
delete_fluid_chorus(fluid_chorus_t *chorus)
{
    fluid_return_if_fail(chorus != NULL);

#ifdef NEW_MOD
    FLUID_FREE(chorus->line);
#else
    FLUID_FREE(chorus->chorusbuf);
    FLUID_FREE(chorus->lookup_tab);
#endif
    FLUID_FREE(chorus);
}

int
fluid_chorus_init(fluid_chorus_t *chorus)
{
    int i;

    /* reset delay line */
    for(i = 0; i < MAX_SAMPLES; i++)
    {
#ifdef NEW_MOD
        chorus->line[i] = 0.0;
#else
        chorus->chorusbuf[i] = 0.0;
#endif
    }

#ifdef NEW_MOD
    /* reset modulators's allpass filter */
    for (i = 0; i< MAX_CHORUS; i++)
    {
        /* initializes 1st order All-Pass interpolator members */
        chorus->mod[i].buffer = 0;       /* previous delay sample value */
        chorus->mod[i].frac_pos_mod = 0; /* fractional position (between consecutives sample) */
    }
#endif //NEW_MOD

    return FLUID_OK;
}

void
fluid_chorus_reset(fluid_chorus_t *chorus)
{
    fluid_chorus_init(chorus);
}

/**
 * Set one or more chorus parameters.
 * @param chorus Chorus instance
 * @param set Flags indicating which chorus parameters to set (#fluid_chorus_set_t)
 * @param nr Chorus voice count (0-99, CPU time consumption proportional to
 *   this value)
 * @param level Chorus level (0.0-10.0)
 * @param speed Chorus speed in Hz (0.29-5.0)
 * @param depth_ms Chorus depth (max value depends on synth sample rate,
 *   0.0-21.0 is safe for sample rate values up to 96KHz)
 * @param type Chorus waveform type (#fluid_chorus_mod)
 */
void
fluid_chorus_set(fluid_chorus_t *chorus, int set, int nr, fluid_real_t level,
                 fluid_real_t speed, fluid_real_t depth_ms, int type)
{
#ifndef NEW_MOD
    int modulation_depth_samples;
#endif
    int i;

    if(set & FLUID_CHORUS_SET_NR)
    {
        chorus->number_blocks = nr;
    }

    if(set & FLUID_CHORUS_SET_LEVEL)
    {
        chorus->level = level;
    }

    if(set & FLUID_CHORUS_SET_SPEED)
    {
        chorus->speed_Hz = speed;
    }

    if(set & FLUID_CHORUS_SET_DEPTH)
    {
        chorus->depth_ms = depth_ms;
    }

    if(set & FLUID_CHORUS_SET_TYPE)
    {
        chorus->type = type;
    }

    if(chorus->number_blocks < 0)
    {
        FLUID_LOG(FLUID_WARN, "chorus: number blocks must be >=0! Setting value to 0.");
        chorus->number_blocks = 0;
    }
    else if(chorus->number_blocks > MAX_CHORUS)
    {
        FLUID_LOG(FLUID_WARN, "chorus: number blocks larger than max. allowed! Setting value to %d.",
                  MAX_CHORUS);
        chorus->number_blocks = MAX_CHORUS;
    }

    if(chorus->speed_Hz < MIN_SPEED_HZ)
    {
        FLUID_LOG(FLUID_WARN, "chorus: speed is too low (min %f)! Setting value to min.",
                  (double) MIN_SPEED_HZ);
        chorus->speed_Hz = MIN_SPEED_HZ;
    }
    else if(chorus->speed_Hz > MAX_SPEED_HZ)
    {
        FLUID_LOG(FLUID_WARN, "chorus: speed must be below %f Hz! Setting value to max.",
                  (double) MAX_SPEED_HZ);
        chorus->speed_Hz = MAX_SPEED_HZ;
    }

    if(chorus->depth_ms < 0.0)
    {
        FLUID_LOG(FLUID_WARN, "chorus: depth must be positive! Setting value to 0.");
        chorus->depth_ms = 0.0;
    }

    if(chorus->level < 0.0)
    {
        FLUID_LOG(FLUID_WARN, "chorus: level must be positive! Setting value to 0.");
        chorus->level = 0.0;
    }
    else if(chorus->level > 10)
    {
        FLUID_LOG(FLUID_WARN, "chorus: level must be < 10. A reasonable level is << 1! "
                  "Setting it to 0.1.");
        chorus->level = 0.1;
    }

#ifdef NEW_MOD
    /* initialize modulation depth (peak to peak) (in samples)*/
    chorus->mod_depth = (int)  (chorus->depth_ms  / 1000.0  /* convert modulation depth in ms to s*/
                                * chorus->sample_rate);
    if(chorus->mod_depth > MAX_SAMPLES)
    {
        FLUID_LOG(FLUID_WARN, "chorus: Too high depth. Setting it to max (%d).", MAX_SAMPLES);
        chorus->mod_depth = MAX_SAMPLES;
        // set depth to maximum to avoid spamming console with above warning
        chorus->depth_ms = (chorus->mod_depth * 1000) / chorus->sample_rate;
    }
    chorus->mod_depth /= 2; /* amplitude is peak to peek / 2 */
#ifdef PRINT
    printf("depth_ms:%f, depth_samples/2:%d\n",chorus->depth_ms, chorus->mod_depth);
#endif
    /* Initializes the modulated center position:
       mod_rate, center_pos_mod,  and index rate.
    */
    set_center_position(chorus); /* must be called before set_xxxx_frequency() */
#ifdef PRINT
    printf("mod_rate:%d\n",chorus->mod_rate);
#endif
    /* initialize modulator frequency */
    for(i = 0; i < chorus->number_blocks; i++)
    {
        set_sinus_frequency(&chorus->mod[i].sinus,
                          chorus->speed_Hz * chorus->mod_rate,
//                          0.1 * chorus->mod_rate,
                          chorus->sample_rate,
                          /* phase offset between modulators waveform */
                          (float)((360.0f/(float) chorus->number_blocks) * i));

        set_triangle_frequency(&chorus->mod[i].triang,
                               chorus->speed_Hz * chorus->mod_rate,
                               chorus->sample_rate,
                               /* phase offset between modulators waveform */
                              (float)i / chorus->number_blocks);
    }

#ifdef PRINT
    printf("lfo type:%d\n",chorus->type);
#endif

    if((chorus->type != FLUID_CHORUS_MOD_SINE) &&
		(chorus->type != FLUID_CHORUS_MOD_TRIANGLE))
    {
        FLUID_LOG(FLUID_WARN, "chorus: Unknown modulation type. Using sinewave.");
        chorus->type = FLUID_CHORUS_MOD_SINE;
    }

#else // ! NEW_MOD
    /* The modulating LFO goes through a full period every x samples: */
    chorus->modulation_period_samples = chorus->sample_rate / chorus->speed_Hz;
    /* The variation in delay time is x: */
    modulation_depth_samples = (int)
                               (chorus->depth_ms / 1000.0  /* convert modulation depth in ms to s*/
                                * chorus->sample_rate);

    /* Depth: Check for too high value through modulation_depth_samples. */
    if(modulation_depth_samples > MAX_SAMPLES)
    {
        FLUID_LOG(FLUID_WARN, "chorus: Too high depth. Setting it to max (%d).", MAX_SAMPLES);
        modulation_depth_samples = MAX_SAMPLES;
        // set depth to maximum to avoid spamming console with above warning
        chorus->depth_ms = (modulation_depth_samples * 1000) / chorus->sample_rate;
    }
#ifdef PRINT
    printf("depth_ms:%f, depth_samples:%d\n",chorus->depth_ms, modulation_depth_samples);
#endif

    /* initialize LFO table */
    switch(chorus->type)
    {
    default:
        FLUID_LOG(FLUID_WARN, "chorus: Unknown modulation type. Using sinewave.");
        chorus->type = FLUID_CHORUS_MOD_SINE;
        /* fall-through */
        
    case FLUID_CHORUS_MOD_SINE:
        fluid_chorus_sine(chorus->lookup_tab, chorus->modulation_period_samples,
                          modulation_depth_samples);
        break;

    case FLUID_CHORUS_MOD_TRIANGLE:
        fluid_chorus_triangle(chorus->lookup_tab, chorus->modulation_period_samples,
                              modulation_depth_samples);
        break;
    }

    for(i = 0; i < chorus->number_blocks; i++)
    {
        /* Set the phase of the chorus blocks equally spaced */
        chorus->phase[i] = (int)((double) chorus->modulation_period_samples
                                 * (double) i / (double) chorus->number_blocks);
    }

    /* Start of the circular buffer */
    chorus->counter = 0;
#endif
#ifdef PRINT
    if(chorus->type == FLUID_CHORUS_MOD_SINE )
        printf("lfo: sinus\n");
    else
        printf("lfo: triangle\n");
#endif
}


void fluid_chorus_processmix(fluid_chorus_t *chorus, const fluid_real_t *in,
                             fluid_real_t *left_out, fluid_real_t *right_out)
{
    int sample_index;
    int i;
    fluid_real_t d_in, d_out;

    for(sample_index = 0; sample_index < FLUID_BUFSIZE; sample_index++)
    {

        d_in = in[sample_index];
        d_out = 0.0f;

# if 0
        /* Debug: Listen to the chorus signal only */
        left_out[sample_index] = 0;
        right_out[sample_index] = 0;
#endif

#ifndef NEW_MOD
        /* Write the current sample into the circular buffer */
        chorus->chorusbuf[chorus->counter] = d_in;
#endif

#ifdef NEW_MOD
        ++chorus->index_rate;
#endif //NEW_MOD

        for(i = 0; i < chorus->number_blocks; i++)
        {
#ifdef NEW_MOD
            d_out = get_mod_delay(chorus, &chorus->mod[i]);
#else //! NEW_MOD
            int ii;
            /* Calculate the delay in subsamples for the delay line of chorus block nr. */

            /* The value in the lookup table is so, that this expression
             * will always be positive.  It will always include a number of
             * full periods of MAX_SAMPLES*INTERPOLATION_SUBSAMPLES to
             * remain positive at all times. */
            int pos_subsamples = (INTERPOLATION_SUBSAMPLES * chorus->counter
                                  - chorus->lookup_tab[chorus->phase[i]]);

            int pos_samples = pos_subsamples / INTERPOLATION_SUBSAMPLES;

            /* modulo divide by INTERPOLATION_SUBSAMPLES */
            pos_subsamples &= INTERPOLATION_SUBSAMPLES_ANDMASK;

            for(ii = 0; ii < INTERPOLATION_SAMPLES; ii++)
            {
                /* Add the delayed signal to the chorus sum d_out Note: The
                 * delay in the delay line moves backwards for increasing
                 * delay!*/

                /* The & in chorusbuf[...] is equivalent to a division modulo
                   MAX_SAMPLES, only faster. */
                d_out += chorus->chorusbuf[pos_samples & MAX_SAMPLES_ANDMASK]
                         * chorus->sinc_table[ii][pos_subsamples];
                pos_samples--;
            }
#endif //NEW_MOD
#ifndef NEW_MOD
            /* Cycle the phase of the modulating LFO */
            chorus->phase[i]++;
            chorus->phase[i] %= (chorus->modulation_period_samples);
#endif //NEW_MOD
        } /* foreach chorus block */

#ifdef NEW_MOD
        if(chorus->index_rate >= chorus->mod_rate)
        {
            chorus->index_rate = 0;
            /* updates center position (center_pos_mod) to the next position
               specified by modulation rate */
            if((chorus->center_pos_mod += chorus->mod_rate) >= chorus->size)
            {
                chorus->center_pos_mod -= chorus->size;
            }
        }
#endif

        d_out *= chorus->level;
        /* Add the chorus sum d_out to output */
        left_out[sample_index] += d_out;
        right_out[sample_index] += d_out;

#ifdef NEW_MOD
        /* Write the current sample into the circular buffer */
        push_in_delay_line(chorus, d_in);
#else
        /* Move forward in circular buffer */
        chorus->counter++;
        chorus->counter %= MAX_SAMPLES;
#endif

    } /* foreach sample */
}

/* Duplication of code ... (replaces sample data instead of mixing) */
void fluid_chorus_processreplace(fluid_chorus_t *chorus, const fluid_real_t *in,
                                 fluid_real_t *left_out, fluid_real_t *right_out)
{
    int sample_index;
    int i;
    fluid_real_t d_in, d_out;

    for(sample_index = 0; sample_index < FLUID_BUFSIZE; sample_index++)
    {

        d_in = in[sample_index];
        d_out = 0.0f;

# if 0
        /* Debug: Listen to the chorus signal only */
        left_out[sample_index] = 0;
        right_out[sample_index] = 0;
#endif

#ifndef NEW_MOD
        /* Write the current sample into the circular buffer */
        chorus->chorusbuf[chorus->counter] = d_in;
#endif

#ifdef NEW_MOD
        ++chorus->index_rate;
#endif //NEW_MOD

        for(i = 0; i < chorus->number_blocks; i++)
        {
#ifdef NEW_MOD
            d_out = get_mod_delay(chorus, &chorus->mod[i]);
#else //! NEW_MOD
            int ii;
            /* Calculate the delay in subsamples for the delay line of chorus block nr. */

            /* The value in the lookup table is so, that this expression
             * will always be positive.  It will always include a number of
             * full periods of MAX_SAMPLES*INTERPOLATION_SUBSAMPLES to
             * remain positive at all times. */
            int pos_subsamples = (INTERPOLATION_SUBSAMPLES * chorus->counter
                                  - chorus->lookup_tab[chorus->phase[i]]);

            int pos_samples = pos_subsamples / INTERPOLATION_SUBSAMPLES;

            /* modulo divide by INTERPOLATION_SUBSAMPLES */
            pos_subsamples &= INTERPOLATION_SUBSAMPLES_ANDMASK;

            for(ii = 0; ii < INTERPOLATION_SAMPLES; ii++)
            {
                /* Add the delayed signal to the chorus sum d_out Note: The
                 * delay in the delay line moves backwards for increasing
                 * delay!*/

                /* The & in chorusbuf[...] is equivalent to a division modulo
                   MAX_SAMPLES, only faster. */
                d_out += chorus->chorusbuf[pos_samples & MAX_SAMPLES_ANDMASK]
                         * chorus->sinc_table[ii][pos_subsamples];
                pos_samples--;
            }
#endif //NEW_MOD
#ifndef NEW_MOD
            /* Cycle the phase of the modulating LFO */
            chorus->phase[i]++;
            chorus->phase[i] %= (chorus->modulation_period_samples);
#endif //NEW_MOD
        } /* foreach chorus block */

#ifdef NEW_MOD
        if(chorus->index_rate >= chorus->mod_rate)
        {
            chorus->index_rate = 0;
            /* updates center position (center_pos_mod) to the next position
               specified by modulation rate */
            if((chorus->center_pos_mod += chorus->mod_rate) >= chorus->size)
            {
                chorus->center_pos_mod -= chorus->size;
            }
        }
#endif

        d_out *= chorus->level;
        /* Store the chorus sum d_out to output */
        left_out[sample_index] = d_out;
        right_out[sample_index] = d_out;

#ifdef NEW_MOD
        /* Write the current sample into the circular buffer */
        push_in_delay_line(chorus, d_in);
#else
        /* Move forward in circular buffer */
        chorus->counter++;
        chorus->counter %= MAX_SAMPLES;
#endif

    } /* foreach sample */
}

#ifndef NEW_MOD
#endif
