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

#ifdef WIN32
#include <windows.h>
#include "fluidsynth_priv.h"
#include "fluid_sys.h"

HINSTANCE fluid_hinstance = NULL;

#ifndef FLUIDSYNTH_NOT_A_DLL
BOOL WINAPI DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  FLUID_LOG(FLUID_DBG, "DllMain\n");
  fluid_set_hinstance((void*) hModule);
  return TRUE;
}
#endif

void fluid_set_hinstance(void* hinstance)
{
  if (fluid_hinstance == NULL) {
    fluid_hinstance = (HINSTANCE) hinstance;
    FLUID_LOG(FLUID_DBG, "DLL instance = %d\n", (int) fluid_hinstance);
  }	
}

void* fluid_get_hinstance(void)
{
  return (void*) fluid_hinstance;
}
#endif

