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


#ifndef FLUIDSYNTH_NOT_A_DLL
BOOL WINAPI DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  FLUID_LOG(FLUID_DBG, "DllMain");
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
      fluid_set_hinstance((void*) hModule);
    break;
  }
  return TRUE;
}
#endif

/**
 * Set the handle to the instance of the application on the Windows platform.
 * @param Application instance pointer
 *
 * The handle is needed to open DirectSound.
 * 
 * @deprecated As of 1.1.9 DirectSound driver uses the desktop window handle, making this function redundant.
 */
void fluid_set_hinstance(void* hinstance)
{
  if (fluid_hinstance == NULL) {
    fluid_hinstance = (HINSTANCE) hinstance;
    FLUID_LOG(FLUID_DBG, "DLL instance = %d", (int) fluid_hinstance);
  }
}

/**
 * Get the handle to the instance of the application on the Windows platform.
 * @return Application instance pointer or NULL if not set
 * 
 * @deprecated As of 1.1.9 DirectSound driver uses the desktop window handle, making this function redundant.
 */
void* fluid_get_hinstance(void)
{
  return (void*) fluid_hinstance;
}
