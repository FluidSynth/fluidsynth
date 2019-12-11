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



/*
  2002 : API design by Peter Hanappe and Antoine Schmitt
  August 2002 : Implementation by Antoine Schmitt as@gratin.org
  as part of the infiniteCD author project
  http://www.infiniteCD.org/
*/

#include "fluid_event.h"
#include "fluid_sys.h"	// timer, threads, etc...
#include "fluid_list.h"

/***************************************************************
 *
 *                           SEQUENCER
 */

#define FLUID_SEQUENCER_EVENTS_MAX	1000

/* Private data for SEQUENCER */
struct _fluid_sequencer_t
{
    unsigned int startMs;
    fluid_atomic_int_t currentMs;
    int useSystemTimer;
    double scale; // ticks per second
    fluid_list_t *clients;
    fluid_seq_id_t clientsID;

    fluid_mutex_t mutex;
};

/* Private data for clients */
typedef struct _fluid_sequencer_client_t
{
    fluid_seq_id_t id;
    char *name;
    fluid_event_callback_t callback;
    void *data;
} fluid_sequencer_client_t;


/* API implementation */

/**
 * Create a new sequencer object which uses the system timer.  Use
 * new_fluid_sequencer2() to specify whether the system timer or
 * fluid_sequencer_process() is used to advance the sequencer.
 * @return New sequencer instance
 */
fluid_sequencer_t *
new_fluid_sequencer(void)
{
    return new_fluid_sequencer2(TRUE);
}

/**
 * Create a new sequencer object.
 * @param use_system_timer If TRUE, sequencer will advance at the rate of the
 *   system clock. If FALSE, call fluid_sequencer_process() to advance
 *   the sequencer.
 * @return New sequencer instance
 * @since 1.1.0
 */
fluid_sequencer_t *
new_fluid_sequencer2(int use_system_timer)
{
    fluid_sequencer_t *seq;

    seq = FLUID_NEW(fluid_sequencer_t);

    if(seq == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
        return NULL;
    }

    FLUID_MEMSET(seq, 0, sizeof(fluid_sequencer_t));

    seq->scale = 1000;	// default value
    seq->useSystemTimer = use_system_timer ? 1 : 0;
    seq->startMs = seq->useSystemTimer ? fluid_curtime() : 0;

    return(seq);
}

/**
 * Free a sequencer object.
 * @note Registered sequencer clients may not be fully freed by this function. Explicitly unregister them with fluid_sequencer_unregister_client().
 * @param seq Sequencer to delete
 */
void
delete_fluid_sequencer(fluid_sequencer_t *seq)
{
    fluid_return_if_fail(seq != NULL);

    /* cleanup clients */
    while(seq->clients)
    {
        fluid_sequencer_client_t *client = (fluid_sequencer_client_t *)seq->clients->data;
        fluid_sequencer_unregister_client(seq, client->id);
    }

    FLUID_FREE(seq);
}

/**
 * Check if a sequencer is using the system timer or not.
 * @param seq Sequencer object
 * @return TRUE if system timer is being used, FALSE otherwise.
 * @since 1.1.0
 */
int
fluid_sequencer_get_use_system_timer(fluid_sequencer_t *seq)
{
    fluid_return_val_if_fail(seq != NULL, FALSE);

    return seq->useSystemTimer;
}


/* clients */

/**
 * Register a sequencer client.
 * @param seq Sequencer object
 * @param name Name of sequencer client
 * @param callback Sequencer client callback or NULL for a source client.
 * @param data User data to pass to the \a callback
 * @return Unique sequencer ID or #FLUID_FAILED on error
 *
 * Clients can be sources or destinations of events.  Sources don't need to
 * register a callback.
 *
 * @note The user must explicitly unregister any registered client with fluid_sequencer_unregister_client()
 * before deleting the sequencer!
 */
fluid_seq_id_t
fluid_sequencer_register_client(fluid_sequencer_t *seq, const char *name,
                                fluid_event_callback_t callback, void *data)
{
    fluid_sequencer_client_t *client;
    char *nameCopy;

    fluid_return_val_if_fail(seq != NULL, FLUID_FAILED);

    client = FLUID_NEW(fluid_sequencer_client_t);

    if(client == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
        return FLUID_FAILED;
    }

    nameCopy = FLUID_STRDUP(name);

    if(nameCopy == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
        FLUID_FREE(client);
        return FLUID_FAILED;
    }

    seq->clientsID++;

    client->name = nameCopy;
    client->id = seq->clientsID;
    client->callback = callback;
    client->data = data;

    seq->clients = fluid_list_append(seq->clients, (void *)client);

    return (client->id);
}

/**
 * Unregister a previously registered client.
 * @param seq Sequencer object
 * @param id Client ID as returned by fluid_sequencer_register_client().
 */
void
fluid_sequencer_unregister_client(fluid_sequencer_t *seq, fluid_seq_id_t id)
{
    fluid_list_t *tmp;

    fluid_return_if_fail(seq != NULL);

    tmp = seq->clients;

    while(tmp)
    {
        fluid_sequencer_client_t *client = (fluid_sequencer_client_t *)tmp->data;

        if(client->id == id)
        {
            if(client->name)
            {
                FLUID_FREE(client->name);
            }

            seq->clients = fluid_list_remove_link(seq->clients, tmp);
            delete1_fluid_list(tmp);
            FLUID_FREE(client);
            return;
        }

        tmp = tmp->next;
    }

    return;
}

/**
 * Count a sequencers registered clients.
 * @param seq Sequencer object
 * @return Count of sequencer clients.
 */
int
fluid_sequencer_count_clients(fluid_sequencer_t *seq)
{
    if(seq == NULL || seq->clients == NULL)
    {
        return 0;
    }

    return fluid_list_size(seq->clients);
}

/**
 * Get a client ID from its index (order in which it was registered).
 * @param seq Sequencer object
 * @param index Index of register client
 * @return Client ID or #FLUID_FAILED if not found
 */
fluid_seq_id_t fluid_sequencer_get_client_id(fluid_sequencer_t *seq, int index)
{
    fluid_list_t *tmp;

    fluid_return_val_if_fail(seq != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(index >= 0, FLUID_FAILED);

    tmp = fluid_list_nth(seq->clients, index);

    if(tmp == NULL)
    {
        return FLUID_FAILED;
    }
    else
    {
        fluid_sequencer_client_t *client = (fluid_sequencer_client_t *)tmp->data;
        return client->id;
    }
}

/**
 * Get the name of a registered client.
 * @param seq Sequencer object
 * @param id Client ID
 * @return Client name or NULL if not found.  String is internal and should not
 *   be modified or freed.
 */
char *
fluid_sequencer_get_client_name(fluid_sequencer_t *seq, fluid_seq_id_t id)
{
    fluid_list_t *tmp;

    fluid_return_val_if_fail(seq != NULL, NULL);

    tmp = seq->clients;

    while(tmp)
    {
        fluid_sequencer_client_t *client = (fluid_sequencer_client_t *)tmp->data;

        if(client->id == id)
        {
            return client->name;
        }

        tmp = tmp->next;
    }

    return NULL;
}

/**
 * Check if a client is a destination client.
 * @param seq Sequencer object
 * @param id Client ID
 * @return TRUE if client is a destination client, FALSE otherwise or if not found
 */
int
fluid_sequencer_client_is_dest(fluid_sequencer_t *seq, fluid_seq_id_t id)
{
    fluid_list_t *tmp;

    fluid_return_val_if_fail(seq != NULL, FALSE);

    tmp = seq->clients;

    while(tmp)
    {
        fluid_sequencer_client_t *client = (fluid_sequencer_client_t *)tmp->data;

        if(client->id == id)
        {
            return (client->callback != NULL);
        }

        tmp = tmp->next;
    }

    return FALSE;
}

/**
 * Send an event immediately.
 * @param seq Sequencer object
 * @param evt Event to send (not copied, used directly)
 */
void
fluid_sequencer_send_now(fluid_sequencer_t *seq, fluid_event_t *evt)
{
    fluid_seq_id_t destID;
    fluid_list_t *tmp;

    fluid_return_if_fail(seq != NULL);
    fluid_return_if_fail(evt != NULL);

    destID = fluid_event_get_dest(evt);

    /* find callback */
    tmp = seq->clients;

    while(tmp)
    {
        fluid_sequencer_client_t *dest = (fluid_sequencer_client_t *)tmp->data;

        if(dest->id == destID)
        {
            if(dest->callback)
            {
                (dest->callback)(fluid_sequencer_get_tick(seq), evt, seq, dest->data);
            }

            return;
        }

        tmp = tmp->next;
    }
}

/**
 * Schedule an event for sending at a later time.
 * @param seq Sequencer object
 * @param evt Event to send (will be copied into internal queue)
 * @param time Time value in ticks (in milliseconds with the default time scale of 1000).
 * @param absolute TRUE if \a time is absolute sequencer time (time since sequencer
 *   creation), FALSE if relative to current time.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_sequencer_send_at(fluid_sequencer_t *seq, fluid_event_t *evt,
                        unsigned int time, int absolute)
{
    unsigned int now = fluid_sequencer_get_tick(seq);

    fluid_return_val_if_fail(seq != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(evt != NULL, FLUID_FAILED);

    /* set absolute */
    if(!absolute)
    {
        time = now + time;
    }

    /* time stamp event */
    fluid_event_set_time(evt, time);
}

/**
 * Remove events from the event queue.
 * @param seq Sequencer object
 * @param source Source client ID to match or -1 for wildcard
 * @param dest Destination client ID to match or -1 for wildcard
 * @param type Event type to match or -1 for wildcard (#fluid_seq_event_type)
 */
void
fluid_sequencer_remove_events(fluid_sequencer_t *seq, fluid_seq_id_t source,
                              fluid_seq_id_t dest, int type)
{
    fluid_return_if_fail(seq != NULL);
    // TODO
}


/*************************************
	time
**************************************/

/**
 * Get the current tick of a sequencer.
 * @param seq Sequencer object
 * @return Current tick value
 */
unsigned int
fluid_sequencer_get_tick(fluid_sequencer_t *seq)
{
    unsigned int absMs;
    double nowFloat;
    unsigned int now;

    fluid_return_val_if_fail(seq != NULL, 0u);

    absMs = seq->useSystemTimer ? (int) fluid_curtime() : fluid_atomic_int_get(&seq->currentMs);
    nowFloat = ((double)(absMs - seq->startMs)) * seq->scale / 1000.0f;
    now = nowFloat;
    return now;
}

/**
 * Set the time scale of a sequencer.
 * @param seq Sequencer object
 * @param scale Sequencer scale value in ticks per second
 *   (default is 1000 for 1 tick per millisecond, max is 1000.0)
 *
 * If there are already scheduled events in the sequencer and the scale is changed
 * the events are adjusted accordingly.
 */
void
fluid_sequencer_set_time_scale(fluid_sequencer_t *seq, double scale)
{
    fluid_return_if_fail(seq != NULL);

    if(scale <= 0)
    {
        FLUID_LOG(FLUID_WARN, "sequencer: scale <= 0 : %f\n", scale);
        return;
    }

    if(scale > 1000.0)
        // Otherwise : problems with the timer = 0ms...
    {
        scale = 1000.0;
    }

    if(seq->scale != scale)
    {
        // TODO
#if 0
        /* re-start timer */
        if(seq->useSystemTimer)
        {
            seq->timer = new_fluid_timer((int)(1000 / seq->scale), _fluid_seq_queue_process, (void *)seq, TRUE, FALSE, TRUE);
        }
#endif
    }
}

/**
 * Get a sequencer's time scale.
 * @param seq Sequencer object.
 * @return Time scale value in ticks per second.
 */
double
fluid_sequencer_get_time_scale(fluid_sequencer_t *seq)
{
    fluid_return_val_if_fail(seq != NULL, 0);
    return seq->scale;
}

/**
 * Advance a sequencer that isn't using the system timer.
 * @param seq Sequencer object
 * @param msec Time to advance sequencer to (absolute time since sequencer start).
 * @since 1.1.0
 */
void
fluid_sequencer_process(fluid_sequencer_t *seq, unsigned int msec)
{
    /* send queued events */
    fluid_atomic_int_set(&seq->currentMs, msec);
}
