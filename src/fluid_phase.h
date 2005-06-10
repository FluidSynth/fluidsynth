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


#ifndef _FLUID_PHASE_H
#define _FLUID_PHASE_H

#if HAVE_CONFIG_H
#include "config.h"
#endif

/*
 *  phase
 */

#define FLUID_INTERP_BITS        8
#define FLUID_INTERP_BITS_MASK   0xff000000
#define FLUID_INTERP_BITS_SHIFT  24
#define FLUID_INTERP_MAX         256

#define FLUID_FRACT_MAX ((double)4294967296.0)

/* fluid_phase_t 
* Purpose:
* Playing pointer for voice playback
* 
* When a sample is played back at a different pitch, the playing pointer in the
* source sample will not advance exactly one sample per output sample.
* This playing pointer is implemented using fluid_phase_t.
* It is a 64 bit number. The higher 32 bits contain the 'index' (number of 
* the current sample), the lower 32 bits the fractional part.
* Access is possible in two ways: 
* -through the 64 bit part 'b64', if the architecture supports 64 bit integers
* -through 'index' and 'fract'
* Note: b64 and index / fract share the same memory location!
*/
typedef union {
    struct{
	/* Note, that the two 32-bit ints form a 64-bit int! */
#ifdef WORDS_BIGENDIAN
	sint32 index;
	uint32 fract;
#else
	uint32 fract;
	sint32 index;
#endif
    } b32;
#ifdef USE_LONGLONG
    long long b64;
#endif
} fluid_phase_t;

/* Purpose:
 * Set a to b.
 * a: fluid_phase_t
 * b: fluid_phase_t
 */
#ifdef USE_LONGLONG
#define fluid_phase_set(a,b) a=b;
#else
#define fluid_phase_set(a, b) { \
  (a).b32.fract = (b).b32.fract; \
  (a).b32.index = (b).b32.index; \
}
#endif

#define fluid_phase_set_int(a, b)   { \
  (a).b32.index = (sint32) (b); \
  (a).b32.fract = 0; \
}

/* Purpose:
 * Sets the phase a to a phase increment given in b.
 * For example, assume b is 0.9. After setting a to it, adding a to
 * the playing pointer will advance it by 0.9 samples. */
#define fluid_phase_set_float(a, b)   { \
  (a).b32.index = (sint32) (b); \
  (a).b32.fract = (uint32) (((double)(b) - (double)((a).b32.index)) * (double)FLUID_FRACT_MAX); \
}

/* Purpose:
 * Return the index and the fractional part, respectively. */
#define fluid_phase_index(_x) \
  ((int)(_x).b32.index)
#define fluid_phase_fract(_x) \
  ((_x).b32.fract)

/* Purpose:
 * Takes the fractional part of the argument phase and
 * calculates the corresponding position in the interpolation table.
 * The fractional position of the playing pointer is calculated with a quite high
 * resolution (32 bits). It would be unpractical to keep a set of interpolation 
 * coefficients for each possible fractional part...
 */
#define fluid_phase_fract_to_tablerow(_x) \
  ((int)(((_x).b32.fract & FLUID_INTERP_BITS_MASK) >> FLUID_INTERP_BITS_SHIFT))

#define fluid_phase_double(_x) \
  ((double)((_x).b32.index) + ((double)((_x).b32.fract) / FLUID_FRACT_MAX))

/* Purpose:
 * Advance a by a step of b (both are fluid_phase_t).
 */
#ifdef USE_LONGLONG
#define fluid_phase_incr(a, b) (a).b64 += (b).b64;
#else
/* The idea to use (a).index += (b).index + ((a).fract < (b).fract) to
   handle wrap-arounds comes from Mozilla's macros to handle 64-bit
   integer on 32-bit platforms. Header prlong.h in the NSPR
   library. www.mozilla.org. */
#define fluid_phase_incr(a, b)  { \
  (a).b32.fract += (b).b32.fract; \
  (a).b32.index += (b).b32.index + ((a).b32.fract < (b).b32.fract); \
}
#endif

/* Purpose:
 * Subtract b from a (both are fluid_phase_t).
 */
#ifdef USE_LONGLONG
#define fluid_phase_decr(a, b) a-=b;
#else
#define fluid_phase_decr(a, b) { \
  (a).b32.index -= b.b32.index - ((a).b32.fract < (b).b32.fract); \
  (a).b32.fract -= b.b32.fract; \
}
#endif

/* Purpose:
 * Subtract b samples from a.
 */
#define fluid_phase_sub_int(a, b) { (a).b32.index -= b; }

#if 0
#define fluid_phase_fract(_x) \
  ((fluid_real_t)((double)((_x).fract) / FLUID_FRACT_MAX))

#define fluid_phase_lt(a, b) \
  (((a).index < (b).index) || (((a).index == (b).index) && ((a).fract < (b).fract)))

#define fluid_phase_gt(a, b) \
  (((a).index > (b).index) || (((a).index == (b).index) && ((a).fract > (b).fract)))

#define fluid_phase_eq(a, b) \
  (((a).index == (b).index) && ((a).fract == (b).fract))
#endif

/* Purpose:
 * The playing pointer is _phase. How many output samples are produced, until the point _p1 in the sample is reached,
 * if _phase advances in steps of _incr? 
 */
#define fluid_phase_steps(_phase,_index,_incr) \
  (int)(((double)(_index) - fluid_phase_double(_phase)) / (double)_incr)

/* Purpose:
 * Creates the expression a.index++.
 * It is slightly different, when USE_LONGLONG is turned on. */
#define fluid_phase_index_plusplus(a) (((a).b32.index)++)

#endif  /* _FLUID_PHASE_H */
