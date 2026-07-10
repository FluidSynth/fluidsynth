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

/*
 * Platform-portable dynamic module loading.
 * On Windows: LoadLibraryW / GetProcAddress / FreeLibrary
 * On POSIX:   dlopen / dlsym / dlclose
 */

#include "fluid_module.h"
#include "fluid_sys.h"

#include <string>

#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#else
#   include <dlfcn.h>
#endif

struct _fluid_module_t
{
    std::string name; /* original path supplied to fluid_module_open */

#if defined(_WIN32)
    HMODULE handle;
#else
    void *handle;
#endif
};

/* Per-thread last-error string, mirroring gmodule's behaviour */
static thread_local std::string fluid_module_last_error;

static void set_module_error(std::string msg)
{
    fluid_module_last_error = std::move(msg);
}

/* ------------------------------------------------------------------ */
/*  Public API                                                          */
/* ------------------------------------------------------------------ */

fluid_module_t *fluid_module_open(const char *name)
{
    if(name == nullptr)
    {
        set_module_error("fluid_module_open: NULL file name");
        return nullptr;
    }

#if defined(_WIN32)
    /* Convert UTF-8 path to UTF-16 for LoadLibraryW */
    int wlen = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
    if(wlen <= 0)
    {
        set_module_error(std::string("MultiByteToWideChar failed: ") + fluid_get_windows_error());
        return nullptr;
    }

    std::wstring wpath(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, name, -1, &wpath[0], wlen);

    /* Suppress the "not found" error dialog box */
    DWORD old_mode = 0;
    SetThreadErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS, &old_mode);
    HMODULE handle = LoadLibraryW(wpath.c_str());
    SetThreadErrorMode(old_mode, nullptr);

    if(handle == nullptr)
    {
        set_module_error(std::string("LoadLibraryW('") + name + "'): " + fluid_get_windows_error());
        return nullptr;
    }
#else
    void *handle = dlopen(name, RTLD_LOCAL | RTLD_NOW);
    if(handle == nullptr)
    {
        const char *err = dlerror();
        set_module_error(err ? err : "dlopen failed (unknown error)");
        return nullptr;
    }
    /* consume the error state */
    dlerror();
#endif

    fluid_module_t *mod = new(std::nothrow) fluid_module_t;
    if(mod == nullptr)
    {
        set_module_error("Out of memory");
#if defined(_WIN32)
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return nullptr;
    }

    mod->name   = name;
    mod->handle = handle;
    fluid_module_last_error.clear();
    return mod;
}

int fluid_module_close(fluid_module_t *mod)
{
    if(mod == nullptr)
        return 0;

    int rc = 0;

#if defined(_WIN32)
    if(!FreeLibrary(mod->handle))
    {
        set_module_error(std::string("FreeLibrary failed: ") + fluid_get_windows_error());
        rc = -1;
    }
#else
    if(dlclose(mod->handle) != 0)
    {
        const char *err = dlerror();
        set_module_error(err ? err : "dlclose failed (unknown error)");
        rc = -1;
    }
    else
    {
        dlerror(); /* clear */
    }
#endif

    delete mod;
    return rc;
}

const char *fluid_module_error(void)
{
    return fluid_module_last_error.c_str();
}

const char *fluid_module_name(fluid_module_t *mod)
{
    if(mod == nullptr)
        return nullptr;
    return mod->name.c_str();
}

int fluid_module_symbol(fluid_module_t *mod, const char *name, void **ptr)
{
    if(mod == nullptr || name == nullptr || ptr == nullptr)
    {
        set_module_error("fluid_module_symbol: invalid argument");
        return 0;
    }

#if defined(_WIN32)
    FARPROC sym = GetProcAddress(mod->handle, name);
    if(sym == nullptr)
    {
        set_module_error(std::string("GetProcAddress('") + name + "'): " + fluid_get_windows_error());
        return 0;
    }
    *ptr = reinterpret_cast<void *>(sym);
#else
    dlerror(); /* clear any previous error */
    void *sym = dlsym(mod->handle, name);
    const char *err = dlerror();
    if(err != nullptr)
    {
        set_module_error(err);
        return 0;
    }
    *ptr = sym;
#endif

    fluid_module_last_error.clear();
    return 1;
}
