cmake_minimum_required(VERSION 3.1)

project (gentables C)

set ( CMAKE_BUILD_TYPE Debug )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

# Add the executable that generates the table
add_executable( make_tables
                make_tables.c
                gen_conv.c
                gen_rvoice_dsp.c)

if ( NOT WIN32 )
    target_link_libraries (make_tables "m")
endif ()
