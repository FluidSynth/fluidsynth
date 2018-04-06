
project(fluidsynth-test)

ENABLE_TESTING()
include ( FluidUnitTest )


ADD_FLUID_TEST(test_sample_cache)


# make test only runs tests, but doesnt build them
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}  DEPENDS test_sample_cache)
