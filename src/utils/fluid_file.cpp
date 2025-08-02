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

#include "fluid_file.h"

#if HAVE_CXX_FILESYSTEM
#include <filesystem>
#endif

#if OSAL_glib || OSAL_cpp11
bool fluid_file_test(const char *path, int flags)
{
#if OSAL_glib
    return g_file_test(path, static_cast<GFileTest>(flags));
#elif OSAL_cpp11 && HAVE_CXX_FILESYSTEM
    try
    {
        std::filesystem::path _path = std::filesystem::u8path(path);
        if ((flags & FLUID_FILE_TEST_EXISTS) != 0)
            return std::filesystem::exists(_path);
        if ((flags & FLUID_FILE_TEST_IS_REGULAR) != 0)
            return std::filesystem::is_regular_file(_path);
    }
    catch (...)
    {
    }

    return false;
#else
    FLUID_LOG(FLUID_ERR, "fluid_file_test is unavailable, returning true");
    return true;
#endif
}
#endif

