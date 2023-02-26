#[=======================================================================[.rst:
FindMPG123
-------

Finds the MPG123 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``MPG123::libmpg123``
  The MPG123 library
``MPG123::libout123``
  The MPG123 library
``MPG123::libsyn123``
  The MPG123 library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``MPG123_FOUND``
  True if all the MPG123 libraries were found.
``MPG123_libmpg123_FOUND``
  True if the decoder library was found.
``MPG123_libout123_FOUND``
  True if the streaming library was found.
``MPG123_libsyn123_FOUND``
  True if the synthesis library was found.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_MPG123 QUIET libmpg123)
pkg_check_modules(PC_OUT123 QUIET libout123)
pkg_check_modules(PC_SYN123 QUIET libsyn123)

# Find the headers and libraries
find_path(
  MPG123_INCLUDE_DIR
  NAMES "mpg123.h"
  PATHS "${PC_MPG123_INCLUDEDIR}")

find_library(
  MPG123_libmpg123_LIBRARY
  NAMES "mpg123"
  PATHS "${PC_MPG123_LIBDIR}")

find_library(
  MPG123_libout123_LIBRARY
  NAMES "out123"
  PATHS "${PC_OUT123_LIBDIR}")

find_library(
  MPG123_libsyn123_LIBRARY
  NAMES "syn123"
  PATHS "${PC_SYN123_LIBDIR}")

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  MPG123 REQUIRED_VARS "MPG123_libmpg123_LIBRARY" "MPG123_libout123_LIBRARY"
                       "MPG123_libsyn123_LIBRARY" "MPG123_INCLUDE_DIR")

# Create the targets
if(MPG123_FOUND AND NOT TARGET MPG123::libmpg123)
  add_library(MPG123::libmpg123 UNKNOWN IMPORTED)
  set_target_properties(
    MPG123::libmpg123
    PROPERTIES IMPORTED_LOCATION "${MPG123_libmpg123_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${PC_MPG123_CFLAGS_OTHER}"
               INTERFACE_INCLUDE_DIRECTORIES "${MPG123_INCLUDE_DIR}")
  set(MPG123_libmpg123_FOUND TRUE)
endif()

if(MPG123_FOUND AND NOT TARGET MPG123::libout123)
  add_library(MPG123::libout123 UNKNOWN IMPORTED)
  set_target_properties(
    MPG123::libout123
    PROPERTIES IMPORTED_LOCATION "${MPG123_libout123_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${PC_OUT123_CFLAGS_OTHER}"
               INTERFACE_INCLUDE_DIRECTORIES "${MPG123_INCLUDE_DIR}")
  set(MPG123_libout123_FOUND TRUE)
endif()

if(MPG123_FOUND AND NOT TARGET MPG123::libsyn123)
  add_library(MPG123::libsyn123 UNKNOWN IMPORTED)
  set_target_properties(
    MPG123::libsyn123
    PROPERTIES IMPORTED_LOCATION "${MPG123_libsyn123_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${PC_SYN123_CFLAGS_OTHER}"
               INTERFACE_INCLUDE_DIRECTORIES "${MPG123_INCLUDE_DIR}")
  set(MPG123_libsyn123_FOUND TRUE)
endif()

mark_as_advanced(MPG123_libmpg123_LIBRARY MPG123_libout123_LIBRARY
                 MPG123_libsyn123_LIBRARY MPG123_INCLUDE_DIR)
