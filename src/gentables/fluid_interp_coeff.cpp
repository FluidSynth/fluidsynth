
#include "rvoice/fluid_rvoice_dsp_tables.h"
#include "gentables/ConstExprArr.hpp"

#include "gcem.hpp"

struct InterpCubicFunctor
{
    //for (i = 0; i < FLUID_INTERP_MAX; i++)
    //{
    //    x = (double)i / (double)FLUID_INTERP_MAX;

    //    interp_coeff[i][0] = (x * (-0.5 + x * (1 - 0.5 * x)));
    //    interp_coeff[i][1] = (1.0 + x * x * (1.5 * x - 2.5));
    //    interp_coeff[i][2] = (x * (0.5 + x * (2.0 - 1.5 * x)));
    //    interp_coeff[i][3] = (0.5 * x * x * (x - 1.0));
    //}
    static constexpr fluid_real_t calc(int i)
    {
#define I_NUM (i / CUBIC_INTERP_ORDER)
#define I_REM (i % CUBIC_INTERP_ORDER)
#define x ((fluid_real_t)I_NUM / (fluid_real_t)FLUID_INTERP_MAX)

        return (I_REM == 0 ? (x * (-0.5 + x * (1 - 0.5 * x))) :
                I_REM == 1 ? (1.0 + x * x * (1.5 * x - 2.5)) :
                I_REM == 2 ? (x * (0.5 + x * (2.0 - 1.5 * x))) :
             /* I_REM == 3 */ (0.5 * x * x * (x - 1.0)));
    }
};

extern "C" const constexpr auto interp_coeff_cpp = ConstExprArr<InterpCubicFunctor, FLUID_INTERP_MAX * CUBIC_INTERP_ORDER>::value;

extern "C" const fluid_real_t *const interp_coeff = interp_coeff_cpp;
