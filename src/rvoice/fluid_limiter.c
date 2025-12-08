/******************************************************************************
 * FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2025  Neoharp development team
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
 *
 */

#include "fluidsynth_priv.h"

#ifdef LIMITER

#include "fluid_limiter.h"
#include "fluid_limiter_impl.h"
#include "fluid_sys.h"

/*----------------------------------------------------------------------------
                            Limiter API
-----------------------------------------------------------------------------*/
/*
* Creates a limiter with default parameters
*
* @param sample_rate actual sample rate needed in Hz.
* @return pointer on the new limiter or NULL if memory error.
* Limiter API.
*/
fluid_limiter_t*
new_fluid_limiter(fluid_real_t sample_rate, fluid_limiter_settings_t* settings)
{
    fluid_limiter_t* lim;

    if(sample_rate <= 0)
    {
        return NULL;
    }

    lim = fluid_limiter_impl_new(sample_rate, settings);

    if(lim == NULL)
    {
        return NULL;
    }

    return lim;
}

/*
* free the limiter.
* @param lim pointer on limiter to free.
* Limiter API.
*/
void
delete_fluid_limiter(fluid_limiter_t *lim)
{
    fluid_return_if_fail(lim != NULL);
    fluid_limiter_impl_delete(lim);
}

/*
* Applies a sample rate change on the limiter.
*
* @param lim the limiter.
* @param sample_rate new sample rate value
* @return FLUID_OK if success, FLUID_FAILED if lim is NULL
* Limiter API.
*/
int
fluid_limiter_samplerate_change(fluid_limiter_t *lim, fluid_real_t sample_rate)
{
    int status = FLUID_OK;

    fluid_return_val_if_fail(lim != NULL, FLUID_FAILED);

    fluid_limiter_impl_set_sample_rate(lim, sample_rate);

    return status;
}

/*-----------------------------------------------------------------------------
* Run the limiter
* @param lim pointer on limiter.
* @param buf_l left buffer to process (will be modified in-place)
* @param buf_r right buffer to process (will be modified in-place)
* Limiter API.
-----------------------------------------------------------------------------*/
void
fluid_limiter_run(fluid_limiter_t *lim, fluid_real_t *buf_l, fluid_real_t *buf_r, int block_count)
{
    int i;
    fluid_real_t* bufs[FLUID_LIMITER_NUM_CHANNELS_AT_ONCE];

    for (i = 0; i < block_count; i++)
    {
#if FLUID_LIMITER_NUM_CHANNELS_AT_ONCE < 2
#error "expected FLUID_LIMITER_NUM_CHANNELS_AT_ONCE >= 2"
#endif
        bufs[0] = buf_l + i*FLUID_BUFSIZE;
        bufs[1] = buf_r + i*FLUID_BUFSIZE;

        fluid_limiter_impl_process_buffers(lim, bufs);
    }
}

#endif /* LIMITER */
