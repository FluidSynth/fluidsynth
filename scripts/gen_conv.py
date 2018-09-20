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


from emit import print_vector, create_header, close_header, print_dec, print_hex, print_fpu


# Declare constants

FLUID_CENTS_HZ_SIZE    = 1200
FLUID_VEL_CB_SIZE      = 128
FLUID_CB_AMP_SIZE      = 1441
FLUID_PAN_SIZE         = 1002
FLUID_PEAK_ATTENUATION = 960.0


# Declare matrices.

fluid_ct2hz_tab   = [0 for j in range(FLUID_CENTS_HZ_SIZE)]
fluid_cb2amp_tab  = [0 for j in range(FLUID_CB_AMP_SIZE)]
fluid_concave_tab = [0 for j in range(FLUID_VEL_CB_SIZE)]
fluid_convex_tab  = [0 for j in range(FLUID_VEL_CB_SIZE)]
fluid_pan_tab     = [0 for j in range(FLUID_PAN_SIZE)]


# MAIN CALCULATION

i = 0
while i < FLUID_CENTS_HZ_SIZE:
    fluid_ct2hz_tab[i] = math.pow(2.0, i / 1200.0)
    i = i + 1


# centibels to amplitude conversion
# Note: SF2.01 section 8.1.3: Initial attenuation range is
# between 0 and 144 dB. Therefore a negative attenuation is
# not allowed.
#

i = 0
while i < FLUID_CB_AMP_SIZE:
    fluid_cb2amp_tab[i] = math.pow(10.0, i / -200.0)
    i = i + 1

# initialize the conversion tables (see fluid_mod.c
# fluid_mod_get_value cases 4 and 8)


# concave unipolar positive transform curve
fluid_concave_tab[0] = 0.0;
fluid_concave_tab[FLUID_VEL_CB_SIZE - 1] = 1.0


# convex unipolar positive transform curve
fluid_convex_tab[0] = 0;
fluid_convex_tab[FLUID_VEL_CB_SIZE - 1] = 1.0

# There seems to be an error in the specs. The equations are
# implemented according to the pictures on SF2.01 page 73.

i = 1
while i < FLUID_VEL_CB_SIZE-1:
    x = (-200.0 / FLUID_PEAK_ATTENUATION) * 2 * math.log10(float(i) / (FLUID_VEL_CB_SIZE - 1))
    fluid_convex_tab[i] = 1.0 - x
    fluid_concave_tab[FLUID_VEL_CB_SIZE - 1 - i] = x
    i = i + 1

# initialize the pan conversion table
x = math.pi / 2.0 / (FLUID_PAN_SIZE - 1.0)

i = 0
while i < FLUID_PAN_SIZE:
    fluid_pan_tab[i] = math.sin(i * x)
    i = i + 1



fp = create_header(sys.argv[1], "fluid_conv_tables")

print_dec(fp, "FLUID_CENTS_HZ_SIZE", FLUID_CENTS_HZ_SIZE)
print_dec(fp, "FLUID_VEL_CB_SIZE",   FLUID_VEL_CB_SIZE)
print_dec(fp, "FLUID_CB_AMP_SIZE",   FLUID_CB_AMP_SIZE)
print_dec(fp, "FLUID_PAN_SIZE",      FLUID_PAN_SIZE)

print_vector(fp, fluid_ct2hz_tab,   "fluid_ct2hz_tab")
print_vector(fp, fluid_cb2amp_tab,  "fluid_cb2amp_tab")
print_vector(fp, fluid_concave_tab, "fluid_concave_tab")
print_vector(fp, fluid_convex_tab,  "fluid_convex_tab")
print_vector(fp, fluid_pan_tab,     "fluid_pan_tab")

close_header(fp, "fluid_conv_tables")

fp = create_header(sys.argv[1], "fluid_conv_const")
print_fpu(fp, "FLUID_PEAK_ATTENUATION", FLUID_PEAK_ATTENUATION)
close_header(fp, "fluid_conv_const")
