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

#include "config.h"

#if defined(WITH_FLOAT)
using fluid_real_t = float;
#else
using fluid_real_t = double;
#endif

#ifdef SIGNALSMITH_SUPPORT

#include "fluidsynth.h"
#include "fluid_limiter.h"
#include "fluid_limiter_impl.h"
#include "signalsmith-basics/limiter.h"

#include <exception>
#include <memory>
#include <new>
#ifdef TRAP_ON_FPE
#include <cfenv>
#endif

#ifdef WITH_FLOAT
using Limiter = signalsmith::basics::LimiterFloat;
#else
using Limiter = signalsmith::basics::LimiterDouble;
#endif

namespace
{
#ifdef TRAP_ON_FPE
/* Signalsmith's ChannelEnvelope default-constructs BoxFilter(0), which divides
 * 1/0 before resizing during configure(). Mask traps only around configure(). */
struct fpe_trap_guard
{
    std::fenv_t env;
    fpe_trap_guard()
    {
        std::feholdexcept(&env);
    }
    ~fpe_trap_guard()
    {
        std::fesetenv(&env);
    }
};
#endif

void apply_limiter_settings(Limiter &lim, const fluid_limiter_settings_t &settings)
{
    lim.inputGain = settings.input_gain;
    lim.outputLimit = settings.output_limit;
    lim.attackMs = settings.attack_ms;
    lim.holdMs = settings.hold_ms;
    lim.releaseMs = settings.release_ms;
    lim.smoothingStages = settings.smoothing_stages;
    lim.linkChannels = settings.link_channels;
}

bool configure_limiter(Limiter &lim, fluid_real_t sample_rate, unsigned int block_size)
{
#ifdef TRAP_ON_FPE
    fpe_trap_guard fpe_guard;
#endif
    if(!lim.configure(sample_rate, block_size,
                      FLUID_LIMITER_NUM_CHANNELS_AT_ONCE,
                      FLUID_LIMITER_NUM_CHANNELS_AT_ONCE))
    {
        fluid_log(FLUID_WARN, "limiter parameters were not accepted");
        return false;
    }
    return true;
}
}

extern "C" void fluid_limiter_impl_set_sample_rate(fluid_limiter_t* lim, fluid_real_t sample_rate, unsigned int block_size)
{
    try
    {
        configure_limiter(*(Limiter*)lim, sample_rate, block_size);
    }
    catch(const std::bad_alloc &)
    {
        fluid_log(FLUID_ERR, "out of memory reconfiguring limiter");
    }
    catch(const std::exception &exc)
    {
        fluid_log(FLUID_ERR, "limiter sample rate change failed: %s", exc.what());
    }
    catch(...)
    {
        fluid_log(FLUID_ERR, "limiter sample rate change failed: unexpected exception");
    }
}

extern "C" fluid_limiter_t *fluid_limiter_impl_new(fluid_real_t sample_rate, const fluid_limiter_settings_t *settings, unsigned int block_size)
{
    try
    {
        /* Pre-size ring buffers for the maximum registered attack + hold range
         * (+1 ms rounding margin for ceil(attack) + ceil(hold)) so runtime
         * parameter changes update in-place without audio-thread reallocation.
         * Actual look-ahead latency still depends only on attackMs. */
        constexpr double max_delay_ms = FLUID_LIMITER_MAX_ATTACK_MS + FLUID_LIMITER_MAX_HOLD_MS + 1.0;
        std::unique_ptr<Limiter> lim(new Limiter(max_delay_ms));

        apply_limiter_settings(*lim, *settings);

        if(!configure_limiter(*lim, sample_rate, block_size))
        {
            return nullptr;
        }

        return lim.release();
    }
    catch(const std::bad_alloc &)
    {
        fluid_log(FLUID_PANIC, "out of memory allocating limiter");
    }
    catch(const std::exception &exc)
    {
        fluid_log(FLUID_ERR, "limiter initialization failed: %s", exc.what());
    }
    catch(...)
    {
        fluid_log(FLUID_ERR, "limiter initialization failed: unexpected exception");
    }

    return nullptr;
}

extern "C" void fluid_limiter_impl_set_params(fluid_limiter_t *lim, const fluid_limiter_settings_t *settings)
{
    apply_limiter_settings(*(Limiter *)lim, *settings);
}

extern "C" void fluid_limiter_impl_delete(fluid_limiter_t* lim)
{
    delete(Limiter*)lim;
}

extern "C" void fluid_limiter_impl_process_buffers(
    fluid_limiter_t *lim,
    fluid_real_t *bufs[FLUID_LIMITER_NUM_CHANNELS_AT_ONCE],
    unsigned int block_size)
{
    ((Limiter*)lim)->process(bufs, block_size);
}

#endif /* SIGNALSMITH_SUPPORT */
