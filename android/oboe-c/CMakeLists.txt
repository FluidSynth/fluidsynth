cmake_minimum_required(VERSION 3.4.1)

project ( oboe-c C CXX )

set ( oboe-c_sources include/oboe-c.h oboe-c.cxx )

add_library ( oboe-c SHARED ${oboe-c_sources} )

# Specify directories which the compiler should look for headers
target_include_directories( oboe-c
                            PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../external/oboe/include
                            PUBLIC include )

target_compile_options( oboe-c PRIVATE -std=c++11
                               PRIVATE -v
                               PRIVATE -Wall
                               PRIVATE "$<$<CONFIG:DEBUG>:-Werror>") # Only include -Werror when building debug config

target_link_libraries( oboe-c PRIVATE log OpenSLES
                       ${CMAKE_CURRENT_SOURCE_DIR}/../external/oboe/build/${ANDROID_ABI}/liboboe.a )
