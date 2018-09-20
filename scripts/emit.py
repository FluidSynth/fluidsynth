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

import string

# Write a text line to a file and adds a newline at the end
def print_ln(fp, text):
	fp.write(text + '\n');


# Create the header file and write the prologue on top.
def create_header(prefix, name):
    fp = open(prefix + '/' + name + ".h", "w")

    print_ln(fp, "/* This file is auto-generated. Do not edit it. */\n")

    prolog = str.upper(name)

    print_ln(fp, "#ifndef __%s_H__" % prolog)
    print_ln(fp, "#define __%s_H__" % prolog)
    print_ln(fp, "")

    return fp


# Write the epilogue and close the header file
def close_header(fp, name):
    print_ln(fp, "")
    print_ln(fp, "#endif /* __%s_H__ */" % str.upper(name))
    fp.close


# Emit a matrix into the header file
def print_matrix(fp, matrix, name):
    x_size = len(matrix[0]);
    y_size = len(matrix);

    print_ln(fp, "static const fluid_real_t %s[%d][%d] = {" % (name, y_size, x_size))
    
    y = 0;
    while y < y_size:
        x = 0

        text = " { "

        while x < x_size-1:
            text = text + "%.15e, " % matrix[y][x]
            x = x + 1

        text = text + "%.15e }" % matrix[y][x]

        if (y < y_size-1):
            text = text + ","

        print_ln(fp, text)
        y = y + 1

    print_ln(fp, "}; /* %s */" % name)
    print_ln(fp, "")


# Emit a vector into the header file
def print_vector(fp, vector, name):
    size = len(vector);

    print_ln(fp, "static const fluid_real_t %s[%d] = {" % (name, size))
    
    n = 0;
    while n < size:
        text = " %.15e" % vector[n]

        if (n < size-1):
            text = text + ","

        print_ln(fp, text)
        n = n + 1

    print_ln(fp, "}; /* %s */" % name)
    print_ln(fp, "")


# Emit a decimal number
def print_dec(fp, name, value):
    print_ln(fp, "#define %-32s %d" % (name, value))


# Emit an hex number
def print_hex(fp, name, value):
    print_ln(fp, "#define %-32s 0x%08X" % (name, value))


# Emit an floating point number
def print_fpu(fp, name, value):
    print_ln(fp, "#define %-32s %f" % (name, value))

