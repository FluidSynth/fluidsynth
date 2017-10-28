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
        fluid_ladspa_node_type_t type, fluid_real_t *host_buffer);
static void delete_fluid_ladspa_node(fluid_ladspa_node_t *node);
static fluid_ladspa_node_t *get_node(fluid_ladspa_fx_t *fx, const char *name);

static int get_plugin_port_idx(const fluid_ladspa_plugin_t *plugin, const char *name);
static const LADSPA_Descriptor *get_plugin_descriptor(const fluid_ladspa_lib_t *lib, const char *name);

static fluid_ladspa_plugin_t *
new_fluid_ladspa_plugin(fluid_ladspa_fx_t *fx, const fluid_ladspa_lib_t *lib, const char *name);
static void delete_fluid_ladspa_plugin(fluid_ladspa_plugin_t *plugin);
static void activate_plugin(fluid_ladspa_plugin_t *plugin);
static void deactivate_plugin(fluid_ladspa_plugin_t *plugin);
static fluid_ladspa_plugin_t *get_plugin(fluid_ladspa_fx_t *fx, const char *name);

static fluid_ladspa_lib_t *new_fluid_ladspa_lib(fluid_ladspa_fx_t *fx, const char *filename);
static void delete_fluid_ladspa_lib(fluid_ladspa_lib_t *lib);
static fluid_ladspa_lib_t *get_ladspa_library(fluid_ladspa_fx_t *fx, const char *filename);
static int load_plugin_library(fluid_ladspa_lib_t *lib);
static void unload_plugin_library(fluid_ladspa_lib_t *lib);

static LADSPA_Data get_default_port_value(fluid_ladspa_plugin_t *plugin, unsigned int port_idx,
        int sample_rate);
static void connect_node_to_port(fluid_ladspa_node_t *node, fluid_ladspa_dir_t dir,
        fluid_ladspa_plugin_t *plugin, int port_idx);
static int create_control_port_nodes(fluid_ladspa_fx_t *fx, fluid_ladspa_plugin_t *plugin);

static int check_all_ports_connected(fluid_ladspa_plugin_t *plugin, const char **name);
static int check_no_inplace_broken(fluid_ladspa_plugin_t *plugin, const char **name1, const char **name2);
static int check_host_output_used(fluid_ladspa_fx_t *fx);
static int check_all_audio_nodes_connected(fluid_ladspa_fx_t *fx, const char **name);

static FLUID_INLINE void mix_plugin_with_host_buffer(fluid_ladspa_node_t *node,
        int num_samples, float gain);

#ifndef WITH_FLOAT
static FLUID_INLINE void copy_host_to_plugin_buffers(fluid_ladspa_fx_t *fx, int num_samples);
static FLUID_INLINE void copy_plugin_to_host_buffers(fluid_ladspa_fx_t *fx, int num_samples);
#endif

/**
 * Creates a new LADSPA effects unit.
 *
 * @param sample_rate sample_rate for the LADSPA plugins
 * @param buffer_size size of all audio buffers
 * @return pointer to the new LADSPA effects unit
 */
fluid_ladspa_fx_t *new_fluid_ladspa_fx(fluid_real_t sample_rate, int buffer_size)
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

    fluid_atomic_int_set(&fx->state, FLUID_LADSPA_INACTIVE);
    fx->buffer_size = buffer_size;

    /* add 0.5 to minimize overall casting error */
    fx->sample_rate = (unsigned long)(sample_rate + 0.5);

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
 * Add host buffers to the LADSPA engine.
 *
 * @param fx LADSPA fx instance
 * @param prefix common name prefix for the created nodes
 * @param num_buffers count of buffers in the left and right arrays
 * @param left array of pointers to left side buffers
 * @param right array of pointers to right side buffers
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_ladspa_add_host_buffers(fluid_ladspa_fx_t *fx, const char *prefix,
        int buffer_count, int buffer_size, fluid_real_t *left[], fluid_real_t *right[])
{
    int i, c;
    char name[99];
    char *side;
    fluid_real_t **bufs;

    LADSPA_API_ENTER(fx);

    if (fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    /* Create left and right nodes for all channels */
    for (c = 0; c < buffer_count; c++)
    {
        for (i = 0; i < 2; i++)
        {
            if (i == 0)
            {
                side = "L";
                bufs = left;
            }
            else
            {
                side = "R";
                bufs = right;
            }

            /* If there is more than one channel, then append a 1-based index to each name */
            if (buffer_count > 1) {
                FLUID_SNPRINTF(name, sizeof(name), "%s%d:%s", prefix, (i + 1), side);
            }
            else
            {
                FLUID_SNPRINTF(name, sizeof(name), "%s:%s", prefix, side);
            }

            if (new_fluid_ladspa_node(fx, name, FLUID_LADSPA_NODE_HOST, bufs[c]) == NULL)
            {
                return FLUID_FAILED;
            }
        }
    }

    return FLUID_OK;
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
 * setting values of  control ports on effects. Anything else, especially
 * adding or removing plugins, nodes or ports, is only allowed in deactivated
 * state.
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
    fx->pending_deactivation = 1;

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

    fx->pending_deactivation = 0;

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
 * Processes audio data via the LADSPA effects unit.
 *
 * FluidSynth calls this function during main output mixing, just after
 * the internal reverb and chorus effects have been processed.
 *
 * It copies audio data from the supplied buffers, runs all LADSPA plugins and copies the
 * resulting audio back into the same buffers.
 *
 * @param fx LADSPA effects instance
 * @param block_count number of blocks to render
 * @param block_size number of samples in a block
 */
void fluid_ladspa_run(fluid_ladspa_fx_t *fx, int block_count, int block_size)
{
    int i;
    int num_samples;
    fluid_ladspa_plugin_t *plugin;

    /* Somebody wants to deactivate the engine, so let's give them a chance to do that.
     * And check that there is at least one plugin loaded, to avoid the overhead of the
     * atomic compare and exchange on an unconfigured LADSPA engine. */
    if (fx->pending_deactivation || fx->num_plugins == 0)
    {
        return;
    }

    /* Inform the engine that we are now running pluings, and bail out if it's not active */
    if (!fluid_atomic_int_compare_and_exchange(&fx->state, FLUID_LADSPA_ACTIVE, FLUID_LADSPA_RUNNING))
    {
        return;
    }

    num_samples = block_count * block_size;

#ifndef WITH_FLOAT
    copy_host_to_plugin_buffers(fx, num_samples);
#endif

    /* Clear all user audio nodes. This is really only necessary if the first plugin
     * writing to the buffer is in 'add' mode. So there's room for optimization here,
     * but it would mean keeping track of which plugin connected to which audio node
     * in which order. */
    for (i = 0; i < fx->num_audio_nodes; i++)
    {
        FLUID_MEMSET(fx->audio_nodes[i]->plugin_buffer, 0, fx->buffer_size * sizeof(LADSPA_Data));
    }

    /* Run each plugin in the order that they were added */
    for (i = 0; i < fx->num_plugins; i++)
    {
        plugin = fx->plugins[i];

        if (plugin->mode == FLUID_LADSPA_MODE_ADD)
        {
            plugin->desc->run_adding(plugin->handle, num_samples);
        }
        else
        {
            plugin->desc->run(plugin->handle, num_samples);
        }
    }

#ifndef WITH_FLOAT
    copy_plugin_to_host_buffers(fx, num_samples);
#endif

    if (!fluid_atomic_int_compare_and_exchange(&fx->state, FLUID_LADSPA_RUNNING, FLUID_LADSPA_ACTIVE))
    {
        FLUID_LOG(FLUID_ERR, "Unable to reset LADSPA running state!");
    }

    /* If deactivation was requested while in running state, notify that we've finished now
     * and deactivation can proceed */
    if (fx->pending_deactivation)
    {
        fluid_cond_mutex_lock(fx->run_finished_mutex);
        fluid_cond_broadcast(fx->run_finished_cond);
        fluid_cond_mutex_unlock(fx->run_finished_mutex);
    }
}

/**
 * Check if the plugin supports the run_adding and set_run_adding_gain
 * interfaces necessary for the FLUID_LADSPA_ADD output mode.
 *
 * @param fx LADSPA fx
 * @param name the name of the plugin
 * @return TRUE if add mode is supported, otherwise FALSE
 */
int fluid_ladspa_plugin_can_add(fluid_ladspa_fx_t *fx, const char *name)
{
    int can_add;
    fluid_ladspa_plugin_t *plugin;

    LADSPA_API_ENTER(fx);

    plugin = get_plugin(fx, name);
    if (plugin == NULL)
    {
        LADSPA_API_RETURN(fx, FALSE);
    }

    can_add = (plugin->desc->run_adding != NULL
            && plugin->desc->set_run_adding_gain != NULL);

    LADSPA_API_RETURN(fx, can_add);
}

/**
 * Set if the plugin should replace everything in the output buffer(s) or add
 * to the buffer(s) with a fixed gain. The default mode is FLUID_LADSPA_MODE_REPLACE.
 *
 * @param fx LADSPA fx instance
 * @param name the name of the plugin
 * @param mode which mode to set: FLUID_LADSPA_MODE_ADD or FLUID_LADSPA_MODE_REPLACE
 * @param gain the gain to apply to the plugin output before adding to output. Ignored for replace mode.
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_ladspa_plugin_mode(fluid_ladspa_fx_t *fx, const char *name,
        fluid_ladspa_mode_t mode, float gain)
{
    fluid_ladspa_plugin_t *plugin;

    LADSPA_API_ENTER(fx);

    if (fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    plugin = get_plugin(fx, name);
    if (plugin == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if (mode == FLUID_LADSPA_MODE_ADD)
    {
        if (!fluid_ladspa_plugin_can_add(fx, name))
        {
            FLUID_LOG(FLUID_ERR, "Plugin '%s' does not support 'add' mode",
                    plugin->desc->Label);
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }

        plugin->desc->set_run_adding_gain(plugin->handle, gain);
    }

    plugin->mode = mode;

    LADSPA_API_RETURN(fx, FLUID_OK);
}

static void clear_ladspa(fluid_ladspa_fx_t *fx)
{
    int i;

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

    /* Delete all user defined nodes (but not the host nodes) */
    for (i = 0; i < fx->num_nodes; i++)
    {
        if (fx->nodes[i]->type != FLUID_LADSPA_NODE_HOST)
        {
            delete_fluid_ladspa_node(fx->nodes[i]);
        }
    }

    /* Fill the list with the host nodes and reset the connection counts */
    for (i = 0; i < fx->num_host_nodes; i++)
    {
        fx->host_nodes[i]->num_inputs = 0;
        fx->host_nodes[i]->num_outputs = 0;
        fx->nodes[i] = fx->host_nodes[i];
    }
    fx->num_nodes = fx->num_host_nodes;

    /* Reset list of user audio nodes */
    fx->num_audio_nodes = 0;
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
 * @param effect_name name of the plugin
 * @param name the port name
 * @return TRUE if port was found, otherwise FALSE
 */
int fluid_ladspa_port_exists(fluid_ladspa_fx_t *fx, const char *effect_name, const char *name)
{
    fluid_ladspa_plugin_t *plugin;
    int port_exists;

    LADSPA_API_ENTER(fx);

    plugin = get_plugin(fx, effect_name);
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

    node = new_fluid_ladspa_node(fx, name, FLUID_LADSPA_NODE_AUDIO, NULL);
    if (node == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Set the value of an effect control port
 *
 * Nodes are searched by case-insensitive string comparison.
 *
 * @param fx LADSPA fx instance
 * @param effect_name name of the effect
 * @param port_name name of the port
 * @param val floating point value
 * @return FLUID_OK on success, FLUID_FAILED on error
 */
int fluid_ladspa_set_control_port(fluid_ladspa_fx_t *fx, const char *effect_name,
        const char *port_name, fluid_real_t val)
{
    fluid_ladspa_node_t *node;
    fluid_ladspa_plugin_t *plugin;
    int port_idx;

    LADSPA_API_ENTER(fx);

    plugin = get_plugin(fx, effect_name);
    if (plugin == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    port_idx = get_plugin_port_idx(plugin, port_name);
    if (port_idx < 0)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    node = plugin->port_nodes[port_idx];
    if (node == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if (node->type != FLUID_LADSPA_NODE_CONTROL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    node->plugin_buffer[0] = val;

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Instantiate a plugin from a LADSPA plugin library
 *
 * @param fx LADSPA effects instance
 * @param effect_name name of the effect (plugin instance)
 * @param lib_name filename of ladspa plugin library
 * @param plugin_name optional, plugin name if there is more than one plugin in the library
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_ladspa_add_plugin(fluid_ladspa_fx_t *fx, const char *effect_name,
        const char *lib_name, const char *plugin_name)
{
    fluid_ladspa_lib_t *lib;
    fluid_ladspa_plugin_t *plugin;

    LADSPA_API_ENTER(fx);
    if (fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if (fx->num_plugins >= FLUID_LADSPA_MAX_PLUGINS)
    {
        FLUID_LOG(FLUID_ERR, "Maximum number of LADSPA plugins reached");
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    lib = get_ladspa_library(fx, lib_name);
    if (lib == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    plugin = new_fluid_ladspa_plugin(fx, lib, plugin_name);
    if (plugin == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    plugin->name = FLUID_STRDUP(effect_name);
    if (plugin->name == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        delete_fluid_ladspa_plugin(plugin);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if (create_control_port_nodes(fx, plugin) != FLUID_OK)
    {
        delete_fluid_ladspa_plugin(plugin);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    fx->plugins[fx->num_plugins++] = plugin;

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Connect an effect port to a name
 *
 * @note There is no corresponding disconnect function. If the connections need to be changed,
 * clear everything with fluid_ladspa_reset and start again from scratch.
 *
 * @param fx LADSPA effects instance
 * @param effect_name name of the effect
 * @param port_name the port name to connect to (case-insensitive prefix match, see get_plugin_port_idx)
 * @param node_name the node name to connect to (case-insensitive)
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_ladspa_connect(fluid_ladspa_fx_t *fx, const char *effect_name,
        const char *port_name, const char *node_name)
{
    fluid_ladspa_plugin_t *plugin;
    fluid_ladspa_node_t *node;
    int port_idx;
    int port_flags;
    int dir;

    LADSPA_API_ENTER(fx);

    if (fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    plugin = get_plugin(fx, effect_name);
    if (plugin == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Effect '%s' not found", effect_name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    port_idx = get_plugin_port_idx(plugin, port_name);
    if (port_idx < 0)
    {
        FLUID_LOG(FLUID_ERR, "Port '%s' not found on effect '%s'", port_name, effect_name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    node = get_node(fx, node_name);
    if (node == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Node '%s' not found", node_name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    port_flags = plugin->desc->PortDescriptors[port_idx];

    /* Check that requested port type matches the node type */
    if (LADSPA_IS_PORT_CONTROL(port_flags) && node->type != FLUID_LADSPA_NODE_CONTROL)
    {
        FLUID_LOG(FLUID_ERR, "Cannot connect control port '%s' on effect '%s' "
                "to an audio node", port_name, effect_name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }
    else if (LADSPA_IS_PORT_AUDIO(port_flags) && node->type == FLUID_LADSPA_NODE_CONTROL)
    {
        FLUID_LOG(FLUID_ERR, "Cannot connect audio port '%s' on effect '%s' "
                "to a control node", port_name, effect_name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if (LADSPA_IS_PORT_INPUT(port_flags))
    {
        dir = FLUID_LADSPA_INPUT;
    }
    else
    {
        dir = FLUID_LADSPA_OUTPUT;
    }

    FLUID_LOG(FLUID_DBG, "Connecting effect '%s': port '%s' %s node '%s'",
              effect_name, port_name,
              (dir == FLUID_LADSPA_INPUT) ? "<" : ">", node_name);

    connect_node_to_port(node, dir, plugin, port_idx);

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
    const char *str;
    const char *str2;
    fluid_ladspa_plugin_t *plugin;

    LADSPA_API_ENTER(fx);

    /* Check that there is at least one plugin */
    if (fx->num_plugins == 0)
    {
        FLUID_SNPRINTF(err, err_size, "No plugins loaded\n");
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    for (i = 0; i < fx->num_plugins; i++)
    {
        plugin = fx->plugins[i];

        if (check_all_ports_connected(plugin, &str) == FLUID_FAILED)
        {
            FLUID_SNPRINTF(err, err_size, "Port '%s' on plugin '%s' is not connected\n",
                            str, plugin->desc->Label);
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }

        if (check_no_inplace_broken(plugin, &str, &str2) == FLUID_FAILED)
        {
                FLUID_SNPRINTF(err, err_size,
                        "Plugin '%s' is in-place broken, '%s' and '%s' are not allowed "
                        "to connect to the same node\n", plugin->desc->Label, str, str2);
                LADSPA_API_RETURN(fx, FLUID_FAILED);
        }
    }

    if (check_host_output_used(fx) == FLUID_FAILED)
    {
        FLUID_SNPRINTF(err, err_size, "No plugin outputs to one the host nodes\n");
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if (check_all_audio_nodes_connected(fx, &str) == FLUID_FAILED)
    {
        FLUID_SNPRINTF(err, err_size, "Audio node '%s' is not fully connected\n", str);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    LADSPA_API_RETURN(fx, FLUID_OK);
}


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
 * If name is optional if the library contains only one plugin.
 *
 * @param lib pointer to fluid_ladspa_lib_t instance
 * @param name name (LADSPA Label) of the plugin
 * @return pointer to LADSPA_Descriptor, NULL on error or if not found
 */
static const LADSPA_Descriptor *get_plugin_descriptor(const fluid_ladspa_lib_t *lib, const char *name)
{
    const LADSPA_Descriptor *desc;
    const LADSPA_Descriptor *last_desc = NULL;
    int i = 0;

    for (i = 0; /* endless */; i++)
    {
        desc = lib->descriptor(i);
        if (desc == NULL)
            break;

        if (name != NULL && FLUID_STRCMP(desc->Label, name) == 0)
        {
            return desc;
        }

        last_desc = desc;
    }

    if (name == NULL)
    {
        if (i == 1)
        {
            return last_desc;
        }
        FLUID_LOG(FLUID_ERR, "Library contains more than one plugin, please specify "
                "the plugin label");
    }

    return NULL;
}

/**
 * Instantiate a new LADSPA plugin from a library and set up the associated
 * control structures needed by the LADSPA fx engine.
 *
 * If the library contains only one plugin, then the name is optional.
 *
 * Plugins are identified by their "Label" in the plugin descriptor structure.
 *
 * @param fx LADSPA fx instance
 * @param lib pointer to fluid_ladspa_lib_t
 * @param name (optional) string name of the plugin (the LADSPA Label)
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

    plugin->port_nodes = FLUID_ARRAY(fluid_ladspa_node_t*, plugin->desc->PortCount);
    if (plugin->port_nodes == NULL)
    {
        delete_fluid_ladspa_plugin(plugin);
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    FLUID_MEMSET(plugin->port_nodes, 0, plugin->desc->PortCount * sizeof(fluid_ladspa_node_t*));

    return plugin;
}

static void delete_fluid_ladspa_plugin(fluid_ladspa_plugin_t *plugin)
{
    if (plugin == NULL)
    {
        return;
    }

    if (plugin->port_nodes != NULL)
    {
        FLUID_FREE(plugin->port_nodes);
    }

    if (plugin->handle != NULL && plugin->desc != NULL && plugin->desc->cleanup != NULL)
    {
        plugin->desc->cleanup(plugin->handle);
    }

    if (plugin->name != NULL)
    {
        FLUID_FREE(plugin->name);
    }

    FLUID_FREE(plugin);
}

static fluid_ladspa_node_t *new_fluid_ladspa_node(fluid_ladspa_fx_t *fx, const char *name,
        fluid_ladspa_node_type_t type, fluid_real_t *host_buffer)
{
    int buffer_size;
    fluid_ladspa_node_t *node;

    /* For named nodes, make sure that the name is unique */
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

    node->name = FLUID_STRDUP(name);
    if (node->name == NULL)
    {
        delete_fluid_ladspa_node(node);
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    node->type = type;
    node->host_buffer = host_buffer;

    if (type == FLUID_LADSPA_NODE_HOST)
    {
        if (node->host_buffer == NULL)
        {
            delete_fluid_ladspa_node(node);
            return NULL;
        }

#ifdef WITH_FLOAT
        /* If the host uses the same floating-point width as LADSPA, then plugins
         * can work in-place on the host buffer. Otherwise well need a separate
         * buffer for type conversion. */
        node->plugin_buffer = node->host_buffer;
#endif
    }

    if (node->plugin_buffer == NULL)
    {
        /* Control nodes only store a single floating-point value */
        buffer_size = (type == FLUID_LADSPA_NODE_CONTROL) ? 1 : fx->buffer_size;

        node->plugin_buffer = FLUID_ARRAY(LADSPA_Data, buffer_size);
        if (node->plugin_buffer == NULL)
        {
            delete_fluid_ladspa_node(node);
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return NULL;
        }
        FLUID_MEMSET(node->plugin_buffer, 0, buffer_size * sizeof(LADSPA_Data));
    }

    fx->nodes[fx->num_nodes++] = node;

    /* Host and user audio nodes are also noted in separate lists to access them
     * quickly during fluid_ladspa_run */
    if (type == FLUID_LADSPA_NODE_HOST)
    {
        fx->host_nodes[fx->num_host_nodes++] = node;
    }
    else if (type == FLUID_LADSPA_NODE_AUDIO)
    {
        fx->audio_nodes[fx->num_audio_nodes++] = node;
    }

    return node;
}

static void delete_fluid_ladspa_node(fluid_ladspa_node_t *node)
{
    /* If plugin_buffer the same as host_buffer, then the plugin_buffer has been
     * provided externally, so don't free */
    if ((node->plugin_buffer != NULL) && ((void *)node->plugin_buffer != (void *)node->host_buffer))
    {
        FLUID_FREE(node->plugin_buffer);
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
 * Retrieve a ladspa_plugin_t instance by it's name.
 *
 * @param fx LADSPA effects instance
 * @param name plugin name
 * @return pointer to plugin or NULL if not found
 */
static fluid_ladspa_plugin_t *get_plugin(fluid_ladspa_fx_t *fx, const char *name)
{
    int i;

    LADSPA_API_ENTER(fx);

    for (i = 0; i < fx->num_plugins; i++)
    {
        if (FLUID_STRNCASECMP(fx->plugins[i]->name, name, FLUID_STRLEN(name)) == 0)
        {
            LADSPA_API_RETURN(fx, fx->plugins[i]);
        }
    }

    printf("Effect '%s' not found!\n", name);
    LADSPA_API_RETURN(fx, NULL);
}

/**
 * Set the passed in float pointer to the default value of a plugin port, as specified
 * by the LADSPA port hints. If no default hints are found or the port is not a control
 * node, it returns 0.0f;
 *
 * The sample rate is needed because some LADSPA port default hints are expressed as a
 * fraction of the current sample rate.
 *
 * @param plugin pointer to plugin instance
 * @param port_idx index of the port in the plugin
 * @param sample_rate the current sample rate of the LADSPA fx
 * @return default port value or 0.0f
 */
static LADSPA_Data get_default_port_value(fluid_ladspa_plugin_t *plugin, unsigned int port_idx,
        int sample_rate)
{
    const LADSPA_PortRangeHint *hint;
    LADSPA_PortRangeHintDescriptor flags;
    LADSPA_Data value = 0.0;
    float low_factor = 0.0;
    float high_factor = 0.0;

    if (port_idx >= plugin->desc->PortCount)
    {
        return value;
    }

    hint = &plugin->desc->PortRangeHints[port_idx];
    flags = hint->HintDescriptor;

    if (!LADSPA_IS_HINT_HAS_DEFAULT(flags))
    {
        return value;
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

    return value;
}

/**
 * Create a control node for each control port on the passed in effect. The value of the
 * node is taken from port hint defaults, if available. This gets run automatically after
 * an effect has been added.
 *
 * @param fx LADSPA fx instance
 * @param plugin plugin instance
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
static int create_control_port_nodes(fluid_ladspa_fx_t *fx, fluid_ladspa_plugin_t *plugin)
{
    unsigned int i;
    fluid_ladspa_node_t *node;
    fluid_ladspa_dir_t dir;
    int port_flags;

    for (i = 0; i < plugin->desc->PortCount; i++)
    {
        port_flags = plugin->desc->PortDescriptors[i];

        if (!LADSPA_IS_PORT_CONTROL(port_flags))
            continue;

        node = new_fluid_ladspa_node(fx, "", FLUID_LADSPA_NODE_CONTROL, NULL);
        if (node == NULL)
        {
            return FLUID_FAILED;
        }

        node->plugin_buffer[0] = get_default_port_value(plugin, i, fx->sample_rate);

        dir = (LADSPA_IS_PORT_INPUT(port_flags)) ? FLUID_LADSPA_INPUT : FLUID_LADSPA_OUTPUT;

        connect_node_to_port(node, dir, plugin, i);
    }

    return FLUID_OK;
}

static void connect_node_to_port(fluid_ladspa_node_t *node, fluid_ladspa_dir_t dir,
        fluid_ladspa_plugin_t *plugin, int port_idx)
{
    plugin->desc->connect_port(plugin->handle, port_idx, node->plugin_buffer);
    plugin->port_nodes[port_idx] = node;

    /* Mark node as connected in the respective direction */
    if (dir == FLUID_LADSPA_INPUT)
    {
        node->num_outputs++;
    }
    else
    {
        node->num_inputs++;
    }
}


/**
 * Check that all ports on the plugin are connected to a node.
 *
 * @param plugin LADSPA plugin instance
 * @param name if check fails, points to the name of first failed port
 * @return FLUID_OK on successful check, otherwise FLUID_FAILED
 */
static int check_all_ports_connected(fluid_ladspa_plugin_t *plugin, const char **name)
{
    unsigned int i;

    for (i = 0; i < plugin->desc->PortCount; i++)
    {
        if (plugin->port_nodes[i] == NULL)
        {
            *name = plugin->desc->PortNames[i];
            return FLUID_FAILED;
        }
    }
    return FLUID_OK;
}

/**
 * In-place broken plugins can't cope with input and output audio ports connected
 * to the same buffer. Check for this condition in the plugin.
 *
 * @param plugin LADSPA plugin instance
 * @param name1 if check fails, points to the first port name
 * @param name2 if check fails, points to the second port name
 * @return FLUID_OK on successful check, otherwise FLUID_FAILED
 */
static int check_no_inplace_broken(fluid_ladspa_plugin_t *plugin, const char **name1, const char **name2)
{
    unsigned int i, k;
    LADSPA_PortDescriptor flags1, flags2;

    if (!LADSPA_IS_INPLACE_BROKEN(plugin->desc->Properties))
    {
        return FLUID_OK;
    }

    for (i = 0; i < plugin->desc->PortCount; i++)
    {
        flags1 = plugin->desc->PortDescriptors[i];

        for (k = 0; k < plugin->desc->PortCount; k++)
        {
            flags2 = plugin->desc->PortDescriptors[k];

            if (i != k
                && plugin->port_nodes[i]->plugin_buffer == plugin->port_nodes[k]->plugin_buffer
                && (flags1 & 0x3) != (flags2 & 0x3) /* first two bits encode direction */
                && LADSPA_IS_PORT_AUDIO(flags1) && LADSPA_IS_PORT_AUDIO(flags2))
            {
                *name1 = plugin->desc->PortNames[i];
                *name2 = plugin->desc->PortNames[k];
                return FLUID_FAILED;
            }
        }
    }
    return FLUID_OK;
}

/**
 * Check that at least one host node is used by a plugin
 *
 * @param fx LADSPA fx instance
 * @return FLUID_OK on successful check, otherwise FLUID_FAILED
 */
static int check_host_output_used(fluid_ladspa_fx_t *fx)
{
    int i;

    for (i = 0; i < fx->num_host_nodes; i++)
    {
        if (fx->host_nodes[i]->num_inputs)
        {
            return FLUID_OK;
        }
    }
    return FLUID_FAILED;
}

/**
 * Check that all user audio nodes have an input and an output
 *
 * @param fx LADSPA fx instance
 * @param name if check fails, points to the name of first failed node
 * @return FLUID_OK on successful check, otherwise FLUID_FAILED
 */
static int check_all_audio_nodes_connected(fluid_ladspa_fx_t *fx, const char **name)
{
    int i;

    for (i = 0; i < fx->num_audio_nodes; i++)
    {
        if (fx->audio_nodes[i]->num_inputs == 0 || fx->audio_nodes[i]->num_outputs == 0)
        {
            *name = fx->audio_nodes[i]->name;
            return FLUID_FAILED;
        }
    }
    return FLUID_OK;
}

static FLUID_INLINE void mix_plugin_with_host_buffer(fluid_ladspa_node_t *node,
        int num_samples, float gain)
{
    int i;

    for (i = 0; i < num_samples; i++)
    {
        node->host_buffer[i] += node->plugin_buffer[i] * gain;
    }
}

#ifndef WITH_FLOAT
/**
 * Copy and type convert host buffers to plugin buffers. Used only if host and LADSPA
 * use different float types.
 */
static FLUID_INLINE void copy_host_to_plugin_buffers(fluid_ladspa_fx_t *fx, int num_samples)
{
    int i, n;
    fluid_ladspa_node_t  *node;

    for (n = 0; n < fx->num_host_nodes; n++)
    {
        node = fx->host_nodes[n];
        /* Only copy host nodes that have at least one output or output, i.e.
         * that are connected to at least one plugin port. */
        if (node->num_inputs > 0 || node->num_outputs > 0)
        {
            for (i = 0; i < num_samples; i++)
            {
                node->plugin_buffer[i] = (LADSPA_Data)node->host_buffer[i];
            }
        }
    }
}

/**
 * Copy and type convert plugin buffers to host buffers. Used only if host and LADSPA
 * use different float types.
 */
static FLUID_INLINE void copy_plugin_to_host_buffers(fluid_ladspa_fx_t *fx, int num_samples)
{
    int i, n;
    fluid_ladspa_node_t  *node;

    for (n = 0; n < fx->num_host_nodes; n++)
    {
        node = fx->host_nodes[n];
        /* Only copy plugin nodes that have at least one input, i.e. that are connected to
         * at least one plugin output */
        if (node->num_inputs > 0)
        {
            for (i = 0; i < num_samples; i++)
            {
                node->host_buffer[i] = (fluid_real_t)node->plugin_buffer[i];
            }
        }
    }
}
#endif /* WITH_FLOAT */
