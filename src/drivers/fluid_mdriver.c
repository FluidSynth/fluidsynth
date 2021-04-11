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

#include "fluid_mdriver.h"
#include "fluid_settings.h"


/*
 * fluid_mdriver_definition
 */
struct _fluid_mdriver_definition_t
{
    const char *name;
    fluid_midi_driver_t *(*new)(fluid_settings_t *settings,
                                handle_midi_event_func_t event_handler,
                                void *event_handler_data,
                                int flags);
    void (*free)(fluid_midi_driver_t *p);
    void (*settings)(fluid_settings_t *settings);
    int flags;
};


static const fluid_mdriver_definition_t fluid_midi_drivers[] =
{
#if JACK_SUPPORT
    {
        "jack_client",
        new_fluid_jack_midi_driver,
        delete_fluid_jack_midi_driver,
        fluid_jack_midi_driver_settings,
        0,
    },
#endif
#if ALSA_SUPPORT
    {
        "alsa_seq",
        new_fluid_alsa_seq_driver,
        delete_fluid_alsa_seq_driver,
        fluid_alsa_seq_driver_settings,
        0,
    },
    {
        "alsa_raw",
        new_fluid_alsa_rawmidi_driver,
        delete_fluid_alsa_rawmidi_driver,
        fluid_alsa_rawmidi_driver_settings,
        0,
    },
#endif
#if OSS_SUPPORT
    {
        "oss",
        new_fluid_oss_midi_driver,
        delete_fluid_oss_midi_driver,
        fluid_oss_midi_driver_settings,
        0,
    },
#endif
#if JACK_SUPPORT
    {
        "jack",
        new_fluid_jack_midi_driver,
        delete_fluid_jack_midi_driver,
        fluid_jack_midi_driver_settings,
        FLUID_MDRIVER_RETRY,
    },
#endif
#if WINMIDI_SUPPORT
    {
        "winmidi",
        new_fluid_winmidi_driver,
        delete_fluid_winmidi_driver,
        fluid_winmidi_midi_driver_settings,
        0,
    },
#endif
#if MIDISHARE_SUPPORT
    {
        "midishare",
        new_fluid_midishare_midi_driver,
        delete_fluid_midishare_midi_driver,
        NULL,
        0,
    },
#endif
#if COREMIDI_SUPPORT
    {
        "coremidi",
        new_fluid_coremidi_driver,
        delete_fluid_coremidi_driver,
        fluid_coremidi_driver_settings,
        0,
    },
#endif
    /* NULL terminator to avoid zero size array if no driver available */
    { NULL, NULL, NULL, NULL, 0 }
};


void fluid_midi_driver_settings(fluid_settings_t *settings)
{
    unsigned int i;

    fluid_settings_register_int(settings, "midi.autoconnect", 0, 0, 1, FLUID_HINT_TOGGLED);

    fluid_settings_register_int(settings, "midi.realtime-prio",
                                FLUID_DEFAULT_MIDI_RT_PRIO, 0, 99, 0);
    
    fluid_settings_register_str(settings, "midi.driver", "", 0);

    for(i = 0; i < FLUID_N_ELEMENTS(fluid_midi_drivers) - 1; i++)
    {
        /* Add the driver to the list of options */
        fluid_settings_add_option(settings, "midi.driver", fluid_midi_drivers[i].name);

        if(fluid_midi_drivers[i].settings != NULL)
        {
            fluid_midi_drivers[i].settings(settings);
        }
    }

    fluid_settings_setstr(settings, "midi.driver", "auto");
}

/**
 * Create a new MIDI driver instance.
 *
 * @param settings Settings used to configure new MIDI driver. See \ref settings_midi for available options.
 * @param handler MIDI handler callback (for example: fluid_midi_router_handle_midi_event()
 *   for MIDI router)
 * @param event_handler_data Caller defined data to pass to 'handler'
 * @return New MIDI driver instance or NULL on error
 *
 * Which MIDI driver is actually created depends on the \ref settings_midi_driver option.
 */
fluid_midi_driver_t *new_fluid_midi_driver(fluid_settings_t *settings, handle_midi_event_func_t handler, void *event_handler_data)
{
    fluid_midi_driver_t *driver = NULL;
    char *valid_options;
    char *selected_option;
    const fluid_mdriver_definition_t *def;
    int probe = fluid_settings_str_equal(settings, "midi.driver", "auto");
    int flags;

    if (probe)
    {
        FLUID_LOG(FLUID_INFO, "Trying to auto-select a MIDI driver");
    }

    for(def = fluid_midi_drivers; def->name != NULL; def++)
    {
        if(!probe && !fluid_settings_str_equal(settings, "midi.driver", def->name))
        {
            continue;
        }

        FLUID_LOG(FLUID_DBG, "Trying '%s' MIDI driver", def->name);

        flags = def->flags;
        if (probe)
        {
            flags |= FLUID_MDRIVER_PROBE;
        }

        driver = def->new(settings, handler, event_handler_data, flags);

        if(driver)
        {
            if (probe)
            {
                FLUID_LOG(FLUID_INFO, "Using '%s' MIDI driver", def->name);
            }
            driver->define = def;
            return driver;
        }

        FLUID_LOG(FLUID_DBG, "'%s' MIDI driver failed to start", def->name);

        if (probe)
        {
            continue;
        }

        break;
    }

    valid_options = fluid_settings_option_concat(settings, "midi.driver", NULL);
    if (valid_options == NULL || valid_options[0] == '\0')
    {
        FLUID_FREE(valid_options);
        FLUID_LOG(FLUID_ERR, "No MIDI drivers available.");
        return NULL;
    }

    if (probe)
    {
        FLUID_LOG(FLUID_ERR,
                "Couldn't auto-select a MIDI driver, tried %s",
                valid_options);
    }
    else
    {
        fluid_settings_dupstr(settings, "midi.driver", &selected_option);
        if (fluid_settings_option_is_valid(settings, "midi.driver", selected_option))
        {
            FLUID_LOG(FLUID_ERR, "Couldn't start the requested MIDI driver '%s'.",
                    selected_option ? selected_option : "NULL");
        }
        else
        {
            FLUID_LOG(FLUID_ERR, "Invalid MIDI driver '%s'.",
                    selected_option ? selected_option : "NULL");
            FLUID_LOG(FLUID_INFO, "Valid MIDI drivers are: %s", valid_options);
        }
        FLUID_FREE(selected_option);
    }

    FLUID_FREE(valid_options);

    return NULL;
}

/**
 * Delete a MIDI driver instance.
 * @param driver MIDI driver to delete
 */
void delete_fluid_midi_driver(fluid_midi_driver_t *driver)
{
    fluid_return_if_fail(driver != NULL);
    driver->define->free(driver);
}
