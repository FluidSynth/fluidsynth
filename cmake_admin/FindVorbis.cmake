#[=======================================================================[.rst:
FindVorbis
-------

Finds the Vorbis library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Vorbis::vorbisc``
  The Vorbis core library
``Vorbis::vorbisenc``
  The Vorbis encoder library
``Vorbis::vorbisfile``
  The Vorbis file library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Vorbis_FOUND``
  True if all vorbis libraries were found.
``Vorbis_Vorbis_FOUND``
  True if the base vorbis library was found.
``Vorbis_Enc_FOUND``
  True if the encoder library was found.
``Vorbis_File_FOUND``
  True if the file library was found.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_VORBIS QUIET vorbis)
pkg_check_modules(PC_VORBISENC QUIET vorbisenc)
pkg_check_modules(PC_VORBISFILE QUIET vorbisfile)

# Find the headers and libraries
find_path(
  Vorbis_INCLUDE_DIR
  NAMES "vorbis/codec.h"
  PATHS "${PC_VORBIS_INCLUDEDIR}")

find_library(
  Vorbis_LIBRARY
  NAMES "vorbis"
  PATHS "${PC_VORBIS_LIBDIR}")

find_library(
  Vorbis_Enc_LIBRARY
  NAMES "vorbisenc"
  PATHS "${PC_VORBISENC_LIBDIR}")

find_library(
  Vorbis_File_LIBRARY
  NAMES "vorbisfile"
  PATHS "${PC_VORBISFILE_LIBDIR}")

# Handle the transitive dependencies
if(PC_VORBIS_LIBRARIES)
  set(_vorbis_link_libraries "${PC_VORBIS_LIBRARIES}")
else()
  if(NOT TARGET Ogg::ogg)
    find_package(Ogg QUIET)
  endif()
  set(_vorbis_link_libraries "Ogg::ogg" ${MATH_LIBRARY})
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Vorbis REQUIRED_VARS "Vorbis_LIBRARY" "Vorbis_Enc_LIBRARY"
                       "Vorbis_File_LIBRARY" "Vorbis_INCLUDE_DIR")

# Create the targets
if(Vorbis_FOUND AND NOT TARGET Vorbis::vorbis)
  add_library(Vorbis::vorbis UNKNOWN IMPORTED)
  set_target_properties(
    Vorbis::vorbis
    PROPERTIES IMPORTED_LOCATION "${Vorbis_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${PC_VORBIS_CFLAGS_OTHER}"
               INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_vorbis_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${PC_VORBIS_LIBDIR}")
  set(Vorbis_Vorbis_FOUND TRUE)
endif()

if(Vorbis_FOUND AND NOT TARGET Vorbis::vorbisenc)
  add_library(Vorbis::vorbisenc UNKNOWN IMPORTED)
  set_target_properties(
    Vorbis::vorbisenc
    PROPERTIES IMPORTED_LOCATION "${Vorbis_Enc_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${PC_VORBISENC_CFLAGS_OTHER}"
               INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "Vorbis::vorbis"
               INTERFACE_LINK_DIRECTORIES "${PC_VORBISENC_LIBDIR}")
  set(Vorbis_Enc_FOUND TRUE)
endif()

if(Vorbis_FOUND AND NOT TARGET Vorbis::vorbisfile)
  add_library(Vorbis::vorbisfile UNKNOWN IMPORTED)
  set_target_properties(
    Vorbis::vorbisfile
    PROPERTIES IMPORTED_LOCATION "${Vorbis_File_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${PC_VORBISFILE_CFLAGS_OTHER}"
               INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "Vorbis::vorbis"
               INTERFACE_LINK_DIRECTORIES "${PC_VORBISFILE_LIBDIR}")
  set(Vorbis_File_FOUND TRUE)
endif()

mark_as_advanced(Vorbis_LIBRARY Vorbis_Enc_LIBRARY Vorbis_File_LIBRARY)
