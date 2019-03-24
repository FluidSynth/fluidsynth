cmake_minimum_required(VERSION 3.6.0)

project ( fluidsynth-assetloader C )

set ( fluidsynth-assetloader_sources fluid_androidasset.c )

add_library ( fluidsynth-assetloader SHARED ${fluidsynth-assetloader_sources} )

target_compile_options ( fluidsynth-assetloader
                        PRIVATE -v
                        PRIVATE -Wall
                        PRIVATE "$<$<CONFIG:DEBUG>:-Werror>") # Only include -Werror when building debug config

include_directories ( ../../include )

set ( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L../../../dist/${ANDROID_ABI} -lfluidsynth" )

target_link_libraries ( fluidsynth-assetloader PRIVATE log android )
