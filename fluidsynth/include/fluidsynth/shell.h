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

#ifndef _FLUIDSYNTH_SHELL_H
#define _FLUIDSYNTH_SHELL_H


#ifdef __cplusplus
extern "C" {
#endif


/*
 *
 *   Shell interface
 *
 *   The shell interface allows you to send simple textual commands to
 *   the synthesizer, to parse a command file, or to read commands
 *   from the stdin or other input streams.
 *
 *   To find the list of currently supported commands, please check the
 *   fluid_cmd.c file.
 *   
 */

FLUIDSYNTH_API fluid_istream_t fluid_get_stdin(void);
FLUIDSYNTH_API fluid_ostream_t fluid_get_stdout(void);

FLUIDSYNTH_API char* fluid_get_userconf(char* buf, int len);
FLUIDSYNTH_API char* fluid_get_sysconf(char* buf, int len);


/** The command structure */

typedef int (*fluid_cmd_func_t)(void* data, int ac, char** av, fluid_ostream_t out);  

typedef struct {
  char* name;                           /** The name of the command, as typed in in the shell */
  char* topic;                          /** The help topic group of this command */ 
  fluid_cmd_func_t handler;              /** Pointer to the handler for this command */
  void* data;                           /** Pointer to the user data */
  char* help;                           /** A help string */
} fluid_cmd_t;


/** The command handler */

/**
    Create a new command handler. If the synth object passed as
    argument is not NULL, the handler will add all the default
    synthesizer commands to the command list.

    \param synth The synthesizer object
    \returns A new command handler
*/
FLUIDSYNTH_API 
fluid_cmd_handler_t* new_fluid_cmd_handler(fluid_synth_t* synth);

FLUIDSYNTH_API 
void delete_fluid_cmd_handler(fluid_cmd_handler_t* handler);

FLUIDSYNTH_API 
void fluid_cmd_handler_set_synth(fluid_cmd_handler_t* handler, fluid_synth_t* synth);

/**
    Register a new command to the handler. The handler makes a private
    copy of the 'cmd' structure passed as argument.

    \param handler A pointer to the command handler
    \param cmd A pointer to the command structure
    \returns 0 if the command was inserted, non-zero if error
*/
FLUIDSYNTH_API 
int fluid_cmd_handler_register(fluid_cmd_handler_t* handler, fluid_cmd_t* cmd);

FLUIDSYNTH_API 
int fluid_cmd_handler_unregister(fluid_cmd_handler_t* handler, char* cmd);


/** Command function */

FLUIDSYNTH_API 
int fluid_command(fluid_cmd_handler_t* handler, char* cmd, fluid_ostream_t out);

FLUIDSYNTH_API 
int fluid_source(fluid_cmd_handler_t* handler, char* filename);

FLUIDSYNTH_API 
void fluid_usershell(fluid_settings_t* settings, fluid_cmd_handler_t* handler);


/** Shell */

FLUIDSYNTH_API 
fluid_shell_t* new_fluid_shell(fluid_settings_t* settings, fluid_cmd_handler_t* handler,
			     fluid_istream_t in, fluid_ostream_t out, int thread);

FLUIDSYNTH_API void delete_fluid_shell(fluid_shell_t* shell);



/** TCP/IP server */

typedef fluid_cmd_handler_t* (*fluid_server_newclient_func_t)(void* data, char* addr);

FLUIDSYNTH_API 
fluid_server_t* new_fluid_server(fluid_settings_t* settings, 
			       fluid_server_newclient_func_t func,
			       void* data);

FLUIDSYNTH_API void delete_fluid_server(fluid_server_t* server);

FLUIDSYNTH_API int fluid_server_join(fluid_server_t* server);


#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_SHELL_H */
