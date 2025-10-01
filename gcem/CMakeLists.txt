################################################################################
##
##   Copyright (C) 2016-2024 Keith O'Hara
##
##   This file is part of the GCE-Math C++ library.
##
##   Licensed under the Apache License, Version 2.0 (the "License");
##   you may not use this file except in compliance with the License.
##   You may obtain a copy of the License at
##
##       http://www.apache.org/licenses/LICENSE-2.0
##
##   Unless required by applicable law or agreed to in writing, software
##   distributed under the License is distributed on an "AS IS" BASIS,
##   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##   See the License for the specific language governing permissions and
##   limitations under the License.
##
################################################################################

cmake_minimum_required(VERSION 3.1)
project(gcem)

set(GCEM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

#

file(STRINGS "${GCEM_INCLUDE_DIR}/gcem_incl/gcem_options.hpp" gcem_version_defines
     REGEX "#define GCEM_VERSION_(MAJOR|MINOR|PATCH)")

foreach(ver ${gcem_version_defines})
    if(ver MATCHES "#define GCEM_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(GCEM_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()

set(${PROJECT_NAME}_VERSION
    ${GCEM_VERSION_MAJOR}.${GCEM_VERSION_MINOR}.${GCEM_VERSION_PATCH})
message(STATUS "GCE-Math version ${${PROJECT_NAME}_VERSION}")

#

if(NOT MSVC)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    
    if(COMPILER_SUPPORTS_CXX11)
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has C++11 support.")
    else()
        message(FATAL_ERROR "Unsupported compiler ${CMAKE_CXX_COMPILER} "
            "GCEM requires a C++11-compatible compiler.")
    endif()
endif()

# options. enable using cmake3 -DGCEM_BUILD_TESTS=ON

OPTION(GCEM_BUILD_TESTS "gcem test suite" OFF)

# install

add_library(gcem INTERFACE)
target_include_directories(gcem INTERFACE $<BUILD_INTERFACE:${GCEM_INCLUDE_DIR}>
                                          $<INSTALL_INTERFACE:include>)

if(GCEM_BUILD_TESTS)
    add_subdirectory(tests)
endif()

#

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(
    TARGETS gcem
    EXPORT ${PROJECT_NAME}-targets
)

export(EXPORT ${PROJECT_NAME}-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

install(DIRECTORY ${GCEM_INCLUDE_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#

set(GCEM_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for gcemConfig.cmake")

configure_package_config_file(cmake_files/${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${GCEM_CMAKECONFIG_INSTALL_DIR})

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${GCEM_CMAKECONFIG_INSTALL_DIR})

install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${GCEM_CMAKECONFIG_INSTALL_DIR})
