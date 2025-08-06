
#include "utils/fluid_conv_tables.h"
#include "gentables/ConstExprArr.hpp"

#include "gcem.hpp"

struct PanFunctor
{
    static constexpr fluid_real_t calc(int i)
    {
        /* initialize the pan conversion table */
        return static_cast<fluid_real_t>(gcem::sin(i * (GCEM_HALF_PI / (FLUID_PAN_SIZE - 1.0L))));
    }
};

extern "C" const constexpr auto fluid_pan_tab_cpp = ConstExprArr<PanFunctor, FLUID_PAN_SIZE>::value;

extern "C" const fluid_real_t *const fluid_pan_tab = fluid_pan_tab_cpp;
