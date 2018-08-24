
ENABLE_TESTING()
include ( FluidUnitTest )

# first define the test target, used by the macros below
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG>  --output-on-failure)


## add unit tests here ##
ADD_FLUID_TEST(test_sample_cache)
ADD_FLUID_TEST(test_sfont_loading)
ADD_FLUID_TEST(test_sample_rate_change)
ADD_FLUID_TEST(test_preset_sample_loading)
ADD_FLUID_TEST(test_pointer_alignment)
ADD_FLUID_TEST(test_seqbind_unregister)
ADD_FLUID_TEST(test_synth_chorus_reverb)
ADD_FLUID_TEST(test_snprintf)

# MSVC - a commercial product - is unable to compile files that exploit the preprocessor for calculating and 
# filling up lookup tables.
# fatal error C1060: compiler is out of heap space
# Online docs recommend to adjust the /Zm flag.
# Tested on a x86_64 machine with 16 GB RAM, setting this flag to 1 or 2000 doesnt make a difference.
# Higher numbers are not accepted...
if ( NOT MSVC )
    ADD_FLUID_TEST(test_constexpr_lookuptab)
endif ( NOT MSVC )

if ( LIBSNDFILE_HASVORBIS )
    ADD_FLUID_TEST(test_sf3_sfont_loading)
endif ( LIBSNDFILE_HASVORBIS )
