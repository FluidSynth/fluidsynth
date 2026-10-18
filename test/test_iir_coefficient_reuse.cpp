#include "test.h"
#include "fluid_iir_filter.h"
#include <algorithm>
#include <cfloat>
#include <cmath>

/* Compare smoothed coefficients with a fresh calculation at the final state. */
static void check_smoothing(fluid_iir_sincos_t *table, fluid_real_t cutoff_step,
                            fluid_real_t q_step)
{
    fluid_iir_filter_t filter = {};
    filter.type = FLUID_IIR_LOWPASS;
    filter.sincos_table = table;
    filter.last_fres = 6000.25;
    filter.last_q = 0.707;
    filter.fres_incr = cutoff_step;
    filter.fres_incr_count = cutoff_step != 0 ? FLUID_BUFSIZE : 0;
    filter.q_incr = q_step;
    filter.q_incr_count = q_step != 0 ? FLUID_BUFSIZE : 0;
    fluid_iir_filter_t bypass = {};
    fluid_real_t samples[FLUID_BUFSIZE] = {};
    fluid_iir_filter_apply(&filter, &bypass, samples, FLUID_BUFSIZE);

    fluid_iir_filter_t expected = filter;
    expected.fres = filter.last_fres;
    expected.filter_startup = 1;
    fluid_iir_filter_calc(&expected, FRES_MAX, 0);
    const IIR_COEFF_T actual[] = {filter.a1, filter.a2, filter.b02, filter.b1};
    const IIR_COEFF_T reference[] = {expected.a1, expected.a2, expected.b02, expected.b1};
    for(int i = 0; i < 4; ++i)
    {
        /* The IIR Release build permits floating-point reassociation. */
        TEST_ASSERT(std::abs(actual[i] - reference[i])
                    <= 8 * FLT_EPSILON * std::max(1.0f, std::abs(reference[i])));
    }
    TEST_ASSERT(filter.fres_incr_count == 0 && filter.q_incr_count == 0);
}

int main()
{
    fluid_iir_sincos_t table[SINCOS_TAB_SIZE];
    fluid_iir_filter_init_table(table, 44100);
    check_smoothing(table, 0.005, 0); // First step must initialize coefficients.
    check_smoothing(table, 0.04, 0); // Cutoff crosses table entries.
    check_smoothing(table, 2, 0); // Large steps bypass coefficient reuse.
    check_smoothing(table, 0, 0.01); // Q changes without a cutoff change.
    check_smoothing(table, 0.04, 0.01);
    return 0;
}
