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

/* Author: Markus Nentwig, nentwig@users.sourceforge.net
 */

#ifndef _FLUID_LADSPA_H
#define _FLUID_LADSPA_H

#include "fluid_sys.h"
#include "fluidsynth_priv.h"

#ifdef LADSPA
#include <ladspa.h>

#define FLUID_LADSPA_MAX_LIBS 100
#define FLUID_LADSPA_MAX_PLUGINS 100
#define FLUID_LADSPA_MAX_NODES 100
#define FLUID_LADSPA_MAX_PATH_LENGTH 512

typedef enum _fluid_ladspa_state_t {
    FLUID_LADSPA_INACTIVE = 0,
    FLUID_LADSPA_ACTIVE,
    FLUID_LADSPA_RUNNING
    
} fluid_ladspa_state_t;

typedef enum _fluid_ladspa_dir_t {
    FLUID_LADSPA_INPUT,
    FLUID_LADSPA_OUTPUT,
    FLUID_LADSPA_FIXED

} fluid_ladspa_dir_t;

typedef enum _fluid_ladspa_node_type_t {
    FLUID_LADSPA_NODE_AUDIO,
    FLUID_LADSPA_NODE_CONTROL,

} fluid_ladspa_node_type_t;

typedef struct _fluid_ladspa_lib_t
{
    char *filename;
    void *dlib;
    LADSPA_Descriptor_Function descriptor;

} fluid_ladspa_lib_t;

typedef struct _fluid_ladspa_port_state_t
{
    int num_inputs;
    int num_outputs;

} fluid_ladspa_port_state_t;

typedef struct _fluid_ladspa_plugin_t
{
    /* plugin instance id unique to the effects unit */
    int id;

    const LADSPA_Descriptor *desc;
    LADSPA_Handle *handle;

    int active;

    /* Used to keep track of the port connection states */
    fluid_ladspa_port_state_t *ports;

} fluid_ladspa_plugin_t;

typedef struct _fluid_ladspa_node_t
{
    char *name;
    fluid_ladspa_node_type_t type;
    LADSPA_Data *buf;

    int num_inputs;
    int num_outputs;

} fluid_ladspa_node_t;

typedef struct _fluid_ladspa_fx_t
{
    unsigned long sample_rate;

    int audio_groups;
    int effects_channels;
    int audio_channels;

    fluid_ladspa_lib_t *libs[FLUID_LADSPA_MAX_LIBS];
    int num_libs;

    fluid_ladspa_node_t *nodes[FLUID_LADSPA_MAX_NODES];
    int num_nodes;

    /* plugins are really plugin instances */
    fluid_ladspa_plugin_t *plugins[FLUID_LADSPA_MAX_PLUGINS];
    int num_plugins;

    /* used to generate the unique plugin ids */
    int next_plugin_id;

    fluid_rec_mutex_t api_mutex;

    int state;
    int pending_deactivation;

    fluid_cond_mutex_t *run_finished_mutex;
    fluid_cond_t *run_finished_cond;

} fluid_ladspa_fx_t;


fluid_ladspa_fx_t *new_fluid_ladspa_fx(fluid_real_t sample_rate, int audio_groups, int effects_channels, int audio_channels);
void delete_fluid_ladspa_fx(fluid_ladspa_fx_t *fx);
int fluid_ladspa_set_sample_rate(fluid_ladspa_fx_t *fx, fluid_real_t sample_rate);

int fluid_ladspa_is_active(fluid_ladspa_fx_t *fx);
int fluid_ladspa_activate(fluid_ladspa_fx_t *fx);
int fluid_ladspa_deactivate(fluid_ladspa_fx_t *fx);
int fluid_ladspa_reset(fluid_ladspa_fx_t *fx);

void fluid_ladspa_run(fluid_ladspa_fx_t *fx, fluid_real_t *left_buf[], fluid_real_t *right_buf[],
                      fluid_real_t *fx_left_buf[], fluid_real_t *fx_right_buf[]);

int fluid_ladspa_add_plugin(fluid_ladspa_fx_t *fx, const char *lib_name, const char *plugin_name);
int fluid_ladspa_port_exists(fluid_ladspa_fx_t *fx, int plugin_id, const char *name);

int fluid_ladspa_add_audio_node(fluid_ladspa_fx_t *fx, const char *name);
int fluid_ladspa_add_control_node(fluid_ladspa_fx_t *fx, const char *name, fluid_real_t val);
int fluid_ladspa_set_control_node(fluid_ladspa_fx_t *fx, const char *name, fluid_real_t val);
int fluid_ladspa_node_exists(fluid_ladspa_fx_t *fx, const char *name);

int fluid_ladspa_connect(fluid_ladspa_fx_t *fx, int plugin_id, fluid_ladspa_dir_t dir,
                         const char *port_name, const char *node_name);
int fluid_ladspa_check(fluid_ladspa_fx_t *fx, char *err, int err_size);
int fluid_ladspa_control_defaults(fluid_ladspa_fx_t *fx);

#endif /* LADSPA */
#endif /* _FLUID_LADSPA_H */
