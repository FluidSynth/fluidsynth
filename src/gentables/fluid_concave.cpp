
#include "utils/fluid_conv_tables.h"
#include "gentables/ConstExprArr.hpp"

#include "gcem.hpp"

struct ConcaveFunctor
{
    /* There seems to be an error in the specs. The equations are
       implemented according to the pictures on SF2.01 page 73. */
    static constexpr fluid_real_t calc(int i)
    {
        constexpr fluid_real_t x = (-200.0L * 2 / FLUID_PEAK_ATTENUATION) * gcem::log(i / (FLUID_VEL_CB_SIZE - 1.0L)) / M_LN10;
        return ((i == 0)
            ? 0
            : ((i == FLUID_VEL_CB_SIZE - 1)
                ? 1
                : (x)
                ));
    }
};

extern "C" const constexpr auto fluid_concave_tab_cpp = ConstExprArr<ConcaveFunctor, FLUID_VEL_CB_SIZE>::value;

extern "C" const fluid_real_t *const fluid_concave_tab = fluid_concave_tab_cpp;
