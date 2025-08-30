# CMake macro for building individual Android test executables
# This is used instead of the Gradle-based approach to build standalone test binaries
# that can be executed directly in the Android emulator

macro ( ADD_FLUID_ANDROID_TEST _test )
    if(ANDROID)
        add_executable( ${_test}_android ${_test}.c )

        # only build this unit test when explicitly requested by "make check-android"
        set_target_properties(${_test}_android PROPERTIES EXCLUDE_FROM_ALL TRUE)

        # Set output directory for easier collection
        set_target_properties(${_test}_android PROPERTIES 
            RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/test/android/$<0:>)

        # import necessary compile flags and dependency libraries
        if ( FLUID_CPPFLAGS )
            set_target_properties ( ${_test}_android PROPERTIES COMPILE_FLAGS ${FLUID_CPPFLAGS} )
        endif ( FLUID_CPPFLAGS )
        
        # Link against the object library and its dependencies
        target_link_libraries( ${_test}_android libfluidsynth-OBJ )

        # use the local include path to look for fluidsynth.h, as we cannot be sure fluidsynth is already installed
        target_include_directories(${_test}_android
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include> # include auto generated headers
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> # include "normal" public (sub-)headers
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src> # include private headers
        $<TARGET_PROPERTY:libfluidsynth-OBJ,INCLUDE_DIRECTORIES> # include all other header search paths needed by libfluidsynth (esp. glib)
        )

        # For Android, we might need additional system libraries
        if(ANDROID)
            target_link_libraries(${_test}_android log)
        endif()

        # append the current unit test to check-android target as dependency
        add_dependencies(check-android ${_test}_android)
    endif(ANDROID)
endmacro ( ADD_FLUID_ANDROID_TEST )