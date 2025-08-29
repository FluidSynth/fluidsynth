
#include "utils/fluid_conv_tables.h"
#include "gentables/ConstExprArr.hpp"

#include "gcem.hpp"

struct ConvexFunctor
{
    /* There seems to be an error in the specs. The equations are
       implemented according to the pictures on SF2.01 page 73. */
    static constexpr fluid_real_t calc(int i)
    {
        return ((i == 0)
            ? 0
            : ((i == FLUID_VEL_CB_SIZE - 1)
                ? 1
                : (1.0 - ((-200.0 * 2 / FLUID_PEAK_ATTENUATION) * gcem::log(i / (FLUID_VEL_CB_SIZE - 1.0)) / static_cast<double>(GCEM_LOG_10)))
                ));
    }
};

extern "C" const constexpr auto fluid_convex_tab_cpp = ConstExprArr<ConvexFunctor, FLUID_VEL_CB_SIZE>::value;

extern "C" const fluid_real_t *const fluid_convex_tab = fluid_convex_tab_cpp;
