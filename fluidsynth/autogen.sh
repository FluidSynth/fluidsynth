#!/bin/sh

# Some poor souls have linux distributions, that don't install pkg-config by default. 
#pkg-config --version does actually nothing, but it will fail and give 'sort of' an error message...

pkg-config --version > /dev/null    \
 && aclocal                         \
 && libtoolize -f                   \
 && autoheader                      \
 && autoconf                        \
 && automake -a

