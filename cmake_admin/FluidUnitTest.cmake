macro ( ADD_FLUID_TEST _test )
    ADD_EXECUTABLE(${_test} ${_test}.c)
    TARGET_LINK_LIBRARIES(${_test} libfluidsynth)

    # use the local include path to look for fluidsynth.h, as we cannot be sure fluidsynth is already installed
    target_include_directories(${_test}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

    ADD_TEST(NAME ${_test} COMMAND ${_test})
endmacro ( ADD_FLUID_TEST )
