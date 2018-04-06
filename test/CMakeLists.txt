
project(fluidsynth-test)


ADD_EXECUTABLE(test_sfont_cache test_sfont_cache.cpp)
TARGET_LINK_LIBRARIES(test_sfont_cache libfluidsynth)
ADD_TEST(NAME test_sfont_cache COMMAND test_sfont_cache)

# make test only runs tests, but doesnt build them
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}  DEPENDS test_sfont_cache)
