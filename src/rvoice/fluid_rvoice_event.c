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

#include "fluid_rvoice_event.h"
#include "fluid_rvoice.h"
#include "fluid_rvoice_mixer.h"
#include "fluid_iir_filter.h"
#include "fluid_lfo.h"
#include "fluid_adsr_env.h"

static int fluid_rvoice_eventhandler_push_LOCAL(fluid_rvoice_eventhandler_t *handler, const fluid_rvoice_event_t *src_event);

static void fluid_rvoice_eventhandler_notify_pending_events(fluid_rvoice_eventhandler_t *handler);
static FLUID_INLINE int fluid_rvoice_eventhandler_has_pending_events(fluid_rvoice_eventhandler_t *handler);

static FLUID_INLINE void
fluid_rvoice_event_dispatch(fluid_rvoice_event_t *event)
{
    event->method(event->object, event->param);
}


/**
 * In order to be able to push more than one event atomically,
 * use push for all events, then use flush to commit them to the
 * queue. If threadsafe is false, all events are processed immediately. */
int
fluid_rvoice_eventhandler_push_int_real(fluid_rvoice_eventhandler_t *handler,
                                        fluid_rvoice_function_t method, void *object, int intparam,
                                        fluid_real_t realparam)
{
    fluid_rvoice_event_t local_event;

    local_event.method = method;
    local_event.object = object;
    local_event.param[0].i = intparam;
    local_event.param[1].real = realparam;

    return fluid_rvoice_eventhandler_push_LOCAL(handler, &local_event);
}

int
fluid_rvoice_eventhandler_push(fluid_rvoice_eventhandler_t *handler, fluid_rvoice_function_t method, void *object, fluid_rvoice_param_t param[MAX_EVENT_PARAMS])
{
    fluid_rvoice_event_t local_event;

    local_event.method = method;
    local_event.object = object;
    FLUID_MEMCPY(&local_event.param, param, sizeof(*param) * MAX_EVENT_PARAMS);

    return fluid_rvoice_eventhandler_push_LOCAL(handler, &local_event);
}

int
fluid_rvoice_eventhandler_push_ptr(fluid_rvoice_eventhandler_t *handler,
                                   fluid_rvoice_function_t method, void *object, void *ptr)
{
    fluid_rvoice_event_t local_event;

    local_event.method = method;
    local_event.object = object;
    local_event.param[0].ptr = ptr;

    return fluid_rvoice_eventhandler_push_LOCAL(handler, &local_event);
}

static int fluid_rvoice_eventhandler_push_LOCAL(fluid_rvoice_eventhandler_t *handler, const fluid_rvoice_event_t *src_event)
{
    fluid_rvoice_event_t *event;
    int old_queue_stored = fluid_atomic_int_add(&handler->queue_stored, 1);

    event = fluid_ringbuffer_get_inptr(handler->queue, old_queue_stored);

    if(event == NULL)
    {
        fluid_atomic_int_add(&handler->queue_stored, -1);
        FLUID_LOG(FLUID_WARN, "Ringbuffer full, try increasing synth.polyphony!");
        return FLUID_FAILED; // Buffer full...
    }

    FLUID_MEMCPY(event, src_event, sizeof(*event));

    fluid_rvoice_eventhandler_notify_pending_events(handler);

    return FLUID_OK;
}


void
fluid_rvoice_eventhandler_finished_voice_callback(fluid_rvoice_eventhandler_t *eventhandler, fluid_rvoice_t *rvoice)
{
    fluid_rvoice_t **vptr = fluid_ringbuffer_get_inptr(eventhandler->finished_voices, 0);

    if(vptr == NULL)
    {
        return;    // Buffer full
    }

    *vptr = rvoice;
    fluid_ringbuffer_next_inptr(eventhandler->finished_voices, 1);
}

fluid_rvoice_eventhandler_t *
new_fluid_rvoice_eventhandler(int queuesize,
                              int finished_voices_size, int bufs, int fx_bufs, int fx_units,
                              fluid_real_t sample_rate_max, fluid_real_t sample_rate,
                              int extra_threads, int prio, int idle_blocks_threshold)
{
    fluid_rvoice_eventhandler_t *eventhandler = FLUID_NEW(fluid_rvoice_eventhandler_t);

    if(eventhandler == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    eventhandler->mixer = NULL;
    eventhandler->queue = NULL;
    eventhandler->finished_voices = NULL;

    fluid_atomic_int_set(&eventhandler->queue_stored, 0);

    eventhandler->finished_voices = new_fluid_ringbuffer(finished_voices_size,
                                    sizeof(fluid_rvoice_t *));

    if(eventhandler->finished_voices == NULL)
    {
        goto error_recovery;
    }

    eventhandler->queue = new_fluid_ringbuffer(queuesize, sizeof(fluid_rvoice_event_t));

    if(eventhandler->queue == NULL)
    {
        goto error_recovery;
    }

    eventhandler->mixer = new_fluid_rvoice_mixer(bufs, fx_bufs, fx_units,
                          sample_rate_max, sample_rate, eventhandler, extra_threads, prio);

    if(eventhandler->mixer == NULL)
    {
        goto error_recovery;
    }

    eventhandler->idle_blocks_threshold = idle_blocks_threshold;
    eventhandler->idle_blocks = 0;
    eventhandler->idle = 0;
    eventhandler->pending_events = new_fluid_cond();
    eventhandler->pending_events_m = new_fluid_cond_mutex();
    if (!eventhandler->pending_events || !eventhandler->pending_events_m)
    {
        goto error_recovery;
    }

    return eventhandler;

error_recovery:
    delete_fluid_rvoice_eventhandler(eventhandler);
    return NULL;
}

int
fluid_rvoice_eventhandler_dispatch_count(fluid_rvoice_eventhandler_t *handler)
{
    return fluid_ringbuffer_get_count(handler->queue);
}


static FLUID_INLINE int
fluid_rvoice_eventhandler_has_pending_events(fluid_rvoice_eventhandler_t *handler)
{
    return (fluid_ringbuffer_get_count(handler->queue) > 0 ||
            handler->queue_stored > 0);
}


/**
 * Call fluid_rvoice_event_dispatch for all events in queue
 * @return number of events dispatched
 */
int
fluid_rvoice_eventhandler_dispatch_all(fluid_rvoice_eventhandler_t *handler)
{
    fluid_rvoice_event_t *event;
    int result = 0;

    while(NULL != (event = fluid_ringbuffer_get_outptr(handler->queue)))
    {
        fluid_rvoice_event_dispatch(event);
        result++;
        fluid_ringbuffer_next_outptr(handler->queue);
    }

    return result;
}

void fluid_rvoice_eventhandler_update_idle_state(
        fluid_rvoice_eventhandler_t *handler, int blockcount)
{
    fluid_return_if_fail(handler->idle_blocks_threshold > 0);

    if (fluid_rvoice_mixer_get_active_voices(handler->mixer) ||
            fluid_rvoice_eventhandler_has_pending_events(handler))
    {
        handler->idle_blocks = 0;
        return;
    }

    if (handler->idle_blocks < handler->idle_blocks_threshold)
    {
        handler->idle_blocks += blockcount;

        if (handler->idle_blocks >= handler->idle_blocks_threshold)
        {
            handler->idle_blocks = 0;
            fluid_cond_mutex_lock(handler->pending_events_m);
            handler->idle = TRUE;
            fluid_cond_mutex_unlock(handler->pending_events_m);
        }
    }
}

int fluid_rvoice_eventhandler_is_idle(fluid_rvoice_eventhandler_t *handler)
{
    int idle;

    fluid_return_val_if_fail(handler->idle_blocks_threshold > 0, FALSE);

    fluid_cond_mutex_lock(handler->pending_events_m);
    idle = handler->idle || fluid_rvoice_eventhandler_has_pending_events(handler);
    fluid_cond_mutex_unlock(handler->pending_events_m);
    return idle;
}

void fluid_rvoice_eventhandler_wait_for_events(fluid_rvoice_eventhandler_t *handler)
{
    fluid_return_if_fail(handler->idle_blocks_threshold > 0);

    fluid_cond_mutex_lock(handler->pending_events_m);
    if (handler->idle && !fluid_rvoice_eventhandler_has_pending_events(handler)) {
        fluid_cond_wait(handler->pending_events, handler->pending_events_m);
    }
    fluid_cond_mutex_unlock(handler->pending_events_m);
}

static void fluid_rvoice_eventhandler_notify_pending_events(fluid_rvoice_eventhandler_t *handler)
{
    fluid_return_if_fail(handler->idle_blocks_threshold > 0);

    fluid_cond_mutex_lock(handler->pending_events_m);
    if (handler->idle) {
        handler->idle = FALSE;
        fluid_cond_signal(handler->pending_events);
    }
    fluid_cond_mutex_unlock(handler->pending_events_m);
}

void
delete_fluid_rvoice_eventhandler(fluid_rvoice_eventhandler_t *handler)
{
    fluid_return_if_fail(handler != NULL);

    if (handler->pending_events_m) {
        delete_fluid_cond_mutex(handler->pending_events_m);
    }

    if (handler->pending_events) {
        delete_fluid_cond(handler->pending_events);
    }

    delete_fluid_rvoice_mixer(handler->mixer);
    delete_fluid_ringbuffer(handler->queue);
    delete_fluid_ringbuffer(handler->finished_voices);
    FLUID_FREE(handler);
}
