## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS=-I m4

SUBDIRS = src doc include cmake_admin
EXTRA_DIST = TODO acinclude.m4 autogen.sh fluidsynth.pc.in \
  fluidsynth.spec.in fluidsynth.spec fluidsynth.anjuta README-OSX \
  README.cmake CMakeLists.txt
DISTCLEANFILES = fluidsynth.pc
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = fluidsynth.pc
