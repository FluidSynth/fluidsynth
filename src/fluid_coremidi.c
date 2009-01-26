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

/* fluid_coremidi.c
 *
 * Driver for Mac OSX native MIDI
 * Pedro Lopez-Cabanillas, Jan 2009
 */

#include "fluidsynth_priv.h"

#if COREMIDI_SUPPORT

#include "fluid_midi.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#include <unistd.h>
#include <CoreServices/CoreServices.h>
#include <CoreMIDI/MIDIServices.h>

typedef struct {
  fluid_midi_driver_t driver;
  MIDIClientRef client;
  MIDIEndpointRef endpoint;
  fluid_midi_parser_t* parser;
} fluid_coremidi_driver_t;

fluid_midi_driver_t* new_fluid_coremidi_driver(fluid_settings_t* settings,
                       handle_midi_event_func_t handler, void* data);
int delete_fluid_coremidi_driver(fluid_midi_driver_t* p);
void fluid_coremidi_callback(const MIDIPacketList *list, void *p, void *src);

void fluid_coremidi_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "midi.coremidi.id", "pid", 0, NULL, NULL);
}

/*
 * new_fluid_coremidi_driver
 */
fluid_midi_driver_t*
new_fluid_coremidi_driver(fluid_settings_t* settings, handle_midi_event_func_t handler, void* data)
{
  fluid_coremidi_driver_t* dev;
  MIDIClientRef client;
  MIDIEndpointRef endpoint;
  char * portname;
  char * id;
  CFStringRef str_portname;
  CFStringRef str_clientname;

  /* not much use doing anything */
  if (handler == NULL) {
    FLUID_LOG(FLUID_ERR, "Invalid argument");
    return NULL;
  }

  dev = FLUID_MALLOC(sizeof(fluid_coremidi_driver_t));
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  dev->client = 0;
  dev->endpoint = 0;
  dev->parser = 0;
  dev->driver.handler = handler;
  dev->driver.data = data;

  dev->parser = new_fluid_midi_parser();
  if (dev->parser == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    goto error_recovery;
  }

  fluid_settings_getstr(settings, "midi.coremidi.id", &id);
  if (FLUID_STRCMP(id, "pid") == 0)
    str_clientname = CFStringCreateWithFormat(NULL, NULL,
                                              CFSTR("FluidSynth %qi"),
                                              (long long) getpid());
  else
    str_clientname = CFStringCreateWithFormat(NULL, NULL,
                                              CFSTR("FluidSynth %s"), id);

  fluid_settings_getstr(settings, "midi.portname", &portname);
  if (strlen(portname) == 0)
    str_portname = CFStringCreateWithFormat(NULL, NULL,
                                            CFSTR("FluidSynth virtual port %qi"),
                                            (long long) getpid());
  else
    str_portname = CFStringCreateWithCString(NULL, portname,
                                             kCFStringEncodingASCII);

  OSStatus result = MIDIClientCreate( str_clientname, NULL, NULL, &client );
  if ( result != noErr ) {
    FLUID_LOG(FLUID_ERR, "Failed to create the MIDI input client");
    goto error_recovery;
  }
  dev->client = client;

  result = MIDIDestinationCreate( client, str_portname,
                                  fluid_coremidi_callback,
                                  (void *)dev, &endpoint );
  if ( result != noErr ) {
    FLUID_LOG(FLUID_ERR, "Failed to create the MIDI input port. MIDI input not available.");
    goto error_recovery;
  }
  dev->endpoint = endpoint;

  return (fluid_midi_driver_t*) dev;

error_recovery:
  delete_fluid_coremidi_driver((fluid_midi_driver_t*) dev);
  return NULL;
}

/*
 * delete_fluid_coremidi_driver
 */
int
delete_fluid_coremidi_driver(fluid_midi_driver_t* p)
{
  fluid_coremidi_driver_t* dev = (fluid_coremidi_driver_t*) p;
  if (dev->client != NULL) {
    MIDIClientDispose(dev->client);
  }
  if (dev->endpoint != NULL) {
    MIDIEndpointDispose(dev->endpoint);
  }
  if (dev->parser != NULL) {
    delete_fluid_midi_parser(dev->parser);
  }
  FLUID_FREE(dev);
  return 0;
}

void
fluid_coremidi_callback(const MIDIPacketList *list, void *p, void *src)
{
  unsigned int i, j;
  fluid_midi_event_t* event;
  fluid_coremidi_driver_t* dev = (fluid_coremidi_driver_t *)p;
  const MIDIPacket *packet = &list->packet[0];
  for ( i = 0; i < list->numPackets; ++i ) {
    for ( j = 0; j < packet->length; ++j ) {
      event = fluid_midi_parser_parse(dev->parser, packet->data[j]);
      if (event != NULL) {
        (*dev->driver.handler)(dev->driver.data, event);
      }
    }
    packet = MIDIPacketNext(packet);
  }
}

#endif /* COREMIDI_SUPPORT */

