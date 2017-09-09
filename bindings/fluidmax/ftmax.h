/***************************************************************************************
 *
 *   Copyright (C) 2004 by IRCAM-Centre Georges Pompidou, Paris, France.
 *   Author: Norbert.Schnell@ircam.fr
 *
 *   FTS -> MAX/MSP utilities
 *
 */

/*
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
#ifndef _FTMAX_H_
#define _FTMAX_H_

#include <stdlib.h>

#ifndef MAGIC
#include "ext.h"
#include "z_dsp.h"
#endif

#define ftm_malloc(s) NewPtr(s)
#define ftmax_realloc NO_realloc
#define ftmax_free(p) DisposePtr((char *)(p))

#define restrict

#define M_TWOPI 6.283185308
//#define M_PI 3.141592654

#define ftmax_object_t t_object
#define ftmax_dsp_object_t t_pxobject

typedef t_atom ftmax_atom_t;
typedef t_symbol *ftmax_symbol_t;
typedef t_messlist ftmax_class_t;
typedef method ftmax_method_t;

#define ftmax_is_int(a) ((a)->a_type == A_LONG)
#define ftmax_is_float(a) ((a)->a_type == A_FLOAT)
#define ftmax_is_number(a) ((a)->a_type == A_LONG || (a)->a_type == A_FLOAT)
#define ftmax_is_symbol(a) ((a)->a_type == A_SYM)

#define ftmax_get_int(a) ((a)->a_w.w_long)
#define ftmax_get_float(a) ((a)->a_w.w_float)
#define ftmax_get_number_int(a) (((a)->a_type == A_LONG)? ((a)->a_w.w_long): (int)((a)->a_w.w_float))
#define ftmax_get_number_float(a) (((a)->a_type == A_FLOAT)? (double)((a)->a_w.w_float): (double)((a)->a_w.w_long))
#define ftmax_get_symbol(a) ((a)->a_w.w_sym)

#define ftmax_set_int(a, v) ((a)->a_type = A_LONG, (a)->a_w.w_long = (v))
#define ftmax_set_float(a, v) ((a)->a_type = A_FLOAT, (a)->a_w.w_float = (v))
#define ftmax_set_symbol(a, v) ((a)->a_type = A_SYM, (a)->a_w.w_sym = (v))

#define ftmax_symbol_name(s) ((s)->s_name)

#define ftmax_new_symbol(s) gensym(s)

extern void ftmax_class_message_varargs(ftmax_class_t *cl, ftmax_symbol_t sym, void *method);

#endif
