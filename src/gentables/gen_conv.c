#define FLUID_TABLE_GENERATOR   1

#include "fluid_conv.c"
#include "make_tables.h"

int gen_conv_table(FILE *fp)
{
    /* Calculate the values */
    fluid_conversion_config();

    /* fluid_ct2hz_tab */
    emit_array(fp, "fluid_ct2hz_tab",   fluid_ct2hz_tab,   FLUID_CENTS_HZ_SIZE);
    emit_array(fp, "fluid_cb2amp_tab",  fluid_cb2amp_tab,  FLUID_CB_AMP_SIZE);
    emit_array(fp, "fluid_concave_tab", fluid_concave_tab, FLUID_VEL_CB_SIZE);
    emit_array(fp, "fluid_convex_tab",  fluid_convex_tab,  FLUID_VEL_CB_SIZE);
    emit_array(fp, "fluid_pan_tab",     fluid_pan_tab,     FLUID_PAN_SIZE);

    return 0;
}
