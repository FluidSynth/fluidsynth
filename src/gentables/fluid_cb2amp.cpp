
#include "utils/fluid_conv_tables.h"
#include "gentables/ConstExprArr.hpp"

#include "gcem.hpp"

struct Cb2AmpFunctor
{
    static constexpr fluid_real_t calc(int i)
    {
        /* centibels to amplitude conversion
         * Note: SF2.01 section 8.1.3: Initial attenuation range is
         * between 0 and 144 dB. Therefore a negative attenuation is
         * not allowed.
         */
       return gcem::pow(10.0L, static_cast<fluid_real_t>(i) / -200.0L);
    }
};

extern "C" const constexpr auto fluid_cb2amp_tab_cpp = ConstExprArr<Cb2AmpFunctor, FLUID_CB_AMP_SIZE>::value;

extern "C" const fluid_real_t *const fluid_cb2amp_tab = fluid_cb2amp_tab_cpp;
