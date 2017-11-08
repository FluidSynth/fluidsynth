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

/* fluid_wnd is the window handle will be use by the dsound driver if not NULL 
 See fluid_win32_set_window() below.
*/
static HWND fluid_wnd = NULL;  
HWND fluid_win32_get_window(void);
/* The class name "FluidSynth" */
const char fluid_window_class_name[] = ("FluidSynth");

#ifndef FLUIDSYNTH_NOT_A_DLL
/* Dll entry point */
BOOL WINAPI DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    FLUID_LOG(FLUID_DBG, "DllMain");
    switch (ul_reason_for_call) {
        /*- Process initialisation */
        case DLL_PROCESS_ATTACH:
	      /* register the window class */
	      {   WNDCLASS myClass;
		        ZeroMemory(&myClass, sizeof(myClass));
		        myClass.lpszClassName = fluid_window_class_name;
		        myClass.hInstance = hModule;
		        myClass.style = CS_GLOBALCLASS;
            /* window procedure for this class */
	          myClass.lpfnWndProc = DefWindowProc;
            /* window class registration */
		        if (! RegisterClass(&myClass))
            {
			         FLUID_LOG(FLUID_DBG, "Error DllMain");
       			   return FALSE;
            }
	      }
        break;
        /*- Process de-initialisation */
        case DLL_PROCESS_DETACH:
	          UnregisterClass(fluid_window_class_name,hModule);
        break;
    }
    return TRUE;
}
#endif

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
