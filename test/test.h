
#pragma once

#include <stdio.h>
#include <stdlib.h>

#define TEST_ASSERT(COND) if (!(COND)) { fprintf(stderr, __FILE__ ":%d assertion ("#COND") failed\n", __LINE__); exit(-1); }

/* macro to test whether a fluidsynth function succeeded or not */
#define TEST_SUCCESS(FLUID_FUNCT) TEST_ASSERT((FLUID_FUNCT) != FLUID_FAILED)
