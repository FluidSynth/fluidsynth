#!/bin/sh

# Add flags for aclocal if needed (Mac OSX)
ACLOCAL_PATH=""
uname -s | grep -q Darwin
if [ $? -eq 0 ]; then
    ACLOCAL_FLAGS=-I/sw/share/aclocal
fi

# Some poor souls have linux distributions, that don't install pkg-config by default. 
#pkg-config --version does actually nothing, but it will fail and give 'sort of' an error message...

pkg-config --version > /dev/null    \
 && aclocal $ACLOCAL_FLAGS          \
 && libtoolize -f                   \
 && autoheader                      \
 && autoconf                        \
 && automake -a
