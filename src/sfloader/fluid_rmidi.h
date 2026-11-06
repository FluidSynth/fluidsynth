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
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef _FLUID_RMIDI_H
#define _FLUID_RMIDI_H

#include "fluid_sys.h"
#include "fluid_sfont.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Bank offset assumed when an embedded bank has no DBNK chunk. */
#define FLUID_RMIDI_DEFAULT_BANK_OFFSET 1

typedef enum
{
    FLUID_RMIDI_BANK_NONE = 0, /* No embedded sound bank */
    FLUID_RMIDI_BANK_SF2,      /* Embedded SoundFont */
    FLUID_RMIDI_BANK_DLS       /* Embedded DLS bank */
} fluid_rmidi_bank_type;

typedef struct _fluid_rmidi_info_t
{
    int is_rmidi;                  /* TRUE if the file is an RMIDI file */
    fluid_long_long_t midi_offset; /* Offset of the MIDI data in the file */
    fluid_long_long_t midi_size;   /* Size of the MIDI data in bytes */
    fluid_long_long_t bank_pos;    /* Offset of the embedded bank's RIFF chunk */
    fluid_long_long_t bank_size;   /* Size of the embedded bank, including its RIFF header */
    fluid_rmidi_bank_type bank_type;
    int bank_offset;               /* Bank offset to apply to the embedded bank */
} fluid_rmidi_info_t;

/* Check whether a memory buffer starts with an RMIDI header. */
int fluid_rmidi_is_rmidi_mem(const void *buffer, size_t length);

/* Parse an RMIDI file that is already loaded into memory. */
int fluid_rmidi_parse_mem(const void *buffer, size_t length, fluid_rmidi_info_t *info);

/* Parse an RMIDI file through the given file callbacks. */
int fluid_rmidi_parse(const fluid_file_callbacks_t *fcbs, void *file,
                      fluid_long_long_t filesize, fluid_rmidi_info_t *info);

/* Parse an RMIDI file from a standard FILE handle. */
int fluid_rmidi_parse_fp(FILE *fp, fluid_long_long_t filesize, fluid_rmidi_info_t *info);

/* Seek to the start of the sound bank of the given type in an open file. */
int fluid_rmidi_seek_bank(const fluid_file_callbacks_t *fcbs, void *file,
                          fluid_long_long_t filesize, fluid_rmidi_bank_type type,
                          fluid_rmidi_info_t *info);

#ifdef __cplusplus
}
#endif

#endif /* _FLUID_RMIDI_H */
