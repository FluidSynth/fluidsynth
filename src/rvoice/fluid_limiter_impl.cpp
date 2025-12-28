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

#include "fluidsynth_priv.h"

#ifdef LIMITER_SUPPORT

#include "fluid_limiter.h"
#include "fluid_limiter_impl.h"
#include "signalsmith-basics/limiter.h"

#ifdef WITH_FLOAT
using Limiter = signalsmith::basics::LimiterFloat;
#else
using Limiter = signalsmith::basics::LimiterDouble;
#endif

extern "C" void fluid_limiter_impl_set_sample_rate(fluid_limiter_t* lim, fluid_real_t sample_rate)
{
    ((Limiter*)lim)->configure(sample_rate, FLUID_BUFSIZE, FLUID_LIMITER_NUM_CHANNELS_AT_ONCE);
}

extern "C" fluid_limiter_t* fluid_limiter_impl_new(fluid_real_t sample_rate, fluid_limiter_settings_t* settings)
{
    auto lim = new Limiter(settings->attack_ms + settings->hold_ms);

    lim->inputGain = settings->input_gain;
    lim->outputLimit = settings->output_limit;
    lim->attackMs = settings->attack_ms;
    lim->holdMs = settings->hold_ms;
    lim->releaseMs = settings->release_ms;
    lim->smoothingStages = settings->smoothing_stages;
    lim->linkChannels = settings->link_channels;

    if (! lim->configure(sample_rate, FLUID_BUFSIZE,
                         FLUID_LIMITER_NUM_CHANNELS_AT_ONCE,
                         FLUID_LIMITER_NUM_CHANNELS_AT_ONCE) )
    {
        FLUID_LOG(FLUID_WARN, "limiter parameters was not accepted");
    }

    return lim;
}

extern "C" void fluid_limiter_impl_delete(fluid_limiter_t* lim)
{
    delete (Limiter*)lim;
}

extern "C" void fluid_limiter_impl_process_buffers(
    fluid_limiter_t* lim,
    fluid_real_t* bufs[FLUID_LIMITER_NUM_CHANNELS_AT_ONCE])
{
    ((Limiter*)lim)->process(bufs, FLUID_BUFSIZE);
}

#endif /* LIMITER_SUPPORT */
