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
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */


#ifndef _FLUID_RVOICE_HANDLER_H
#define _FLUID_RVOICE_HANDLER_H

#include "fluid_rvoice.h"
#include "fluid_sys.h"

typedef struct _fluid_rvoice_handler_t fluid_rvoice_handler_t;

struct _fluid_rvoice_handler_t {
	fluid_rvoice_t** voices; /* Sorted so that all nulls are last */
	int polyphony; /* Length of voices array */
	int active_voices; /* Number of non-null voices */
#if 0
	fluid_rvoice_t** finished_voices; /* List of voices who have finished */
        int finished_voice_count;
#endif	
	void (*remove_voice_callback)(void*, fluid_rvoice_t*); /**< Recieve this callback every time a voice is removed */
	void* remove_voice_callback_userdata;
};

int fluid_rvoice_handler_add_voice(fluid_rvoice_handler_t* handler, fluid_rvoice_t* voice);
int fluid_rvoice_handler_set_polyphony(fluid_rvoice_handler_t* handler, int value); 

void fluid_rvoice_handler_render(fluid_rvoice_handler_t* handler, 
                                int blockcount, int bufcount, 
                                fluid_real_t** bufs);


static FLUID_INLINE void
fluid_rvoice_handler_set_voice_callback(
  fluid_rvoice_handler_t* handler,
  void (*func)(void*, fluid_rvoice_t*),
  void* userdata)
{
  handler->remove_voice_callback_userdata = userdata;
  handler->remove_voice_callback = func;
}
  
#if 0
static FLUID_INLINE fluid_rvoice_t**
fluid_rvoice_handler_get_finished_voices(fluid_rvoice_handler_t* handler,
                                           int* count)
{
  *count = handler->finished_voice_count;
  return handler->finished_voices;
}

static inline void 
fluid_rvoice_handler_clear_finished_voices(fluid_rvoice_handler_t* handler) 
{
  handler->finished_voice_count = 0;
}
#endif

fluid_rvoice_handler_t* new_fluid_rvoice_handler(void);
void delete_fluid_rvoice_handler(fluid_rvoice_handler_t* handler);

#endif

