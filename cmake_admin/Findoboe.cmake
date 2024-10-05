#[=======================================================================[.rst:
FindOboe
-------

Finds the Oboe library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``oboe::oboe``
  The Oboe library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Oboe_FOUND``
  True if the system has the Oboe library.

For compatibility with upstream, the following variables are also set:

``Oboe_INCLUDE_DIR``
``Oboe_INCLUDE_DIRS``
``Oboe_LIBRARY``
``Oboe_LIBRARIES``
``OBOE_INCLUDE_DIR``
``OBOE_INCLUDE_DIRS``
``OBOE_LIBRARY``
``OBOE_LIBRARIES``
``OBOE_FOUND``

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_OBOE QUIET oboe-1.0)

# Find the headers and library
find_path(
  Oboe_INCLUDE_DIR
  NAMES "oboe/Oboe.h"
  HINTS "${PC_OBOE_INCLUDEDIR}")

find_library(
  _oboe_library
  NAMES "oboe"
  HINTS "${PC_OBOE_LIBDIR}")

# Extract additional flags if pkg-config is available
if(PC_OBOE_FOUND)
  get_target_properties_from_pkg_config("${_oboe_library}" "PC_OBOE" "_oboe")
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Oboe REQUIRED_VARS "_oboe_library"
                                                    "Oboe_INCLUDE_DIR")

# Create the target
if(Oboe_FOUND AND NOT TARGET oboe::oboe)
  add_library(oboe::oboe UNKNOWN IMPORTED)
  set_target_properties(
    oboe::oboe
    PROPERTIES IMPORTED_LOCATION "${_oboe_library}"
               INTERFACE_COMPILE_OPTIONS "${_oboe_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${Oboe_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_oboe_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_oboe_link_directories}")

  # Set additional variables for compatibility with upstream config
  set(Oboe_INCLUDE_DIRS "${Oboe_INCLUDE_DIR}")
  set(Oboe_LIBRARY oboe::oboe)
  set(Oboe_LIBRARIES oboe::oboe)
  set(OBOE_INCLUDE_DIR "${${Oboe_INCLUDE_DIR}}")
  set(OBOE_INCLUDE_DIRS "${${Oboe_INCLUDE_DIR}}")
  set(OBOE_LIBRARY oboe::oboe)
  set(OBOE_LIBRARIES oboe::oboe)
  set(OBOE_FOUND TRUE)
endif()

mark_as_advanced(_oboe_library)
