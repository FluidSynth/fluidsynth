#include "fluid_lfo.h"

DECLARE_FLUID_RVOICE_FUNCTION(fluid_lfo_set_incr)
{
    fluid_lfo_t *lfo = obj;
    fluid_real_t increment = param[0].real;

    /* Preserve the triangle direction when updating its frequency (#1310). */
    lfo->increment = lfo->increment < 0.0f ? -increment : increment;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_lfo_set_delay)
{
    fluid_lfo_t *lfo = obj;
    unsigned int delay = param[0].i;

    lfo->delay = delay;
}
