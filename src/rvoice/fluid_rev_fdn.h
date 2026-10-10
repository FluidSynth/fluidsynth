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
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */


#pragma once

#include "fluid_rev.h"
#include "fluid_rev_filters.h"
#include "fluidsynth_priv.h"

#ifdef __cplusplus


/*----------------------------------------------------------------------------
                        Configuration macros at compiler time.

 3 macros are usable at compiler time:
  - NBR_DELAYs: number of delay lines. 8 (default) or 12.
  - ROOMSIZE_RESPONSE_LINEAR: allows to choose an alternate response for
    roomsize parameter.
  - DENORMALISING enable denormalising handling.
-----------------------------------------------------------------------------*/
//#define INFOS_PRINT /* allows message to be printed on the console. */

/* Number of delay lines (must be only 8 or 12)
  8 is the default.
 12 produces a better quality but is +50% cpu expensive.
*/
#define NBR_DELAYS 8 /* default*/


/*----------------------------------------------------------------------------
             Internal FDN late structures and static functions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 Delay absorbent low pass filter
-----------------------------------------------------------------------------*/
using fdn_delay_lpf = fluid_reverb_delay_damping<fluid_real_t>;

/*-----------------------------------------------------------------------------
 Delay line :
 The delay line is composed of the line plus an absorbent low pass filter
 to get frequency dependent reverb time.
-----------------------------------------------------------------------------*/
using delay_line = fluid_reverb_delay_line<fluid_real_t, fdn_delay_lpf>;


/*-----------------------------------------------------------------------------
 Modulator for modulated delay line
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 Sinusoidal modulator
-----------------------------------------------------------------------------*/
/* modulator are integrated in modulated delay line */
typedef struct
{
    fluid_real_t   a1;          /* Coefficient: a1 = 2 * cos(w) */
    fluid_real_t   buffer1;     /* buffer1 */
    fluid_real_t   buffer2;     /* buffer2 */
    fluid_real_t   reset_buffer2;/* reset value of buffer2 */
} sinus_modulator;

/*-----------------------------------------------------------------------------
 Modulated delay line. The line is composed of:
 - the delay line with its damping low pass filter.
 - the sinusoidal modulator.
 - center output position modulated by the modulator.
 - variable rate control of center output position.
 - first order All-Pass interpolator.
-----------------------------------------------------------------------------*/
typedef struct
{
    /* delay line with damping low pass filter member */
    delay_line dl; /* delayed line */
    /*---------------------------*/
    /* Sinusoidal modulator member */
    sinus_modulator mod; /* sinus modulator */
    /*-------------------------*/
    /* center output position members */
    fluid_real_t  center_pos_mod; /* center output position modulated by modulator */
    int          mod_depth;   /* modulation depth (in samples) */
    /*-------------------------*/
    /* variable rate control of center output position */
    int index_rate;  /* index rate to know when to update center_pos_mod */
    int mod_rate;    /* rate at which center_pos_mod is updated */
    /*-------------------------*/
    /* first order All-Pass interpolator members */
    fluid_real_t  frac_pos_mod; /* fractional position part between samples) */
    /* previous value used when interpolating using fractional */
    fluid_real_t  buffer;
} mod_delay_line;

/*-----------------------------------------------------------------------------
 Late structure
-----------------------------------------------------------------------------*/
struct _fluid_late
{
    fluid_real_t samplerate;       /* sample rate */
    fluid_real_t sample_rate_max;  /* sample rate maximum */
    /*----- High pass tone corrector -------------------------------------*/
    fluid_real_t tone_buffer;
    fluid_real_t b1, b2;
    /*----- Modulated delay lines lines ----------------------------------*/
    mod_delay_line mod_delay_lines[NBR_DELAYS];
    /*-----------------------------------------------------------------------*/
    /* Output coefficients for separate Left and right stereo outputs */
    fluid_real_t out_left_gain[NBR_DELAYS]; /* Left delay lines' output gains */
    fluid_real_t out_right_gain[NBR_DELAYS];/* Right delay lines' output gains*/
};

typedef struct _fluid_late   fluid_late;
/*-----------------------------------------------------------------------------
 fluidsynth reverb structure
-----------------------------------------------------------------------------*/
struct fluid_revmodel_fdn : public _fluid_revmodel_t
{
    /* reverb parameters */
    fluid_real_t roomsize; /* acting on reverb time */
    fluid_real_t damp; /* acting on frequency dependent reverb time */
    fluid_real_t level, wet1, wet2; /* output level */
    fluid_real_t width; /* width stereo separation */

    /* fdn reverberation structure */
    fluid_late  late;

    fluid_revmodel_fdn(fluid_real_t sample_rate_max, fluid_real_t sample_rate);
    ~fluid_revmodel_fdn() override;

    void processmix(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out) override;
    void processreplace(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out) override;
    void reset() override;
    void set(int set, fluid_real_t roomsize, fluid_real_t damping,
             fluid_real_t width, fluid_real_t level) override;
    int samplerate_change(fluid_real_t sample_rate) override;

private:
    template<bool MIX>
    void process(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out);
};

typedef struct fluid_revmodel_fdn fluid_revmodel_fdn_t;

#endif

