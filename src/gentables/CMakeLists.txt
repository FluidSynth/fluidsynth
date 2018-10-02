cmake_minimum_required(VERSION 3.1)

# remove $CC from the current environment and by that force cmake to look for a (working) C compiler,
# which hopefully will be the host compiler
unset(ENV{CC})

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

if ( WIN32 )
    add_definitions ( -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS )
else ( WIN32 )
    target_link_libraries (make_tables "m")
endif ()
