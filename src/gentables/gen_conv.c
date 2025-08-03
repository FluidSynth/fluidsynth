
#include "utils/fluid_conv_tables.h"
#include "make_tables.h"


/* conversion tables */
static double fluid_ct2hz_tab[FLUID_CENTS_HZ_SIZE];
static double fluid_cb2amp_tab[FLUID_CB_AMP_SIZE];
static double fluid_concave_tab[FLUID_VEL_CB_SIZE];
static double fluid_convex_tab[FLUID_VEL_CB_SIZE];
static double fluid_pan_tab[FLUID_PAN_SIZE];

/*
 * void fluid_synth_init
 *
 * Does all the initialization for this module.
 */
static void fluid_conversion_config(void)
{
    int i;
    long double x;


    /* initialize the conversion tables (see fluid_mod.c
       fluid_mod_get_value cases 4 and 8) */

    /* concave unipolar positive transform curve */
    fluid_concave_tab[0] = 0.0;
    fluid_concave_tab[FLUID_VEL_CB_SIZE - 1] = 1.0;

    /* convex unipolar positive transform curve */
    fluid_convex_tab[0] = 0;
    fluid_convex_tab[FLUID_VEL_CB_SIZE - 1] = 1.0;

    /* There seems to be an error in the specs. The equations are
       implemented according to the pictures on SF2.01 page 73. */

    for(i = 1; i < FLUID_VEL_CB_SIZE - 1; i++)
    {
        x = (-200.0L * 2 / FLUID_PEAK_ATTENUATION) * logl(i / (FLUID_VEL_CB_SIZE - 1.0L)) / M_LN10;
        fluid_convex_tab[i] = (1.0L - x);
        fluid_concave_tab[(FLUID_VEL_CB_SIZE - 1) - i] =  x;
    }
}


void gen_conv_table(FILE *fp)
{
    /* Calculate the values */
    fluid_conversion_config();

    /* fluid_ct2hz_tab */
    EMIT_ARRAY(fp, fluid_ct2hz_tab);
    EMIT_ARRAY(fp, fluid_cb2amp_tab);
    EMIT_ARRAY(fp, fluid_concave_tab);
    EMIT_ARRAY(fp, fluid_convex_tab);
    EMIT_ARRAY(fp, fluid_pan_tab);
}

