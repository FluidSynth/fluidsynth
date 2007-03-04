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


/* fluid_winmidi.c
 *
 * Drivers for Windows MIDI
 */

#include "fluidsynth_priv.h"

#if WINMIDI_SUPPORT

#include "fluid_midi.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"
#include <windows.h>

#define BUFFER_LENGTH 512

typedef struct {
  fluid_midi_driver_t driver;
  HMIDIIN hmidiin;
} fluid_winmidi_driver_t;

static char fluid_winmidi_error_buffer[256];

#define msg_type(_m)  ((unsigned char)(_m & 0xf0))
#define msg_chan(_m)  ((unsigned char)(_m & 0x0f))
#define msg_p1(_m)    ((_m >> 8) & 0x7f)
#define msg_p2(_m)    ((_m >> 16) & 0x7f)

fluid_midi_driver_t* new_fluid_winmidi_driver(fluid_settings_t* settings,
					    handle_midi_event_func_t handler, void* data);

int delete_fluid_winmidi_driver(fluid_midi_driver_t* p);

void CALLBACK fluid_winmidi_callback(HMIDIIN hmi, UINT wMsg, DWORD dwInstance,
				    DWORD msg, DWORD extra);
static char* fluid_winmidi_input_error(int no);
int fluid_winmidi_driver_status(fluid_midi_driver_t* p);

/*
 * new_fluid_winmidi_driver
 */
fluid_midi_driver_t*
new_fluid_winmidi_driver(fluid_settings_t* settings,
			handle_midi_event_func_t handler, void* data)
{
  fluid_winmidi_driver_t* dev;
  MMRESULT res;
  UINT i, err, num;
  MIDIINCAPS in_caps;
  int midi_num = 0;

  /* not much use doing anything */
  if (handler == NULL) {
    FLUID_LOG(FLUID_ERR, "Invalid argument");
    return NULL;
  }

  dev = FLUID_MALLOC(sizeof(fluid_winmidi_driver_t));
  if (dev == NULL) {
    return NULL;
  }

  dev->hmidiin = NULL;
  dev->driver.handler = handler;
  dev->driver.data = data;

  /* check if there any midi devices installed */
  num = midiInGetNumDevs();
  if (num == 0) {
    FLUID_LOG(FLUID_ERR, "no MIDI in devices found");
    goto error_recovery;
  }

  /* find the device */
  for (i = 0; i < num; i++) {
    res = midiInGetDevCaps(i, &in_caps, sizeof(LPMIDIINCAPS));
    if (res == MMSYSERR_NOERROR) {
    }
  }

  /* try opening the device */
  err = midiInOpen(&dev->hmidiin, midi_num,
		   (DWORD) fluid_winmidi_callback,
		   (DWORD) dev, CALLBACK_FUNCTION);
  if (err != MMSYSERR_NOERROR) {
    FLUID_LOG(FLUID_WARN, "Couldn't open MIDI input: %s (error %d)",
	     fluid_winmidi_input_error(err), err);
    goto error_recovery;
  }

  if (midiInStart(dev->hmidiin) != MMSYSERR_NOERROR) {
    FLUID_LOG(FLUID_ERR, "Failed to start the MIDI input. MIDI input not available.");
    goto error_recovery;
  }

  return (fluid_midi_driver_t*) dev;

 error_recovery:
  delete_fluid_winmidi_driver((fluid_midi_driver_t*) dev);
  return NULL;
}

/*
 * delete_fluid_winmidi_driver
 */
int
delete_fluid_winmidi_driver(fluid_midi_driver_t* p)
{
  fluid_winmidi_driver_t* dev = (fluid_winmidi_driver_t*) p;
  if (dev->hmidiin != NULL) {
    midiInStop(dev->hmidiin);
    midiInReset(dev->hmidiin);
    midiInClose(dev->hmidiin);
  }
  FLUID_FREE(dev);
  return 0;
}

void CALLBACK
fluid_winmidi_callback(HMIDIIN hmi, UINT wMsg, DWORD dwInstance, DWORD msg, DWORD extra)
{
  fluid_winmidi_driver_t* dev = (fluid_winmidi_driver_t *) dwInstance;

  switch (wMsg) {
  case MIM_OPEN:
    break;

  case MIM_CLOSE:
    break;

  case MIM_DATA:
    {
      fluid_midi_event_t event;

      event.type = msg_type(msg);
      event.channel = msg_chan(msg);
      event.param1 = msg_p1(msg);
      event.param2 = msg_p2(msg);
      (*dev->driver.handler)(dev->driver.data, &event);
    }
    break;

  case MIM_LONGDATA:
    break;

  case MIM_ERROR:
    break;

  case MIM_LONGERROR:
    break;

  case MIM_MOREDATA:
    break;
  }
}

int
fluid_winmidi_driver_status(fluid_midi_driver_t* p)
{
  return 0;
}

static char*
fluid_winmidi_input_error(int no)
{
  midiInGetErrorText(no, fluid_winmidi_error_buffer, 256);
  return fluid_winmidi_error_buffer;
}

#endif /* WINMIDI_SUPPORT */
