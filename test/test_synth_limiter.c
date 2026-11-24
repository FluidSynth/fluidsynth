#include "test.h"
#include "fluidsynth.h"
#include "fluid_synth.h"

#include <math.h>

/* Verify enabling the Signalsmith limiter initializes and renders cleanly, including under TRAP_ON_FPE. */
int main(void)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    float left[FLUID_BUFSIZE];
    float right[FLUID_BUFSIZE];
    int i;

    TEST_ASSERT(settings != NULL);
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.limiter.active", 1));

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    TEST_SUCCESS(fluid_synth_write_float(synth, FLUID_BUFSIZE, left, 0, 1, right, 0, 1));
    for(i = 0; i < FLUID_BUFSIZE; i++)
    {
        TEST_ASSERT(isfinite(left[i]) && isfinite(right[i]));
    }

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
