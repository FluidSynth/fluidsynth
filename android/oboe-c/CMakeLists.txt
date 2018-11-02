cmake_minimum_required(VERSION 3.4.1)

set (oboe-c_sources include/oboe-c.h oboe-c.cxx)

add_library(oboe-c STATIC ${oboe-c_sources} ../external/oboe/build/${ANDROID_ABI}/liboboe.a)

# Specify directories which the compiler should look for headers
target_include_directories(oboe-c
                           PUBLIC include ../external/oboe/include)

target_compile_options(oboe-c PRIVATE -std=c++11
                              PRIVATE -v
                              PRIVATE -Wall
                              PRIVATE "$<$<CONFIG:DEBUG>:-Werror>") # Only include -Werror when building debug config

target_link_libraries(oboe-c PRIVATE log OpenSLES -v)
