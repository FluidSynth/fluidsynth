macro ( ADD_FLUID_TEST _test )
    ADD_EXECUTABLE(${_test} ${_test}.c)
    TARGET_LINK_LIBRARIES(${_test} libfluidsynth)

    # use the local include path to look for fluidsynth.h, as we cannot be sure fluidsynth is already installed
    target_include_directories(${_test}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include> # include auto generated headers
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include> # include "normal" public (sub-)headers
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src> # include private headers
    $<TARGET_PROPERTY:libfluidsynth,INCLUDE_DIRECTORIES> # include all other header search paths needed by libfluidsynth (esp. glib)
    )

    # add the test to ctest
    ADD_TEST(NAME ${_test} COMMAND ${_test})
    
    # append the current unit test to check-target as dependency
    add_dependencies(check ${_test})

endmacro ( ADD_FLUID_TEST )
