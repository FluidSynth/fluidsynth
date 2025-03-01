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

#include "fluid_iir_filter.h"
#include "fluid_conv.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>

// Calculate the filter coefficients with single precision
typedef float IIR_COEFF_T;

struct sincos_t
{
    IIR_COEFF_T sin;
    IIR_COEFF_T cos;
};

enum
{
    CENTS_STEP = 10 /* cents */,
    SINCOS_TAB_SIZE = ((13500 /* upper fc in cents */ - 1500 /* lower fc in cents */) / CENTS_STEP) + 1 /* add one because asking for 13500 cents must yield a valid coefficient */,
};

sincos_t sincos_table[SINCOS_TAB_SIZE];

extern "C" void fluid_iir_filter_init_table(fluid_real_t sample_rate)
{
    for(int fres_cents = 1500, i=0; fres_cents <= 13500; fres_cents += CENTS_STEP, i++)
    {
        fluid_real_t fres = fluid_ct2hz(fres_cents);
        IIR_COEFF_T omega = (IIR_COEFF_T)(2.0 * M_PI) * (fres / sample_rate);
        IIR_COEFF_T sin_coeff = std::sin(omega);
        IIR_COEFF_T cos_coeff = std::cos(omega);
        // i == (fres_cents - 1500) / CENTS_STEP;
        sincos_table[i].sin = sin_coeff;
        sincos_table[i].cos = cos_coeff;
    }
}

template<typename R>
static R interp_lin(R y0, R y1, R x0, R x1, R x)
{
    return std::fabs(x1 - x0 < 1) // do not interpolate, if difference is less than a single cent
    ? y0
    : (y0 * (x1 - x) + y1 * (x - x0)) / (x1 - x0);
}

template<typename R>
static void interp_sin_cos(R fres, sincos_t *coeff)
{
    R diff = (fres - 1500.0f) / CENTS_STEP;

    unsigned idx_prev = std::max(std::min(static_cast<int>(std::floor(diff)), SINCOS_TAB_SIZE - 1), 0);
    unsigned idx_next = std::max(std::min(static_cast<int>(std::ceil(diff)), SINCOS_TAB_SIZE - 1), 0);

    sincos_t prev = sincos_table[idx_prev];
    sincos_t next = sincos_table[idx_next];

    R x0 = idx_prev * CENTS_STEP + 1500;
    R x1 = idx_next * CENTS_STEP + 1500;

    coeff->sin = interp_lin<R>(prev.sin, next.sin, x0, x1, fres);
    coeff->cos = interp_lin<R>(prev.cos, next.cos, x0, x1, fres);
}


template<typename R, bool GAIN_NORM, enum fluid_iir_filter_type TYPE>
static inline void fluid_iir_filter_calculate_coefficients(R fres,
                                                                 R q,
                                                                 R output_rate,
                                                                 R *FLUID_RESTRICT a1_out,
                                                                 R *FLUID_RESTRICT a2_out,
                                                                 R *FLUID_RESTRICT b02_out,
                                                                 R *FLUID_RESTRICT b1_out)
{
    R filter_gain = 1.0f;

    /*
     * Those equations from Robert Bristow-Johnson's `Cookbook
     * formulae for audio EQ biquad filter coefficients', obtained
     * from Harmony-central.com / Computer / Programming. They are
     * the result of the bilinear transform on an analogue filter
     * prototype. To quote, `BLT frequency warping has been taken
     * into account for both significant frequency relocation and for
     * bandwidth readjustment'. */

    sincos_t coeff;
    interp_sin_cos<R>(fres, &coeff);

#ifdef DBG_FILTER
    {
        sincos_t coeff_accurate;
        fluid_real_t fres_hz = fluid_ct2hz(fres);
        R omega = (R)(2.0 * M_PI) * (fres_hz / output_rate);
        coeff_accurate.sin = std::sin(omega);
        coeff_accurate.cos = std::cos(omega);

        std::cerr << "fres: " << std::fixed << std::setprecision(2) << fres_hz << " Hz  |  "
                  << "fres: " << std::fixed << std::setprecision(2) << fres << " Cents  |  "
                  << "sin: " << std::fixed << std::setprecision(6) << coeff.sin << "  |  "
                  << "sin_accurate: " << std::fixed << std::setprecision(6) << coeff_accurate.sin << "  |  "
                  << "abs(sin_diff): " << std::fixed << std::setprecision(6)
                  << std::fabs(coeff.sin - coeff_accurate.sin) << "   |  "
                  << "cos: " << std::fixed << std::setprecision(6) << coeff.cos << "  |  "
                  << "cos_accurate: " << std::fixed << std::setprecision(6) << coeff_accurate.cos << "  |  "
                  << "abs(cos_diff): " << std::fixed << std::setprecision(6)
                  << std::fabs(coeff.cos - coeff_accurate.cos) << std::endl;
    }
#endif


    R alpha_coeff = coeff.sin / (2.0f * q);
    R a0_inv = 1.0f / (1.0f + alpha_coeff);

    /* Calculate the filter coefficients. All coefficients are
     * normalized by a0. Think of `a1' as `a1/a0'.
     *
     * Here a couple of multiplications are saved by reusing common expressions.
     * The original equations should be:
     *  iir_filter->b0=(1.-cos_coeff)*a0_inv*0.5*filter_gain;
     *  iir_filter->b1=(1.-cos_coeff)*a0_inv*filter_gain;
     *  iir_filter->b2=(1.-cos_coeff)*a0_inv*0.5*filter_gain; */

    /* "a" coeffs are same for all 3 available filter types */
    R a1_temp = -2.0f * coeff.cos * a0_inv;
    R a2_temp = (1.0f - alpha_coeff) * a0_inv;
    R b02_temp, b1_temp;

    if (GAIN_NORM)
    {
        /* SF 2.01 page 59:
         *
         *  The SoundFont specs ask for a gain reduction equal to half the
         *  height of the resonance peak (Q).  For example, for a 10 dB
         *  resonance peak, the gain is reduced by 5 dB.  This is done by
         *  multiplying the total gain with sqrt(1/Q).  `Sqrt' divides dB
         *  by 2 (100 lin = 40 dB, 10 lin = 20 dB, 3.16 lin = 10 dB etc)
         *  The gain is later factored into the 'b' coefficients
         *  (numerator of the filter equation).  This gain factor depends
         *  only on Q, so this is the right place to calculate it.
         */
        filter_gain /= std::sqrt(q);
    }

    switch (TYPE)
    {
        case FLUID_IIR_HIGHPASS:
            b1_temp = (1.0f + coeff.cos) * a0_inv * filter_gain;
    
            /* both b0 -and- b2 */
            b02_temp = b1_temp * 0.5f;
    
            b1_temp *= -1.0f;
            break;
    
        case FLUID_IIR_LOWPASS:
            b1_temp = (1.0f - coeff.cos) * a0_inv * filter_gain;
    
            /* both b0 -and- b2 */
            b02_temp = b1_temp * 0.5f;
            break;
    
        default:
            /* filter disabled, should never get here */
            return;
    }

    *a1_out = a1_temp;
    *a2_out = a2_temp;
    *b02_out = b02_temp;
    *b1_out = b1_temp;
}

/**
 * Applies a low- or high-pass filter with variable cutoff frequency and quality factor
 * for a given biquad transfer function:
 *          b0 + b1*z^-1 + b2*z^-2
 *  H(z) = ------------------------
 *          a0 + a1*z^-1 + a2*z^-2
 *
 * Also modifies filter state accordingly.
 * @param iir_filter Filter parameter
 * @param dsp_buf Pointer to the synthesized audio data
 * @param count Count of samples in dsp_buf
 */
/*
 * Variable description:
 * - dsp_a1, dsp_a2: Filter coefficients for the the previously filtered output signal
 * - dsp_b0, dsp_b1, dsp_b2: Filter coefficients for input signal
 * - coefficients normalized to a0
 *
 * A couple of variables are used internally, their results are discarded:
 * - dsp_i: Index through the output buffer
 * - dsp_centernode: delay line for the IIR filter
 * - dsp_hist1: same
 * - dsp_hist2: same
 */
template<bool GAIN_NORM, bool AMPLIFY, enum fluid_iir_filter_type TYPE>
static void
fluid_iir_filter_apply_local(fluid_iir_filter_t *iir_filter, fluid_real_t *dsp_buf, unsigned int count, fluid_real_t output_rate)
{
    // FLUID_IIR_Q_LINEAR may switch the filter off by setting Q==0
    // Due to the linear smoothing, last_q may not exactly become zero.
    if (iir_filter->type == FLUID_IIR_DISABLED || FLUID_FABS(iir_filter->last_q) < Q_MIN)
    {
        return;
    }
    else
    {
        /* IIR filter sample history */
        fluid_real_t dsp_hist1 = iir_filter->hist1;
        fluid_real_t dsp_hist2 = iir_filter->hist2;

        /* IIR filter coefficients */
        enum { N_COEFFS =
#ifdef DBG_FILTER
            1
#else
            16
#endif
        };
        
        /* IIR filter coefficients */
        float dsp_a1 = iir_filter->a1;
        float dsp_a2 = iir_filter->a2;
        float dsp_b02 = iir_filter->b02;
        float dsp_b1 = iir_filter->b1;

        int fres_incr_count = iir_filter->fres_incr_count;
        int q_incr_count = iir_filter->q_incr_count;
        
        fluid_real_t dsp_amp = iir_filter->amp;
        fluid_real_t dsp_amp_incr = iir_filter->amp_incr;
        float fres = iir_filter->last_fres;
        float q = iir_filter->last_q;
        
        const float fres_incr = iir_filter->fres_incr;
        const float q_incr = iir_filter->q_incr;

        /* filter (implement the voice filter according to SoundFont standard) */

        /* Check for denormal number (too close to zero). */
        if (FLUID_FABS(dsp_hist1) < 1e-20f)
        {
            dsp_hist1 = 0.0f; /* FIXME JMG - Is this even needed? */
        }

        /* Two versions of the filter loop. One, while the filter is
         * changing towards its new setting. The other, if the filter
         * doesn't change.
         */

        unsigned int dsp_i;
        for (dsp_i = 0; dsp_i < count; dsp_i++)
        {
            /* The filter is implemented in Direct-II form. */
            fluid_real_t dsp_centernode = dsp_buf[dsp_i] - dsp_a1 * dsp_hist1 - dsp_a2 * dsp_hist2;
            fluid_real_t sample = dsp_b02 * (dsp_centernode + dsp_hist2) + dsp_b1 * dsp_hist1;
            dsp_hist2 = dsp_hist1;
            dsp_hist1 = dsp_centernode;

            /* Alternatively, it could be implemented in Transposed Direct Form II */
            // fluid_real_t dsp_input = dsp_buf[dsp_i];
            // dsp_buf[dsp_i] = dsp_b02 * dsp_input + dsp_hist1;
            // dsp_hist1 = dsp_b1 * dsp_input - dsp_a1 * dsp_buf[dsp_i] + dsp_hist2;
            // dsp_hist2 = dsp_b02 * dsp_input - dsp_a2 * dsp_buf[dsp_i];

            if(AMPLIFY)
            {
                dsp_buf[dsp_i] = dsp_amp * sample;
                dsp_amp += dsp_amp_incr;
            }
            else
            {
                dsp_buf[dsp_i] = sample;
            }

            if(fres_incr_count > 0 || q_incr_count > 0)
            {
                if(fres_incr_count > 0)
                {
                    --fres_incr_count;
                    fres += fres_incr;
                }
                if(q_incr_count > 0)
                {
                    --q_incr_count;
                    q += q_incr;
                }
                
                LOG_FILTER("last_fres: %.2f Cents  |  target_fres: %.2f Cents  |---|  last_q: %.4f  |  target_q: %.4f", iir_filter->last_fres, iir_filter->target_fres, iir_filter->last_q, iir_filter->target_q);
                
                fluid_iir_filter_calculate_coefficients<IIR_COEFF_T, GAIN_NORM, TYPE>(fres, q, output_rate, &dsp_a1, &dsp_a2, &dsp_b02, &dsp_b1);
            }
        }

        iir_filter->hist1 = dsp_hist1;
        iir_filter->hist2 = dsp_hist2;
        iir_filter->a1 = dsp_a1;
        iir_filter->a2 = dsp_a2;
        iir_filter->b02= dsp_b02;
        iir_filter->b1 = dsp_b1;

        iir_filter->last_fres = fres;
        iir_filter->fres_incr_count = fres_incr_count;
        iir_filter->last_q = q;
        iir_filter->q_incr_count = q_incr_count;
        iir_filter->amp = dsp_amp;
    }
}

extern "C" void fluid_iir_filter_apply(fluid_iir_filter_t *resonant_filter,
                                       fluid_iir_filter_t *resonant_custom_filter,
                                       fluid_real_t *dsp_buf,
                                       unsigned int count,
                                       fluid_real_t output_rate)
{
    if(resonant_custom_filter->flags & FLUID_IIR_NO_GAIN_AMP)
    {
        if(resonant_custom_filter->type == FLUID_IIR_HIGHPASS)
        {
            fluid_iir_filter_apply_local<false, false, FLUID_IIR_HIGHPASS>(resonant_custom_filter, dsp_buf, count, output_rate);
        }
        else
        {
            fluid_iir_filter_apply_local<false, false, FLUID_IIR_LOWPASS>(resonant_custom_filter, dsp_buf, count, output_rate);
        }
    }
    else
    {
        if(resonant_custom_filter->type == FLUID_IIR_HIGHPASS)
        {
            fluid_iir_filter_apply_local<true, false, FLUID_IIR_HIGHPASS>(resonant_custom_filter, dsp_buf, count, output_rate);
        }
        else
        {
            fluid_iir_filter_apply_local<true, false, FLUID_IIR_LOWPASS>(resonant_custom_filter, dsp_buf, count, output_rate);
        }
    }

    // This is the last filter in the chain - the default SF2 filter that always runs. This one must apply the final envelope gain.
    fluid_iir_filter_apply_local<true, true, FLUID_IIR_LOWPASS>(resonant_filter, dsp_buf, count, output_rate);
}

void fluid_iir_filter_calc(fluid_iir_filter_t *iir_filter,
                           fluid_real_t output_rate,
                           fluid_real_t fres_mod)
{
    unsigned int calc_coeff_flag = FALSE;
    fluid_real_t fres, fres_diff;
    
    if(iir_filter->type == FLUID_IIR_DISABLED)
    {
        return;
    }

    /* calculate the frequency of the resonant filter in Hz */
    fres = fluid_ct2hz(iir_filter->fres + fres_mod);

    /* I removed the optimization of turning the filter off when the
     * resonance frequency is above the maximum frequency. Instead, the
     * filter frequency is set to a maximum of 0.45 times the sampling
     * rate. For a 44100 kHz sampling rate, this amounts to 19845
     * Hz. The reason is that there were problems with anti-aliasing when the
     * synthesizer was run at lower sampling rates. Thanks to Stephan
     * Tassart for pointing me to this bug. By turning the filter on and
     * clipping the maximum filter frequency at 0.45*srate, the filter
     * is used as an anti-aliasing filter. */

    if(fres > 0.45f * output_rate)
    {
        fres = 0.45f * output_rate;
    }
    else if(fres < 5.f)
    {
        fres = 5.f;
    }

    LOG_FILTER("%f + %f = %f cents = %f Hz | Q: %f", iir_filter->fres, fres_mod, iir_filter->fres + fres_mod, fres, iir_filter->last_q);
    
    fres = fluid_hz2ct(fres);
    
    /* if filter enabled and there is a significant frequency change.. */
    fres_diff = fres - iir_filter->last_fres;
    if(iir_filter->filter_startup)
    {
        // The filer was just starting up, make sure to calculate initial coefficients for the initial Q value, even though the fres may not have changed
        calc_coeff_flag = TRUE;
        
        iir_filter->fres_incr_count = 0;
        iir_filter->last_fres = fres;
        iir_filter->filter_startup = (FLUID_FABS(iir_filter->last_q) < Q_MIN); // filter coefficients will not be initialized when Q is small
    }
    else if(FLUID_FABS(fres_diff) > 0.01f)
    {
        fluid_real_t fres_incr_count = FLUID_BUFSIZE;
        fluid_real_t num_buffers = iir_filter->last_q;
        fluid_clip(num_buffers, 1, 5);
        // For high values of Q, the phase gets really steep. To prevent clicks when quickly modulating fres in this case, we need to smooth out "slower".
        // This is done by simply using Q times FLUID_BUFSIZE samples for the interpolation to complete, capped at 5.
        // 5 was chosen because the phase doesn't really get any steeper when continuing to increase Q.
        fres_incr_count *= num_buffers;
        iir_filter->fres_incr = fres_diff / (fres_incr_count);
        iir_filter->fres_incr_count = static_cast<int>(fres_incr_count + 0.5);
#ifdef DBG_FILTER
        iir_filter->target_fres = fres;
#endif

        // The filter coefficients have to be recalculated (filter cutoff has changed).
        calc_coeff_flag = TRUE;
    }
    else
    {
        // We do not account for any change of Q here - if it was changed q_incro_count will be non-zero and recalculating the coeffs
        // will be taken care of in fluid_iir_filter_apply().
    }

    IIR_COEFF_T output_rate_f = static_cast<IIR_COEFF_T>(output_rate);
    IIR_COEFF_T last_fres_f = static_cast<IIR_COEFF_T>(iir_filter->last_fres);
    IIR_COEFF_T last_q_f = static_cast<IIR_COEFF_T>(iir_filter->last_q);
    if (calc_coeff_flag && !iir_filter->filter_startup)
    {
        if((iir_filter->flags & FLUID_IIR_NO_GAIN_AMP))
        {
            if(iir_filter->type == FLUID_IIR_HIGHPASS)
            {
                fluid_iir_filter_calculate_coefficients<IIR_COEFF_T, false, FLUID_IIR_HIGHPASS>(
                last_fres_f,
                last_q_f,
                output_rate_f,
                &iir_filter->a1,
                &iir_filter->a2,
                &iir_filter->b02,
                &iir_filter->b1);
            }
            else
            {
                fluid_iir_filter_calculate_coefficients<IIR_COEFF_T, false, FLUID_IIR_LOWPASS>(
                last_fres_f,
                last_q_f,
                output_rate_f,
                &iir_filter->a1,
                &iir_filter->a2,
                &iir_filter->b02,
                &iir_filter->b1);
            }
        }
        else
        {
            if(iir_filter->type == FLUID_IIR_HIGHPASS)
            {
                fluid_iir_filter_calculate_coefficients<IIR_COEFF_T, true, FLUID_IIR_HIGHPASS>(
                last_fres_f,
                last_q_f,
                output_rate_f,
                &iir_filter->a1,
                &iir_filter->a2,
                &iir_filter->b02,
                &iir_filter->b1);
            }
            else
            {
                fluid_iir_filter_calculate_coefficients<IIR_COEFF_T, true, FLUID_IIR_LOWPASS>(
                last_fres_f,
                last_q_f,
                output_rate_f,
                &iir_filter->a1,
                &iir_filter->a2,
                &iir_filter->b02,
                &iir_filter->b1);
            }
        }
    }
}
