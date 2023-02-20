#[=======================================================================[.rst:
FindOgg
-------

Finds the Ogg library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Ogg::ogg``
  The Ogg library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Ogg_FOUND``
  True if the system has the Ogg library.

For compatibility with upstream, the following variables are also set:

``Ogg_INCLUDE_DIR``
``Ogg_INCLUDE_DIRS``
``Ogg_LIBRARY``
``Ogg_LIBRARIES``
``OGG_INCLUDE_DIR``
``OGG_INCLUDE_DIRS``
``OGG_LIBRARY``
``OGG_LIBRARIES``
``OGG_FOUND``

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_OGG QUIET ogg)

# Find the headers and library
find_path(
  Ogg_INCLUDE_DIR
  NAMES "ogg/ogg.h"
  PATHS "${PC_OGG_INCLUDEDIR}")

find_library(
  _ogg_library
  NAMES "ogg"
  PATHS "${PC_OGG_LIBDIR}")

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ogg REQUIRED_VARS "_ogg_library"
                                                    "Ogg_INCLUDE_DIR")

# Create the target
if(Ogg_FOUND AND NOT TARGET Ogg::ogg)
  add_library(Ogg::ogg UNKNOWN IMPORTED)
  set_target_properties(
    Ogg::ogg
    PROPERTIES IMPORTED_LOCATION "${_ogg_library}"
               INTERFACE_COMPILE_OPTIONS "${PC_OGG_CFLAGS_OTHER}"
               INTERFACE_INCLUDE_DIRECTORIES "${Ogg_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${PC_OGG_LIBRARIES}"
               INTERFACE_LINK_DIRECTORIES "${PC_OGG_LIBDIR}")

  # Set additional variables for compatibility with upstream config
  set(Ogg_INCLUDE_DIRS "${Ogg_INCLUDE_DIR}")
  set(Ogg_LIBRARY Ogg::ogg)
  set(Ogg_LIBRARIES Ogg::ogg)
  set(OGG_INCLUDE_DIR "${${Ogg_INCLUDE_DIR}}")
  set(OGG_INCLUDE_DIRS "${${Ogg_INCLUDE_DIR}}")
  set(OGG_LIBRARY Ogg::ogg)
  set(OGG_LIBRARIES Ogg::ogg)
  set(OGG_FOUND TRUE)
endif()

mark_as_advanced(_ogg_library)
