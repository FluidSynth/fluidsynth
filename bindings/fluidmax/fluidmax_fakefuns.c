/***************************************************************************************
 *
 *  fluidsynth~
 *
 *  Fluid Synth soundfont synthesizer for Max/MSP.
 *
 *  Fluid Synth is written by Peter Hanappe et al.
 *  see http://www.fluidsynth.org/
 *
 *  Max/MSP integration by Norbert Schnell ATR IRCAM - Centre Pompidou
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *  
 *  See file COPYING.LIB for further informations on licensing terms.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
#include "ftmax.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_settings.h"

unsigned int 
fluid_curtime()
{
  return (unsigned int)gettime();
}

double
fluid_utime(void)
{
  return 0.0;
}

typedef void fluid_timer_t;
typedef int(*fluid_timer_callback_t)(void* data, unsigned int msec);

fluid_timer_t * 
new_fluid_timer(int msec, fluid_timer_callback_t callback, void* data, int new_thread, int auto_destroy)
{
  /* just call it right away */
  (*callback)(data, msec);
  
  return NULL;
}

void 
fluid_sys_config()
{
}

int 
fluid_log(int level, char* fmt, ...)
{
  char buf[1024];
  
  va_list args; 
  va_start (args, fmt); 
  vsprintf(buf, fmt, args); 
  va_end (args); 

  if ((level >= 0) && (level < LAST_LOG_LEVEL)) 
    post("fluidsynth~: %s", buf);

  return -1; 
}

void
fluid_shell_settings(fluid_settings_t *settings)
{
}

void
fluid_audio_driver_settings(fluid_settings_t *settings)
{
  fluid_settings_register_str(settings, "audio.driver", "", 0, NULL, NULL);
}

void
fluid_midi_driver_settings(fluid_settings_t *settings)
{  
  fluid_settings_register_str(settings, "midi.driver", "", 0, NULL, NULL);
}
