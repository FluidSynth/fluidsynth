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

fluid_mutex_t _atomic_lock = new std::mutex();

static thread_local std::map<fluid_private_t, void *> _private_data;


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
        FLUID_LOG(FLUID_ERR, "Out of memory");
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
        FLUID_LOG(FLUID_ERR, "Out of memory on mutex allocation");
}

void fluid_mutex_destroy(fluid_mutex_t mutex)
{
    delete static_cast<std::mutex *>(mutex);
}

void _fluid_mutex_lock(fluid_mutex_t *mutex)
{
    if (*mutex == nullptr)
    {
        fluid_mutex_lock(_atomic_lock);
        if (*mutex == nullptr)
            _fluid_mutex_init(mutex);
        fluid_mutex_unlock(_atomic_lock);
    }

    static_cast<std::mutex *>(*mutex)->lock();
}

void fluid_mutex_unlock(fluid_mutex_t mutex)
{
    static_cast<std::mutex *>(mutex)->unlock();
}

void _fluid_rec_mutex_init(fluid_rec_mutex_t *mutex)
{
    *mutex = new(std::nothrow) std::recursive_mutex();
    if (*mutex == nullptr)
        FLUID_LOG(FLUID_ERR, "Out of memory on recursive mutex allocation");
}

void fluid_rec_mutex_destroy(fluid_rec_mutex_t mutex)
{
    delete static_cast<std::recursive_mutex *>(mutex);
}

void fluid_rec_mutex_lock(fluid_rec_mutex_t mutex)
{
    static_cast<std::recursive_mutex *>(mutex)->lock();
}

void fluid_rec_mutex_unlock(fluid_rec_mutex_t mutex)
{
    static_cast<std::recursive_mutex *>(mutex)->unlock();
}

void fluid_cond_mutex_lock(fluid_cond_mutex_t *mutex)
{
    static_cast<std::mutex *>(mutex)->lock();
}

void fluid_cond_mutex_unlock(fluid_cond_mutex_t *mutex)
{
    static_cast<std::mutex *>(mutex)->unlock();
}

fluid_cond_mutex_t *new_fluid_cond_mutex(void)
{
    std::mutex *mutex = new(std::nothrow) std::mutex();
    if (mutex == nullptr)
        FLUID_LOG(FLUID_ERR, "Out of memory on condition mutex allocation");
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
    std::unique_lock<std::mutex> lock(*static_cast<std::mutex *>(mutex));
    static_cast<std::condition_variable *>(cond)->wait(lock);
}

fluid_cond_t new_fluid_cond(void)
{
    std::condition_variable *cond = new(std::nothrow) std::condition_variable();
    if (cond == nullptr)
        FLUID_LOG(FLUID_ERR, "Out of memory on condition variable allocation");
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
    _private_data.erase(priv);
}

void *fluid_private_get(fluid_private_t priv)
{
    return _private_data.find(priv)->second;
}

void fluid_private_set(fluid_private_t priv, void *value)
{
    _private_data.emplace(priv, value);
}

#if __cplusplus >= 201703L

#include <filesystem>

bool fluid_file_test(const char *path, int flags)
{
    try
    {
        if ((flags & FLUID_FILE_TEST_EXISTS) != 0)
            return std::filesystem::exists(path);
        if ((flags & FLUID_FILE_TEST_IS_REGULAR) != 0)
            return std::filesystem::is_regular_file(path);
    }
    catch (...)
    {
    }

    return false;
}

int fluid_stat(const char *path, fluid_stat_buf_t *buffer)
{
    try
    {
        auto mtime = std::filesystem::last_write_time(path).time_since_epoch();
        buffer->st_mtime = std::chrono::duration_cast<std::chrono::seconds>(mtime).count();
        return FLUID_OK;
    }
    catch (...)
    {
    }

    buffer->st_mtime = 0;
    return FLUID_FAILED;
}

#else

bool fluid_file_test(const char *path, int flags)
{
    FLUID_LOG(FLUID_ERR, "fluid_file_test is unavailable, returning true");
    return true;
}

int fluid_stat(const char *path, fluid_stat_buf_t *buffer)
{
    FLUID_LOG(FLUID_ERR, "fluid_stat is unavailable, returning -1");
    return -1;
}

#endif
