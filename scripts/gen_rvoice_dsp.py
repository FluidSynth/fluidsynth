# FluidSynth - A Software Synthesizer
#
# Copyright (C) 2003  Peter Hanappe and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2.1 of
# the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free
# Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA
#

import math
import sys


from emit import print_matrix, create_header, close_header, print_dec, print_hex


# Declare constants

FLUID_INTERP_BITS       = 8
FLUID_INTERP_BITS_MASK  = 0xff000000
FLUID_INTERP_BITS_SHIFT = 24
FLUID_INTERP_MAX        = 256
SINC_INTERP_ORDER       = 7


# Declare matrices.

interp_coeff_linear = [[0 for i in range(2)] for j in range(FLUID_INTERP_MAX)]
interp_coeff        = [[0 for i in range(4)] for j in range(FLUID_INTERP_MAX)]
sinc_table7         = [[0 for i in range(7)] for j in range(FLUID_INTERP_MAX)]


# MAIN CALCULATION

# Initialize the coefficients for the interpolation. The math comes
# from a mail, posted by Olli Niemitalo to the music-dsp mailing
# list (I found it in the music-dsp archives
# http://www.smartelectronix.com/musicdsp/).  */

i = 0
while i < FLUID_INTERP_MAX:
    x = float(i) / FLUID_INTERP_MAX
    interp_coeff[i][0] = x * (-0.5 + x * (1 - 0.5 * x))
    interp_coeff[i][1] = 1.0 + x * x * (1.5 * x - 2.5)
    interp_coeff[i][2] = x * (0.5 + x * (2.0 - 1.5 * x))
    interp_coeff[i][3] = 0.5 * x * x * (x - 1.0)
    interp_coeff_linear[i][0] = 1.0 - x
    interp_coeff_linear[i][1] = x
    i = i + 1

# Offset in terms of whole samples
i = 0

while i < SINC_INTERP_ORDER:
    # Offset in terms of fractional samples ('subsamples')
    i2 = 0
    while i2 < FLUID_INTERP_MAX:
        # center on middle of table
        i_shifted = float(i) - SINC_INTERP_ORDER / 2.0 + float(i2) / FLUID_INTERP_MAX

        # sinc(0) cannot be calculated straightforward (limit needed for 0/0)
        if math.fabs(i_shifted) > 0.000001:
            arg = math.pi * i_shifted
            v = math.sin(arg) / arg
            # Hanning window
            v *= 0.5 * (1.0 + math.cos(2.0 * arg / SINC_INTERP_ORDER))
        else:
            v = 1.0

        sinc_table7[FLUID_INTERP_MAX - i2 - 1][i] = v
        i2 = i2 + 1

    i = i + 1


fp = create_header(sys.argv[1], "fluid_rvoice_tables")

print_matrix(fp, interp_coeff_linear, "interp_coeff_linear")
print_matrix(fp, interp_coeff,        "interp_coeff")
print_matrix(fp, sinc_table7,         "sinc_table7")

close_header(fp, "fluid_rvoice_tables")

fp = create_header(sys.argv[1], "fluid_phase_const")
print_dec(fp, "FLUID_INTERP_BITS",       FLUID_INTERP_BITS)
print_hex(fp, "FLUID_INTERP_BITS_MASK",  FLUID_INTERP_BITS_MASK)
print_dec(fp, "FLUID_INTERP_BITS_SHIFT", FLUID_INTERP_BITS_SHIFT)
print_dec(fp, "FLUID_INTERP_MAX",        FLUID_INTERP_MAX)
print_dec(fp, "SINC_INTERP_ORDER",       SINC_INTERP_ORDER)
close_header(fp, "fluid_phase_const")
