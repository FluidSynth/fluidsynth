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


#ifndef _FLUID_STRTOK_H
#define _FLUID_STRTOK_H

#include "fluidsynth_priv.h"


/** string tokenizer */
typedef struct {
  char* string;
  char* delimiters;
  int offset;
  int len;
} fluid_strtok_t;

fluid_strtok_t* new_fluid_strtok(char* s, char* d);
int delete_fluid_strtok(fluid_strtok_t* st);
int fluid_strtok_set(fluid_strtok_t* st, char* s, char* d);
char* fluid_strtok_next_token(fluid_strtok_t* st);
int fluid_strtok_has_more(fluid_strtok_t* st);
int fluid_strtok_char_index(char c, char* s);


#endif /* _FLUID_STRTOK_H */
