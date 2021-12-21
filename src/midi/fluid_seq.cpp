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

#ifdef __cplusplus
extern "C" {
#endif

#include "fluid_seq.h"
#include "fluid_event.h"
#include "fluid_seq_queue.h"
#include "fluid_sys.h"

#ifdef __cplusplus
}
#endif

#include "fluid_cxx_wrapper.hpp"

#include <mutex>
#include <vector>
#include <atomic>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <new> // std::bad_alloc

/***************************************************************
 *
 *                           SEQUENCER
 */

#define FLUID_SEQUENCER_EVENTS_MAX	1000

/* Private data for clients */
typedef struct _fluid_sequencer_client_t
{
    fluid_seq_id_t id;
    std::string name;
    fluid_event_callback_t callback;
    void *data;
} fluid_sequencer_client_t;

/* Private data for SEQUENCER */
struct _fluid_sequencer_t
{
    // A backup of currentMs when we have received the last scale change
    unsigned int startMs = 0;

    // The number of milliseconds passed since we have started the sequencer,
    // as indicated by the synth's sample timer
    std::atomic<int> currentMs{0};

    // A backup of cur_ticks when we have received the last scale change
    unsigned int start_ticks = 0;

    // The tick count which we've used for the most recent event dispatching
    unsigned int cur_ticks = 0;

    int useSystemTimer = 0;

    // The current time scale in ticks per second.
    // If you think of MIDI, this is equivalent to: (PPQN / 1000) / (USPQN / 1000)
    double scale = 1000;

    std::vector<std::unique_ptr<fluid_sequencer_client_t>> clients;
    fluid_seq_id_t clientsID = 0;

    // Pointer to the C++ event queue
    void *queue = nullptr;
    std::recursive_mutex mutex;
    
    _fluid_sequencer_t(bool use_system_timer)
    {
        if(use_system_timer)
        {
            FLUID_LOG(FLUID_WARN, "sequencer: Usage of the system timer has been deprecated!");
        }

        this->useSystemTimer = use_system_timer ? 1 : 0;
        this->startMs = this->useSystemTimer ? fluid_curtime() : 0;

        this->queue = new_fluid_seq_queue(FLUID_SEQUENCER_EVENTS_MAX);
        if(this->queue == NULL)
        {
            FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
            throw std::bad_alloc();
        }
    }
    
    ~_fluid_sequencer_t()
    {
        /* cleanup clients */
        for(auto& client : this->clients)
        {
            this->unregister_client(client->id);
        }
        delete_fluid_seq_queue(this->queue);
    }
    
    fluid_seq_id_t register_client(const char *name, fluid_event_callback_t callback, void *data)
    {
        std::unique_ptr<fluid_sequencer_client_t> client(new fluid_sequencer_client_t);

        int id = ++this->clientsID;

        client->name = std::string(name);
        client->id = id;
        client->callback = callback;
        client->data = data;

        this->clients.push_back(std::move(client));

        return (id);
    }
    
    int unregister_client(fluid_seq_id_t id)
    {
        fluid_event_t evt;
        unsigned int now = this->get_tick();

        fluid_event_clear(&evt);
        fluid_event_unregistering(&evt);
        fluid_event_set_dest(&evt, id);
        fluid_event_set_time(&evt, now);

        auto result = std::find_if(this->clients.begin(), this->clients.end(),
                        [=](std::unique_ptr<fluid_sequencer_client_t>& client)
                        {
                            return client->id == id;
                        });
        
        if(result == this->clients.end())
        {
            // provided client id is not in the list, probably already deleted
            return FLUID_FAILED;
        }

        // client found, remove it from the list to avoid recursive call when calling callback
        std::unique_ptr<fluid_sequencer_client_t> client_to_remove = std::move(*result);
        this->clients.erase(result);

        // call the callback (if any), to free underlying memory (e.g. seqbind structure)
        if (client_to_remove->callback != NULL)
        {
            (client_to_remove->callback)(now, &evt, this, client_to_remove->data);
        }
        
        return FLUID_OK;
        // client_to_remove will be automatically deleted here
    }
    
    const char* get_client_name(fluid_seq_id_t id)
    {
        auto result = std::find_if(this->clients.begin(), this->clients.end(),
                        [=](std::unique_ptr<fluid_sequencer_client_t>& client)
                        {
                            return client->id == id;
                        });
        
        if(result == this->clients.end())
        {
            // provided client id is not in the list
            return nullptr;
        }

        return (*result)->name.c_str();
    }
    
    bool client_is_dest(fluid_seq_id_t id)
    {
        auto result = std::find_if(this->clients.begin(), this->clients.end(),
                        [=](std::unique_ptr<fluid_sequencer_client_t>& client)
                        {
                            return client->id == id;
                        });
        
        if(result == this->clients.end())
        {
            // provided client id is not in the list
            return false;
        }

        return (*result)->callback != nullptr;
    }
    
    int send_now(fluid_event_t *evt)
    {
        fluid_seq_id_t destID = fluid_event_get_dest(evt);

        auto result = std::find_if(this->clients.begin(), this->clients.end(),
                        [=](std::unique_ptr<fluid_sequencer_client_t>& client)
                        {
                            return client->id == destID;
                        });
        
        if(result == this->clients.end())
        {
            // provided client id is not in the list
            return FLUID_FAILED;
        }

        auto& dest = (*result);
        if(fluid_event_get_type(evt) == FLUID_SEQ_UNREGISTERING)
        {
            this->unregister_client(destID);
        }
        else
        {
            if(dest->callback)
            {
                (dest->callback)(this->get_tick(), evt, this, dest->data);
            }
        }
        return FLUID_OK;
    }
    
    int send_at(fluid_event_t *evt, unsigned int time, int absolute)
    {
        unsigned int now = this->get_tick();

        /* set absolute */
        if(!absolute)
        {
            time = now + time;
        }

        /* time stamp event */
        fluid_event_set_time(evt, time);

        std::lock_guard<std::recursive_mutex> l(this->mutex);
        return fluid_seq_queue_push(this->queue, evt);
    }
    
    unsigned int get_tick()
    {
        return this->get_tick(this->currentMs);
    }
    
    unsigned int get_tick(unsigned int cur_msec)
    {
        unsigned int absMs = this->useSystemTimer ? (unsigned int) fluid_curtime() : cur_msec;
        double nowFloat = ((double)(absMs - this->startMs)) * this->scale / 1000.0f;
        unsigned int now = nowFloat;
        return this->start_ticks + now;
    }
    
    int set_time_scale(double scale)
    {
        if(scale != scale)
        {
            FLUID_LOG(FLUID_WARN, "sequencer: scale NaN\n");
            return FLUID_FAILED;
        }

        if(scale <= 0)
        {
            FLUID_LOG(FLUID_WARN, "sequencer: scale <= 0 : %f\n", scale);
            return FLUID_FAILED;
        }

        this->scale = scale;
        this->startMs = this->currentMs;
        this->start_ticks = this->cur_ticks;
        
        return FLUID_OK;
    }
    
    int process(unsigned int msec)
    {
        this->currentMs = msec;
        this->cur_ticks = this->get_tick(msec);

        std::lock_guard<std::recursive_mutex> l(this->mutex);
        fluid_seq_queue_process(this->queue, this, this->cur_ticks);
        
        return FLUID_OK;
    }
};


/* API implementation */

/**
 * Create a new sequencer object which uses the system timer.
 *
 * @return New sequencer instance
 *
 * Use new_fluid_sequencer2() to specify whether the system timer or
 * fluid_sequencer_process() is used to advance the sequencer.
 *
 * @deprecated As of fluidsynth 2.1.1 the use of the system timer has been deprecated.
 */
fluid_sequencer_t *
new_fluid_sequencer(void)
{
    return new_fluid_sequencer2(TRUE);
}

/**
 * Create a new sequencer object.
 *
 * @param use_system_timer If TRUE, sequencer will advance at the rate of the
 *   system clock. If FALSE, call fluid_sequencer_process() to advance
 *   the sequencer.
 * @return New sequencer instance
 *
 * @note As of fluidsynth 2.1.1 the use of the system timer has been deprecated.
 *
 * @since 1.1.0
 */
fluid_sequencer_t *
new_fluid_sequencer2(int use_system_timer)
{
    return guardedCall([&]
    {
        return new _fluid_sequencer_t(use_system_timer);
    },
    nullptr);
}

/**
 * Free a sequencer object.
 *
 * @param seq Sequencer to delete
 *
 * @note Before fluidsynth 2.1.1 registered sequencer clients may not be fully freed by this function.
 */
void
delete_fluid_sequencer(fluid_sequencer_t *seq)
{
    delete seq;
}

/**
 * Check if a sequencer is using the system timer or not.
 *
 * @param seq Sequencer object
 * @return TRUE if system timer is being used, FALSE otherwise.
 *
 * @deprecated As of fluidsynth 2.1.1 the usage of the system timer has been deprecated.
 *
 * @since 1.1.0
 */
int
fluid_sequencer_get_use_system_timer(fluid_sequencer_t *seq)
{
    fluid_return_val_if_fail(seq != NULL, FLUID_FAILED);
    return seq->useSystemTimer;
}


/* clients */

/**
 * Register a sequencer client.
 *
 * @param seq Sequencer object
 * @param name Name of sequencer client
 * @param callback Sequencer client callback or NULL for a source client.
 * @param data User data to pass to the \a callback
 * @return Unique sequencer ID or #FLUID_FAILED on error
 *
 * Clients can be sources or destinations of events. Sources don't need to
 * register a callback.
 *
 * @note Implementations are encouraged to explicitly unregister any registered client with fluid_sequencer_unregister_client() before deleting the sequencer.
 */
fluid_seq_id_t
fluid_sequencer_register_client(fluid_sequencer_t *seq, const char *name,
                                fluid_event_callback_t callback, void *data)
{
    fluid_return_val_if_fail(seq != NULL, FLUID_FAILED);
    return guardedCall([&]
    {
        return seq->register_client(name, callback, data);
    },
    FLUID_FAILED);
}

/**
 * Unregister a previously registered client.
 *
 * @param seq Sequencer object
 * @param id Client ID as returned by fluid_sequencer_register_client().
 *
 * The client's callback function will receive a FLUID_SEQ_UNREGISTERING event right before it is being unregistered.
 */
void
fluid_sequencer_unregister_client(fluid_sequencer_t *seq, fluid_seq_id_t id)
{
    fluid_return_if_fail(seq != NULL);
    (void)guardedCall([&]
    {
        return seq->unregister_client(id);
    },
    FLUID_FAILED);
}

/**
 * Count a sequencers registered clients.
 *
 * @param seq Sequencer object
 * @return Count of sequencer clients.
 */
int
fluid_sequencer_count_clients(fluid_sequencer_t *seq)
{
    fluid_return_val_if_fail(seq != NULL, FLUID_FAILED);
    return seq->clients.size();
}

/**
 * Get a client ID from its index (order in which it was registered).
 *
 * @param seq Sequencer object
 * @param index Index of register client
 * @return Client ID or #FLUID_FAILED if not found
 */
fluid_seq_id_t fluid_sequencer_get_client_id(fluid_sequencer_t *seq, int index)
{
    fluid_return_val_if_fail(seq != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(0 <= index && index < fluid_sequencer_count_clients(seq), FLUID_FAILED);

    return seq->clients[index]->id;
}

/**
 * Get the name of a registered client.
 *
 * @param seq Sequencer object
 * @param id Client ID
 * @return Client name or NULL if not found.  String is internal and should not
 *   be modified or freed.
 */
const char *
fluid_sequencer_get_client_name(fluid_sequencer_t *seq, fluid_seq_id_t id)
{
    fluid_return_val_if_fail(seq != NULL, NULL);
    return guardedCall([&]
    {
        return seq->get_client_name(id);
    },
    nullptr);
}

/**
 * Check if a client is a destination client, i.e. whether is has a callback function registered or not.
 *
 * @param seq Sequencer object
 * @param id Client ID
 * @return TRUE if client is a destination client, FALSE otherwise or if not found
 */
int
fluid_sequencer_client_is_dest(fluid_sequencer_t *seq, fluid_seq_id_t id)
{
    fluid_return_val_if_fail(seq != NULL, false);
    return guardedCall([&]
    {
        return seq->client_is_dest(id);
    },
    false);
}

/**
 * Send an event immediately.
 *
 * @param seq Sequencer object
 * @param evt Event to send (not copied, used directly)
 */
void
fluid_sequencer_send_now(fluid_sequencer_t *seq, fluid_event_t *evt)
{
    fluid_return_if_fail(seq != NULL);
    fluid_return_if_fail(evt != NULL);

    (void)guardedCall([&]
    {
        return seq->send_now(evt);
    },
    FLUID_FAILED);
}


/**
 * Schedule an event for sending at a later time.
 *
 * @param seq Sequencer object
 * @param evt Event to send (will be copied into internal queue)
 * @param time Time value in ticks (in milliseconds with the default time scale of 1000).
 * @param absolute TRUE if \a time is absolute sequencer time (time since sequencer
 *   creation), FALSE if relative to current time.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * @note The sequencer sorts events according to their timestamp \c time. For events that have
 * the same timestamp, fluidsynth (as of version 2.2.0) uses the following order, according to
 * which events will be dispatched to the client's callback function.
 *  - #FLUID_SEQ_SYSTEMRESET events precede any other event type.
 *  - #FLUID_SEQ_UNREGISTERING events succeed #FLUID_SEQ_SYSTEMRESET and precede other event type.
 *  - #FLUID_SEQ_NOTEON and #FLUID_SEQ_NOTE events succeed any other event type.
 *  - Otherwise the order is undefined.
 * \n
 * Or mathematically: #FLUID_SEQ_SYSTEMRESET < #FLUID_SEQ_UNREGISTERING < ... < (#FLUID_SEQ_NOTEON && #FLUID_SEQ_NOTE)
 *
 * @warning Be careful with relative ticks when sending many events! See #fluid_event_callback_t for details.
 */
int
fluid_sequencer_send_at(fluid_sequencer_t *seq, fluid_event_t *evt,
                        unsigned int time, int absolute)
{
    fluid_return_val_if_fail(seq != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(evt != NULL, FLUID_FAILED);

    return guardedCall([&]
    {
        return seq->send_at(evt, time, absolute);
    },
    FLUID_FAILED);
}

/**
 * Remove events from the event queue.
 *
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

    std::lock_guard<std::recursive_mutex> l(seq->mutex);    
    fluid_seq_queue_remove(seq->queue, source, dest, type);
}


/*************************************
	time
**************************************/


/**
 * Get the current tick of the sequencer scaled by the time scale currently set.
 *
 * @param seq Sequencer object
 * @return Current tick value
 */
unsigned int
fluid_sequencer_get_tick(fluid_sequencer_t *seq)
{
    fluid_return_val_if_fail(seq != NULL, 0u);
    return guardedCall([&]
    {
        return seq->get_tick();
    },
    0u);
}

/**
 * Set the time scale of a sequencer.
 *
 * @param seq Sequencer object
 * @param scale Sequencer scale value in ticks per second
 *   (default is 1000 for 1 tick per millisecond)
 *
 * If there are already scheduled events in the sequencer and the scale is changed
 * the events are adjusted accordingly.
 *
 * @note May only be called from a sequencer callback or initially when no event dispatching happens.
 * Otherwise it will mess up your event timing, because you have zero control over which events are
 * affected by the scale change.
 */
void
fluid_sequencer_set_time_scale(fluid_sequencer_t *seq, double scale)
{
    fluid_return_if_fail(seq != NULL);
    (void)guardedCall([&]
    {
        return seq->set_time_scale(scale);
    },
    FLUID_FAILED);
}

/**
 * Get a sequencer's time scale.
 *
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
 * Advance a sequencer.
 *
 * @param seq Sequencer object
 * @param msec Time to advance sequencer to (absolute time since sequencer start).
 *
 * If you have registered the synthesizer as client (fluid_sequencer_register_fluidsynth()), the synth
 * will take care of calling fluid_sequencer_process(). Otherwise it is up to the user to
 * advance the sequencer manually.
 *
 * @since 1.1.0
 */
void
fluid_sequencer_process(fluid_sequencer_t *seq, unsigned int msec)
{
    fluid_return_if_fail(seq != NULL);
    (void)guardedCall([&]
    {
        return seq->process(msec);
    },
    FLUID_FAILED);
}


/**
 * @internal
 * only used privately by fluid_seqbind and only from sequencer callback, thus lock acquire is not needed.
 */
void fluid_sequencer_invalidate_note(fluid_sequencer_t *seq, fluid_seq_id_t dest, fluid_note_id_t id)
{
    fluid_seq_queue_invalidate_note_private(seq->queue, dest, id);
}
