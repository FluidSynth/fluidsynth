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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02110-1301, USA.
 *
 */
 
/************************************************************************
 *
 *  versions:
 *   (14): adapted to latest code base
 *   (13): ???
 *   (12): fixed voice stealing
 *   (11): fixed arguments of fluidmax_tuning_octave() (third has to be float)
 *   (10): added micro-tuning methods
 *    (9): bug fix: now polyphony and # of midi channel arguments take effect
 *    (8): added message resample permitting to chose resampling interpolation method
 *    (7): added names for soundfonts (== file name without path and postfix)
 *    (6): added message 'info'
 *    (5): fixed bogus path translation at file loading
 * 
 */
#define FLUIDMAX_VERSION "01/2009 (14)"

#include "ftmax.h"
#include "fluidsynth.h"
#include "fluid_synth.h"
#include "fluid_sfont.h"
#include "fluid_chan.h"
#include "fluidsynth/version.h"

typedef struct
{
  ftmax_dsp_object_t obj;
  
  fluid_synth_t *synth;
  fluid_settings_t *settings;
  int reverb;
  int chorus;
  int mute;
  void *outlet;
} fluidmax_t;

static t_messlist *fluidmax_class;

static ftmax_symbol_t sym_on = NULL;
static ftmax_symbol_t sym_off = NULL;
static ftmax_symbol_t sym_undefined = NULL;
static ftmax_symbol_t sym_gain = NULL;
static ftmax_symbol_t sym_channels = NULL;
static ftmax_symbol_t sym_channel = NULL;
static ftmax_symbol_t sym_soundfonts = NULL;
static ftmax_symbol_t sym_soundfont = NULL;
static ftmax_symbol_t sym_presets = NULL;
static ftmax_symbol_t sym_preset = NULL;
static ftmax_symbol_t sym_reverb = NULL;
static ftmax_symbol_t sym_chorus = NULL;
static ftmax_symbol_t sym_polyphony = NULL;
static ftmax_symbol_t sym_nearest = NULL;
static ftmax_symbol_t sym_linear = NULL;
static ftmax_symbol_t sym_cubic = NULL;
static ftmax_symbol_t sym_sinc = NULL;

/***************************************************************
 *
 *  generators
 *
 */
typedef struct
{
  int index;
  const char *name;
  const char *unit;
} fluidmax_gen_descr_t; 

static fluidmax_gen_descr_t fluidmax_gen_info[] =
{
  {0, "startAddrsOffset", "samples"}, 
  {1, "endAddrsOffset", "samples"}, 
  {2, "startloopAddrsOffset", "samples"}, 
  {3, "endloopAddrsOffset", "samples"}, 
  {4, "startAddrsCoarseOffset", "32k samples"}, 
  {5, "modLfoToPitch", "cent fs"}, 
  {6, "vibLfoToPitch", "cent fs"}, 
  {7, "modEnvToPitch", "cent fs"}, 
  {8, "initialFilterFc", "cent 8.176 Hz"}, 
  {9, "initialFilterQ", "cB"}, 
  {10, "modLfoToFilterFc", "cent fs"}, 
  {11, "modEnvToFilterFc", "cent fs "}, 
  {12, "endAddrsCoarseOffset", "32k samples"}, 
  {13, "modLfoToVolume", "cB fs"}, 
  {14, "unused1", ""}, 
  {15, "chorusEffectsSend", "0.1%"}, 
  {16, "reverbEffectsSend", "0.1% "}, 
  {17, "pan", "0.1%"}, 
  {18, "unused2", ""}, 
  {19, "unused3", ""}, 
  {20, "unused4", ""}, 
  {21, "delayModLFO", "timecent"}, 
  {22, "freqModLFO", "cent 8.176 "}, 
  {23, "delayVibLFO", "timecent "}, 
  {24, "freqVibLFO", "cent 8.176"}, 
  {25, "delayModEnv", "timecent"}, 
  {26, "attackModEnv", "timecent "}, 
  {27, "holdModEnv", "timecent"}, 
  {28, "decayModEnv", "timecent"}, 
  {29, "sustainModEnv", "-0.1%"}, 
  {30, "releaseModEnv", "timecent"}, 
  {31, "keynumToModEnvHold", "tcent/key"}, 
  {32, "keynumToModEnvDecay", "tcent/key"}, 
  {33, "delayVolEnv", "timecent"}, 
  {34, "attackVolEnv", "timecent"}, 
  {35, "holdVolEnv", "timecent"}, 
  {36, "decayVolEnv", "timecent"}, 
  {37, "sustainVolEnv", "cB"}, 
  {38, "releaseVolEnv", "timecent "}, 
  {39, "keynumToVolEnvHold", "tcent/key"}, 
  {40, "keynumToVolEnvDecay", "tcent/key "}, 
  {41, "instrument", ""}, 
  {42, "reserved1", ""}, 
  {43, "keyRange MIDI", ""},  
  {44, "velRange MIDI", ""}, 
  {45, "startloopAddrsCoarseOffset", "samples"}, 
  {46, "keynum MIDI", ""}, 
  {47, "velocity MIDI", ""}, 
  {48, "initialAttenuation", "cB"}, 
  {49, "reserved2", ""}, 
  {50, "endloopAddrsCoarseOffset", "samples"}, 
  {51, "coarseTune", "semitone"}, 
  {52, "fineTune", "cent"}, 
  {53, "sampleId", ""}, 
  {54, "sampleModes", "Bit Flags"}, 
  {55, "reserved3", ""}, 
  {56, "scaleTuning", "cent/key"}, 
  {57, "exclusiveClass", "arbitrary#"}, 
  {58, "unused5", ""},
  {59, "unused6", ""},
  {60, "unused7", ""},
  {61, "unused8", ""},
  {62, "unused9", ""},
  {63, "unused10", ""}
};

/***************************************************************
 *
 *  dsp
 *
 */
static t_int *
fluidmax_perform(t_int *w)
{
  fluidmax_t *self = (fluidmax_t *)(w[1]);
  t_float *left = (t_float *)(w[2]);
  t_float *right = (t_float *)(w[3]);
  int n_tick = (int)(w[4]);

  if(self->mute == 0)
    fluid_synth_write_float(self->synth, n_tick, left, 0, 1, right, 0, 1);
 else
 {
  int i;
  
  for(i=0; i<n_tick; i++)
    left[i] = right[i] = 0.0;
 }
  
  return (w + 5);
}

static void 
fluidmax_dsp(fluidmax_t *self, t_signal **sp, short *count)
{
  int n_tick = sp[0]->s_n;
        
  dsp_add(fluidmax_perform, 4, self, sp[0]->s_vec, sp[1]->s_vec, n_tick);
}

/***************************************************************
 *
 *  load utlilities
 *
 */
static char *
fluidmax_translate_fullpath(char *maxpath, char *fullpath)
{
  int i;

  strcpy(fullpath, "/Volumes/");
  
  for(i=0; maxpath[i] != ':'; i++)
    fullpath[i + 9] = maxpath[i];
    
  /* skip ':' */
  i++;
    
  strcpy(fullpath + i + 8, maxpath + i);
  
  return fullpath;
}
    
static ftmax_symbol_t
fluidmax_get_stripped_name(const char *fullpath)
{
  char stripped[1024];
  int i;
  
  for(i=strlen(fullpath)-1; i>=0; i--)
  {
    if(fullpath[i] == '/')
      break;
  }
  
  if(i != 0)
    i++;    
  
  strcpy(stripped, fullpath + i);
  
  for(i=0; stripped[i] != '\0'; i++)
  {
    if((stripped[i] == '.') && 
       (stripped[i + 1] == 's' || stripped[i + 1] == 'S') && 
       (stripped[i + 2] == 'f' || stripped[i + 2] == 'F') && 
       (stripped[i + 3] == '2'))
    {
      stripped[i] = '\0';
      break;
    }
  }
  
  return ftmax_new_symbol(stripped);
}
    
static ftmax_symbol_t
fluidmax_sfont_get_name(fluid_sfont_t *sfont)
{
  return fluidmax_get_stripped_name(fluid_sfont_get_name(sfont));
}
    
static fluid_sfont_t *
fluidmax_sfont_get_by_name(fluidmax_t *self, ftmax_symbol_t name)
{
  int n = fluid_synth_sfcount(self->synth);
  int i;
  
  for(i=0; i<n; i++)
  {
    fluid_sfont_t *sfont = fluid_synth_get_sfont(self->synth, i);

    if(fluidmax_sfont_get_name(sfont) == name)
      return sfont;
  }

  return NULL;
}

static void 
fluidmax_do_load(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0 && ftmax_is_symbol(at))  
  {
    const char *filename = ftmax_symbol_name(ftmax_get_symbol(at));
    ftmax_symbol_t name = fluidmax_get_stripped_name(filename);
    fluid_sfont_t *sf = fluidmax_sfont_get_by_name(self, name);
    
    if(sf == NULL)
    {
      int id = fluid_synth_sfload(self->synth, filename, 0);
    
      if(id >= 0)
      {
        post("fluidsynth~: loaded soundfont '%s' (id %d)", ftmax_symbol_name(name), id);

        fluid_synth_program_reset(self->synth);
        
        outlet_bang(self->outlet);
      }
      else
        error("fluidsynth~: cannot load soundfont from file '%s'", filename);
    }
    else
    {
      error("fluidsynth~: soundfont named '%s' is already loaded", ftmax_symbol_name(name));
      return;
    }
  }
}

static void
fluidmax_load_with_dialog(t_object *o, t_symbol *s, short argc, t_atom *argv)
{
  char filename[256];
  char maxpath[1024];
  char fullpath[1024];
  long type;
  short path;
  
  open_promptset("open SoundFont 2 file");
  
  if(open_dialog(filename, &path, &type, 0, 0))
    return;
    
  if(path_topotentialname(path, filename, maxpath, 0) == 0)
  {
    ftmax_atom_t a;
    
    ftmax_set_symbol(&a, ftmax_new_symbol(fluidmax_translate_fullpath(maxpath, fullpath)));
    fluidmax_do_load(o, NULL, 1, &a);
  }
}  

/***************************************************************
 *
 *  user methods
 *
 */
static void 
fluidmax_load(t_object *o, Symbol *s, short ac, Atom *at)
{ 
  if(ac == 0)
    defer(o, (method)fluidmax_load_with_dialog, NULL, 0, 0);
  else
  {
    if(ftmax_is_symbol(at))
    {
      ftmax_symbol_t name = ftmax_get_symbol(at);
      char *string = (char *)ftmax_symbol_name(name);
      
      if(string[0] == '/')
        defer(o, (method)fluidmax_do_load, NULL, ac, at);
      else
      {
        char maxpath[1024];
        char fullpath[1024];
        short path;
        long type;
        ftmax_atom_t a;
        
        if(locatefile_extended(string, &path, &type, 0, 0) || path_topotentialname(path, string, maxpath, 0) != 0)
        {
          error("fluidsynth~: cannot find file '%s'", string);
          return;
        }
        
        ftmax_set_symbol(&a, ftmax_new_symbol(fluidmax_translate_fullpath(maxpath, fullpath)));
        defer(o, (method)fluidmax_do_load, NULL, 1, &a);
      }
    }
  }
}

static void 
fluidmax_unload(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0)
  {
    if(ftmax_is_number(at))  
    {
      int id = ftmax_get_number_int(at);
      fluid_sfont_t *sf = fluid_synth_get_sfont_by_id(self->synth, id);
      
      if(sf != NULL)
      {  
        ftmax_symbol_t name = fluidmax_sfont_get_name(sf);
        
        if(fluid_synth_sfunload(self->synth, id, 0) >= 0)
        {
          post("fluidsynth~: unloaded soundfont '%s' (id %d)", ftmax_symbol_name(name), id);
          return;
        }
      }
            
      error("fluidsynth~: cannot unload soundfont %d", id);
    }
    else if (ftmax_is_symbol(at))
    {
      ftmax_symbol_t sym = ftmax_get_symbol(at);
      
      if(sym == ftmax_new_symbol("all"))
      {
        fluid_sfont_t *sf = fluid_synth_get_sfont(self->synth, 0);
        
        fluid_synth_system_reset(self->synth);

        while(sf != NULL)
        {
          ftmax_symbol_t name = fluidmax_sfont_get_name(sf);
          unsigned int id = fluid_sfont_get_id(sf);
        
          if(fluid_synth_sfunload(self->synth, id, 0) >= 0)
            post("fluidsynth~: unloaded soundfont '%s' (id %d)", ftmax_symbol_name(name), id);
          else
            error("fluidsynth~: cannot unload soundfont '%s' (id %d)", ftmax_symbol_name(name), id);
        
          sf = fluid_synth_get_sfont(self->synth, 0);
        }
      }
      else
      {
        fluid_sfont_t *sf = fluidmax_sfont_get_by_name(self, sym);
        
        if(sf != NULL)
        {
          unsigned int id = fluid_sfont_get_id(sf);
          
          if(fluid_synth_sfunload(self->synth, id, 0) >= 0)
          {
            post("fluidsynth~: unloaded soundfont '%s' (id %d)", ftmax_symbol_name(sym), id);
            return;
          }
        }
        
        error("fluidsynth~: cannot unload soundfont '%s'", ftmax_symbol_name(sym));
      }
    }
  }
}

static void 
fluidmax_reload(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0)
  {
    if(ftmax_is_number(at))  
    {
      int id = ftmax_get_number_int(at);
      fluid_sfont_t *sf = fluid_synth_get_sfont_by_id(self->synth, id);
      
      if(sf != NULL)
      {
        if (fluid_synth_sfreload(self->synth, id) >= 0)
        {
          post("fluidsynth~: reloaded soundfont '%s' (id %d)", id);
          return;
        }

        error("fluidsynth~: cannot reload soundfont %d", id);
      }
    }
    else if (ftmax_is_symbol(at))
    {
      ftmax_symbol_t sym = ftmax_get_symbol(at);
      
      if(sym == ftmax_new_symbol("all"))
      {
        int n = fluid_synth_sfcount(self->synth);
        int i;
        
        fluid_synth_system_reset(self->synth);

        for(i=0; i<n; i++)
        {
          fluid_sfont_t *sf = fluid_synth_get_sfont(self->synth, i);
          ftmax_symbol_t name = fluidmax_sfont_get_name(sf);
          unsigned int id = fluid_sfont_get_id(sf);
          
        
          if (fluid_synth_sfreload(self->synth, id) >= 0)
            post("fluidsynth~: reloaded soundfont '%s' (id %d)", ftmax_symbol_name(name), id);
          else
            error("fluidsynth~: cannot reload soundfont '%s' (id %d)", ftmax_symbol_name(name), id);
        }
      }
      else
      {
        fluid_sfont_t *sf = fluidmax_sfont_get_by_name(self, sym);
        
        if(sf != NULL)
        {
          unsigned int id = fluid_sfont_get_id(sf);
          
          if(fluid_synth_sfreload(self->synth, id) >= 0)
          {
            post("fluidsynth~: reloaded soundfont '%s' (id %d)", ftmax_symbol_name(sym), id);
            return;
          }
        }
        
        error("fluidsynth~: cannot reload soundfont '%s'", ftmax_symbol_name(sym));
      }
    }
  }
}

static void 
fluidmax_note(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0 && ftmax_is_number(at))
  {
    int velocity = 64;
    int channel = 1;
    
    switch(ac)
    {
      default:
      case 3:
        if(ftmax_is_number(at + 2))
        {
          channel = ftmax_get_number_int(at + 2);
                    
          if(channel < 1)
            channel = 1;
          else if(channel > fluid_synth_count_midi_channels(self->synth))
            channel = fluid_synth_count_midi_channels(self->synth);
        }
      case 2:
        if(ftmax_is_number(at + 1))
          velocity = ftmax_get_number_int(at + 1);
      case 1:
        fluid_synth_noteon(self->synth, channel - 1, ftmax_get_number_int(at), velocity);
      case 0:
        break;
    }
  }
}

static void 
fluidmax_list(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_note(o, NULL, ac, at);
}


static void 
fluidmax_control_change(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0 && ftmax_is_number(at))
  {
    int value = 64;
    int channel = 1;
    
    switch(ac)
    {
      default:
      case 3:
        if(ftmax_is_number(at + 2))
        {
          channel = ftmax_get_number_int(at + 2);
          
          if(channel < 1)
            channel = 1;
          else if(channel > fluid_synth_count_midi_channels(self->synth))
            channel = fluid_synth_count_midi_channels(self->synth);
        }
      case 2:
        if(ftmax_is_number(at + 1))
          value = ftmax_get_number_int(at + 1);
      case 1:
        fluid_synth_cc(self->synth, channel - 1, ftmax_get_number_int(at), value);
      case 0:
        break;
    }
  }
}

static void 
fluidmax_mod(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 1 && ftmax_is_number(at) && ftmax_is_number(at + 1))
  {
    int param = ftmax_get_number_int(at);
    float value = ftmax_get_number_float(at + 1);
    int channel = 1;
    
    if(ac > 2 && ftmax_is_number(at + 2))
    {
      channel = ftmax_get_number_int(at + 2);
      
      if(channel < 1)
        channel = 1;
      else if(channel > fluid_synth_count_midi_channels(self->synth))
        channel = fluid_synth_count_midi_channels(self->synth);
    }

    fluid_synth_set_gen(self->synth, channel - 1, param, value);
  }
}

static void 
fluidmax_pitch_bend(t_object *o, Symbol *s, short ac, Atom *at)
{  
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0 && ftmax_is_number(at))
  {
    int channel = 1;
    double bend = 0.0;
    
    if(ac > 1 && ftmax_is_number(at + 1))
    {
      channel = ftmax_get_number_int(at + 1);
          
      if(channel < 1)
        channel = 1;
      else if(channel > fluid_synth_count_midi_channels(self->synth))
        channel = fluid_synth_count_midi_channels(self->synth);
    }
      
    bend = ftmax_get_number_float(at);
    
    if(bend < 0.0)
      bend = 0.0;
    else if(bend > 127.0)
      bend = 127.0;
  
    fluid_synth_pitch_bend(self->synth, channel - 1, (int)(bend * 128.0));
  }
}

static void 
fluidmax_pitch_bend_wheel(t_object *o, Symbol *s, short ac, Atom *at)
{  
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0 && ftmax_is_number(at))
  {
    int channel = 1;
    
    if(ac > 1 && ftmax_is_number(at + 1))
      channel = ftmax_get_number_int(at + 1);
      
    fluid_synth_pitch_wheel_sens(self->synth, channel - 1, ftmax_get_number_int(at));
  }
}

static void 
fluidmax_program_change(t_object *o, Symbol *s, short ac, Atom *at)
{  
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0 && ftmax_is_number(at))
  {
    int channel = 1;
    
    if(ac > 1 && ftmax_is_number(at + 1))
    {
      channel = ftmax_get_number_int(at + 1);
          
      if(channel < 1)
        channel = 1;
      else if(channel > fluid_synth_count_midi_channels(self->synth))
        channel = fluid_synth_count_midi_channels(self->synth);
    }
      
    fluid_synth_program_change(self->synth, channel - 1, ftmax_get_number_int(at));
  }
}

static void 
fluidmax_bank_select(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0 && ftmax_is_number(at))
  {
    int channel = 1;
    unsigned int sf_id;
    unsigned int bank_num;
    unsigned int prog_num;
    
    if(ac > 1 && ftmax_is_number(at + 1))
    {
      channel = ftmax_get_number_int(at + 1);
          
      if(channel < 1)
        channel = 1;
      else if(channel > fluid_synth_count_midi_channels(self->synth))
        channel = fluid_synth_count_midi_channels(self->synth);
    }
      
    fluid_synth_bank_select(self->synth, channel - 1, ftmax_get_number_int(at));
    fluid_synth_get_program(self->synth, channel - 1, &sf_id, &bank_num, &prog_num);
    fluid_synth_program_change(self->synth, channel - 1, prog_num);
  }
}

static void 
fluidmax_select(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  unsigned int bank = 0;
  unsigned int preset = 0;  
  int channel = 1;
  
  switch(ac)
  {
    default:
    case 4:
      if(ftmax_is_number(at + 3))
        channel = ftmax_get_number_int(at + 3);

      if(channel < 1)
        channel = 1;
      else if(channel > fluid_synth_count_midi_channels(self->synth))
        channel = fluid_synth_count_midi_channels(self->synth);
        
    case 3:
      if(ftmax_is_number(at + 2))
        preset = ftmax_get_number_int(at + 2);
        
    case 2:
      if(ftmax_is_number(at + 1))
        bank = ftmax_get_number_int(at + 1);
        
    case 1:
      if(ftmax_is_number(at))
        fluid_synth_program_select(self->synth, channel - 1, ftmax_get_number_int(at), bank, preset);
      else if(ftmax_is_symbol(at))
      {
        ftmax_symbol_t name = ftmax_get_symbol(at);
        fluid_sfont_t *sf = fluidmax_sfont_get_by_name(self, name);
        
        if(sf != NULL)
          fluid_synth_program_select(self->synth, channel - 1, fluid_sfont_get_id(sf), bank, preset);
        else
          error("fluidsynth~ select:Êcannot find soundfont named '%s'", ftmax_symbol_name(name));
      }
    case 0:
      break;
  }
}

static void 
fluidmax_reverb(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac == 0)
  {
    fluid_synth_set_reverb_on(self->synth, 1);
    fluid_revmodel_reset(self->synth->reverb);
    self->reverb = 1;
  }
  else if(ftmax_is_number(at))
  {
    double room = fluid_synth_get_reverb_roomsize(self->synth);
    double damping = fluid_synth_get_reverb_damp(self->synth);
    double width = fluid_synth_get_reverb_width(self->synth);

    fluid_synth_set_reverb_on(self->synth, 1);
    self->reverb = 1;
    
    switch(ac)
    {
      default:
      case 4:
        if(ftmax_is_number(at + 3))
          width = ftmax_get_number_float(at + 3);
      case 3:
        if(ftmax_is_number(at + 2))
          damping = ftmax_get_number_float(at + 2);
      case 2:
        if(ftmax_is_number(at + 1))
          room = ftmax_get_number_float(at + 1);
      case 1:
        fluid_synth_set_reverb(self->synth, room, damping, width, ftmax_get_number_float(at));
      case 0:
        break;
    }
  }
  else if(ftmax_is_symbol(at))
  {
    ftmax_symbol_t sym = ftmax_get_symbol(at);
    
    if(sym == sym_on)
    {
      fluid_synth_set_reverb_on(self->synth, 1);
      self->reverb = 1;
    }
    else if(sym == sym_off)
    {
      fluid_synth_set_reverb_on(self->synth, 0);
      self->reverb = 0;
    }
  }
}

static void 
fluidmax_chorus(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac == 0)
  {
    fluid_synth_set_chorus_on(self->synth, 1);
    fluid_chorus_reset(self->synth->chorus);
    self->chorus = 1;
  }
  else if(ftmax_is_number(at))
  {
    double speed = fluid_synth_get_chorus_speed_Hz(self->synth);
    double depth = fluid_synth_get_chorus_depth_ms(self->synth);
    int type = fluid_synth_get_chorus_type(self->synth);
    int nr = fluid_synth_get_chorus_nr(self->synth);

    fluid_synth_set_chorus_on(self->synth, 1);
    self->chorus = 1;

    switch(ac)
    {
      default:
      case 5:
        if(ftmax_is_number(at + 4))
          nr = ftmax_get_number_int(at + 4);
      case 4:
        if(ftmax_is_number(at + 3))
          type = ftmax_get_number_int(at + 3);
      case 3:
        if(ftmax_is_number(at + 2))
          depth = ftmax_get_number_float(at + 2);
      case 2:
        if(ftmax_is_number(at + 1))
          speed = ftmax_get_number_float(at + 1);
      case 1:
        fluid_synth_set_chorus(self->synth, nr, ftmax_get_number_float(at), speed, depth, type);
      case 0:
        break;
    }
  }
  else if(ftmax_is_symbol(at))
  {
    ftmax_symbol_t sym = ftmax_get_symbol(at);
    
    if(sym == sym_on)
    {
      fluid_synth_set_chorus_on(self->synth, 1);
      self->chorus = 1;
    }
    else if(sym == sym_off)
    {
      fluid_synth_set_chorus_on(self->synth, 0);
      self->chorus = 0;
    }
  }
}

static void 
fluidmax_set_gain(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0 && ftmax_is_number(at))
  {
    float gain = ftmax_get_number_float(at);
    
    fluid_synth_set_gain(self->synth, gain);
  }
}

static void 
fluidmax_set_resampling_method(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0)
  {
    if(ftmax_is_number(at))
    {
      int ip = ftmax_get_int(at);
      
      if(ip == 0)
        fluid_synth_set_interp_method(self->synth, -1, FLUID_INTERP_NONE);
      else if(ip < 3)
        fluid_synth_set_interp_method(self->synth, -1, FLUID_INTERP_LINEAR);
      else if(ip < 6)
        fluid_synth_set_interp_method(self->synth, -1, FLUID_INTERP_4THORDER);
      else
        fluid_synth_set_interp_method(self->synth, -1, FLUID_INTERP_7THORDER);
    }
    else if(ftmax_is_symbol(at))
    {
      ftmax_symbol_t sym = ftmax_get_symbol(at);
      
      if(sym == sym_nearest)
        fluid_synth_set_interp_method(self->synth, -1, FLUID_INTERP_NONE);
      else if(sym == sym_linear)
        fluid_synth_set_interp_method(self->synth, -1, FLUID_INTERP_LINEAR);
      else if(sym == sym_cubic)
        fluid_synth_set_interp_method(self->synth, -1, FLUID_INTERP_4THORDER);
      else if(sym == sym_sinc)
        fluid_synth_set_interp_method(self->synth, -1, FLUID_INTERP_7THORDER);      
      else
        error("fluidsynth~: undefined resampling method: %s", ftmax_symbol_name(sym));
    }
  }
}

static void 
fluidmax_panic(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;

  fluid_synth_system_reset(self->synth);
}

static void 
fluidmax_reset(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  int n = fluid_synth_count_midi_channels(self->synth);
  int i;

  for (i=0; i<n; i++)
    fluid_channel_reset(self->synth->channel[i]);
}

static void 
fluidmax_mute(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  int mute = 1;
  
  if(ac > 0 && ftmax_is_number(at))
    mute = (ftmax_get_number_int(at) != 0);
  
  fluid_synth_system_reset(self->synth);
   
  self->mute = mute;
}

static void 
fluidmax_unmute(t_object *o)
{
  ftmax_atom_t a;
  
  ftmax_set_int(&a, 0);  
  fluidmax_mute(o, NULL, 1, &a);
}

/* 
int fluid_synth_count_audio_channels (fluid_synth_t *synth)
int fluid_synth_count_audio_groups (fluid_synth_t *synth)
int fluid_synth_count_effects_channels (fluid_synth_t *synth)
*/

static void 
fluidmax_tuning_octave(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  ftmax_symbol_t name;
  int tuning_bank = 0;
  int tuning_prog = 0;
  double pitch[12];
  int i, n;
  
  if(ac > 0 && ftmax_is_symbol(at))
  {
    name = ftmax_get_symbol(at);
    at++;
    ac--;
  }
  
  n = ac - 2;
  if(n > 12)
    n = 12;

  if(ac > 0 && ftmax_is_number(at))
    tuning_bank = ftmax_get_number_int(at) % 128;
  
  if(ac > 1 && ftmax_is_number(at + 1))
    tuning_prog = ftmax_get_number_int(at) % 128;
    
  for(i=0; i<n; i++)
  {
    if(ftmax_is_number(at + i + 2))
      pitch[i] = ftmax_get_number_float(at + i + 2);
    else
      pitch[i] = 0.0;
  }
  
  for(; i<12; n++)
    pitch[i] = 0.0;

  fluid_synth_create_octave_tuning(self->synth, tuning_bank, tuning_prog, ftmax_symbol_name(name), pitch);
}

static void 
fluidmax_tuning_select(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  int tuning_bank = 0;
  int tuning_prog = 0;
  int channel = 1;
  
  if(ac > 0 && ftmax_is_number(at))
    tuning_bank = ftmax_get_number_int(at) % 128;
    
  if(ac > 1 && ftmax_is_number(at + 1))
    tuning_prog = ftmax_get_number_int(at + 1) % 128;
  
  if(ac > 2 && ftmax_is_number(at + 2))
    channel = ftmax_get_number_int(at + 2);
    
  if(channel < 1)
    channel = 1;
  else if(channel > fluid_synth_count_midi_channels(self->synth))
    channel = fluid_synth_count_midi_channels(self->synth);
    
  fluid_synth_select_tuning(self->synth, channel - 1, tuning_bank, tuning_prog);
}

static void 
fluidmax_tuning_reset(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  int channel = 0;
  
  if(ac > 0 && ftmax_is_number(at))
    channel = ftmax_get_number_int(at);

  if(channel < 1)
    channel = 1;
  else if(channel > fluid_synth_count_midi_channels(self->synth))
    channel = fluid_synth_count_midi_channels(self->synth);
    
  fluid_synth_reset_tuning(self->synth, channel - 1);
}

/* more tuning ??
fluid_synth_create_key_tuning (fluid_synth_t *synth, int tuning_bank, int tuning_prog, char *name, double *pitch)
fluid_synth_tune_notes (fluid_synth_t *synth, int tuning_bank, int tuning_prog, int len, int *keys, double *pitch, int apply)
fluid_synth_tuning_iteration_start (fluid_synth_t *synth)
fluid_synth_tuning_iteration_next (fluid_synth_t *synth, int *bank, int *prog)
fluid_synth_tuning_dump (fluid_synth_t *synth, int bank, int prog, char *name, int len, double *pitch)
*/

static void
fluidmax_version(t_object *o)
{
  post("fluidsynth~, version %s (based on FluidSynth %s)", FLUIDMAX_VERSION, FLUIDSYNTH_VERSION);
  post("  FluidSynth is written by Peter Hanappe et al. (see fluidsynth.org)");
  post("  Max/MSP integration by Norbert Schnell IMTR IRCAM - Centre Pompidou");
}

extern fluid_gen_info_t fluid_gen_info[];

static void 
fluidmax_print(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0)
  {
    if(ftmax_is_symbol(at))
    {
      ftmax_symbol_t sym = ftmax_get_symbol(at);
      
      if(sym == sym_soundfonts)
      {
        int n = fluid_synth_sfcount(self->synth);
        int i;
        
        if(n > 0)
          post("fluidsynth~ soundfonts:");
        else
          post("fluidsynth~: no soundfonts loaded");          
        
        for(i=0; i<n; i++)
        {
          fluid_sfont_t *sf = fluid_synth_get_sfont(self->synth, i);
          ftmax_symbol_t name = fluidmax_sfont_get_name(sf);
          unsigned int id = fluid_sfont_get_id(sf);
          
          post("  %d: '%s' (id %d)", i, ftmax_symbol_name(name), id);
        }
      }
      else if(sym == sym_presets)
      {
        int n = fluid_synth_sfcount(self->synth);
        
        if(n > 0)
        {
          if(ac > 1)
          {
            fluid_sfont_t *sf = NULL;
            ftmax_symbol_t name;
         
            if(ftmax_is_symbol(at + 1))
            {
              name = ftmax_get_symbol(at + 1);
              sf = fluidmax_sfont_get_by_name(self, name);
            }
            else if(ftmax_is_int(at + 1))
            {
              int id = ftmax_get_int(at + 1);
              
              sf = fluid_synth_get_sfont_by_id(self->synth, id);
              name = fluidmax_sfont_get_name(sf);
            }
            
            if(sf != NULL)
            {
              fluid_preset_t preset;
              
              fluid_sfont_iteration_start(sf);
              
              post("fluidsynth~ presets of soundfont '%s':", ftmax_symbol_name(name));
              
              while(fluid_sfont_iteration_next(sf, &preset) > 0)
              {
                char *preset_str = fluid_preset_get_name(&preset);
                ftmax_symbol_t preset_name = ftmax_new_symbol(preset_str);
                int bank_num = fluid_preset_get_banknum(&preset);
                int prog_num = fluid_preset_get_num(&preset);
                
                post("  '%s': bank %d, program %d", ftmax_symbol_name(preset_name), bank_num, prog_num);
              }
            }
          }
          else
          {
            int i;

            post("fluidsynth~ presets:");
            
            for(i=0; i<128; i++)
            {
              int j;
              
              for(j=0; j<128; j++)
              {
                fluid_preset_t *preset = NULL;
                fluid_sfont_t *sf = NULL;
                int k;
                
                for(k=0; k<n; k++)
                {
                  sf = fluid_synth_get_sfont(self->synth, k);                  
                  preset = fluid_sfont_get_preset(sf, i, j);
                  
                  if(preset != NULL)
                    break;
                }
                
                if(preset != NULL)
                {
                  ftmax_symbol_t sf_name = fluidmax_sfont_get_name(sf);
                  char *preset_str = fluid_preset_get_name(preset);
                  ftmax_symbol_t preset_name = ftmax_new_symbol(preset_str);
                  
                  post("  '%s': soundfont '%s', bank %d, program %d", 
                    ftmax_symbol_name(preset_name), ftmax_symbol_name(sf_name), i, j);
                }
              }
            }
          }
        }
        else
          error("fluidsynth~: no soundfonts loaded");
      }
      else if(sym == sym_channels)
      {
        int n = fluid_synth_count_midi_channels(self->synth);
        int i;
        
        post("fluidsynth~ channels:");
        
        for(i=0; i<n; i++)
        {
          fluid_preset_t *preset = fluid_synth_get_channel_preset(self->synth, i);
          
          if(preset != NULL)
          {
            char *preset_str = fluid_preset_get_name(preset);
            ftmax_symbol_t preset_name = ftmax_new_symbol(preset_str);
            unsigned int sf_id;
            unsigned int bank_num;
            unsigned int prog_num;
            fluid_sfont_t *sf;
            
            fluid_synth_get_program(self->synth, i, &sf_id, &bank_num, &prog_num);
            sf = fluid_synth_get_sfont_by_id(self->synth, sf_id);
            
            post("  %d: soundfont '%s', bank %d, program %d: '%s'", 
              i + 1, ftmax_symbol_name(fluidmax_sfont_get_name(sf)), bank_num, prog_num, ftmax_symbol_name(preset_name));
          }
          else
            post("  channel %d: no preset", i + 1);
        }
      }
      else if(sym == ftmax_new_symbol("generators"))
      {
        int channel = 1;
        int n = GEN_LAST;
        int i;
        
        if(ac > 1 && ftmax_is_number(at + 1))
          channel = ftmax_get_number_int(at + 1);

        if(channel < 1)
          channel = 1;
        else if(channel > fluid_synth_count_midi_channels(self->synth))
          channel = fluid_synth_count_midi_channels(self->synth);
          
        post("fluidsynth~ generators of channel %d:", channel);
        
        for(i=0; i<n; i++)
        {
          const char *name = fluidmax_gen_info[i].name;
          const char *unit = fluidmax_gen_info[i].unit;
          double incr = fluid_synth_get_gen(self->synth, channel - 1, i);
          double min = fluid_gen_info[i].min;
          double max = fluid_gen_info[i].max;
          
          post("  %d '%s': %s %g [%g ... %g] (%s)", i, name, (incr >= 0)? "": "-" , fabs(incr), min, max, unit);
        }
      }
      else if(sym == sym_gain)
      {
        double gain = fluid_synth_get_gain(self->synth);

        post("gain: %g", gain);
      }
      else if(sym == sym_reverb)
      {
        double level = fluid_synth_get_reverb_level(self->synth);
        double room = fluid_synth_get_reverb_roomsize(self->synth);
        double damping = fluid_synth_get_reverb_damp(self->synth);
        double width = fluid_synth_get_reverb_width(self->synth);
        
        if(self->reverb != 0)
        {
          post("fluidsynth~ current reverb parameters:");
          post("  level: %f", level);
          post("  room size: %f", room);
          post("  damping: %f", damping);
          post("  width: %f", width);
        }
        else
          post("fluidsynth~: reverb off");        
      }
      else if(sym == sym_chorus)
      {
        if(self->chorus != 0)
        {
          double level = fluid_synth_get_chorus_level(self->synth);
          double speed = fluid_synth_get_chorus_speed_Hz(self->synth);
          double depth = fluid_synth_get_chorus_depth_ms(self->synth);
          int type = fluid_synth_get_chorus_type(self->synth);
          int nr = fluid_synth_get_chorus_nr(self->synth);
          
          post("fluidsynth~ current chorus parameters:");
          post("  level: %f", level);
          post("  speed: %f Hz", speed);
          post("  depth: %f msec", depth);
          post("  type: %d (%s)", type, type? "triangle": "sine");
          post("  %d units", nr);
        }
        else
          post("fluidsynth~: chorus off");
      }
    }
  }
}

static void 
fluidmax_info(t_object *o, Symbol *s, short ac, Atom *at)
{
  fluidmax_t *self = (fluidmax_t *)o;
  
  if(ac > 0)
  {
    if(ftmax_is_symbol(at))
    {
      ftmax_symbol_t sym = ftmax_get_symbol(at);
      
      if(sym == sym_soundfonts)
      {
        int n = fluid_synth_sfcount(self->synth);
        int i;
        
        for(i=0; i<n; i++)
        {
          fluid_sfont_t *sf = fluid_synth_get_sfont(self->synth, i);
          unsigned int id = fluid_sfont_get_id(sf);
          ftmax_atom_t a[2];
          
          ftmax_set_int(a, i);
          ftmax_set_symbol(a + 1, fluidmax_sfont_get_name(sf));
          ftmax_set_int(a + 2, id);
          outlet_anything(self->outlet, sym_soundfont, 3, a);
        }
      }
      else if(sym == sym_presets)
      {
        int n = fluid_synth_sfcount(self->synth);
        
        if(n > 0)
        {
          if(ac > 1)
          {
            fluid_sfont_t *sf = NULL;
            ftmax_symbol_t sf_name;
         
            if(ftmax_is_symbol(at + 1))
            {
              sf_name = ftmax_get_symbol(at + 1);
              sf = fluidmax_sfont_get_by_name(self, sf_name);
            }
            else if(ftmax_is_int(at + 1))
            {
              int id = ftmax_get_int(at + 1);
              
              sf = fluid_synth_get_sfont_by_id(self->synth, id);
              sf_name = fluidmax_sfont_get_name(sf);
            }
            
            if(sf != NULL)
            {
              fluid_preset_t preset;
              
              fluid_sfont_iteration_start(sf);
              
              while(fluid_sfont_iteration_next(sf, &preset) > 0)
              {
                char *preset_str = fluid_preset_get_name(&preset);
                ftmax_symbol_t preset_name = ftmax_new_symbol(preset_str);
                int bank_num = fluid_preset_get_banknum(&preset);
                int prog_num = fluid_preset_get_num(&preset);
                ftmax_atom_t a[4];
                
                ftmax_set_symbol(a , preset_name);
                ftmax_set_symbol(a + 1, sf_name);
                ftmax_set_int(a + 2, bank_num);
                ftmax_set_int(a + 3, prog_num);
                outlet_anything(self->outlet, sym_preset, 4, a);
              }
            }
          }
          else
          {
            int i;

            for(i=0; i<128; i++)
            {
              int j;
              
              for(j=0; j<128; j++)
              {
                fluid_preset_t *preset = NULL;
                fluid_sfont_t *sf = NULL;
                int k;
                
                for(k=0; k<n; k++)
                {
                  sf = fluid_synth_get_sfont(self->synth, k);                  
                  preset = fluid_sfont_get_preset(sf, i, j);
                  
                  if(preset != NULL)
                    break;
                }
                
                if(preset != NULL)
                {
                  ftmax_symbol_t sf_name = fluidmax_sfont_get_name(sf);
                  char *preset_str = fluid_preset_get_name(preset);
                  ftmax_symbol_t preset_name = ftmax_new_symbol(preset_str);
                  ftmax_atom_t a[4];
                  
                  ftmax_set_symbol(a , preset_name);
                  ftmax_set_symbol(a + 1, sf_name);
                  ftmax_set_int(a + 2, i);
                  ftmax_set_int(a + 3, j);
                  outlet_anything(self->outlet, sym_preset, 4, a);
                }
              }
            }
          }
        }
        else
          error("fluidsynth~ info: no soundfonts loaded");
      }
      else if(sym == sym_channels)
      {
        int n = fluid_synth_count_midi_channels(self->synth);
        int i;
        
        for(i=0; i<n; i++)
        {
          fluid_preset_t *preset = fluid_synth_get_channel_preset(self->synth, i);
          
          if(preset != NULL)
          {
            char *preset_str = fluid_preset_get_name(preset);
            ftmax_symbol_t preset_name = ftmax_new_symbol(preset_str);
            unsigned int sf_id, bank_num, prog_num;
            fluid_sfont_t *sf;
            ftmax_atom_t a[5];
            
            fluid_synth_get_program(self->synth, i, &sf_id, &bank_num, &prog_num);
            sf = fluid_synth_get_sfont_by_id(self->synth, sf_id);

            ftmax_set_int(a, i + 1);
            ftmax_set_symbol(a + 1, fluidmax_sfont_get_name(sf));
            ftmax_set_int(a + 2, bank_num);
            ftmax_set_int(a + 3, prog_num);
            ftmax_set_symbol(a + 4, preset_name);
            outlet_anything(self->outlet, sym_channel, 5, a);
          }
          else
          {
            ftmax_atom_t a[2];
            
            ftmax_set_int(a, i + 1);
            ftmax_set_symbol(a + 1, sym_undefined);
            outlet_anything(self->outlet, sym_channel, 2, a);
          }
        }
      }
      else if(sym == sym_gain)
      {
        ftmax_atom_t a;
        double gain = fluid_synth_get_gain(self->synth);

        ftmax_set_float(&a, gain);
        outlet_anything(self->outlet, sym_channel, 1, &a);
      }
      else if(sym == sym_reverb)
      {
        if(self->reverb != 0)
        {
          double level = fluid_synth_get_reverb_level(self->synth);
          double room = fluid_synth_get_reverb_roomsize(self->synth);
          double damping = fluid_synth_get_reverb_damp(self->synth);
          double width = fluid_synth_get_reverb_width(self->synth);
          ftmax_atom_t a[4];
          
          ftmax_set_float(a, level);
          ftmax_set_float(a + 1, room);
          ftmax_set_float(a + 2, damping);
          ftmax_set_float(a + 3, width);
          outlet_anything(self->outlet, sym_reverb, 4, a);          
        }
        else
        {
          ftmax_atom_t a;
          
          ftmax_set_symbol(&a, sym_off);
          outlet_anything(self->outlet, sym_reverb, 1, &a);
        }
      }
      else if(sym == sym_chorus)
      {
        if(self->chorus != 0)
        {
          double level = fluid_synth_get_chorus_level(self->synth);
          double speed = fluid_synth_get_chorus_speed_Hz(self->synth);
          double depth = fluid_synth_get_chorus_depth_ms(self->synth);
          int type = fluid_synth_get_chorus_type(self->synth);
          int nr = fluid_synth_get_chorus_nr(self->synth);
          ftmax_atom_t a[5];
          
          ftmax_set_float(a, level);
          ftmax_set_float(a + 1, speed);
          ftmax_set_float(a + 2, depth);
          ftmax_set_int(a + 3, type);
          ftmax_set_int(a + 4, nr);
          outlet_anything(self->outlet, sym_chorus, 5, a);
        }
        else
        {
          ftmax_atom_t a;
          
          ftmax_set_symbol(&a, sym_off);
          outlet_anything(self->outlet, sym_chorus, 1, &a);
        }
      }
      else if(sym == sym_polyphony)
      {
        int polyphony = fluid_synth_get_polyphony(self->synth);
        ftmax_atom_t a;
        
        ftmax_set_int(&a, polyphony);
        outlet_anything(self->outlet, sym_polyphony, 1, &a);
      }
    }
  }
}

/***************************************************************
 *
 *  class
 *
 */
static void *
fluidmax_new(Symbol *s, short ac, Atom *at)
{  
  fluidmax_t *self = (fluidmax_t *)newobject(fluidmax_class);
  int polyphony = 256;
  int midi_channels = 16;

  self->outlet = outlet_new(self, "anything");

  dsp_setup((t_pxobject *)self, 0);
  outlet_new(self, "signal");
  outlet_new(self, "signal");

  self->synth = NULL;
  self->settings = new_fluid_settings();
  self->reverb = 0;
  self->chorus = 0;
  self->mute = 0;
  
  if(ac > 0 && ftmax_is_number(at))
  {
    polyphony = ftmax_get_number_int(at);
    ac--;
    at++;
  }
  
  if(ac > 0 && ftmax_is_number(at))
  {
    midi_channels = ftmax_get_number_int(at);
    ac--;
    at++;
  }
  
  if(ac > 0 && ftmax_is_symbol(at))
  {
    fluidmax_load((t_object *)self, NULL, 1, at);
  }
  
  if(self->settings != NULL)
  {
    fluid_settings_setint(self->settings, "synth.midi-channels", midi_channels);
    fluid_settings_setint(self->settings, "synth.polyphony", polyphony);
    fluid_settings_setnum(self->settings, "synth.gain", 0.600000);
    fluid_settings_setnum(self->settings, "synth.sample-rate", sys_getsr());
  
    self->synth = new_fluid_synth(self->settings);
    
    if(self->synth != NULL)
    {
      fluid_synth_set_reverb_on(self->synth, 0);
      fluid_synth_set_chorus_on(self->synth, 0);      
    
      if(ac > 0 && ftmax_is_symbol(at))
        fluidmax_load((t_object *)self, NULL, ac, at);

      return self;
    }

    delete_fluid_settings(self->settings);
  }

  error("fluidsynth~: cannot create FluidSynth core");
  
  return NULL;
}

static void
fluidmax_free(t_pxobject *o)
{
  fluidmax_t *self = (fluidmax_t *)o;

  if(self->settings != NULL )
    delete_fluid_settings(self->settings);

  if(self->synth != NULL )
    delete_fluid_synth(self->synth);

  dsp_free(o);
}

int 
main(void)
{
  setup(&fluidmax_class, (method)fluidmax_new, (method)fluidmax_free, (short)sizeof(fluidmax_t), 0, A_GIMME, 0);
  dsp_initclass();

  addmess((method)fluidmax_dsp, "dsp", A_CANT, 0);

  addmess((method)fluidmax_version, "version", 0);
  addmess((method)fluidmax_print, "print", A_GIMME, 0);

  addmess((method)fluidmax_load, "load", A_GIMME, 0);
  addmess((method)fluidmax_unload, "unload", A_GIMME, 0);
  addmess((method)fluidmax_reload, "reload", A_GIMME, 0);
  addmess((method)fluidmax_info, "info", A_GIMME, 0);
  
  addmess((method)fluidmax_panic, "panic", A_GIMME, 0);
  addmess((method)fluidmax_reset, "reset", A_GIMME, 0);
  addmess((method)fluidmax_mute, "mute", A_GIMME, 0);
  addmess((method)fluidmax_unmute, "unmute", 0);

  /*addmess((method)fluidmax_tuning_keys, "tuning-keys", A_GIMME, 0);*/
  addmess((method)fluidmax_tuning_octave, "tuning-octave", A_GIMME, 0);
  addmess((method)fluidmax_tuning_select, "tuning-select", A_GIMME, 0);
  addmess((method)fluidmax_tuning_reset, "tuning-reset", A_GIMME, 0);

  addmess((method)fluidmax_reverb, "reverb", A_GIMME, 0);
  addmess((method)fluidmax_chorus, "chorus", A_GIMME, 0);  
  addmess((method)fluidmax_set_gain, "gain", A_GIMME, 0);  
  addmess((method)fluidmax_set_resampling_method, "resample", A_GIMME, 0);  
    
  addmess((method)fluidmax_note, "note", A_GIMME, 0);
  addmess((method)fluidmax_list, "list", A_GIMME, 0);

  addmess((method)fluidmax_control_change, "control", A_GIMME, 0);
  addmess((method)fluidmax_mod, "mod", A_GIMME, 0);
  
  addmess((method)fluidmax_pitch_bend, "bend", A_GIMME, 0);
  addmess((method)fluidmax_pitch_bend_wheel, "wheel", A_GIMME, 0);

  addmess((method)fluidmax_program_change, "program", A_GIMME, 0);
  addmess((method)fluidmax_bank_select, "bank", A_GIMME, 0);
  addmess((method)fluidmax_select, "select", A_GIMME, 0);

  sym_on = ftmax_new_symbol("on");
  sym_off = ftmax_new_symbol("off");
  sym_undefined = ftmax_new_symbol("undefined");
  sym_gain = ftmax_new_symbol("gain");
  sym_channels = ftmax_new_symbol("channels");
  sym_channel = ftmax_new_symbol("channel");
  sym_soundfonts = ftmax_new_symbol("soundfonts");
  sym_soundfont = ftmax_new_symbol("soundfont");
  sym_presets = ftmax_new_symbol("presets");
  sym_preset = ftmax_new_symbol("preset");
  sym_reverb = ftmax_new_symbol("reverb");
  sym_chorus = ftmax_new_symbol("chorus");
  sym_polyphony = ftmax_new_symbol("polyphony");
  sym_nearest = ftmax_new_symbol("nearest");
  sym_linear = ftmax_new_symbol("linear");
  sym_cubic = ftmax_new_symbol("cubic");
  sym_sinc = ftmax_new_symbol("sinc");

  
  fluidmax_version(NULL);
  
  return 0;
}
