
#include "utils/fluid_conv_tables.h"
#include "gentables/ConstExprArr.hpp"

#include "gcem.hpp"

struct Ct2HzFunctor
{
    static constexpr fluid_real_t calc(int i)
    {
        // 6,875 is just a factor that we already multiply into the lookup table to save
        // that multiplication in fluid_ct2hz_real()
        // 6.875 Hz because 440Hz / 2^6
        return 6.875 * gcem::pow(2.0, static_cast<fluid_real_t>(i) / 1200.0);
    }
};

extern "C" const constexpr auto fluid_ct2hz_tab_cpp = ConstExprArr<Ct2HzFunctor, FLUID_CENTS_HZ_SIZE>::value;

extern "C" const fluid_real_t *const fluid_ct2hz_tab = fluid_ct2hz_tab_cpp;
