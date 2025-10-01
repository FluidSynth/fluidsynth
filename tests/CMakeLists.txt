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

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(gcem-test)

    find_package(gcem REQUIRED CONFIG)
    set(GCEM_INCLUDE_DIR ${gcem_INCLUDE_DIRS})
endif ()

message(STATUS "Forcing tests build type to Release")
set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)

#

if(NOT MSVC)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    
    if(COMPILER_SUPPORTS_CXX11)
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0 -Wall -Wextra --coverage -fno-inline -fno-inline-small-functions -fno-default-inline")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0 -Wall -Wextra --coverage -fno-inline")
        endif()
    else()
        message(FATAL_ERROR "Unsupported compiler ${CMAKE_CXX_COMPILER} "
            "GCEM requires a C++11-compatible compiler.")
    endif()
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MP /bigobj")
    set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
endif()

#

file(GLOB GCEM_TESTS_SOURCES **.cpp **.c)
set( GCEM_TESTS ${GCEM_TESTS_SOURCES})

set(GCEM_TESTS_EX "")

foreach( file_ ${GCEM_TESTS} )
    get_filename_component(filename ${file_} NAME)
    string( REPLACE ".cpp" ".test" testname ${filename} )

    add_executable( ${testname} EXCLUDE_FROM_ALL ${filename} )
    target_link_libraries( ${testname} gcem )
    
    list(APPEND GCEM_TESTS_EX ${testname})
endforeach()

add_custom_target(gcem_tests DEPENDS ${GCEM_TESTS_EX})

#
