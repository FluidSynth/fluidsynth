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
#define FLUID_LADSPA_MAX_EFFECTS 100
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

} fluid_ladspa_dir_t;

typedef enum _fluid_ladspa_node_type_t {
    FLUID_LADSPA_NODE_AUDIO = 1,
    FLUID_LADSPA_NODE_CONTROL = 2,
    FLUID_LADSPA_NODE_EFFECT = 4,
    FLUID_LADSPA_NODE_HOST = 8,
    FLUID_LADSPA_NODE_USER = 16,

} fluid_ladspa_node_type_t;

typedef enum _fluid_ladspa_mode_t {
    FLUID_LADSPA_MODE_REPLACE = 0,
    FLUID_LADSPA_MODE_ADD,

} fluid_ladspa_mode_t;

typedef struct _fluid_ladspa_lib_t
{
    char *filename;
    void *dlib;
    LADSPA_Descriptor_Function descriptor;

} fluid_ladspa_lib_t;

typedef struct _fluid_ladspa_node_t
{
    char *name;
    fluid_ladspa_node_type_t type;

    /* The buffer that LADSPA effects read and/or write to.
     * If FluidSynth has been compiled WITH_FLOAT, then this
     * points to the host buffer, otherwise it's a separate
     * buffer. */
    LADSPA_Data *effect_buffer;

    /* Only set for host nodes, points to the host buffer */
    fluid_real_t *host_buffer;

    int num_inputs;
    int num_outputs;

} fluid_ladspa_node_t;

typedef struct _fluid_ladspa_effect_t
{
    char *name;

    const LADSPA_Descriptor *desc;
    LADSPA_Handle *handle;

    int active;

    /* Decides if to replace data in output buffer (default) or add to it */
    fluid_ladspa_mode_t mode;

    /* Used to keep track of the port connection state */
    fluid_ladspa_node_t **port_nodes;

} fluid_ladspa_effect_t;

typedef struct _fluid_ladspa_fx_t
{
    unsigned long sample_rate;

    int audio_groups;
    int effects_channels;
    int audio_channels;
    int buffer_size;

    fluid_ladspa_lib_t *libs[FLUID_LADSPA_MAX_LIBS];
    int num_libs;

    fluid_ladspa_node_t *nodes[FLUID_LADSPA_MAX_NODES];
    int num_nodes;

    /* Pointers to host nodes in nodes array */
    fluid_ladspa_node_t *host_nodes[FLUID_LADSPA_MAX_NODES];
    int num_host_nodes;

    /* Pointers to user audio nodes in nodes array */
    fluid_ladspa_node_t *audio_nodes[FLUID_LADSPA_MAX_NODES];
    int num_audio_nodes;

    fluid_ladspa_effect_t *effects[FLUID_LADSPA_MAX_EFFECTS];
    int num_effects;

    fluid_rec_mutex_t api_mutex;

    fluid_atomic_int_t state;
    int pending_deactivation;

    fluid_cond_mutex_t *run_finished_mutex;
    fluid_cond_t *run_finished_cond;

} fluid_ladspa_fx_t;


fluid_ladspa_fx_t *new_fluid_ladspa_fx(fluid_real_t sample_rate, int buffer_size);
void delete_fluid_ladspa_fx(fluid_ladspa_fx_t *fx);
int fluid_ladspa_add_host_buffers(fluid_ladspa_fx_t *fx, const char *prefix,
        int buffer_count, int buffer_size, fluid_real_t *left[], fluid_real_t *right[]);
int fluid_ladspa_set_sample_rate(fluid_ladspa_fx_t *fx, fluid_real_t sample_rate);
void fluid_ladspa_run(fluid_ladspa_fx_t *fx, int block_count, int block_size);

int fluid_ladspa_is_active(fluid_ladspa_fx_t *fx);
int fluid_ladspa_activate(fluid_ladspa_fx_t *fx);
int fluid_ladspa_deactivate(fluid_ladspa_fx_t *fx);
int fluid_ladspa_reset(fluid_ladspa_fx_t *fx);

int fluid_ladspa_add_effect(fluid_ladspa_fx_t *fx, const char *effect_name,
        const char *lib_name, const char *plugin_name);
int fluid_ladspa_effect_can_add(fluid_ladspa_fx_t *fx, const char *name);
int fluid_ladspa_set_effect_mode(fluid_ladspa_fx_t *fx, const char *name,
        fluid_ladspa_mode_t mode, float gain);
int fluid_ladspa_effect_port_exists(fluid_ladspa_fx_t *fx, const char *effect_name, const char *port_name);
int fluid_ladspa_host_port_exists(fluid_ladspa_fx_t *fx, const char *name);
int fluid_ladspa_buffer_exists(fluid_ladspa_fx_t *fx, const char *name);

int fluid_ladspa_add_buffer(fluid_ladspa_fx_t *fx, const char *name);
int fluid_ladspa_set_effect_control(fluid_ladspa_fx_t *fx, const char *effect_name,
        const char *port_name, float val);

int fluid_ladspa_connect(fluid_ladspa_fx_t *fx, const char *effect_name,
        const char *port_name, const char *name);
int fluid_ladspa_check(fluid_ladspa_fx_t *fx, char *err, int err_size);

#endif /* LADSPA */
#endif /* _FLUID_LADSPA_H */
