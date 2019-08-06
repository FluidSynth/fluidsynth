
#pragma once

#include <stdio.h>
#include <stdlib.h>

#if defined(WIN32)
/* MSVC++ runtime opens a dialog when abort() is called, saying that abort() has been called...
 * This however causes the unit tests executed by the CI build to be stuck forever.
 * Thus suppress the dialog on windows.
 */
#define TEST_ABORT _set_abort_behavior(0, _WRITE_ABORT_MSG); abort()
#else
#define TEST_ABORT abort()
#endif

#define TEST_ASSERT(COND) do { if (!(COND)) { fprintf(stderr, __FILE__ ":%d assertion (%s) failed\n", __LINE__, #COND); TEST_ABORT; } } while (0)

/* macro to test whether a fluidsynth function succeeded or not */
#define TEST_SUCCESS(FLUID_FUNCT) TEST_ASSERT((FLUID_FUNCT) != FLUID_FAILED)
