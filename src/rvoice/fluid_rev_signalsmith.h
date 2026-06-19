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

#pragma once

#include "config.h"

// Hack to prevent inclusion of glib here
#ifdef OSAL_glib
#undef OSAL_glib
#endif

#include "fluid_rev.h"

#if defined(WITH_FLOAT)
using fluid_real_t = float;
#else
using fluid_real_t = double;
#endif

#ifdef SIGNALSMITH_SUPPORT

#include <memory>

/**
 * Reverb engine based on Signalsmith Audio's FDN reverb.
 *
 * Parameter mapping from FluidSynth's four reverb parameters:
 *
 *  room-size [0,1]  ->  roomMs  (10 – 200 ms, room physical size)
 *                   ->  rt20    (0.5 – 10 s,  decay time to –20 dB)
 *                       Both increase together so that larger rooms also
 *                       have a longer tail, matching typical acoustic behaviour.
 *
 *  damp      [0,1]  ->  highDampRate  (1.0 – 5.0, per-loop HF attenuation)
 *                   ->  highCutHz     (20 kHz – 4 kHz, exponential mapping)
 *                       Together they model increasing absorption at high
 *                       frequencies, from bright (damp=0) to muffled (damp=1).
 *
 *  width     [0,100]->  stereo mixing coefficients wet1/wet2.
 *                       Normalised to [0,1] before use; values above 1 are
 *                       clamped.  width=0 produces a mono reverb tail;
 *                       width=1 (or 100) gives fully decorrelated stereo.
 *
 *  level     [0,1]  ->  wet   (reverb output gain inside the engine)
 *                       dry is always 0: the dry signal is handled externally
 *                       by the FluidSynth rvoice mixer.
 */
struct fluid_revmodel_signalsmith : public _fluid_revmodel_t
{
    explicit fluid_revmodel_signalsmith(fluid_real_t sample_rate);
    ~fluid_revmodel_signalsmith() override;

    void processmix(const fluid_real_t *in, fluid_real_t *left_out,
                    fluid_real_t *right_out) override;
    void processreplace(const fluid_real_t *in, fluid_real_t *left_out,
                        fluid_real_t *right_out) override;

    void reset() override;
    void set(int set, fluid_real_t roomsize, fluid_real_t damping,
             fluid_real_t width, fluid_real_t level) override;
    int samplerate_change(fluid_real_t sample_rate) override;

private:
    struct Impl;
    std::unique_ptr<Impl> d;

    /** Recompute all signalsmith reverb parameters from the stored values. */
    void updateParams();

    template<bool MIX>
    void process(const fluid_real_t *in, fluid_real_t *left_out,
                 fluid_real_t *right_out);
};

typedef struct fluid_revmodel_signalsmith fluid_revmodel_signalsmith_t;

#endif
