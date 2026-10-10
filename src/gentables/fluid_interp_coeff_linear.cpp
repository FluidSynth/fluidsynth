
#include "rvoice/fluid_rvoice_dsp_tables.h"
#include "gentables/ConstExprArr.hpp"

#include "gcem.hpp"

struct InterpLinearFunctor
{
    //for (i = 0; i < FLUID_INTERP_MAX; i++)
    //{
    //    x = (double)i / (double)FLUID_INTERP_MAX;
    //    interp_coeff_linear[i][0] = (1.0 - x);
    //    interp_coeff_linear[i][1] = x;
    //}
    static constexpr fluid_real_t calc(int i)
    {
#define I_NUM (i / LINEAR_INTERP_ORDER)
#define I_REM (i % LINEAR_INTERP_ORDER)
#define x ((fluid_real_t)I_NUM / (fluid_real_t)FLUID_INTERP_MAX)

        return (I_REM == 0 ? (1.0 - x) : x);
    }
};

extern "C" const constexpr auto interp_coeff_linear_cpp = ConstExprArr<InterpLinearFunctor, FLUID_INTERP_MAX * LINEAR_INTERP_ORDER>::value;

extern "C" const fluid_real_t *const interp_coeff_linear = interp_coeff_linear_cpp;
