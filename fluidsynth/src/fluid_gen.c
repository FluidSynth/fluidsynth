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


#include "fluid_gen.h"
#include "fluid_chan.h"

/* fluid_gen_set_default_values
 *
 * Set an array of generators to their initial value 
 */
int 
fluid_gen_set_default_values(fluid_gen_t* gen)
{
  int i;

  for (i = 0; i < GEN_LAST; i++) {
    gen[i].flags = GEN_UNUSED;
    gen[i].mod = 0.0;
    gen[i].nrpn = 0.0;
  }

  gen[GEN_STARTADDROFS].val = (fluid_real_t) 0.0;              /* SF2.01 section 8.1.3 #0 */
  gen[GEN_ENDADDROFS].val = (fluid_real_t) 0.0f;               /* # 1  */
  gen[GEN_STARTLOOPADDROFS].val = (fluid_real_t) 0.0f;         /* # 2  */
  gen[GEN_ENDLOOPADDROFS].val = (fluid_real_t) 0.0f;           /* # 3  */
  gen[GEN_STARTADDRCOARSEOFS].val = (fluid_real_t) 0.0f;       /* # 4  */
  gen[GEN_MODLFOTOPITCH].val = (fluid_real_t) 0.0f;            /* # 5  */
  gen[GEN_VIBLFOTOPITCH].val = (fluid_real_t) 0.0f;            /* # 6  */
  gen[GEN_MODENVTOPITCH].val = (fluid_real_t) 0.0f;            /* # 7  */
  gen[GEN_FILTERFC].val = (fluid_real_t) 13500.0f;             /* # 8  */
  gen[GEN_FILTERQ].val = (fluid_real_t) 0.0f;                  /* # 9  */
  gen[GEN_MODLFOTOFILTERFC].val = (fluid_real_t) 0.0f;         /* # 10 */
  gen[GEN_MODENVTOFILTERFC].val = (fluid_real_t) 0.0f;         /* # 11 */
  gen[GEN_ENDADDRCOARSEOFS].val = (fluid_real_t) 0.0f;         /* # 12 */
  gen[GEN_MODLFOTOVOL].val = (fluid_real_t) 0.0f;              /* # 13 */
  /*                                                             # 14 */
  gen[GEN_CHORUSSEND].val = (fluid_real_t) 0.0f;               /* # 15 */
  gen[GEN_REVERBSEND].val = (fluid_real_t) 0.0f;               /* # 16 */
  gen[GEN_PAN].val = (fluid_real_t) 0.0f;                      /* # 17 */
  /*                                                             # 18 - # 20 */
  gen[GEN_MODLFODELAY].val = (fluid_real_t) -12000.0f;         /* # 21 => instantaneous */
  gen[GEN_MODLFOFREQ].val = (fluid_real_t) 0.0f;               /* # 22 => 8.176 Hz */
  gen[GEN_VIBLFODELAY].val = (fluid_real_t) -12000.0f;         /* # 23 => instantaneous */
  gen[GEN_VIBLFOFREQ].val = (fluid_real_t) 0.0f;               /* # 24 => 8.176 Hz */
  gen[GEN_MODENVDELAY].val = (fluid_real_t) -12000.0f;         /* # 25 => instantaneous */
  gen[GEN_MODENVATTACK].val = (fluid_real_t) -12000.0f;        /* # 26 => instantaneous */
  gen[GEN_MODENVHOLD].val = (fluid_real_t) -12000.0f;          /* # 27 => instantaneous */
  gen[GEN_MODENVDECAY].val = (fluid_real_t) -12000.0f;         /* # 28 => instantaneous */
  gen[GEN_MODENVSUSTAIN].val = (fluid_real_t) 0.0f;            /* # 29 => 0 dB */
  gen[GEN_MODENVRELEASE].val = (fluid_real_t) -12000.0f;       /* # 30 => instantaneous */
  gen[GEN_KEYTOMODENVHOLD].val = (fluid_real_t) 0.0f;          /* # 31 */
  gen[GEN_KEYTOMODENVDECAY].val = (fluid_real_t) 0.0f;         /* # 32 */
  gen[GEN_VOLENVDELAY].val = (fluid_real_t) -12000.0f;         /* # 33 */
  gen[GEN_VOLENVATTACK].val = (fluid_real_t) -12000.0f;        /* # 34 */
  gen[GEN_VOLENVHOLD].val = (fluid_real_t) -12000.0f;          /* # 35 */
  gen[GEN_VOLENVDECAY].val = (fluid_real_t) -12000.0f;         /* # 36 */
  gen[GEN_VOLENVSUSTAIN].val = (fluid_real_t) 0.0f;            /* # 37 */
  gen[GEN_VOLENVRELEASE].val = (fluid_real_t) -12000.0f;       /* # 38 */
  gen[GEN_KEYTOVOLENVHOLD].val = (fluid_real_t) 0.0f;          /* # 39 */
  gen[GEN_KEYTOVOLENVDECAY].val = (fluid_real_t) 0.0f;         /* # 40 */
  /*                                                             # 41 - # 42 */
  /* GEN_KEYRANGE, GEN_VELRANGE are handled in fluid_defsfont.c   # 43 - # 44 */
  gen[GEN_STARTLOOPADDRCOARSEOFS].val = (fluid_real_t) 0.0f;   /* # 45 */
  gen[GEN_KEYNUM].val = (fluid_real_t) -1.0f;                  /* # 46 => disabled */
  gen[GEN_VELOCITY].val = (fluid_real_t) -1.0f;                /* # 47 => disabled */
  gen[GEN_ATTENUATION].val = (fluid_real_t) 0.0f;              /* # 48 => 0 dB */
  /*                                                             # 49 */
  gen[GEN_ENDLOOPADDRCOARSEOFS].val = (fluid_real_t) 0.0f;     /* # 50 */
  gen[GEN_COARSETUNE].val = (fluid_real_t) 0.0f;               /* # 51 */
  gen[GEN_FINETUNE].val = (fluid_real_t) 0.0f;                 /* # 52 */
  gen[GEN_SAMPLEID].val = (fluid_real_t) 0.0f;                 /* # 53 ??? */
  gen[GEN_SAMPLEMODE].val = (fluid_real_t) 0.0f;               /* # 54 => no loop */
  /*                                                             # 55 */
  gen[GEN_SCALETUNE].val = (fluid_real_t) 100.0f;              /* # 56 => 1 semitone / key */
  gen[GEN_EXCLUSIVECLASS].val = (fluid_real_t) 0.0f;           /* # 57 => no exclusive class */
  gen[GEN_OVERRIDEROOTKEY].val = (fluid_real_t) -1.0f;         /* # 58 => disabled */
  gen[GEN_PITCH].val = 0.0f;

  return FLUID_OK;
}


/* fluid_gen_init
 *
 * Set an array of generators to their initial value 
 */
int 
fluid_gen_init(fluid_gen_t* gen, fluid_channel_t* channel)
{
  int i;

  fluid_gen_set_default_values(gen);

  for (i = 0; i < GEN_LAST; i++) {
    gen[i].nrpn = fluid_channel_get_gen(channel, i);
  }

  return FLUID_OK;
}


fluid_real_t fluid_gen_map_nrpn(int gen, int data)
{
  fluid_real_t value = (float) data - 8192.0f;

  fluid_clip(value, -8192, 8192);

  switch (gen) {
  default:
  case GEN_OVERRIDEROOTKEY:
  case GEN_PITCH:
  case GEN_KEYRANGE:
  case GEN_VELRANGE:
  case GEN_SAMPLEMODE:
  case GEN_SAMPLEID:
  case GEN_INSTRUMENT:
  case GEN_KEYNUM:
  case GEN_EXCLUSIVECLASS:
    return 0.0;

  case GEN_STARTADDROFS:
  case GEN_ENDADDROFS:
  case GEN_STARTLOOPADDROFS:
  case GEN_ENDLOOPADDROFS:
  case GEN_STARTADDRCOARSEOFS:
  case GEN_FILTERQ:
  case GEN_ENDADDRCOARSEOFS:
  case GEN_MODLFOTOVOL:
  case GEN_CHORUSSEND:
  case GEN_REVERBSEND:
  case GEN_PAN:
  case GEN_KEYTOMODENVHOLD:
  case GEN_KEYTOMODENVDECAY:
  case GEN_MODENVSUSTAIN:
  case GEN_VOLENVSUSTAIN:
  case GEN_KEYTOVOLENVHOLD:
  case GEN_KEYTOVOLENVDECAY:
  case GEN_STARTLOOPADDRCOARSEOFS:
  case GEN_ENDLOOPADDRCOARSEOFS:
  case GEN_VELOCITY:
  case GEN_ATTENUATION:
  case GEN_COARSETUNE:
  case GEN_FINETUNE:
  case GEN_SCALETUNE:
    return value;

  case GEN_MODLFOTOPITCH:
  case GEN_VIBLFOTOPITCH:
  case GEN_MODENVTOPITCH:
  case GEN_FILTERFC:
  case GEN_MODLFOTOFILTERFC:
  case GEN_MODENVTOFILTERFC:
  case GEN_MODLFODELAY:
  case GEN_VIBLFODELAY:
  case GEN_MODENVDELAY:
  case GEN_MODENVATTACK:
  case GEN_MODENVHOLD:
  case GEN_MODENVDECAY:
  case GEN_MODENVRELEASE:
  case GEN_VOLENVDELAY:
  case GEN_VOLENVATTACK:
  case GEN_VOLENVHOLD:
  case GEN_VOLENVDECAY:
  case GEN_VOLENVRELEASE:
    return 2 * value;

  case GEN_MODLFOFREQ:
  case GEN_VIBLFOFREQ:
    return 4 * value;
  }

  return 0.0f;
}
