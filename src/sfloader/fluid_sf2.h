/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * SoundFont loading code borrowed from Smurf SoundFont Editor by Josh Green
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


#ifndef _FLUID_SF2_H
#define _FLUID_SF2_H


#include "fluid_gen.h"
#include "fluid_list.h"
#include "fluid_mod.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"


/* Sound Font structure defines */

typedef struct _SFVersion
{ /* version structure */
    unsigned short major;
    unsigned short minor;
} SFVersion;

typedef struct _SFMod
{ /* Modulator structure */
    unsigned short src; /* source modulator */
    unsigned short dest; /* destination generator */
    signed short amount; /* signed, degree of modulation */
    unsigned short amtsrc; /* second source controls amnt of first */
    unsigned short trans; /* transform applied to source */
} SFMod;

typedef union _SFGenAmount { /* Generator amount structure */
    signed short sword; /* signed 16 bit value */
    unsigned short uword; /* unsigned 16 bit value */
    struct
    {
        unsigned char lo; /* low value for ranges */
        unsigned char hi; /* high value for ranges */
    } range;
} SFGenAmount;

typedef struct _SFGen
{ /* Generator structure */
    unsigned short id; /* generator ID */
    SFGenAmount amount; /* generator value */
} SFGen;

typedef struct _SFZone
{ /* Sample/instrument zone structure */
    fluid_list_t *instsamp; /* instrument/sample pointer for zone */
    fluid_list_t *gen; /* list of generators */
    fluid_list_t *mod; /* list of modulators */
} SFZone;

typedef struct _SFSample
{ /* Sample structure */
    char name[21]; /* Name of sample */
    unsigned char samfile; /* Loaded sfont/sample buffer = 0/1 */
    unsigned int start; /* Offset in sample area to start of sample */
    unsigned int end; /* Offset from start to end of sample,
             this is the last point of the
             sample, the SF spec has this as the
             1st point after, corrected on
             load/save */
    unsigned int loopstart; /* Offset from start to start of loop */
    unsigned int loopend; /* Offset from start to end of loop,
                 marks the first point after loop,
                 whose sample value is ideally
                 equivalent to loopstart */
    unsigned int samplerate; /* Sample rate recorded at */
    unsigned char origpitch; /* root midi key number */
    signed char pitchadj; /* pitch correction in cents */
    unsigned short sampletype; /* 1 mono,2 right,4 left,linked 8,0x8000=ROM */
    fluid_sample_t *fluid_sample; /* Imported sample (fixed up in fluid_defsfont_load) */
} SFSample;

typedef struct _SFInst
{ /* Instrument structure */
    char name[21]; /* Name of instrument */
    fluid_list_t *zone; /* list of instrument zones */
} SFInst;

typedef struct _SFPreset
{ /* Preset structure */
    char name[21]; /* preset name */
    unsigned short prenum; /* preset number */
    unsigned short bank; /* bank number */
    unsigned int libr; /* Not used (preserved) */
    unsigned int genre; /* Not used (preserved) */
    unsigned int morph; /* Not used (preserved) */
    fluid_list_t *zone; /* list of preset zones */
} SFPreset;

/* NOTE: sffd is also used to determine if sound font is new (NULL) */
typedef struct _SFData
{ /* Sound font data structure */
    SFVersion version; /* sound font version */
    SFVersion romver; /* ROM version */

    unsigned int samplepos; /* position within sffd of the sample chunk */
    unsigned int samplesize; /* length within sffd of the sample chunk */

    unsigned int sample24pos; /* position within sffd of the sm24 chunk, set to zero if no 24 bit
                                 sample support */
    unsigned int sample24size; /* length within sffd of the sm24 chunk */

    char *fname; /* file name */
    FILE *sffd; /* loaded sfont file descriptor */
    fluid_list_t *info; /* linked list of info strings (1st byte is ID) */
    fluid_list_t *preset; /* linked list of preset info */
    fluid_list_t *inst; /* linked list of instrument info */
    fluid_list_t *sample; /* linked list of sample info */
} SFData;

/* functions */


/*-----------------------------------sffile.h----------------------------*/
/*
   File structures and routines (used to be in sffile.h)
*/

/* sfont file data structures */
typedef struct _SFChunk
{ /* RIFF file chunk structure */
    unsigned int id; /* chunk id */
    unsigned int size; /* size of the following chunk */
} SFChunk;

typedef struct _SFPhdr
{
    unsigned char name[20]; /* preset name */
    unsigned short preset; /* preset number */
    unsigned short bank; /* bank number */
    unsigned short pbagndx; /* index into preset bag */
    unsigned int library; /* just for preserving them */
    unsigned int genre; /* Not used */
    unsigned int morphology; /* Not used */
} SFPhdr;

typedef struct _SFBag
{
    unsigned short genndx; /* index into generator list */
    unsigned short modndx; /* index into modulator list */
} SFBag;

typedef struct _SFIhdr
{
    char name[20]; /* Name of instrument */
    unsigned short ibagndx; /* Instrument bag index */
} SFIhdr;

typedef struct _SFShdr
{ /* Sample header loading struct */
    char name[20]; /* Sample name */
    unsigned int start; /* Offset to start of sample */
    unsigned int end; /* Offset to end of sample */
    unsigned int loopstart; /* Offset to start of loop */
    unsigned int loopend; /* Offset to end of loop */
    unsigned int samplerate; /* Sample rate recorded at */
    unsigned char origpitch; /* root midi key number */
    signed char pitchadj; /* pitch correction in cents */
    unsigned short samplelink; /* Not used */
    unsigned short sampletype; /* 1 mono,2 right,4 left,linked 8,0x8000=ROM */
} SFShdr;

/* Public functions  */
SFData *fluid_sf2_load(const char *fname, const fluid_file_callbacks_t *fcbs);
void fluid_sf2_close(SFData *sf, const fluid_file_callbacks_t *fcbs);

#endif /* _FLUID_SF2_H */
