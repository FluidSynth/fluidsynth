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

#include "fluid_rev.h"
#include "fluid_sys.h"
#include "fluid_rev_filters.h"
#include "fluid_rev_fdn.h"
#include "fluid_rev_freeverb.h"
#include "fluid_rev_lexverb.h"
#include "fluid_rev_dattorro.h"

#include <exception>
#include <new>

fluid_revmodel_t *
new_fluid_revmodel(fluid_real_t sample_rate_max, fluid_real_t sample_rate, int reverb_type)
{
    try
    {
        fluid_revmodel_t *rev = NULL;

        if(reverb_type == FLUID_REVERB_TYPE_FREEVERB)
        {
            rev = new fluid_revmodel_freeverb(sample_rate);
        }
        else if(reverb_type == FLUID_REVERB_TYPE_LEXVERB)
        {
            rev = new fluid_revmodel_lexverb(sample_rate);
        }
        else if(reverb_type == FLUID_REVERB_TYPE_DATTORRO)
        {
            rev = new fluid_revmodel_dattorro(sample_rate);
        }
        else
        {
            rev = new fluid_revmodel_fdn(sample_rate_max, sample_rate);
        }

        return rev;
    }
    catch(const std::bad_alloc &)
    {
        FLUID_LOG(FLUID_ERR, "Reverb initialization failed: out of memory");
    }
    catch(const std::exception &exc)
    {
        FLUID_LOG(FLUID_ERR, "Reverb initialization failed: %s", exc.what());
    }
    catch(...)
    {
        FLUID_LOG(FLUID_ERR, "Reverb initialization failed: unexpected exception");
    }

    return NULL;
}

/*
 * free the reverb.
 * Note that while the reverb is used by calling any fluid_revmodel_processXXX()
 * function, calling delete_fluid_revmodel() isn't multi task safe because
 * delay line are freed. To deal properly with this issue follow the steps:
 *
 * 1) Stop reverb processing (i.e disable calling of any fluid_revmodel_processXXX().
 *    reverb functions.
 * 2) Delete the reverb by calling delete_fluid_revmodel().
 *
 * @param rev pointer on reverb to free.
 * Reverb API.
 */
void
delete_fluid_revmodel(fluid_revmodel_t *rev)
{
    try
    {
        delete rev;
    }
    catch(const std::exception &exc)
    {
        FLUID_LOG(FLUID_ERR, "Reverb cleanup failed: %s", exc.what());
    }
    catch(...)
    {
        FLUID_LOG(FLUID_ERR, "Reverb cleanup failed: unexpected exception");
    }
}

void fluid_revmodel_processmix(fluid_revmodel_t *rev, const fluid_real_t *in,
                               fluid_real_t *left_out, fluid_real_t *right_out)
{
    fluid_return_if_fail(rev != NULL);
    try
    {
        rev->processmix(in, left_out, right_out);
    }
    catch(const std::exception &exc)
    {
        FLUID_LOG(FLUID_ERR, "Reverb processing failed: %s", exc.what());
    }
    catch(...)
    {
        FLUID_LOG(FLUID_ERR, "Reverb processing failed: unexpected exception");
    }
}

void fluid_revmodel_processreplace(fluid_revmodel_t *rev, const fluid_real_t *in,
                                   fluid_real_t *left_out, fluid_real_t *right_out)
{
    fluid_return_if_fail(rev != NULL);
    try
    {
        rev->processreplace(in, left_out, right_out);
    }
    catch(const std::exception &exc)
    {
        FLUID_LOG(FLUID_ERR, "Reverb processing failed: %s", exc.what());
    }
    catch(...)
    {
        FLUID_LOG(FLUID_ERR, "Reverb processing failed: unexpected exception");
    }
}

void fluid_revmodel_reset(fluid_revmodel_t *rev)
{
    fluid_return_if_fail(rev != NULL);
    try
    {
        rev->reset();
    }
    catch(const std::exception &exc)
    {
        FLUID_LOG(FLUID_ERR, "Reverb reset failed: %s", exc.what());
    }
    catch(...)
    {
        FLUID_LOG(FLUID_ERR, "Reverb reset failed: unexpected exception");
    }
}

void fluid_revmodel_set(fluid_revmodel_t *rev, int set, fluid_real_t roomsize,
                        fluid_real_t damping, fluid_real_t width, fluid_real_t level)
{
    fluid_return_if_fail(rev != NULL);
    try
    {
        rev->set(set, roomsize, damping, width, level);
    }
    catch(const std::exception &exc)
    {
        FLUID_LOG(FLUID_ERR, "Reverb update failed: %s", exc.what());
    }
    catch(...)
    {
        FLUID_LOG(FLUID_ERR, "Reverb update failed: unexpected exception");
    }
}

int fluid_revmodel_samplerate_change(fluid_revmodel_t *rev, fluid_real_t sample_rate)
{
    fluid_return_val_if_fail(rev != NULL, FLUID_FAILED);
    try
    {
        return rev->samplerate_change(sample_rate);
    }
    catch(const std::bad_alloc &)
    {
        FLUID_LOG(FLUID_ERR, "Reverb samplerate change failed: out of memory");
    }
    catch(const std::exception &exc)
    {
        FLUID_LOG(FLUID_ERR, "Reverb samplerate change failed: %s", exc.what());
    }
    catch(...)
    {
        FLUID_LOG(FLUID_ERR, "Reverb samplerate change failed: unexpected exception");
    }

    return FLUID_FAILED;
}
