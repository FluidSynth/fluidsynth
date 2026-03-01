/******************************************************************************
 * FluidSynth - A Software Synthesizer
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

#include "fluid_sys.h"
#include "fluid_rev_lexverb.h"
#include "fluid_conv_tables.h"
#include "fluid_conv.h"

constexpr float LEX_TRIM = 0.7f;
constexpr float LEX_SCALE_WET_WIDTH = 0.2f;
constexpr float LEX_DELAY_SCALE = 100.0f;

static int fluid_lexverb_ms_to_buf_length(float ms, fluid_real_t sample_rate)
{
    return static_cast<int>(ms * (sample_rate * (1 / 1000.0f)) + 0.5f);
}

static float fluid_lexverb_lpf(fluid_reverb_delay_damping<float> &filter, float input)
{
    filter.buffer = filter.b0 * input + filter.a1 * filter.buffer;
    return filter.buffer;
}

static void fluid_lexverb_set_lpf_coeffs(fluid_reverb_delay_damping<float> &filter, float b0)
{
    filter.b0 = b0;
    filter.a1 = 1.0f - b0;
}

static void fluid_lexverb_setup_blocks(fluid_revmodel_lexverb_t *rev)
{
    int i;
    for(i = 0; i < NUM_OF_AP_SECTS; ++i)
    {
        int length = fluid_lexverb_ms_to_buf_length(LEX_REVERB_PARMS[i].length, rev->cached_sample_rate);

        rev->ap[i].set_feedback(LEX_REVERB_PARMS[i].coef);
        rev->ap[i].set_buffer(length);
        rev->ap[i].set_index(1);
        rev->ap[i].set_last_output(0.0f);
        rev->ap[i].delay.damping.buffer = 0.0f;
    }

    for(i = 0; i < NUM_OF_DELAY_SECTS; ++i)
    {
        int index = NUM_OF_AP_SECTS + i;
        int length = fluid_lexverb_ms_to_buf_length(LEX_REVERB_PARMS[index].length, rev->cached_sample_rate);
        FLUID_LOG(FLUID_DBG, "LEXverb delay line %d: default length = %d samples", i, length);
        length *= LEX_DELAY_SCALE;
        length++; // prevent zero length delay lines
        FLUID_LOG(FLUID_DBG, "LEXverb delay line %d: MAX length = %d samples", i, length);

        rev->dl[i].set_coefficient(LEX_REVERB_PARMS[index].coef);
        rev->dl[i].set_buffer(length);
        rev->dl[i].set_positions(1, 1);
        rev->dl[i].set_last_output(0.0f);
        rev->dl[i].damping.buffer = 0.0f;
    }

    rev->reset();
}

static void fluid_lexverb_update(fluid_revmodel_lexverb_t *rev)
{
    fluid_real_t wet = (rev->level) /
                       (1.0f + rev->width * LEX_SCALE_WET_WIDTH);

    rev->wet1 = wet * (rev->width / 2.0f + 0.5f);
    rev->wet2 = wet * ((1.0f - rev->width) / 2.0f);

    for(int i = 0; i < NUM_OF_DELAY_SECTS; ++i)
    {
        int index = NUM_OF_AP_SECTS + i;
        int length = fluid_lexverb_ms_to_buf_length(LEX_REVERB_PARMS[index].length, rev->cached_sample_rate);
        length *= LEX_DELAY_SCALE * fluid_concave(rev->roomsize * FLUID_VEL_CB_SIZE);
        length++; // prevent zero length delay lines
        FLUID_LOG(FLUID_DBG, "Lexverb delay line %d: length = %d samples", i, length);
        rev->dl[i].set_buffer(length);
    }

    float damp_b0 = 1.0f - static_cast<float>(rev->damp);
    fluid_lexverb_set_lpf_coeffs(rev->ap[4].delay.damping, damp_b0);
    fluid_lexverb_set_lpf_coeffs(rev->ap[9].delay.damping, damp_b0);
    fluid_lexverb_set_lpf_coeffs(rev->dl[0].damping, damp_b0);
    fluid_lexverb_set_lpf_coeffs(rev->dl[1].damping, damp_b0);
}

static void fluid_lexverb_process_sample(fluid_revmodel_lexverb_t *rev, float input,
                                         float *out_left, float *out_right)
{
    fluid_reverb_allpass<float> *ap = rev->ap;
    fluid_reverb_delay_line<float> *dl = rev->dl;
    float output;

    output = ap[0].process(input * LEX_TRIM); // technically, the left input sample should be here
    output = ap[1].process(output);
    float left_feedback = fluid_lexverb_lpf(ap[9].delay.damping, ap[9].get_last_output());
    float left_cross = fluid_lexverb_lpf(dl[1].damping, dl[1].process(left_feedback));
    output = ap[2].process(output + left_cross * dl[1].get_coefficient());
    output = ap[3].process(output);
    output = ap[4].process(output);
    *out_left = output;

    output = ap[5].process(input * LEX_TRIM); // technically, the right input sample should be here
    output = ap[6].process(output);
    float right_feedback = fluid_lexverb_lpf(ap[4].delay.damping, ap[4].get_last_output());
    float right_cross = fluid_lexverb_lpf(dl[0].damping, dl[0].process(right_feedback));
    output = ap[7].process(output + right_cross * dl[0].get_coefficient());
    output = ap[8].process(output);
    output = ap[9].process(output);
    *out_right = output;
}

fluid_revmodel_lexverb::fluid_revmodel_lexverb(fluid_real_t sample_rate)
    : roomsize(0.0f),
      damp(0.0f),
      level(0.0f),
      wet1(0.0f),
      wet2(0.0f),
      width(0.0f),
      cached_sample_rate(sample_rate)
{
    if(sample_rate <= 0.0f)
    {
        throw std::invalid_argument("Sample rate must be positive");
    }

    fluid_lexverb_setup_blocks(this);
}

fluid_revmodel_lexverb::~fluid_revmodel_lexverb() = default;


void fluid_revmodel_lexverb::processmix(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out)
{
    process<true>(in, left_out, right_out);
}

void fluid_revmodel_lexverb::processreplace(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out)
{
    process<false>(in, left_out, right_out);
}

template<bool MIX>
void fluid_revmodel_lexverb::process(const fluid_real_t *in, fluid_real_t *left_out,
                                     fluid_real_t *right_out)
{
    int i;

    for(i = 0; i < FLUID_BUFSIZE; ++i)
    {
        float left = 0.0f;
        float right = 0.0f;

        fluid_lexverb_process_sample(this, (float)in[i], &left, &right);

        fluid_real_t out_left = left * wet1 + right * wet2;
        fluid_real_t out_right = right * wet1 + left * wet2;

        if(MIX)
        {
            left_out[i] += out_left;
            right_out[i] += out_right;
        }
        else
        {
            left_out[i] = out_left;
            right_out[i] = out_right;
        }
    }
}

void fluid_revmodel_lexverb::reset()
{
    int i;

    for(i = 0; i < NUM_OF_AP_SECTS; ++i)
    {
        if(this->ap[i].has_buffer())
        {
            this->ap[i].fill_buffer(0.0f);
            this->ap[i].set_index(1);
        }
        this->ap[i].set_last_output(0.0f);
        this->ap[i].delay.damping.buffer = 0.0f;
    }

    for(i = 0; i < NUM_OF_DELAY_SECTS; ++i)
    {
        if(this->dl[i].has_buffer())
        {
            this->dl[i].fill_buffer(0.0f);
            this->dl[i].set_positions(1, 1);
        }
        this->dl[i].set_last_output(0.0f);
        this->dl[i].damping.buffer = 0.0f;
    }
}

void fluid_revmodel_lexverb::set(int set, fluid_real_t roomsize, fluid_real_t damping,
                                 fluid_real_t width, fluid_real_t level)
{
    if(set & FLUID_REVMODEL_SET_ROOMSIZE)
    {
        fluid_clip(roomsize, 0.0f, 1.0f);
        this->roomsize = roomsize;
    }

    if(set & FLUID_REVMODEL_SET_DAMPING)
    {
        fluid_clip(damping, 0.0f, 1.0f);
        this->damp = damping;
    }

    if(set & FLUID_REVMODEL_SET_WIDTH)
    {
        fluid_clip(width, 0.0f, 100.0f);
        this->width = width;
    }

    if(set & FLUID_REVMODEL_SET_LEVEL)
    {
        fluid_clip(level, 0.0f, 1.0f);
        this->level = level;
    }

    fluid_lexverb_update(this);
}

int fluid_revmodel_lexverb::samplerate_change(fluid_real_t sample_rate)
{
    FLUID_LOG(FLUID_ERR, "LEXverb: sample rate change is not supported");
    return FLUID_FAILED;
}
