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


#ifndef _FLUID_IO_H
#define _FLUID_IO_H


/** Read a line from the input stream.

   \returns 0 if end-of-stream, -1 if error, non zero otherwise
*/
int fluid_istream_readline(fluid_istream_t in, char* prompt, char* buf, int len);


/** Read a line from the input stream.

   \returns The number of bytes written. If an error occured, -1 is
   returned.
*/
int fluid_ostream_printf(fluid_ostream_t out, char* format, ...);



#endif /* _FLUID_IO_H */
