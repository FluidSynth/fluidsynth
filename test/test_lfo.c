#include "test.h"
#include "fluid_lfo.h"

/* Check frequency updates in both directions and reset after descent. */
int main(void)
{
    const struct
    {
        int steps;
        fluid_real_t increment, expected_increment;
    } cases[] =
    {
        {2, 0.25, 0.25},
        {5, 0.25, -0.25},
        {2, 0.125, 0.125},
        {5, 0.125, -0.125}
    };
    fluid_lfo_t lfo = {0};
    fluid_rvoice_param_t params[MAX_EVENT_PARAMS] = {{0}};
    int c, step;

    /* Updating the frequency must preserve the position and direction. */
    for(c = 0; c < (int)FLUID_N_ELEMENTS(cases); c++)
    {
        fluid_real_t value;
        fluid_lfo_reset(&lfo);
        params[0].real = 0.25;
        fluid_lfo_set_incr(&lfo, params);
        for(step = 0; step < cases[c].steps; step++)
        {
            fluid_lfo_calc(&lfo, 0);
        }
        value = fluid_lfo_get_val(&lfo);
        params[0].real = cases[c].increment;
        fluid_lfo_set_incr(&lfo, params);
        TEST_ASSERT(fluid_lfo_get_val(&lfo) == value);
        TEST_ASSERT(lfo.increment == cases[c].expected_increment);
        fluid_lfo_calc(&lfo, 0);
        TEST_ASSERT(fluid_lfo_get_val(&lfo) == value + cases[c].expected_increment);
    }

    /* A new note starts ascending even if the previous LFO was descending. */
    TEST_ASSERT(lfo.increment < 0);
    fluid_lfo_reset(&lfo);
    TEST_ASSERT(fluid_lfo_get_val(&lfo) == 0);
    params[0].real = 0.25;
    fluid_lfo_set_incr(&lfo, params);
    fluid_lfo_calc(&lfo, 0);
    TEST_ASSERT(fluid_lfo_get_val(&lfo) == 0.25);

    return EXIT_SUCCESS;
}
