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

#ifndef _FLUID_CMD_H
#define _FLUID_CMD_H

#include "fluidsynth_priv.h"


void fluid_shell_settings(fluid_settings_t* settings);


/** some help functions */
int fluid_is_number(char* a);
int fluid_is_empty(char* a);
char* fluid_expand_path(char* path, char* new_path, int len);

/** the handlers for the command lines */
int fluid_handle_help(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_quit(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_noteon(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_noteoff(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_pitch_bend(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_pitch_bend_range(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_cc(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_prog(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_select(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_inst(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_channels(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_load(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_unload(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_reload(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_fonts(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_mstat(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_reverbpreset(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_reverbsetroomsize(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_reverbsetdamp(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_reverbsetwidth(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_reverbsetlevel(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_chorusnr(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_choruslevel(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_chorusspeed(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_chorusdepth(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_chorus(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_reverb(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_gain(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_interp(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_interpc(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_tuning(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_tune(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_settuning(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_resettuning(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_tunings(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_dumptuning(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_reset(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_source(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_echo(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);

int fluid_handle_set(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_get(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_info(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_settings(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);

int fluid_handle_router_clear(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_router_default(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_router_begin(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_router_end(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_router_chan(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_router_par1(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);
int fluid_handle_router_par2(fluid_cmd_handler_t* handler, int ac, char** av, fluid_ostream_t out);


fluid_cmd_t* fluid_cmd_copy(fluid_cmd_t* cmd);
void delete_fluid_cmd(fluid_cmd_t* cmd);

int fluid_cmd_handler_handle(fluid_cmd_handler_t* handler,
			    int ac, char** av,
			    fluid_ostream_t out);



void fluid_server_remove_client(fluid_server_t* server, fluid_client_t* client);
void fluid_server_add_client(fluid_server_t* server, fluid_client_t* client);


fluid_client_t* new_fluid_client(fluid_server_t* server,
			       fluid_settings_t* settings,
			       fluid_socket_t sock);

void delete_fluid_client(fluid_client_t* client);
void fluid_client_quit(fluid_client_t* client);


#endif /* _FLUID_CMD_H */
