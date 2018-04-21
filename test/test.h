
#pragma once

#include <stdio.h>
#include <stdlib.h>

#define TEST_ASSERT(COND) do { if (!(COND)) { fprintf(stderr, __FILE__ ":%d assertion (%s) failed\n", __LINE__, #COND); exit(-1); } } while (0)

/* macro to test whether a fluidsynth function succeeded or not */
#define TEST_SUCCESS(FLUID_FUNCT) TEST_ASSERT((FLUID_FUNCT) != FLUID_FAILED)
