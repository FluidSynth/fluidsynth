/* Haiku MidiKit2 Driver for FluidSynth
 *
 * Copyright (C) 2022  Cacodemon345
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

/* Parts of code taken from the CoreMIDI driver. */

extern "C" {
#include "fluid_midi.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"
}

#include <MidiKit.h>
#include <midi2/MidiRoster.h>
#include <midi2/MidiConsumer.h>
#include <midi2/MidiProducer.h>
#include <SupportKit.h>

typedef struct
{
    fluid_midi_driver_t driver;
    fluid_midi_parser_t* parser;

    BMidiLocalConsumer *m_consumer;
    int autoconn_inputs;
} fluid_midikit2_driver_t;

class fluid_midikit2_input : public BMidiLocalConsumer
{
    void Data(uchar *data, size_t length, bool atomic, bigtime_t time) override
    {
        if (atomic && callback)
        {
            fluid_midi_event_t *event = NULL;
            snooze_until(time, B_SYSTEM_TIMEBASE);
            for (int i = 0; i < length; i++)
            {
                event = fluid_midi_parser_parse(callback->parser, data[i]);
            }
            if (event)
            {
                callback->driver.handler(callback->driver.data, event);
            }
        }
    }
public:
    fluid_midikit2_driver_t* callback;
};

void fluid_midikit2_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "midi.midikit2.id", "pid", 0);
}

static void fluid_coremidi_autoconnect(fluid_midikit2_driver_t *dev)
{
    int32 id = 0;
    BMidiProducer* producer = NULL;
    while ((producer = BMidiRoster::NextProducer(&id)) != NULL)
    {
        producer->Connect(dev->m_consumer);
        producer->Release();
    }
}

fluid_midi_driver_t*
new_fluid_midikit2_driver(fluid_settings_t *settings, handle_midi_event_func_t handler, void *data)
{
    fluid_midikit2_driver_t* driver = NULL;
    char* id = NULL;
    char clientid[128];

    memset(clientid, 0, sizeof(clientid));
    driver = FLUID_MALLOC(sizeof(fluid_midikit2_driver_t));

    if(driver == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    driver->driver.handler = handler;
    driver->driver.data = data;

    driver->parser = new_fluid_midi_parser();

    if(driver->parser == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        FLUID_FREE(driver);
        return NULL;
    }

    fluid_settings_dupstr(settings, "midi.midikit2.id", &id);
    if(id != NULL)
    {
        if(FLUID_STRCMP(id, "pid") == 0)
        {
            FLUID_SNPRINTF(clientid, sizeof(clientid), "FluidSynth virtual port (%d)", getpid());
        }
        else
        {
            FLUID_SNPRINTF(clientid, sizeof(clientid), "FluidSynth virtual port (%s)", id);
        }

        FLUID_FREE(id);   /* -- free id string */
    }
    else strncpy(clientid, "FluidSynth virtual port", sizeof(clientid));

    driver->m_consumer = new fluid_midikit2_input;
    if(!driver->m_consumer->IsValid())
    {
        FLUID_LOG(FLUID_ERR, "Failed to create the MIDI consumer");
        driver->m_consumer->Release();
        delete_fluid_midi_parser(driver->parser);
        FLUID_FREE(driver);
        return NULL;
    }

    ((fluid_midikit2_input*)driver->m_consumer)->callback = driver;
    driver->m_consumer->SetName(clientid);
    driver->m_consumer->Register();
    fluid_settings_getint(settings, "midi.autoconnect", &driver->autoconn_inputs);

    if (driver->autoconn_inputs)
    {
        fluid_coremidi_autoconnect(driver);
    }

    return (fluid_midi_driver_t *)driver;
}

void
delete_fluid_midikit2_driver(fluid_midi_driver_t *p)
{
    fluid_return_if_fail(p != NULL);
    if (p)
    {
        fluid_midikit2_driver_t* driver = (fluid_midikit2_driver_t*)p;
        driver->m_consumer->Unregister();
        driver->m_consumer->Release();
        delete_fluid_midi_parser(driver->parser);
        FLUID_FREE(driver);
    }
}
