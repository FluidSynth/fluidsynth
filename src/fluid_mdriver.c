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

#include "fluid_mdriver.h"
#include "fluid_settings.h"


/* ALSA */
#if ALSA_SUPPORT
fluid_midi_driver_t* new_fluid_alsa_rawmidi_driver(fluid_settings_t* settings,
						 handle_midi_event_func_t handler, 
						 void* event_handler_data);
int delete_fluid_alsa_rawmidi_driver(fluid_midi_driver_t* p);
void fluid_alsa_rawmidi_driver_settings(fluid_settings_t* settings);

fluid_midi_driver_t* new_fluid_alsa_seq_driver(fluid_settings_t* settings,
					     handle_midi_event_func_t handler, 
					     void* event_handler_data);
int delete_fluid_alsa_seq_driver(fluid_midi_driver_t* p);
void fluid_alsa_seq_driver_settings(fluid_settings_t* settings);
#endif

/* OSS */
#if OSS_SUPPORT
fluid_midi_driver_t* new_fluid_oss_midi_driver(fluid_settings_t* settings,
					     handle_midi_event_func_t handler, 
					     void* event_handler_data);
int delete_fluid_oss_midi_driver(fluid_midi_driver_t* p);
void fluid_oss_midi_driver_settings(fluid_settings_t* settings);
#endif

/* Windows MIDI service */
#if WINMIDI_SUPPORT
fluid_midi_driver_t* new_fluid_winmidi_driver(fluid_settings_t* settings,
					    handle_midi_event_func_t handler, 
					    void* event_handler_data);
int delete_fluid_winmidi_driver(fluid_midi_driver_t* p);
#endif

/* definitions for the MidiShare driver */
#if MIDISHARE_SUPPORT
fluid_midi_driver_t* new_fluid_midishare_midi_driver(fluid_settings_t* settings,
						   void* event_handler_data, 
						   handle_midi_event_func_t handler);
int delete_fluid_midishare_midi_driver(fluid_midi_driver_t* p);
#endif



/*
 * fluid_mdriver_definition
 */
struct fluid_mdriver_definition_t {
  char* name;
  fluid_midi_driver_t* (*new)(fluid_settings_t* settings, 
			     handle_midi_event_func_t event_handler, 
			     void* event_handler_data);
  int (*free)(fluid_midi_driver_t* p);
  void (*settings)(fluid_settings_t* settings);
};


struct fluid_mdriver_definition_t fluid_midi_drivers[] = {
#if OSS_SUPPORT
  { "oss", 
    new_fluid_oss_midi_driver, 
    delete_fluid_oss_midi_driver, 
    fluid_oss_midi_driver_settings },
#endif
#if ALSA_SUPPORT
  { "alsa_raw", 
    new_fluid_alsa_rawmidi_driver, 
    delete_fluid_alsa_rawmidi_driver, 
    fluid_alsa_rawmidi_driver_settings },
  { "alsa_seq", 
    new_fluid_alsa_seq_driver, 
    delete_fluid_alsa_seq_driver, 
    fluid_alsa_seq_driver_settings },
#endif
#if WINMIDI_SUPPORT
  { "winmidi", 
    new_fluid_winmidi_driver, 
    delete_fluid_winmidi_driver, 
    NULL },
#endif
#if MIDISHARE_SUPPORT
  { "midishare", 
    new_fluid_midishare_midi_driver, 
    delete_fluid_midishare_midi_driver, 
    NULL },
#endif
  { NULL, NULL, NULL, NULL }
};



void fluid_midi_driver_settings(fluid_settings_t* settings)
{  
  int i;

  /* Set the default driver */
#if ALSA_SUPPORT
  fluid_settings_register_str(settings, "midi.driver", "alsa_seq", 0, NULL, NULL);
#elif OSS_SUPPORT
  fluid_settings_register_str(settings, "midi.driver", "oss", 0, NULL, NULL);
#elif WINMIDI_SUPPORT
  fluid_settings_register_str(settings, "midi.driver", "winmidi", 0, NULL, NULL);
#elif MIDISHARE_SUPPORT
  fluid_settings_register_str(settings, "midi.driver", "midishare", 0, NULL, NULL);
#else
  fluid_settings_register_str(settings, "midi.driver", "", 0, NULL, NULL);
#endif

  /* Add all drivers to the list of options */
#if ALSA_SUPPORT
  fluid_settings_add_option(settings, "midi.driver", "alsa_seq");
  fluid_settings_add_option(settings, "midi.driver", "alsa_raw");
#endif
#if OSS_SUPPORT
  fluid_settings_add_option(settings, "midi.driver", "oss");
#endif
#if WINMIDI_SUPPORT
  fluid_settings_add_option(settings, "midi.driver", "winmidi");
#endif
#if MIDISHARE_SUPPORT
  fluid_settings_add_option(settings, "midi.driver", "midishare");
#endif

  for (i = 0; fluid_midi_drivers[i].name != NULL; i++) {
    if (fluid_midi_drivers[i].settings != NULL) {
      fluid_midi_drivers[i].settings(settings);
    }
  }  
}



fluid_midi_driver_t* new_fluid_midi_driver(fluid_settings_t* settings, handle_midi_event_func_t handler, void* event_handler_data)
{
  int i;
  fluid_midi_driver_t* driver = NULL;
  for (i = 0; fluid_midi_drivers[i].name != NULL; i++) {
    if (fluid_settings_str_equal(settings, "midi.driver", fluid_midi_drivers[i].name)) {
      FLUID_LOG(FLUID_DBG, "Using '%s' midi driver", fluid_midi_drivers[i].name);
      driver = fluid_midi_drivers[i].new(settings, handler, event_handler_data);
      if (driver) {
	driver->name = fluid_midi_drivers[i].name;
      }
      return driver;
    }
  }

  FLUID_LOG(FLUID_ERR, "Couldn't find the requested midi driver");
  return NULL;
}

void delete_fluid_midi_driver(fluid_midi_driver_t* driver)
{
  int i;

  for (i = 0; fluid_midi_drivers[i].name != NULL; i++) {
    if (fluid_midi_drivers[i].name == driver->name) {
      fluid_midi_drivers[i].free(driver);
      return;
    }
  }  
}
