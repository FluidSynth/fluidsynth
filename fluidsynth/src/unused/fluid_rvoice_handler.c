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


#include "fluid_rvoice_handler.h"


fluid_rvoice_handler_t* new_fluid_rvoice_handler(void)
{
  fluid_rvoice_handler_t* handler;

  handler = FLUID_NEW(fluid_rvoice_handler_t);
  if (handler == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  FLUID_MEMSET(handler, 0, sizeof(fluid_rvoice_handler_t));
  
  return handler;
}

void delete_fluid_rvoice_handler(fluid_rvoice_handler_t* handler)
{
  if (handler == NULL) 
    return;
  
#if 0
  FLUID_FREE(handler->finished_voices);
#endif
  FLUID_FREE(handler->voices);
  FLUID_FREE(handler);
}


int 
fluid_rvoice_handler_add_voice(fluid_rvoice_handler_t* handler, fluid_rvoice_t* voice)
{
  if (handler->active_voices >= handler->polyphony) {
    FLUID_LOG(FLUID_WARN, "Trying to exceed polyphony in fluid_rvoice_handler_add_voice");
    return FLUID_FAILED;
  }
  handler->voices[handler->active_voices++] = voice;
  return FLUID_OK;
}

/**
 * Update polyphony - max number of voices (NOTE: not hard real-time capable)
 * @return FLUID_OK or FLUID_FAILED
 */
int 
fluid_rvoice_handler_set_polyphony(fluid_rvoice_handler_t* handler, int value)
{
  void* newptr;
  if (handler->active_voices > value) 
    return FLUID_FAILED;
#if 0
  if (handler->finished_voice_count > value) 
    return FLUID_FAILED;
#endif

  newptr = FLUID_REALLOC(handler->voices, value * sizeof(fluid_rvoice_t*));
  if (newptr == NULL) 
    return FLUID_FAILED;
  handler->voices = newptr;
#if 0
  newptr = FLUID_REALLOC(handler->finished_voices, value * sizeof(fluid_rvoice_t*));
  if (newptr == NULL) 
    return FLUID_FAILED;
  handler->finished_voices = newptr;
#endif   

  handler->polyphony = value;
  return FLUID_OK;
}

static void
fluid_rvoice_handler_remove_voice(fluid_rvoice_handler_t* handler, int index)
{
#if 0
  if (handler->finished_voice_count < handler->polyphony)
    handler->finished_voices[handler->finished_voice_count++] = handler->voices[index];
#endif

  if (handler->remove_voice_callback != NULL)
    handler->remove_voice_callback(handler->remove_voice_callback_userdata, 
                                   handler->voices[index]);
  
  handler->active_voices--;
  if (index < handler->active_voices) /* Move the last voice into the "hole" */
    handler->voices[index] = handler->voices[handler->active_voices];
}

/**
 * Synthesize one voice
 * @return Number of samples written 
 */
#if 0
static inline int
fluid_rvoice_handler_write_one(fluid_rvoice_handler_t* handler, int index, 
                               fluid_real_t* buf, int blockcount)
{
  int i, result = 0;
  fluid_rvoice_t* voice = handler->voices[index];
  for (i=0; i < blockcount; i++) {
    int s = fluid_rvoice_write(voice, buf);
    if (s == -1) {
      FLUID_MEMSET(buf, 0, FLUID_BUFSIZE*sizeof(fluid_real_t));
      s = FLUID_BUFSIZE;
    }
    buf += s;
    result += s;
  }
  return result;
}
#endif 

/**
 * Synthesize one voice and add to buffer.
 * NOTE: If return value is less than blockcount*FLUID_BUFSIZE, that means 
 * voice has been finished, removed and possibly replaced with another voice.
 * @return Number of samples written 
 */
static inline int
fluid_rvoice_handler_mix_one(fluid_rvoice_handler_t* handler, int index, 
                               fluid_real_t** bufs, unsigned int blockcount, unsigned int bufcount)
{
  unsigned int i, j=0, result = 0;
  fluid_rvoice_t* voice = handler->voices[index];

  fluid_real_t local_buf[FLUID_BUFSIZE*blockcount];

  for (i=0; i < blockcount; i++) {
    int s = fluid_rvoice_write(voice, &local_buf[FLUID_BUFSIZE*i]);
    if (s == -1) {
      s = FLUID_BUFSIZE; /* Voice is quiet, TODO: optimize away memset/mix */
      FLUID_MEMSET(&local_buf[FLUID_BUFSIZE*i], 0, FLUID_BUFSIZE*sizeof(fluid_real_t*));
    } 
    result += s;
    if (s < FLUID_BUFSIZE) {
      j = 1;
      break;
    }
  }
  fluid_rvoice_buffers_mix(&voice->buffers, local_buf, result, bufs, bufcount);

  if (j)
    fluid_rvoice_handler_remove_voice(handler, index);

  return result;
}

static inline void
fluid_resetbufs(int blockcount, int bufcount, fluid_real_t** bufs)
{
  int i;
  for (i=0; i < bufcount; i++)
    FLUID_MEMSET(bufs[i], 0, blockcount * FLUID_BUFSIZE * sizeof(fluid_real_t));
}

/**
 * Single-threaded scenario, no worker threads 
 */
static inline void
fluid_rvoice_handler_render_loop_simple(fluid_rvoice_handler_t* handler, 
                            int blockcount, int bufcount, fluid_real_t** bufs)
{
  int i;
  int scount = blockcount * FLUID_BUFSIZE;
  for (i=0; i < handler->active_voices; i++) {
    int s = fluid_rvoice_handler_mix_one(handler, i, bufs, blockcount, bufcount);
    if (s < scount) i--; /* Need to render the moved voice as well */
  }
}


/**
 * @param blockcount number of samples to render is blockcount*FLUID_BUFSIZE
 * @param bufcount number of buffers to render into
 * @param bufs array of bufcount buffers, each containing blockcount*FLUID_BUFSIZE samples
 */
void 
fluid_rvoice_handler_render(fluid_rvoice_handler_t* handler, 
                            int blockcount, int bufcount, fluid_real_t** bufs)
{
  fluid_resetbufs(blockcount, bufcount, bufs);
  fluid_rvoice_handler_render_loop_simple(handler, blockcount, bufcount, bufs);
}
