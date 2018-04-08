
project(fluidsynth-test)

ENABLE_TESTING()
include ( FluidUnitTest )

## add unit tests here ##
ADD_FLUID_TEST(test_sample_cache)
ADD_FLUID_TEST(test_sfont_loading)


add_custom_target(check
COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG>  --output-on-failure
DEPENDS
test_sample_cache
test_sfont_loading
)
