
#include "test.h"
#include "utils/fluid_conv_tables.h"
#include "rvoice/fluid_rvoice_dsp_tables.h"

#include "utils/fluid_sys.h"

#include "test_lookup_table_conv.h"
#include "test_lookup_table_rvoice.h"

extern const fluid_real_t *const fluid_cb2amp_tab;
extern const fluid_real_t *const fluid_ct2hz_tab;
extern const fluid_real_t *const fluid_concave_tab;
extern const fluid_real_t *const fluid_convex_tab;
extern const fluid_real_t *const fluid_pan_tab;

extern const fluid_real_t *const interp_coeff_linear;
extern const fluid_real_t *const interp_coeff;
extern const fluid_real_t *const sinc_table7;

int table_compare(const fluid_real_t *actual, const fluid_real_t *expected, int size)
{
    int i, res = FLUID_OK;
    for (i = 0; i < size; i++)
    {
        static const fluid_real_t EPS = 1e-4;
        fluid_real_t x = actual[i];
        fluid_real_t y = expected[i];
        fluid_real_t diff = (x - y);
        int ok = FLUID_FABS(diff) < EPS;
        if (!ok)
        {
            FLUID_LOG(FLUID_INFO, "Comparing %.9f and %.9f at index %d failed, diff is %.9f", x, y, i, diff);
            res = FLUID_FAILED;
        }
    }
    return res;
}

int main(void)
{
    TEST_SUCCESS(table_compare(fluid_ct2hz_tab, fluid_ct2hz_tab_expected, FLUID_CENTS_HZ_SIZE));
    TEST_SUCCESS(table_compare(fluid_cb2amp_tab, fluid_cb2amp_tab_expected, FLUID_CB_AMP_SIZE));
    TEST_SUCCESS(table_compare(fluid_concave_tab, fluid_concave_tab_expected, FLUID_VEL_CB_SIZE));
    TEST_SUCCESS(table_compare(fluid_convex_tab, fluid_convex_tab_expected, FLUID_VEL_CB_SIZE));
    TEST_SUCCESS(table_compare(fluid_pan_tab, fluid_pan_tab_expected, FLUID_PAN_SIZE));

    TEST_SUCCESS(table_compare(interp_coeff_linear, (const fluid_real_t*)&interp_coeff_linear_expected[0], FLUID_INTERP_MAX * LINEAR_INTERP_ORDER));
    TEST_SUCCESS(table_compare(interp_coeff, (const fluid_real_t*)&interp_coeff_expected[0], FLUID_INTERP_MAX * CUBIC_INTERP_ORDER));
    TEST_SUCCESS(table_compare(sinc_table7, (const fluid_real_t*)&sinc_table7_expected[0], FLUID_INTERP_MAX * SINC_INTERP_ORDER));

    return EXIT_SUCCESS;
}

