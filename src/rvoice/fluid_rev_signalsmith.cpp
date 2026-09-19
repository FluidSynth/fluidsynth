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

#include "fluid_rev_signalsmith.h"
#include "fluid_conv_tables.h"
#include "fluid_conv.h"

#ifdef SIGNALSMITH_SUPPORT

#include "signalsmith-basics/reverb.h"

#include <cmath>
#include <new>
#include <stdexcept>

/* Select sample type matching fluid_real_t */
#ifdef WITH_FLOAT
using SsReverb = signalsmith::basics::ReverbFloat;
#else
using SsReverb = signalsmith::basics::ReverbDouble;
#endif

struct fluid_revmodel_signalsmith::Impl
{
    std::unique_ptr<SsReverb> reverb_impl = std::make_unique<SsReverb>();

    fluid_real_t roomsize = 0;
    fluid_real_t damp = 0;
    fluid_real_t width = 0;
    fluid_real_t level = 0;
    fluid_real_t wet1 = 0;          /**< Stereo mixing coefficient for same-channel contribution */
    fluid_real_t wet2 = 0;          /**< Stereo mixing coefficient for cross-channel contribution */
};

/* ---- Parameter mapping constants ----------------------------------------- */

/* room-size [0,1] -> roomMs: physical room size in milliseconds,
   MAX adjusted to match room-size scaling with fluidsynth's other reverb engines */
static constexpr double SS_ROOM_MIN_MS = 10.0;
static constexpr double SS_ROOM_MAX_MS = 120.0;

/* room-size [0,1] -> rt20: decay time to –20 dB in seconds */
static constexpr double SS_RT20_MIN = 0.5;
static constexpr double SS_RT20_MAX = 6.0;

/* damp [0,1] -> highDampRate: per-loop high-frequency gain multiplier
   1.0 = unity (no damping), larger values attenuate highs more aggressively */
static constexpr double SS_HIGH_DAMP_RATE_MIN = 1.0;
static constexpr double SS_HIGH_DAMP_RATE_MAX = 5.0;

/* damp [0,1] -> highCutHz: static output high-cut filter
   Exponential mapping from 20 kHz (13500 cents at damp=0) to ~4 kHz (damp=1). */
static constexpr double SS_HIGH_CUT_BASE = 13500.0;

/* Fixed parameters not exposed by FluidSynth */
static constexpr double SS_LOW_CUT_HZ        = 80.0;  /* Remove sub-bass */
static constexpr double SS_LOW_DAMP_RATE     = 1.0;   /* No extra low damping */
static constexpr double SS_EARLY_REFLECTIONS = 1.5;   /* Early reflections gain */
static constexpr double SS_DETUNE            = 2.0;   /* Subtle chorus detuning */

/* ---- Construction / destruction ------------------------------------------ */

fluid_revmodel_signalsmith::fluid_revmodel_signalsmith(fluid_real_t sample_rate)
    : d(std::make_unique<Impl>())
{
    if(sample_rate <= 0.0f)
    {
        throw std::invalid_argument("Sample rate must be positive");
    }

    d->reverb_impl->configure(sample_rate, FLUID_BUFSIZE, 2);

    /* Set fixed parameters */
    d->reverb_impl->lowCutHz    = SS_LOW_CUT_HZ;
    d->reverb_impl->lowDampRate = SS_LOW_DAMP_RATE;
    d->reverb_impl->early       = SS_EARLY_REFLECTIONS;
    d->reverb_impl->detune      = SS_DETUNE;
    /* Dry signal is handled by the FluidSynth rvoice mixer */
    d->reverb_impl->dry         = 0.0;

    updateParams();
    d->reverb_impl->configure(sample_rate, FLUID_BUFSIZE, 2);
}

fluid_revmodel_signalsmith::~fluid_revmodel_signalsmith() = default;

/* ---- Parameter update ---------------------------------------------------- */

void fluid_revmodel_signalsmith::updateParams()
{
    /* Level -> wet amplitude */
    d->reverb_impl->wet = static_cast<double>(d->level);

    /* Room size -> room dimensions and decay time */
    d->reverb_impl->roomMs = SS_ROOM_MIN_MS + fluid_concave(d->roomsize * FLUID_VEL_CB_SIZE) * (SS_ROOM_MAX_MS - SS_ROOM_MIN_MS);
    d->reverb_impl->rt20   = SS_RT20_MIN   + fluid_concave(d->roomsize * FLUID_VEL_CB_SIZE) * (SS_RT20_MAX   - SS_RT20_MIN);

    /* Damping -> high-frequency rolloff (static cut + per-loop attenuation) */
    d->reverb_impl->highDampRate = SS_HIGH_DAMP_RATE_MIN + d->damp * (SS_HIGH_DAMP_RATE_MAX - SS_HIGH_DAMP_RATE_MIN);
    //d->reverb_impl->highCutHz    = SS_HIGH_CUT_BASE * std::pow(SS_HIGH_CUT_EXP, static_cast<double>(d->damp));
    d->reverb_impl->highCutHz    = fluid_ct2hz(8500 + ((SS_HIGH_CUT_BASE - 8500) * (1 - d->damp)));

    /* Width -> stereo mixing coefficients.
       Normalise to [0,1]; values above 1 are clamped.
         norm_width = 0 -> wet1 = 0.5, wet2 = 0.5  (mono fold-down)
         norm_width = 1 -> wet1 = 1.0, wet2 = 0.0  (full stereo)        */
    float norm_width = std::min(static_cast<float>(d->width) / 100.0f, 1.0f);
    d->wet1 = static_cast<fluid_real_t>(norm_width * 0.5f + 0.5f);
    d->wet2 = static_cast<fluid_real_t>((1.0f - norm_width) * 0.5f);
}

/* ---- Public API ---------------------------------------------------------- */

void fluid_revmodel_signalsmith::reset()
{
    d->reverb_impl->reset();
}

void fluid_revmodel_signalsmith::set(int set, fluid_real_t roomsize,
                                     fluid_real_t damping, fluid_real_t width,
                                     fluid_real_t level)
{
    if(set & FLUID_REVMODEL_SET_ROOMSIZE)
    {
        fluid_clip(roomsize, 0.0f, 1.0f);
        d->roomsize = roomsize;
    }

    if(set & FLUID_REVMODEL_SET_DAMPING)
    {
        fluid_clip(damping, 0.0f, 1.0f);
        d->damp = damping;
    }

    if(set & FLUID_REVMODEL_SET_WIDTH)
    {
        fluid_clip(width, 0.0f, 100.0f);
        d->width = width;
    }

    if(set & FLUID_REVMODEL_SET_LEVEL)
    {
        fluid_clip(level, 0.0f, 1.0f);
        d->level = level;
    }

    updateParams();
}

int fluid_revmodel_signalsmith::samplerate_change(fluid_real_t sample_rate)
{
    d->reverb_impl->configure(sample_rate, FLUID_BUFSIZE, 2);
    return FLUID_OK;
}

void fluid_revmodel_signalsmith::processmix(const fluid_real_t *in,
                                            fluid_real_t *left_out,
                                            fluid_real_t *right_out)
{
    process<true>(in, left_out, right_out);
}

void fluid_revmodel_signalsmith::processreplace(const fluid_real_t *in,
                                                fluid_real_t *left_out,
                                                fluid_real_t *right_out)
{
    process<false>(in, left_out, right_out);
}

/* ---- Audio processing ---------------------------------------------------- */

template<bool MIX>
void fluid_revmodel_signalsmith::process(const fluid_real_t *in,
                                         fluid_real_t *left_out,
                                         fluid_real_t *right_out)
{
    using Sample = typename SsReverb::Sample;

    /* Temporary stereo I/O buffers (stack-allocated, FLUID_BUFSIZE = 64 samples) */
    Sample in_l[FLUID_BUFSIZE], in_r[FLUID_BUFSIZE];
    Sample out_l[FLUID_BUFSIZE], out_r[FLUID_BUFSIZE];

    /* Expand mono input to stereo */
    for(int i = 0; i < FLUID_BUFSIZE; ++i)
    {
        in_l[i] = in_r[i] = static_cast<Sample>(in[i]);
    }

    Sample *inputs[2]  = {in_l, in_r};
    Sample *outputs[2] = {out_l, out_r};

    d->reverb_impl->process(inputs, outputs, FLUID_BUFSIZE);

    /* Apply stereo width mixing and write to output buffers.
       wet1 and wet2 are computed from the width parameter in updateParams():
         out_left  = rev_left  * wet1 + rev_right * wet2
         out_right = rev_right * wet1 + rev_left  * wet2        */
    for(int i = 0; i < FLUID_BUFSIZE; ++i)
    {
        fluid_real_t wl = static_cast<fluid_real_t>(out_l[i]) * d->wet1
                        + static_cast<fluid_real_t>(out_r[i]) * d->wet2;
        fluid_real_t wr = static_cast<fluid_real_t>(out_r[i]) * d->wet1
                        + static_cast<fluid_real_t>(out_l[i]) * d->wet2;

        if(MIX)
        {
            left_out[i]  += wl;
            right_out[i] += wr;
        }
        else
        {
            left_out[i]  = wl;
            right_out[i] = wr;
        }
    }
}

#endif
