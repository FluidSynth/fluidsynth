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

#ifndef _FLUIDSYNTH_GEN_H
#define _FLUIDSYNTH_GEN_H

#ifdef __cplusplus
extern "C" {
#endif


  /** List of generator numbers
      Soundfont 2.01 specifications section 8.1.3 */
enum fluid_gen_type {
  GEN_STARTADDROFS,
  GEN_ENDADDROFS,
  GEN_STARTLOOPADDROFS,
  GEN_ENDLOOPADDROFS,
  GEN_STARTADDRCOARSEOFS,
  GEN_MODLFOTOPITCH,
  GEN_VIBLFOTOPITCH,
  GEN_MODENVTOPITCH,
  GEN_FILTERFC,
  GEN_FILTERQ,
  GEN_MODLFOTOFILTERFC,
  GEN_MODENVTOFILTERFC,
  GEN_ENDADDRCOARSEOFS,
  GEN_MODLFOTOVOL,
  GEN_UNUSED1,
  GEN_CHORUSSEND,
  GEN_REVERBSEND,
  GEN_PAN,
  GEN_UNUSED2,
  GEN_UNUSED3,
  GEN_UNUSED4,
  GEN_MODLFODELAY,
  GEN_MODLFOFREQ,
  GEN_VIBLFODELAY,
  GEN_VIBLFOFREQ,
  GEN_MODENVDELAY,
  GEN_MODENVATTACK,
  GEN_MODENVHOLD,
  GEN_MODENVDECAY,
  GEN_MODENVSUSTAIN,
  GEN_MODENVRELEASE,
  GEN_KEYTOMODENVHOLD,
  GEN_KEYTOMODENVDECAY,
  GEN_VOLENVDELAY,
  GEN_VOLENVATTACK,
  GEN_VOLENVHOLD,
  GEN_VOLENVDECAY,
  GEN_VOLENVSUSTAIN,
  GEN_VOLENVRELEASE,
  GEN_KEYTOVOLENVHOLD,
  GEN_KEYTOVOLENVDECAY,
  GEN_INSTRUMENT,
  GEN_RESERVED1,
  GEN_KEYRANGE,
  GEN_VELRANGE,
  GEN_STARTLOOPADDRCOARSEOFS,
  GEN_KEYNUM,
  GEN_VELOCITY,
  GEN_ATTENUATION,
  GEN_RESERVED2,
  GEN_ENDLOOPADDRCOARSEOFS,
  GEN_COARSETUNE,
  GEN_FINETUNE,
  GEN_SAMPLEID,
  GEN_SAMPLEMODE,
  GEN_RESERVED3,
  GEN_SCALETUNE,
  GEN_EXCLUSIVECLASS,
  GEN_OVERRIDEROOTKEY,

  /* the initial pitch is not a "standard" generator. It is not
   * mentioned in the list of generator in the SF2 specifications. It
   * is used, however, as the destination for the default pitch wheel
   * modulator. */
  GEN_PITCH,
  GEN_LAST 
};


  /*
   *  fluid_gen_t
   *  Sound font generator
   */
typedef struct _fluid_gen_t
{
  unsigned char flags; /* is it used or not */
  double val;          /* The nominal value */
  double mod;          /* Change by modulators */
  double nrpn;         /* Change by NRPN messages */
} fluid_gen_t;

enum fluid_gen_flags
{
  GEN_UNUSED,
  GEN_SET,
  GEN_ABS_NRPN
};

  /** Reset an array of generators to the SF2.01 default values */
FLUIDSYNTH_API int fluid_gen_set_default_values(fluid_gen_t* gen);



#ifdef __cplusplus
}
#endif
#endif /* _FLUIDSYNTH_GEN_H */

