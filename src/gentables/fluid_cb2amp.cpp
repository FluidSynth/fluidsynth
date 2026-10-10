
#include "utils/fluid_conv_tables.h"
#include "gentables/ConstExprArr.hpp"

#define GCEM_E static_cast<double>(2.7182818284590452353602874713526624977572L)
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
       return gcem::pow(10.0, static_cast<fluid_real_t>(i) / -200.0);
    }
};

extern "C" const constexpr auto fluid_cb2amp_tab_cpp = ConstExprArr<Cb2AmpFunctor, FLUID_CB_AMP_SIZE>::value;

extern "C" const fluid_real_t *const fluid_cb2amp_tab = fluid_cb2amp_tab_cpp;
