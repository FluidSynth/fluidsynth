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

#include "fluid_sys.h"

#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <new>
#include <thread>

static std::mutex atomic_lock;
fluid_mutex_t _atomic_lock = &atomic_lock;

static thread_local std::map<fluid_private_t, void *> private_data;


void fluid_msleep(unsigned int msecs)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(msecs));
}

double fluid_utime()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(now).count();
}

static void thread_wrapper(fluid_thread_func_t func, void *data)
{
    try
    {
        func(data);
    }
    catch (...)
    {
        FLUID_LOG(FLUID_ERR, "Exception thrown in thread function");
    }
}

fluid_thread_t *
new_fluid_thread(const char *name, fluid_thread_func_t func, void *data, int prio_level, int detach)
{
    if (func == nullptr)
        return nullptr;

    fluid_thread_info_t *info = nullptr;

    try
    {
        if (prio_level > 0)
        {
            info = new fluid_thread_info_t;
            info->func = func;
            info->data = data;
            info->prio_level = prio_level;

            func = fluid_thread_high_prio;
            data = info;
        }

        std::thread *thread = new std::thread(thread_wrapper, func, data);
        if (detach)
            thread->detach();

        return thread;
    }
    catch (const std::bad_alloc &)
    {
        FLUID_LOG(FLUID_PANIC, "Out of memory on thread allocation");
    }
    catch (...)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create thread");
    }

    delete info;
    return nullptr;
}

void delete_fluid_thread(fluid_thread_t *_thread)
{
    std::thread *thread = static_cast<std::thread *>(_thread);
    if (thread->joinable())
    {
        if (thread->get_id() == std::this_thread::get_id())
        {
            // Thread is deleting itself, detach from the thread object
            thread->detach();
        }
        else
        {
            FLUID_LOG(FLUID_ERR, "deleting thread that is still joinable");
        }
    }

    delete thread;
}

int fluid_thread_join(fluid_thread_t *thread)
{
    static_cast<std::thread *>(thread)->join();
    return FLUID_OK;
}

void _fluid_mutex_init(fluid_mutex_t *mutex)
{
    *mutex = new(std::nothrow) std::mutex();
    if (*mutex == nullptr)
        FLUID_LOG(FLUID_PANIC, "Out of memory on mutex allocation");
}

void fluid_mutex_destroy(fluid_mutex_t mutex)
{
    delete static_cast<std::mutex *>(mutex);
}

template<class T>
static void ensure_lock_mutex(T *mutex)
{
    do
    {
        try
        {
            mutex->lock();
        }
        catch (...)
        {
            continue;
        }
    }
    while (false);
}

void _fluid_mutex_lock(fluid_mutex_t *mutex)
{
    if (*mutex == nullptr)
    {
        // First use of a statically initialized mutex
        fluid_mutex_lock(_atomic_lock);
        if (*mutex == nullptr)
            _fluid_mutex_init(mutex);
        fluid_mutex_unlock(_atomic_lock);
    }

    ensure_lock_mutex(static_cast<std::mutex *>(*mutex));
}

void fluid_mutex_unlock(fluid_mutex_t mutex)
{
    static_cast<std::mutex *>(mutex)->unlock();
}

void _fluid_rec_mutex_init(fluid_rec_mutex_t *mutex)
{
    *mutex = new(std::nothrow) std::recursive_mutex();
    if (*mutex == nullptr)
        FLUID_LOG(FLUID_PANIC, "Out of memory on recursive mutex allocation");
}

void fluid_rec_mutex_destroy(fluid_rec_mutex_t mutex)
{
    delete static_cast<std::recursive_mutex *>(mutex);
}

void fluid_rec_mutex_lock(fluid_rec_mutex_t mutex)
{
    ensure_lock_mutex(static_cast<std::recursive_mutex *>(mutex));
}

void fluid_rec_mutex_unlock(fluid_rec_mutex_t mutex)
{
    static_cast<std::recursive_mutex *>(mutex)->unlock();
}

void fluid_cond_mutex_lock(fluid_cond_mutex_t *mutex)
{
    ensure_lock_mutex(static_cast<std::mutex *>(mutex));
}

void fluid_cond_mutex_unlock(fluid_cond_mutex_t *mutex)
{
    static_cast<std::mutex *>(mutex)->unlock();
}

fluid_cond_mutex_t *new_fluid_cond_mutex(void)
{
    std::mutex *mutex = new(std::nothrow) std::mutex();
    if (mutex == nullptr)
        FLUID_LOG(FLUID_PANIC, "Out of memory on condition mutex allocation");
    return mutex;
}

void delete_fluid_cond_mutex(fluid_cond_mutex_t *mutex)
{
    delete static_cast<std::mutex *>(mutex);
}

void fluid_cond_signal(fluid_cond_t cond)
{
    static_cast<std::condition_variable *>(cond)->notify_one();
}

void fluid_cond_broadcast(fluid_cond_t cond)
{
    static_cast<std::condition_variable *>(cond)->notify_all();
}

void fluid_cond_wait(fluid_cond_t cond, fluid_cond_mutex_t *mutex)
{
    std::unique_lock<std::mutex> lock(*static_cast<std::mutex *>(mutex), std::adopt_lock);
    static_cast<std::condition_variable *>(cond)->wait(lock);
    lock.release();
}

fluid_cond_t new_fluid_cond(void)
{
    std::condition_variable *cond = new(std::nothrow) std::condition_variable();
    if (cond == nullptr)
        FLUID_LOG(FLUID_PANIC, "Out of memory on condition variable allocation");
    return cond;
}

void delete_fluid_cond(fluid_cond_t cond)
{
    delete static_cast<std::condition_variable *>(cond);
}

void _fluid_private_init(fluid_private_t *priv)
{
    *priv = priv;
}

void fluid_private_free(fluid_private_t priv)
{
    private_data.erase(priv);
}

void *fluid_private_get(fluid_private_t priv)
{
    return private_data[priv];
}

void fluid_private_set(fluid_private_t priv, void *value)
{
    private_data[priv] = value;
}
