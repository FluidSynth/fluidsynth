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


#include "fluid_strtok.h"
#include "fluidsynth_priv.h"

/*
 * new_fluid_strtok
 */
fluid_strtok_t* new_fluid_strtok(char* s, char* d) 
{
  fluid_strtok_t* st;
  st = FLUID_NEW(fluid_strtok_t);
  if (st == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  /* Careful! the strings are not copied for speed */
  st->string = s;
  st->delimiters = d;
  st->offset = 0;
  st->len = (s == NULL)? 0 : strlen(s);
  return st;
}

int delete_fluid_strtok(fluid_strtok_t* st) 
{
  if (st == NULL) {
    FLUID_LOG(FLUID_ERR, "Null pointer");
    return 0;
  }
  free(st);
  return 0;
}

int fluid_strtok_set(fluid_strtok_t* st, char* s, char* d)
{
  /* Careful! the strings are not copied for speed */
  st->string = s;
  st->delimiters = d;
  st->offset = 0;
  st->len = (s == NULL)? 0 : strlen(s);
  return 0;
}

char* fluid_strtok_next_token(fluid_strtok_t* st) 
{
  int start = st->offset;
  int end;
  if ((st == NULL) || (st->string == NULL) || (st->delimiters == NULL)) {
    FLUID_LOG(FLUID_ERR, "Null pointer");
    return NULL;
  }
  if (start >= st->len) {
    return NULL;
  }
  while (fluid_strtok_char_index(st->string[start], st->delimiters) >= 0) {
    if (start == st->len) {
      return NULL;
    }
    start++;
  }
  end = start + 1;
  while (fluid_strtok_char_index(st->string[end], st->delimiters) < 0) {
    if (end == st->len) {
      break;
    }
    end++;
  }
  st->string[end] = 0;
  st->offset = end + 1;
  return &st->string[start];
}

int fluid_strtok_has_more(fluid_strtok_t* st) 
{
  int cur = st->offset;
  if ((st == NULL) || (st->string == NULL) || (st->delimiters == NULL)) {
    FLUID_LOG(FLUID_ERR, "Null pointer");
    return -1;
  }
  while (cur < st->len) {
    if (fluid_strtok_char_index(st->string[cur], st->delimiters) < 0) {
      return -1;
    }
    cur++;
  }
  return 0;
}

int fluid_strtok_char_index(char c, char* s) 
{
  int i;
  if (s == NULL) {
    FLUID_LOG(FLUID_ERR, "Null pointer");
    return -1;
  }
  for (i = 0; s[i] != 0; i++) {
    if (s[i] == c) {
      return i;
    }
  }
  return -1;
}

