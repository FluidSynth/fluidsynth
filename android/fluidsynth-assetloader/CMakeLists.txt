cmake_minimum_required(VERSION 3.6.0)

project ( fluidsynth-assetloader C )

set ( fluidsynth-assetloader_sources fluid_androidasset.c )

add_library ( fluidsynth-assetloader SHARED ${fluidsynth-assetloader_sources} )

target_compile_options ( fluidsynth-assetloader
                        PRIVATE -v
                        PRIVATE -Wall
                        PRIVATE "$<$<CONFIG:DEBUG>:-Werror>") # Only include -Werror when building debug config

find_package ( PkgConfig REQUIRED )

pkg_check_modules ( PKG_FLUIDSYNTH REQUIRED fluidsynth )

include_directories ( ${PKG_FLUIDSYNTH_INCLUDE_DIRS} )

set ( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${PKG_FLUIDSYNTH_LDFLAGS}" )

target_link_libraries ( fluidsynth-assetloader PRIVATE log android )
