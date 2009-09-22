/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

/*
 * Josh Green <josh@resonance.org>
 * 2009-05-28
 */

#include "fluid_event_queue.h"
#include "fluidsynth_priv.h"


/**
 * Create a lock free queue with a fixed maximum count and size of elements.
 * @param count Count of elements in queue (fixed max number of queued elements)
 * @return New lock free queue or NULL if out of memory (error message logged)
 *
 * Lockless FIFO queues don't use any locking mechanisms and can therefore be
 * advantageous in certain situations, such as passing data between a lower
 * priority thread and a higher "real time" thread, without potential lock
 * contention which could stall the high priority thread.  Note that there may
 * only be one producer thread and one consumer thread.
 */
fluid_event_queue_t *
fluid_event_queue_new (int count)
{
  fluid_event_queue_t *queue;

  fluid_return_val_if_fail (count > 0, NULL);

  queue = FLUID_NEW (fluid_event_queue_t);

  if (!queue)
  {
    FLUID_LOG (FLUID_ERR, "Out of memory");
    return NULL;
  }

  queue->array = FLUID_ARRAY (fluid_event_queue_elem_t, count);

  if (!queue->array)
  {
    FLUID_FREE (queue);
    FLUID_LOG (FLUID_ERR, "Out of memory");
    return NULL;
  }

  /* Clear array, in case dynamic pointer reclaiming is being done */
  FLUID_MEMSET (queue->array, 0, sizeof (fluid_event_queue_elem_t) * count);

  queue->totalcount = count;
  queue->count = 0;
  queue->in = 0;
  queue->out = 0;

  return (queue);
}

/**
 * Free an event queue.
 * @param queue Lockless queue instance
 *
 * Care must be taken when freeing a queue, to ensure that the consumer and
 * producer threads will no longer access it.
 */
void
fluid_event_queue_free (fluid_event_queue_t *queue)
{
  FLUID_FREE (queue->array);
  FLUID_FREE (queue);
}


/**
 * Get pointer to next input array element in queue.
 * @param queue Lockless queue instance
 * @return Pointer to array element in queue to store data to or NULL if queue is full
 *
 * This function along with fluid_queue_next_inptr() form a queue "push"
 * operation and is split into 2 functions to avoid an element copy.  Note that
 * the returned array element pointer may contain the data of a previous element
 * if the queue has wrapped around.  This can be used to reclaim pointers to
 * allocated memory, etc.
 */
FLUID_INLINE fluid_event_queue_elem_t *
fluid_event_queue_get_inptr (fluid_event_queue_t *queue)
{
  return fluid_atomic_int_get (&queue->count) == queue->totalcount ? NULL
    : queue->array + queue->in;
}

/**
 * Advance the input queue index to complete a "push" operation.
 * @param queue Lockless queue instance
 *
 * This function along with fluid_queue_get_inptr() form a queue "push"
 * operation and is split into 2 functions to avoid element copy.
 */
FLUID_INLINE void
fluid_event_queue_next_inptr (fluid_event_queue_t *queue)
{
  fluid_atomic_int_inc (&queue->count);

  if (++queue->in == queue->totalcount)
    queue->in = 0;
}

/**
 * Get pointer to next output array element in queue.
 * @param queue Lockless queue instance
 * @return Pointer to array element data in the queue or NULL if empty, can only
 *   be used up until fluid_queue_next_outptr() is called.
 *
 * This function along with fluid_queue_next_outptr() form a queue "pop"
 * operation and is split into 2 functions to avoid an element copy.
 */
FLUID_INLINE fluid_event_queue_elem_t *
fluid_event_queue_get_outptr (fluid_event_queue_t *queue)
{
  return fluid_atomic_int_get (&queue->count) == 0 ? NULL
    : queue->array + queue->out;
}

/**
 * Advance the output queue index to complete a "pop" operation.
 * @param queue Lockless queue instance
 *
 * This function along with fluid_queue_get_outptr() form a queue "pop"
 * operation and is split into 2 functions to avoid an element copy.
 */
FLUID_INLINE void
fluid_event_queue_next_outptr (fluid_event_queue_t *queue)
{
  fluid_atomic_int_dec_and_test (&queue->count);

  if (++queue->out == queue->totalcount)
    queue->out = 0;
}
