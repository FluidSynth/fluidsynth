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

#ifdef WIN32
#include "fluidsynth_priv.h"
#include "fluid_sys.h"

static HINSTANCE fluid_hinstance = NULL;
static HWND fluid_wnd = NULL;
HWND fluid_win32_get_window(void);

#ifndef FLUIDSYNTH_NOT_A_DLL
/* Dll entry point */
BOOL WINAPI DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  FLUID_LOG(FLUID_DBG, "DllMain");
  fluid_hinstance = hModule;
  return TRUE;
}
#endif


/**
 * Get the handle to the instance of the application on the Windows platform.
 * @return Application instance pointer or NULL if not set
 */
void* fluid_get_hinstance(void)
{
  return (void*) fluid_hinstance;
}

/* sets the application window handle
 * @param hwnd the window handle
 *
 * If an application set a window handle this window will
 * be used by dsound driver for cooperative use.
 *
 * It is not mandatory for an application to set a window handle
 * in this case dsound driver will create its own window.
 *
*/
void fluid_win32_set_window(HWND hwnd)
{
    fluid_wnd = hwnd;
}

HWND fluid_win32_get_window(void)
{
  return fluid_wnd;
}

#endif	// #ifdef WIN32
