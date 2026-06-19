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


#ifndef _FLUID_REV_FREEVERB_H
#define _FLUID_REV_FREEVERB_H

#include "fluid_sys.h"
#include "fluid_rev.h"
#include "fluid_rev_filters.h"

using fluid_allpass = fluid_reverb_allpass<fluid_real_t, FLUID_REVERB_ALLPASS_FREEVERB>;
using fluid_comb = fluid_reverb_comb<fluid_real_t>;


constexpr const int numcombs = 8;
constexpr const int numallpasses = 4;

struct fluid_revmodel_freeverb : public _fluid_revmodel_t
{
    fluid_real_t roomsize;
    fluid_real_t damp;
    fluid_real_t level, wet1, wet2;
    fluid_real_t width;
    fluid_real_t gain;
    /*
     The following are all declared inline
     to remove the need for dynamic allocation
     with its subsequent error-checking messiness
    */
    /* Comb filters */
    fluid_comb combL[numcombs];
    fluid_comb combR[numcombs];
    /* Allpass filters */
    fluid_allpass allpassL[numallpasses];
    fluid_allpass allpassR[numallpasses];

    explicit fluid_revmodel_freeverb(fluid_real_t sample_rate);
    ~fluid_revmodel_freeverb() override;

    void processmix(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out) override;
    void processreplace(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out) override;

    void reset() override;
    void set(int set, fluid_real_t roomsize, fluid_real_t damping, fluid_real_t width, fluid_real_t level) override;
    int samplerate_change(fluid_real_t sample_rate) override;

private:
    template<bool mix>
    void process(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out);
};

typedef struct fluid_revmodel_freeverb fluid_revmodel_freeverb_t;


#endif /* _FLUID_REV_FREEVERB_H */
