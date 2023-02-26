#[=======================================================================[.rst:
Findlibffi
-------

Finds the libffi library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``libffi``
  The ffi library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``libffi_FOUND``
  True if the system has the Ffi library.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_LIBFFI QUIET libffi)

# Find the headers and library
find_path(
  libffi_INCLUDE_DIR
  NAMES "ffi.h"
  PATHS "${PC_LIBFFI_INCLUDE_DIR}")

find_library(
  libffi_LIBRARY
  NAMES "ffi" "libffi"
  PATHS "${PC_FFI_LIBDIR}")

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libffi REQUIRED_VARS "libffi_LIBRARY"
                                                       "libffi_INCLUDE_DIR")

if(libffi_FOUND AND NOT TARGET libffi)
  add_library(libffi UNKNOWN IMPORTED)
  set_target_properties(
    libffi
    PROPERTIES IMPORTED_LOCATION "${libffi_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${PC_FFI_CFLAGS_OTHER}"
               INTERFACE_INCLUDE_DIRECTORIES "${libffi_INCLUDE_DIR}")
endif()

mark_as_advanced(libffi_LIBRARY libffi_INCLUDE_DIR)
