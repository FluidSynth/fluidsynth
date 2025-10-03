#[=======================================================================[.rst:
FindGCEM
-------

Finds the GCEM library or downloads it from Github.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``GCEM::GCEM``
  The GCEM library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``GCEM_FOUND``
  True if the system has the GCEM library.

#]=======================================================================]

# Find the headers and library
find_path(GCEM_INCLUDE_DIR NAMES "gcem.hpp" PATHS "${CMAKE_SOURCE_DIR}/gcem/include")

include(FindPackageHandleStandardArgs)

set(GCEM_REVISION "012ae73c6d0a2cb09ffe86475f5c6fba3926e200")
set(GCEM_HASH "28159274c54e9640354852e172d10d88eb159f4e7f2fea42edbcd20105ed3526")
set(GCEM_ZIP_URL "https://github.com/kthohr/gcem/archive/${GCEM_REVISION}.zip")
if(NOT GCEM_INCLUDE_DIR)
  if(${CMAKE_VERSION} VERSION_LESS "3.18")
    message(FATAL_ERROR
      "The 'gcem' directory seems to be empty or incomplete and your CMake version (${CMAKE_VERSION}) is less than 3.18.\n"
      "Automatic download is not supported. Please download GCEM manually from:\n"
      "${GCEM_ZIP_URL}\n"
      "and extract its contents into the 'gcem' directory in the repository root."
    )
  else()
    message(NOTICE "The 'gcem' submodule directory seems to be empty or incomplete. Attempting to download gcem from Github.")
    set(GCEM_ZIP_FILE "${CMAKE_BINARY_DIR}/gcem.zip")
    set(GCEM_EXTRACT_DIR "${CMAKE_BINARY_DIR}/gcem-extracted")

    file(DOWNLOAD "${GCEM_ZIP_URL}" "${GCEM_ZIP_FILE}"
          SHOW_PROGRESS
          INACTIVITY_TIMEOUT 10
          STATUS GCEM_DOWNLOAD_STATUS
          LOG GCEM_DOWNLOAD_LOG
          EXPECTED_HASH SHA256=${GCEM_HASH}
          )
    list(GET GCEM_DOWNLOAD_STATUS 0 GCEM_DOWNLOAD_CODE)
    if(NOT GCEM_DOWNLOAD_CODE EQUAL 0)
      file(REMOVE ${GCEM_ZIP_FILE})
      message(FATAL_ERROR
        "Failed to download GCEM from ${GCEM_ZIP_URL} (status code: ${GCEM_DOWNLOAD_CODE}).\n"
        "Either checkout git submodules (run: git submodule update --init) or download manually from:\n"
        "${GCEM_ZIP_URL}\n"
        "and extract its contents into the 'gcem' directory in the repository root.\n"
        "Log output (if any):\n${GCEM_DOWNLOAD_LOG}\n"
      )
    endif()

    file(ARCHIVE_EXTRACT
      INPUT "${GCEM_ZIP_FILE}"
      DESTINATION "${GCEM_EXTRACT_DIR}"
    )

    set(GCEM_INCLUDE_DIR "${GCEM_EXTRACT_DIR}/gcem-${GCEM_REVISION}/include")

    message(STATUS "gcem source downloaded and extracted to ${GCEM_EXTRACT_DIR}")
  endif()
endif()

# Forward the result to CMake
find_package_handle_standard_args(GCEM REQUIRED_VARS "GCEM_INCLUDE_DIR")

mark_as_advanced(GCEM_INCLUDE_DIR)
