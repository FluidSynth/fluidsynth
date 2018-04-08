
#pragma once

#include <stdio.h>
#include <stdlib.h>

#define TEST_ASSERT(COND) if (!(COND)) { fprintf(stderr, __FILE__ ":%d assertion ("#COND") failed\n", __LINE__); exit(-1); }

#define TEST_ASSERT_EQ(LHS, RHS) TEST_ASSERT((LHS) == (RHS))
#define TEST_ASSERT_NEQ(LHS, RHS) TEST_ASSERT((LHS) != (RHS))
#define TEST_SUCCESS(FLUID_FUNCT) TEST_ASSERT_NEQ((FLUID_FUNCT), FLUID_FAILED)
