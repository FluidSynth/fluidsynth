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

#ifndef _FLUIDSYNTH_LOG_H
#define _FLUIDSYNTH_LOG_H


#ifdef __cplusplus
extern "C" {
#endif


  /**
   *
   *  Logging interface
   *
   *  The default logging function of the fluidsynth prints its messages
   *  to the stderr. The synthesizer uses four level of messages: FLUID_PANIC,
   *  ERR, WARN, and FLUID_DBG. They are commented in the definition below.
   *
   *  A client application can install a new log function to handle the
   *  messages differently. In the following example, the application
   *  sets a callback function to display "FLUID_PANIC" messages in a dialog,
   *  and ignores all other messages by setting the log function to
   *  NULL:
   *
   *  ...
   *  fluid_set_log_function(FLUID_PANIC, show_dialog, (void*) root_window);
   *  fluid_set_log_function(ERR, NULL, NULL);
   *  fluid_set_log_function(WARN, NULL, NULL);
   *  fluid_set_log_function(FLUID_DBG, NULL, NULL);
   *  ...
   *
   */

enum fluid_log_level { 
  FLUID_PANIC,   /* the synth can't function correctly any more */
  FLUID_ERR,     /* the synth can function, but with serious limitation */
  FLUID_WARN,    /* the synth might not function as expected */
  FLUID_INFO,    /* verbose messages */
  FLUID_DBG,     /* debugging messages */
  LAST_LOG_LEVEL
};

typedef void (*fluid_log_function_t)(int level, char* message, void* data);

  /** fluid_set_log_function installs a new log function for the
   *  specified level. It returns the previously installed function. 
   */
FLUIDSYNTH_API 
fluid_log_function_t fluid_set_log_function(int level, fluid_log_function_t fun, void* data);

  
  /** fluid_default_log_function is the fluid's default log function. It
   *  prints to the stderr. */
FLUIDSYNTH_API void fluid_default_log_function(int level, char* message, void* data);

  /** print a message to the log */
FLUIDSYNTH_API int fluid_log(int level, char * fmt, ...);




#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_LOG_H */
