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

/* This module: 3/2002
 * Author: Markus Nentwig, nentwig@users.sourceforge.net
 *
 * Complete rewrite: 10/2017
 * Author: Marcus Weseloh
 */

#include "fluidsynth_priv.h"

#ifdef LADSPA

#include "fluid_ladspa.h"
#include "fluid_sys.h"
#include <dlfcn.h>
#include <math.h>

#define LADSPA_API_ENTER(_fx) (fluid_rec_mutex_lock((_fx)->api_mutex))

#define LADSPA_API_RETURN(_fx, _ret)          \
    fluid_rec_mutex_unlock((_fx)->api_mutex); \
    return (_ret)

static void clear_ladspa(fluid_ladspa_fx_t *fx);

static fluid_ladspa_node_t *new_fluid_ladspa_node(fluid_ladspa_fx_t *fx, const char *name,
                                                  fluid_ladspa_node_type_t type, int buf_size);
static void delete_fluid_ladspa_node(fluid_ladspa_node_t *node);
static int create_input_output_nodes(fluid_ladspa_fx_t *fx);
static fluid_ladspa_node_t *get_node(fluid_ladspa_fx_t *fx, const char *name);

static int get_plugin_port_idx(const fluid_ladspa_plugin_t *plugin, const char *name);
static const LADSPA_Descriptor *get_plugin_descriptor(const fluid_ladspa_lib_t *lib, const char *name);

static fluid_ladspa_plugin_t *
new_fluid_ladspa_plugin(fluid_ladspa_fx_t *fx, const fluid_ladspa_lib_t *lib, const char *name);
static void delete_fluid_ladspa_plugin(fluid_ladspa_plugin_t *plugin);
static void activate_plugin(fluid_ladspa_plugin_t *plugin);
static void deactivate_plugin(fluid_ladspa_plugin_t *plugin);
static fluid_ladspa_plugin_t *get_plugin_by_id(fluid_ladspa_fx_t *fx, int id);

static fluid_ladspa_lib_t *new_fluid_ladspa_lib(fluid_ladspa_fx_t *fx, const char *filename);
static void delete_fluid_ladspa_lib(fluid_ladspa_lib_t *lib);
static fluid_ladspa_lib_t *get_ladspa_library(fluid_ladspa_fx_t *fx, const char *filename);
static int load_plugin_library(fluid_ladspa_lib_t *lib);
static void unload_plugin_library(fluid_ladspa_lib_t *lib);

static FLUID_INLINE void node_to_buffer(fluid_ladspa_node_t *node, fluid_real_t *buffer);
static FLUID_INLINE void buffer_to_node(fluid_real_t *buffer, fluid_ladspa_node_t *node);

static int set_default_port_value(fluid_ladspa_plugin_t *plugin, unsigned int port_idx,
        int sample_rate, LADSPA_Data *data);
static void connect_node_to_port(fluid_ladspa_node_t *node, fluid_ladspa_dir_t dir,
        fluid_ladspa_plugin_t *plugin, int port_idx);
static int connect_default_control_nodes(fluid_ladspa_fx_t *fx, fluid_ladspa_plugin_t *plugin);

/**
 * Creates a new LADSPA effects unit.
 *
 * @param sample_rate sample_rate for the LADSPA plugins
 * @param audio_groups number of input audio channels (stereo)
 * @param effects_channels number of input effects channels (stereo)
 * @param audio_channels number of output audio channels (stereo)
 * @return pointer to the new LADSPA effects unit
 */
fluid_ladspa_fx_t *new_fluid_ladspa_fx(fluid_real_t sample_rate, int audio_groups, int effects_channels, int audio_channels)
{
    fluid_ladspa_fx_t *fx;

    fx = FLUID_NEW(fluid_ladspa_fx_t);
    if (fx == NULL)
    {
        return NULL;
    }
    FLUID_MEMSET(fx, 0, sizeof(fluid_ladspa_fx_t));

    /* Setup recursive mutex to protect access to public API */
    fluid_rec_mutex_init(fx->api_mutex);

    fx->state = FLUID_LADSPA_INACTIVE;

    /* add 0.5 to minimize overall casting error */
    fx->sample_rate = (unsigned long)(sample_rate + 0.5);

    fx->audio_groups = audio_groups;
    fx->effects_channels = effects_channels;
    fx->audio_channels = audio_channels;

    /* Setup mutex and cond used to signal that fluid_ladspa_run has finished */
     fx->run_finished_mutex = new_fluid_cond_mutex();
     if (fx->run_finished_mutex == NULL)
     {
         delete_fluid_ladspa_fx(fx);
         return NULL;
     }
     fx->run_finished_cond = new_fluid_cond();
     if (fx->run_finished_cond == NULL)
     {
         delete_fluid_ladspa_fx(fx);
         return NULL;
     }

    /* Finally, create the nodes that carry audio in and out of the LADSPA unit.
     * They will always be the first in the fx->nodes array and not removed on
     * fluid_ladspa_reset but only when this LADSPA fx instance is deleted. */
    if (create_input_output_nodes(fx) != FLUID_OK)
    {
        delete_fluid_ladspa_fx(fx);
        return NULL;
    }

    return fx;
}

/**
 * Destroys and frees a LADSPA effects unit previously created
 * with new_fluid_ladspa_fx.
 *
 * @note This function does not check the engine state for
 * possible users, so make sure that you only call this
 * if you are sure nobody is using the engine anymore (especially
 * that nobody calls fluid_ladspa_run)
 *
 * @param fx LADSPA effects instance
 */
void delete_fluid_ladspa_fx(fluid_ladspa_fx_t *fx)
{
    int i;

    clear_ladspa(fx);

    /* clear the remaining input or output nodes */
    for (i = 0; i < fx->num_nodes; i++)
    {
        delete_fluid_ladspa_node(fx->nodes[i]);
    }

    if (fx->run_finished_cond != NULL)
    {
        delete_fluid_cond(fx->run_finished_cond);
    }

    if (fx->run_finished_mutex != NULL)
    {
        delete_fluid_cond_mutex(fx->run_finished_mutex);
    }

    fluid_rec_mutex_destroy(fx->api_mutex);

    FLUID_FREE(fx);
}

/**
 * Set the sample rate of the LADSPA effects.
 *
 * Resets the LADSPA effects if the sample rate is different from the
 * previous sample rate.
 *
 * @param fx LADSPA fx instance
 * @param sample_rate new sample rate
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_ladspa_set_sample_rate(fluid_ladspa_fx_t *fx, fluid_real_t sample_rate)
{
    unsigned long new_sample_rate;

    LADSPA_API_ENTER(fx);

    /* Add 0.5 to minimize rounding errors */
    new_sample_rate = (unsigned long)(sample_rate + 0.5);

    if (fx->sample_rate == new_sample_rate)
    {
        LADSPA_API_RETURN(fx, FLUID_OK);
    }

    if (fluid_ladspa_is_active(fx))
    {
        if (fluid_ladspa_reset(fx) != FLUID_OK)
        {
            FLUID_LOG(FLUID_ERR, "Failed to reset LADSPA, unable to change sample rate");
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }
    }

    fx->sample_rate = new_sample_rate;

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Check if the LADSPA engine is currently used to render audio
 *
 * If an engine is active, the only allowed user actions are deactivation or
 * changing user control nodes. Anything else, especially adding or removing
 * plugins, nodes or ports, is only allowed in deactivated state.
 *
 * @param fx LADSPA fx instance
 * @return TRUE if LADSPA effects engine is active, otherwise FALSE
 */
int fluid_ladspa_is_active(fluid_ladspa_fx_t *fx)
{
    int is_active;

    LADSPA_API_ENTER(fx);

    is_active = (fluid_atomic_int_get(&fx->state) != FLUID_LADSPA_INACTIVE);

    LADSPA_API_RETURN(fx, is_active);
}

/**
 * Activate the LADSPA fx instance and each configured plugin.
 *
 * @param fx LADSPA fx instance
 * @return FLUID_OK if activation succeeded or already active, otherwise FLUID_FAILED
 */
int fluid_ladspa_activate(fluid_ladspa_fx_t *fx)
{
    int i;

    LADSPA_API_ENTER(fx);

    if (fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if (fluid_ladspa_check(fx, NULL, 0) != FLUID_OK)
    {
        FLUID_LOG(FLUID_ERR, "LADSPA check failed, unable to activate effects");
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    for (i = 0; i < fx->num_plugins; i++)
    {
        activate_plugin(fx->plugins[i]);
    }

    if (!fluid_atomic_int_compare_and_exchange(&fx->state, FLUID_LADSPA_INACTIVE, FLUID_LADSPA_ACTIVE))
    {
        for (i = 0; i < fx->num_plugins; i++)
        {
            deactivate_plugin(fx->plugins[i]);
        }
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Deactivate a LADSPA fx instance and all configured plugins.
 *
 * @note This function may sleep.
 *
 * @param fx LADSPA fx instance
 * @return FLUID_OK if deactivation succeeded, otherwise FLUID_FAILED
 */
int fluid_ladspa_deactivate(fluid_ladspa_fx_t *fx)
{
    int i;

    LADSPA_API_ENTER(fx);

    /* If we are already inactive, then simply return success */
    if (fluid_atomic_int_get(&fx->state) == FLUID_LADSPA_INACTIVE)
    {
        LADSPA_API_RETURN(fx, FLUID_OK);
    }

    /* Notify fluid_ladspa_run that we would like to deactivate and that it should
     * send us a signal when its done if it is currently running */
    fluid_atomic_int_set(&fx->pending_deactivation, 1);

    fluid_cond_mutex_lock(fx->run_finished_mutex);
    while (!fluid_atomic_int_compare_and_exchange(&fx->state, FLUID_LADSPA_ACTIVE, FLUID_LADSPA_INACTIVE))
    {
        fluid_cond_wait(fx->run_finished_cond, fx->run_finished_mutex);
    }
    fluid_cond_mutex_unlock(fx->run_finished_mutex);

    /* Now that we're inactive, deactivate all plugins and return success */
    for (i = 0; i < fx->num_plugins; i++)
    {
        deactivate_plugin(fx->plugins[i]);
    }

    fluid_atomic_int_set(&fx->pending_deactivation, 0);

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Reset the LADSPA effects engine: Deactivate LADSPA if currently active, remove all
 * plugin instances, remove all user nodes and unload all libraries.
 *
 * @param fx LADSPA fx instance
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_ladspa_reset(fluid_ladspa_fx_t *fx)
{
    LADSPA_API_ENTER(fx);

    if (fluid_ladspa_is_active(fx))
    {
        if (fluid_ladspa_deactivate(fx) != FLUID_OK)
        {
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }
    }

    clear_ladspa(fx);

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Processes a block of audio data via the LADSPA effects unit.
 *
 * FluidSynth calls this function during main output mixing, just after
 * the internal reverb and chorus effects have been processed.
 *
 * It reads data from the supplied buffers, runs all LADSPA plugins and writes the
 * resulting audio back into the same buffers.
 *
 * @param fx LADSPA effects instance
 * @param left_buf array of pointers into the left audio group buffers
 * @param right_buf array of pointers into the right audio group buffers
 * @param fx_left_buf array of pointers into the left effects buffers
 * @param fx_right_buf array of pointers into the right effects buffers
 */
void fluid_ladspa_run(fluid_ladspa_fx_t *fx, fluid_real_t *left_buf[], fluid_real_t *right_buf[],
                      fluid_real_t *fx_left_buf[], fluid_real_t *fx_right_buf[])
{
    int i, n;

    /* Somebody wants to deactivate the engine, so let's give them a chance to do that.
     * And check that there is at least one plugin loaded, to avoid the overhead of the
     * atomic compare and exchange on an unconfigured LADSPA engine. */
    if (fluid_atomic_int_get(&fx->pending_deactivation) || fx->num_plugins == 0)
    {
        return;
    }

    /* Inform the engine that we are now running pluings, and bail out if it's not active */
    if (!fluid_atomic_int_compare_and_exchange(&fx->state, FLUID_LADSPA_ACTIVE, FLUID_LADSPA_RUNNING))
    {
        return;
    }

    /* The input and output nodes are always first in the fx->nodes array, in the order:
     * main inputs, effect inputs, main outputs.
     * All inputs and outputs use two nodes (stereo), left and right channels interleaved. */
    n = 0;

    /* Incoming main audio data */
    for (i = 0; i < fx->audio_groups; i++)
    {
        buffer_to_node(left_buf[i], fx->nodes[n++]);
        buffer_to_node(right_buf[i], fx->nodes[n++]);
    }

    /* Incoming effects audio data */
    for (i = 0; i < fx->effects_channels; i++)
    {
        buffer_to_node(fx_left_buf[i], fx->nodes[n++]);
        buffer_to_node(fx_right_buf[i], fx->nodes[n++]);
    }

    /* Run each plugin on a block of data */
    for (i = 0; i < fx->num_plugins; i++)
    {
        fx->plugins[i]->desc->run(fx->plugins[i]->handle, FLUID_BUFSIZE);
    }

    /* Copy the data from the output nodes back to the output buffers */
    for (i = 0; i < fx->audio_channels; i++)
    {
        node_to_buffer(fx->nodes[n++], left_buf[i]);
        node_to_buffer(fx->nodes[n++], right_buf[i]);
    }

    if (!fluid_atomic_int_compare_and_exchange(&fx->state, FLUID_LADSPA_RUNNING, FLUID_LADSPA_ACTIVE))
    {
        FLUID_LOG(FLUID_ERR, "Unable to reset LADSPA running state!");
    }

    /* If deactivation was requested while in running state, notify that we've finished now
     * and deactivation can proceed */
    if (fluid_atomic_int_get(&fx->pending_deactivation))
    {
        fluid_cond_mutex_lock(fx->run_finished_mutex);
        fluid_cond_broadcast(fx->run_finished_cond);
        fluid_cond_mutex_unlock(fx->run_finished_mutex);
    }
}

/**
 * Create the nodes to get data in and out of the LADSPA effects unit.
 *
 * The nodes are named from LADSPAs perspective. So the "in*" and "send*" nodes carry audio data
 * into LADSPA, "out*" nodes carry audio data back out.
 *
 * System nodes are referenced from the fx->nodes array, just like user created nodes. The system
 * nodes are always the first in the array. The order of nodes is:
 *  in1_L, in1_R, ..., send1_L, send1_R, ..., out1_L, out1_R, ..., [user created nodes ...]
 *
 * @param fx LADSPA fx instance
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
static int create_input_output_nodes(fluid_ladspa_fx_t *fx)
{
    char name[99];
    int i;

    /* Create left and right input nodes for each audio group. */
    for (i = 0; i < fx->audio_groups; i++)
    {
        FLUID_SNPRINTF(name, sizeof(name), "in%i_L", (i + 1));
        if (new_fluid_ladspa_node(fx, name, FLUID_LADSPA_NODE_AUDIO, FLUID_BUFSIZE) == NULL)
        {
            return FLUID_FAILED;
        }

        FLUID_SNPRINTF(name, sizeof(name), "in%i_R", (i + 1));
        if (new_fluid_ladspa_node(fx, name, FLUID_LADSPA_NODE_AUDIO, FLUID_BUFSIZE) == NULL)
        {
            return FLUID_FAILED;
        }
    }

     /* Create left and right input nodes for each effect channel */
    for (i = 0; i < fx->effects_channels; i++)
    {
        FLUID_SNPRINTF(name, sizeof(name), "send%i_L", (i + 1));
        if (new_fluid_ladspa_node(fx, name, FLUID_LADSPA_NODE_AUDIO, FLUID_BUFSIZE) == NULL)
        {
            return FLUID_FAILED;
        }

        FLUID_SNPRINTF(name, sizeof(name), "send%i_R", (i + 1));
        if (new_fluid_ladspa_node(fx, name, FLUID_LADSPA_NODE_AUDIO, FLUID_BUFSIZE) == NULL)
        {
            return FLUID_FAILED;
        }
    }

    /* Create left and right output nodes for each audio channel. */
    for (i = 0; i < fx->audio_channels; i++)
    {
        FLUID_SNPRINTF(name, sizeof(name), "out%i_L", (i + 1));
        if (new_fluid_ladspa_node(fx, name, FLUID_LADSPA_NODE_AUDIO, FLUID_BUFSIZE) == NULL)
        {
            return FLUID_FAILED;
        }

        FLUID_SNPRINTF(name, sizeof(name), "out%i_R", (i + 1));
        if (new_fluid_ladspa_node(fx, name, FLUID_LADSPA_NODE_AUDIO, FLUID_BUFSIZE) == NULL)
        {
            return FLUID_FAILED;
        }
    }

    return FLUID_OK;
}

static void clear_ladspa(fluid_ladspa_fx_t *fx)
{
    int i;
    int num_system_nodes;

    /* Deactivate and free all plugins */
    for (i = 0; i < fx->num_plugins; i++)
    {
        deactivate_plugin(fx->plugins[i]);
        delete_fluid_ladspa_plugin(fx->plugins[i]);
    }
    fx->num_plugins = 0;

    /* Unload and free all libraries */
    for (i = 0; i < fx->num_libs; i++)
    {
        unload_plugin_library(fx->libs[i]);
        delete_fluid_ladspa_lib(fx->libs[i]);
    }
    fx->num_libs = 0;

    /* Free all user defined nodes, leaving system nodes untouched */
    num_system_nodes = 2 * (fx->audio_groups + fx->effects_channels + fx->audio_channels);
    for (i = num_system_nodes; i < fx->num_nodes; i++)
    {
        delete_fluid_ladspa_node(fx->nodes[i]);
    }
    fx->num_nodes = num_system_nodes;
}

/**
 * Check if a named node exists. Nodes are searched by case-insensitive string comparison.
 *
 * @param fx LADSPA fx instance
 * @param name the node name string
 * @return TRUE if the node exists, otherwise FALSE
 */
int fluid_ladspa_node_exists(fluid_ladspa_fx_t *fx, const char *name)
{
    int node_exists;

    LADSPA_API_ENTER(fx);

    node_exists = (get_node(fx, name) != NULL);

    LADSPA_API_RETURN(fx, node_exists);
}

/**
 * Check if the named port exists on a plugin instance.
 *
 * The string match is case-insensitive and treats spaces and underscores as equal.
 *
 * @param fx LADSPA fx instance
 * @param plugin_id integer plugin id as returned by fluid_ladspa_add_*_node
 * @param name the port name
 * @return TRUE if port was found, otherwise FALSE
 */
int fluid_ladspa_port_exists(fluid_ladspa_fx_t *fx, int plugin_id, const char *name)
{
    fluid_ladspa_plugin_t *plugin;
    int port_exists;

    LADSPA_API_ENTER(fx);

    plugin = get_plugin_by_id(fx, plugin_id);
    if (plugin == NULL)
    {
        LADSPA_API_RETURN(fx, FALSE);
    }

    port_exists = (get_plugin_port_idx(plugin, name) != -1);

    LADSPA_API_RETURN(fx, port_exists);
}

/**
 * Create and add a new LADSPA audio node.
 *
 * Audio nodes are used to connect the output of one plugin to the input of
 * another.
 *
 * @param fx LADSPA effects instance
 * @param name name of the new node
 * @return FLUID_OK on success, FLUID_FAILED on error
 */
int fluid_ladspa_add_audio_node(fluid_ladspa_fx_t *fx, const char *name)
{
    fluid_ladspa_node_t *node;

    LADSPA_API_ENTER(fx);
    if (fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    node = new_fluid_ladspa_node(fx, name, FLUID_LADSPA_NODE_AUDIO, FLUID_BUFSIZE);
    if (node == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Create and add a new LADSPA control node that can be set manually.
 *
 * @param fx LADSPA effects instance
 * @param name name of the new node
 * @param val the initial float value of the node
 * @return FLUID_OK on success, FLUID_FAILED on error
 */
int fluid_ladspa_add_control_node(fluid_ladspa_fx_t *fx, const char *name, fluid_real_t val)
{
    fluid_ladspa_node_t *node;

    LADSPA_API_ENTER(fx);
    if (fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    node = new_fluid_ladspa_node(fx, name, FLUID_LADSPA_NODE_CONTROL, 1);
    if (node == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    node->buf[0] = val;

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Set the value of a user constrol node
 *
 * Nodes are searched by case-insensitive string comparison.
 *
 * @param fx LADSPA fx instance
 * @param name node name string
 * @param val floating point value
 * @return FLUID_OK on success, FLUID_FAILED on error
 */
int fluid_ladspa_set_control_node(fluid_ladspa_fx_t *fx, const char *name, fluid_real_t val)
{
    fluid_ladspa_node_t *node;

    LADSPA_API_ENTER(fx);

    node = get_node(fx, name);
    if (node == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if (node->type != FLUID_LADSPA_NODE_CONTROL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    node->buf[0] = val;

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Instantiate a plugin from a LADSPA plugin library and return it's unique id
 *
 * The returned id can be used to reference this plugin instance at a later time,
 * for example when connecting the plugin instance ports with nodes. Plugin instance
 * ids are never reused in a LADSPA fx instance.
 *
 * @param fx LADSPA effects instance
 * @param lib_name filename of ladspa plugin library
 * @param plugin_name plugin name (the unique label of the plugin in the LADSPA library)
 * @return integer plugin id or -1 on error
 */
int fluid_ladspa_add_plugin(fluid_ladspa_fx_t *fx, const char *lib_name, const char *plugin_name)
{
    fluid_ladspa_lib_t *lib;
    fluid_ladspa_plugin_t *plugin;

    LADSPA_API_ENTER(fx);
    if (fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, -1);
    }

    if (fx->num_plugins >= FLUID_LADSPA_MAX_PLUGINS)
    {
        FLUID_LOG(FLUID_ERR, "Maximum number of LADSPA plugins reached");
        LADSPA_API_RETURN(fx, -1);
    }

    lib = get_ladspa_library(fx, lib_name);
    if (lib == NULL)
    {
        LADSPA_API_RETURN(fx, -1);
    }

    plugin = new_fluid_ladspa_plugin(fx, lib, plugin_name);
    if (plugin == NULL)
    {
        LADSPA_API_RETURN(fx, -1);
    }

    plugin->id = fx->next_plugin_id++;
    fx->plugins[fx->num_plugins++] = plugin;

    LADSPA_API_RETURN(fx, plugin->id);
}

/**
 * Connect an input or output plugin port to a node
 *
 * @note There is no corresponding disconnect function. If the connections need to be changed,
 * clear everything with fluid_ladspa_reset and start again from scratch.
 *
 * @param fx LADSPA effects instance
 * @param plugin_id the integer plugin id as returned by fluid_ladspa_add_plugin
 * @param dir connect to port as FLUID_LADSPA_INPUT or FLUID_LADSPA_OUTPUT
 * @param port_name the port name to connect to (case-insensitive prefix match, see get_plugin_port_idx)
 * @param node_name the node name to connect to (case-insensitive)
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_ladspa_connect(fluid_ladspa_fx_t *fx, int plugin_id, fluid_ladspa_dir_t dir,
                         const char *port_name, const char *node_name)
{
    fluid_ladspa_plugin_t *plugin;
    fluid_ladspa_node_t *node;
    int port_idx;
    int port_flags;
    const char *full_port_name;
    const char *plugin_name;

    LADSPA_API_ENTER(fx);

    if (fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }
    plugin = get_plugin_by_id(fx, plugin_id);
    if (plugin == NULL)
    {
        FLUID_LOG(FLUID_ERR, "LADSPA plugin with ID %d not found", plugin_id);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    port_idx = get_plugin_port_idx(plugin, port_name);
    if (port_idx < 0)
    {
        FLUID_LOG(FLUID_ERR, "LADSPA plugin port '%s' not found on plugin '%s'", port_name,
                  plugin->desc->Label);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    /* fixed node, create a new 'anonymous' node and interpret the node name as it's fixed float
     * value */
    if (dir == FLUID_LADSPA_FIXED)
    {
        node = new_fluid_ladspa_node(fx, "", FLUID_LADSPA_NODE_CONTROL, 1);
        if (node == NULL)
        {
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }
        node->buf[0] = atof(node_name);
    }
    else
    {
        node = get_node(fx, node_name);
        if (node == NULL)
        {
            FLUID_LOG(FLUID_ERR, "LADSPA node '%s' not found", node_name);
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }
    }

    /* For easier access during sanity checks */
    port_flags = plugin->desc->PortDescriptors[port_idx];
    full_port_name = plugin->desc->PortNames[port_idx];
    plugin_name = plugin->desc->Label;

    /* Check that requested direction matches with port direction */
    if (dir == FLUID_LADSPA_INPUT && !LADSPA_IS_PORT_INPUT(port_flags))
    {
        FLUID_LOG(FLUID_ERR, "Port '%s' on plugin '%s' is not an input port", full_port_name, plugin_name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }
    else if (dir == FLUID_LADSPA_OUTPUT && !LADSPA_IS_PORT_OUTPUT(port_flags))
    {
        FLUID_LOG(FLUID_ERR, "Port '%s' on plugin '%s' is not an output port", full_port_name, plugin_name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    /* Check that requested port type matches the node type */
    if (LADSPA_IS_PORT_CONTROL(port_flags) && node->type == FLUID_LADSPA_NODE_AUDIO)
    {
        FLUID_LOG(FLUID_ERR, "Cannot connect control port '%s' on plugin '%s' to an audio node",
                  full_port_name, plugin_name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }
    else if (LADSPA_IS_PORT_AUDIO(port_flags) && node->type != FLUID_LADSPA_NODE_AUDIO)
    {
        FLUID_LOG(FLUID_ERR, "Cannot connect audio port '%s' on plugin '%s' to a control node",
                  full_port_name, plugin_name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    FLUID_LOG(FLUID_DBG, "Connecting LADSPA plugin '%s': port '%s' %s node '%s'", plugin->desc->Label,
              full_port_name, (dir == FLUID_LADSPA_INPUT) ? "<" : ">", node_name);

    connect_node_to_port(node, dir, plugin, port_idx);

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Find all unconnected control ports on all configured plugins and fill any unconnected ports with
 * their default value, if available.
 *
 * @param fx LADSPA fx instance
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_ladspa_control_defaults(fluid_ladspa_fx_t *fx)
{
    int i;

    LADSPA_API_ENTER(fx);

    if (fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    for (i = 0; i < fx->num_plugins; i++)
    {
        if (connect_default_control_nodes(fx, fx->plugins[i]) != FLUID_OK)
        {
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }
    }

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Do a sanity check for problems in the LADSPA setup
 *
 * @param fx LADSPA fx instance
 * @param err pointer to string that should be filled with an error message, if applicable
 * @param err_size size of the error buffer
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_ladspa_check(fluid_ladspa_fx_t *fx, char *err, int err_size)
{
    int i;
    unsigned int k;
    int has_connections;
    int num_system_nodes;
    fluid_ladspa_plugin_t *plugin;
    LADSPA_PortDescriptor port_flags;

    LADSPA_API_ENTER(fx);

    /* Check that there is at least one plugin */
    if (fx->num_plugins == 0)
    {
        FLUID_SNPRINTF(err, err_size, "No plugins loaded\n");
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    /* Check that all plugin ports are connected */
    for (i = 0; i < fx->num_plugins; i++)
    {
        plugin = fx->plugins[i];

        for (k = 0; k < plugin->desc->PortCount; k++)
        {
            port_flags = plugin->desc->PortDescriptors[k];

            if (LADSPA_IS_PORT_INPUT(port_flags) && plugin->ports[k].num_inputs == 0)
            {
                FLUID_SNPRINTF(err, err_size, "Input port '%s' on plugin '%s' is not connected\n",
                               plugin->desc->PortNames[k], plugin->desc->Label);
                LADSPA_API_RETURN(fx, FLUID_FAILED);
            }
            else if (LADSPA_IS_PORT_OUTPUT(port_flags) && plugin->ports[k].num_outputs == 0)
            {
                FLUID_SNPRINTF(err, err_size, "Output port '%s' on plugin '%s' is not connected\n",
                               plugin->desc->PortNames[k], plugin->desc->Label);
                LADSPA_API_RETURN(fx, FLUID_FAILED);
            }
        }
    }

    /* Check that at least one system input is used */
    has_connections = 0;
    for (i = 0; i < 2 * (fx->audio_groups + fx->effects_channels); i++)
    {
        if (fx->nodes[i]->num_outputs)
        {
            has_connections = 1;
            break;
        }
    }
    if (!has_connections)
    {
        FLUID_SNPRINTF(err, err_size, "No system input nodes are connected\n");
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    num_system_nodes = 2 * (fx->audio_groups + fx->effects_channels + fx->audio_channels);

    /* Check that at least one system output is used */
    has_connections = 0;
    for (i = 2 * (fx->audio_groups + fx->effects_channels); i < num_system_nodes; i++)
    {
        if (fx->nodes[i]->num_inputs)
        {
            has_connections = 1;
            break;
        }
    }
    if (!has_connections)
    {
        FLUID_SNPRINTF(err, err_size, "No system output nodes are connected\n");
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    /* Check that custom audio nodes have both input and output and control nodes have either
     * an input or output */
    for (i = num_system_nodes; i < fx->num_nodes; i++)
    {
        if (fx->nodes[i]->type == FLUID_LADSPA_NODE_AUDIO &&
            (fx->nodes[i]->num_inputs == 0 || fx->nodes[i]->num_outputs == 0))
        {
            FLUID_SNPRINTF(err, err_size, "Audio node '%s' is not connected on input and output\n",
                           fx->nodes[i]->name);
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }

        if (fx->nodes[i]->type == FLUID_LADSPA_NODE_CONTROL && (fx->nodes[i]->num_outputs == 0)
                && (fx->nodes[i]->num_inputs == 0))
        {
            FLUID_SNPRINTF(err, err_size, "Control node '%s' is not connected\n", fx->nodes[i]->name);
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }
    }

    LADSPA_API_RETURN(fx, FLUID_OK);
}

#ifdef WITH_FLOAT

static FLUID_INLINE void buffer_to_node(fluid_real_t *buffer, fluid_ladspa_node_t *node)
{
    /* If the node is not used by any plugin, then we don't need to fill it */
    if (node->num_outputs > 0)
    {
        FLUID_MEMCPY(node->buf, buffer, FLUID_BUFSIZE * sizeof(float));
    }
}

static FLUID_INLINE void node_to_buffer(fluid_ladspa_node_t *node, fluid_real_t *buffer)
{
    /* If the node has no inputs, then we don't need to copy it to the node */
    if (node->num_inputs > 0)
    {
        FLUID_MEMCPY(buffer, node->buf, FLUID_BUFSIZE * sizeof(float));
    }
}

#else /* WITH_FLOAT */

static FLUID_INLINE void buffer_to_node(fluid_real_t *buffer, fluid_ladspa_node_t *node)
{
    int i;

    /* If the node is not used by any plugin, then we don't need to fill it */
    if (node->num_outputs > 0)
    {
        for (i = 0; i < FLUID_BUFSIZE; i++)
        {
            node->buf[i] = (LADSPA_Data)buffer[i];
        }
    }
}

static FLUID_INLINE void node_to_buffer(fluid_ladspa_node_t *node, fluid_real_t *buffer)
{
    int i;

    /* If the node has no inputs, then we don't need to copy it to the node */
    if (node->num_inputs > 0)
    {
        for (i = 0; i < FLUID_BUFSIZE; i++)
        {
            buffer[i] = (fluid_real_t)node->buf[i];
        }
    }
}

#endif /* WITH_FLOAT */

static void activate_plugin(fluid_ladspa_plugin_t *plugin)
{
    if (!plugin->active)
    {
        plugin->active = 1;
        if (plugin->desc->activate != NULL)
        {
            plugin->desc->activate(plugin->handle);
        }
    }
}

static void deactivate_plugin(fluid_ladspa_plugin_t *plugin)
{
    if (plugin->active)
    {
        plugin->active = 0;
        if (plugin->desc->deactivate != NULL)
        {
            plugin->desc->deactivate(plugin->handle);
        }
    }
}

/**
 * Return a LADSPA node by name. Nodes are searched case insensitive.
 *
 * @param fx LADSPA fx instance
 * @param name the node name string
 * @return a fluid_ladspa_node_t pointer or NULL if not found
 */
static fluid_ladspa_node_t *get_node(fluid_ladspa_fx_t *fx, const char *name)
{
    int i;

    for (i = 0; i < fx->num_nodes; i++)
    {
        if (FLUID_STRCASECMP(fx->nodes[i]->name, name) == 0)
        {
            return fx->nodes[i];
        }
    }

    return NULL;
}

/**
 * Return a LADSPA plugin port index by name, using a 'fuzzy match'.
 *
 * Returns the first plugin port which matches the name. If no exact match is
 * found, returns the port that starts with the specified name, but only if there is
 * only one such match.
 *
 * @param plugin pointer to fluid_ladspa_plugin_t
 * @param name the port name
 * @return index of the port in the plugin or -1 on error
 */
static int get_plugin_port_idx(const fluid_ladspa_plugin_t *plugin, const char *name)
{
    unsigned int i;
    int port = -1;

    for (i = 0; i < plugin->desc->PortCount; i++)
    {
        if (FLUID_STRNCASECMP(plugin->desc->PortNames[i], name, FLUID_STRLEN(name)) == 0)
        {
            /* exact match, return immediately */
            if (FLUID_STRLEN(plugin->desc->PortNames[i]) == FLUID_STRLEN(name))
            {
                return i;
            }

            /* more than one prefix match should be treated as not found */
            if (port != -1)
            {
                return -1;
            }

            port = i;
        }
    }

    return port;
}

/**
 * Return a LADSPA descriptor structure for a plugin in a LADSPA library.
 *
 * @param lib pointer to fluid_ladspa_lib_t instance
 * @param name name (LADSPA Label) of the plugin
 * @return pointer to LADSPA_Descriptor, NULL on error or if not found
 */
static const LADSPA_Descriptor *get_plugin_descriptor(const fluid_ladspa_lib_t *lib, const char *name)
{
    const LADSPA_Descriptor *desc;
    int i = 0;

    while (1)
    {
        desc = lib->descriptor(i++);
        if (desc == NULL)
        {
            return NULL;
        }

        if (FLUID_STRCMP(desc->Label, name) == 0)
        {
            return desc;
        }
    }
}

/**
 * Instantiate a new LADSPA plugin from a library and set up the associated
 * control structures needed by the LADSPA fx engine.
 *
 * Plugins are identified by their "Label" in the plugin descriptor structure.
 *
 * @param fx LADSPA fx instance
 * @param lib pointer to fluid_ladspa_lib_t
 * @param name string name of the plugin (the LADSPA Label)
 * @return pointer to the new ladspa_plugin_t structure or NULL on error
 */
static fluid_ladspa_plugin_t *
new_fluid_ladspa_plugin(fluid_ladspa_fx_t *fx, const fluid_ladspa_lib_t *lib, const char *name)
{
    fluid_ladspa_plugin_t *plugin;

    plugin = FLUID_NEW(fluid_ladspa_plugin_t);
    if (plugin == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    FLUID_MEMSET(plugin, 0, sizeof(fluid_ladspa_plugin_t));

    plugin->desc = get_plugin_descriptor(lib, name);
    if (plugin->desc == NULL)
    {
        delete_fluid_ladspa_plugin(plugin);
        return NULL;
    }

    plugin->handle = plugin->desc->instantiate(plugin->desc, fx->sample_rate);
    if (plugin->handle == NULL)
    {
        delete_fluid_ladspa_plugin(plugin);
        FLUID_LOG(FLUID_ERR, "Unable to instantiate plugin '%s' from '%s'", name, lib->filename);
        return NULL;
    }

    plugin->ports = FLUID_ARRAY(fluid_ladspa_port_state_t, plugin->desc->PortCount);
    if (plugin->ports == NULL)
    {
        delete_fluid_ladspa_plugin(plugin);
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    FLUID_MEMSET(plugin->ports, 0, plugin->desc->PortCount * sizeof(fluid_ladspa_port_state_t));

    return plugin;
}

static void delete_fluid_ladspa_plugin(fluid_ladspa_plugin_t *plugin)
{
    if (plugin == NULL)
    {
        return;
    }

    if (plugin->ports != NULL)
    {
        FLUID_FREE(plugin->ports);
    }

    if (plugin->handle != NULL && plugin->desc != NULL && plugin->desc->cleanup != NULL)
    {
        plugin->desc->cleanup(plugin->handle);
    }

    FLUID_FREE(plugin);
}

static fluid_ladspa_node_t *new_fluid_ladspa_node(fluid_ladspa_fx_t *fx, const char *name,
                                                  fluid_ladspa_node_type_t type, int buf_size)
{
    fluid_ladspa_node_t *node;

    /* check if node with this name exits already */
    if (FLUID_STRLEN(name) > 0 && get_node(fx, name) != NULL)
    {
        return NULL;
    }

    if (fx->num_nodes >= FLUID_LADSPA_MAX_NODES)
    {
        FLUID_LOG(FLUID_ERR, "Maximum number of nodes reached");
        return NULL;
    }

    node = FLUID_NEW(fluid_ladspa_node_t);
    if (node == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    FLUID_MEMSET(node, 0, sizeof(fluid_ladspa_node_t));

    node->type = type;

    node->name = FLUID_STRDUP(name);
    if (node->name == NULL)
    {
        delete_fluid_ladspa_node(node);
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    node->buf = FLUID_ARRAY(LADSPA_Data, buf_size);
    if (node->buf == NULL)
    {
        delete_fluid_ladspa_node(node);
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(node->buf, 0, buf_size * sizeof(LADSPA_Data));

    fx->nodes[fx->num_nodes++] = node;

    return node;
}

static void delete_fluid_ladspa_node(fluid_ladspa_node_t *node)
{
    if (node->buf != NULL)
    {
        FLUID_FREE(node->buf);
    }

    if (node->name != NULL)
    {
        FLUID_FREE(node->name);
    }

    FLUID_FREE(node);
}

static fluid_ladspa_lib_t *get_ladspa_library(fluid_ladspa_fx_t *fx, const char *filename)
{
    int i;
    fluid_ladspa_lib_t *lib;

    /* check if we have loaded this lib before and return it if found */
    for (i = 0; i < fx->num_libs; i++)
    {
        if (FLUID_STRCMP(fx->libs[i]->filename, filename) == 0)
        {
            return fx->libs[i];
        }
    }

    if (fx->num_libs >= FLUID_LADSPA_MAX_LIBS)
    {
        FLUID_LOG(FLUID_ERR, "Maximum number of LADSPA libraries reached");
        return NULL;
    }

    lib = new_fluid_ladspa_lib(fx, filename);
    if (lib == NULL)
    {
        return NULL;
    }

    if (load_plugin_library(lib) != FLUID_OK)
    {
        delete_fluid_ladspa_lib(lib);
        return NULL;
    }

    fx->libs[fx->num_libs++] = lib;

    return lib;
}

static fluid_ladspa_lib_t *new_fluid_ladspa_lib(fluid_ladspa_fx_t *fx, const char *filename)
{
    fluid_ladspa_lib_t *lib;

    lib = FLUID_NEW(fluid_ladspa_lib_t);
    if (lib == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    FLUID_MEMSET(lib, 0, sizeof(fluid_ladspa_lib_t));

    lib->filename = FLUID_STRDUP(filename);
    if (lib->filename == NULL)
    {
        delete_fluid_ladspa_lib(lib);
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    return lib;
}

static void delete_fluid_ladspa_lib(fluid_ladspa_lib_t *lib)
{
    if (lib->filename != NULL)
    {
        FLUID_FREE(lib->filename);
    }

    FLUID_FREE(lib);
}

static int load_plugin_library(fluid_ladspa_lib_t *lib)
{
    char filepath[FLUID_LADSPA_MAX_PATH_LENGTH];
    char *error;
    char *ladspa_path;

    /* If the library name does not contain a slash, then try to load from
     * LADSPA_PATH */
    if (FLUID_STRCHR(lib->filename, '/') == NULL)
    {
        ladspa_path = getenv("LADSPA_PATH");
        if (ladspa_path == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Unable to load LADSPA plugin '%s'. Use slashes in the "
                                 "LADSPA library filename or set the LADSPA_PATH variable.",
                      lib->filename);
            return FLUID_FAILED;
        }
        FLUID_SNPRINTF(filepath, FLUID_LADSPA_MAX_PATH_LENGTH, "%s/%s", ladspa_path, lib->filename);
    }
    else
    {
        FLUID_SNPRINTF(filepath, FLUID_LADSPA_MAX_PATH_LENGTH, "%s", lib->filename);
    }

    dlerror();
    lib->dlib = dlopen(filepath, RTLD_NOW);
    error = dlerror();
    if (lib->dlib == NULL || error)
    {
        if (error == NULL)
        {
            error = "Unknown error";
        }
        FLUID_LOG(FLUID_ERR, "Unable to load LADSPA plugin library '%s': %d", filepath, error);
        return FLUID_FAILED;
    }

    lib->descriptor = (LADSPA_Descriptor_Function)dlsym(lib->dlib, "ladspa_descriptor");
    error = dlerror();
    if (lib->descriptor == NULL || error)
    {
        if (error == NULL)
        {
            error = "Unknown error";
        }
        dlclose(lib->dlib);
        FLUID_LOG(FLUID_ERR, "Unable to find ladspa_descriptor in '%': %s", filepath, error);
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

static void unload_plugin_library(fluid_ladspa_lib_t *lib)
{
    dlclose(lib->dlib);
    lib->dlib = NULL;
    lib->descriptor = NULL;
}

/**
 * Retrieve a ladspa_plugin_t instance by it's plugin id, as returned from
 * fluid_ladspa_add_plugin.
 *
 * @note The returned pointer is only guaranteed to be valid as long as the
 * caller holds the LADSPA API lock. Calles should not store the pointer
 * but the plugin id to retrieve it at a later time.
 *
 * @param fx LADSPA effects instance
 * @param id plugin id (integer)
 * @return pointer to plugin or NULL if not found
 */
static fluid_ladspa_plugin_t *get_plugin_by_id(fluid_ladspa_fx_t *fx, int id)
{
    int i;

    LADSPA_API_ENTER(fx);

    for (i = 0; i < fx->num_plugins; i++)
    {
        if (fx->plugins[i]->id == id)
        {
            LADSPA_API_RETURN(fx, fx->plugins[i]);
        }
    }

    LADSPA_API_RETURN(fx, NULL);
}

/**
 * Return the default value of a plugin port, as specified by the port hints.
 *
 * @param plugin pointer to plugin instance
 * @param port_idx index of the port in the plugin
 * @param sample_rate the current sample rate of the LADSPA fx
 * @param pointer to LADSPA_Data value to set
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
static int set_default_port_value(fluid_ladspa_plugin_t *plugin, unsigned int port_idx, int sample_rate,
        LADSPA_Data *data)
{
    const LADSPA_PortRangeHint *hint;
    LADSPA_PortRangeHintDescriptor flags;
    LADSPA_Data value;
    float low_factor = 0.0;
    float high_factor = 0.0;

    if (port_idx >= plugin->desc->PortCount)
    {
        return FLUID_FAILED;
    }

    hint = &plugin->desc->PortRangeHints[port_idx];
    flags = hint->HintDescriptor;

    if (!LADSPA_IS_HINT_HAS_DEFAULT(flags))
    {
        return FLUID_FAILED;
    }

    if (LADSPA_IS_HINT_DEFAULT_0(flags))
    {
        value = 0.0;
    }
    else if (LADSPA_IS_HINT_DEFAULT_1(flags))
    {
        value = 1.0;
    }
    else if (LADSPA_IS_HINT_DEFAULT_100(flags))
    {
        value = 100.0;
    }
    else if (LADSPA_IS_HINT_DEFAULT_440(flags))
    {
        value = 440.0;
    }
    /* defaults based on lower or upper bounds must consider HINT_SAMPLE_RATE */
    else {
        if (LADSPA_IS_HINT_DEFAULT_MINIMUM(flags))
        {
            low_factor = 1.0;
            high_factor = 0.0;
        }
        else if (LADSPA_IS_HINT_DEFAULT_LOW(flags))
        {
            low_factor = 0.75;
            high_factor = 0.25;
        }
        else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(flags))
        {
            low_factor = 0.5;
            high_factor = 0.5;
        }
        else if (LADSPA_IS_HINT_DEFAULT_HIGH(flags))
        {
            low_factor = 0.25;
            high_factor = 0.75;
        }
        else if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(flags))
        {
            low_factor = 0.0;
            high_factor = 1.0;
        }

        if (LADSPA_IS_HINT_LOGARITHMIC(flags) && low_factor > 0 && high_factor > 0)
        {
            value = exp(log(hint->LowerBound) * low_factor + log(hint->UpperBound) * high_factor);
        }
        else
        {
            value = (hint->LowerBound * low_factor + hint->UpperBound * high_factor);
        }

        if (LADSPA_IS_HINT_SAMPLE_RATE(flags))
        {
            value *= sample_rate;
        }
    }

    if (LADSPA_IS_HINT_INTEGER(flags))
    {
        /* LADSPA doesn't specify which rounding method to use, so lets keep it simple... */
        value = floor(value + 0.5);
    }

    *data = value;

    return FLUID_OK;
}

/**
 * For each control port that hasn't got an input yet, create a node based on the port hints
 * specified in the plugin (if available).
 *
 * @param fx LADSPA fx instance
 * @param plugin plugin instance
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
static int connect_default_control_nodes(fluid_ladspa_fx_t *fx, fluid_ladspa_plugin_t *plugin)
{
    unsigned int i;
    LADSPA_Data value;
    fluid_ladspa_node_t *node;
    fluid_ladspa_dir_t dir;
    int port_flags;

    for (i = 0; i < plugin->desc->PortCount; i++)
    {
        port_flags = plugin->desc->PortDescriptors[i];

        if (plugin->ports[i].num_inputs > 0 || !LADSPA_IS_PORT_CONTROL(port_flags))
        {
            continue;
        }

        if (set_default_port_value(plugin, i, fx->sample_rate, &value) == FLUID_OK)
        {
            node = new_fluid_ladspa_node(fx, "", FLUID_LADSPA_NODE_CONTROL, 1);
            if (node == NULL)
            {
                return FLUID_FAILED;
            }
            node->buf[0] = value;

            dir = LADSPA_IS_PORT_INPUT(port_flags) ? FLUID_LADSPA_INPUT : FLUID_LADSPA_OUTPUT;

            connect_node_to_port(node, dir, plugin, i);
        }
    }

    return FLUID_OK;
}

static void connect_node_to_port(fluid_ladspa_node_t *node, fluid_ladspa_dir_t dir,
        fluid_ladspa_plugin_t *plugin, int port_idx)
{
    plugin->desc->connect_port(plugin->handle, port_idx, node->buf);

    /* Mark port and node as connected in the respective direction */
    if (dir == FLUID_LADSPA_INPUT || dir == FLUID_LADSPA_FIXED)
    {
        plugin->ports[port_idx].num_inputs++;
        node->num_outputs++;
    }
    else
    {
        plugin->ports[port_idx].num_outputs++;
        node->num_inputs++;
    }
}


#endif /*LADSPA*/
