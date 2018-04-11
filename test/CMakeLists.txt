
project(fluidsynth-test)

ENABLE_TESTING()
include ( FluidUnitTest )

## add unit tests here ##
ADD_FLUID_TEST(test_sample_cache)
ADD_FLUID_TEST(test_sfont_loading)
ADD_FLUID_TEST(test_defsfont_preset_iteration)
ADD_FLUID_TEST(test_sample_rate_change)

if ( LIBSNDFILE_HASVORBIS )
    ADD_FLUID_TEST(test_sf3_sfont_loading)
    ADD_FLUID_TEST(test_sf3_defsfont_preset_iteration)
endif ( LIBSNDFILE_HASVORBIS )


add_custom_target(check
COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG>  --output-on-failure
DEPENDS
test_sample_cache
test_sfont_loading
test_defsfont_preset_iteration
test_sample_rate_change

test_sf3_sfont_loading
test_sf3_defsfont_preset_iteration
)
