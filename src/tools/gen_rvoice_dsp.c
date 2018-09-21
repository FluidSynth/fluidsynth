#define FLUID_TABLE_GENERATOR   1

#include "fluid_rvoice_dsp.c"

#include "make_tables.h"

static fluid_real_t cb_interp_coeff_linear(int y, int x) { return interp_coeff_linear[y][x]; }
static fluid_real_t cb_interp_coeff       (int y, int x) { return interp_coeff[y][x]; }
static fluid_real_t cb_sinc_table7        (int y, int x) { return sinc_table7[y][x]; }

int gen_rvoice_table_dsp (FILE *fp)
{
    /* Calculate the values */
    fluid_rvoice_dsp_config();

    /* Emit the matrices */
    emit_matrix(fp, "interp_coeff_linear", cb_interp_coeff_linear, FLUID_INTERP_MAX, 2);
    emit_matrix(fp, "interp_coeff",        cb_interp_coeff,        FLUID_INTERP_MAX, 4);
    emit_matrix(fp, "sinc_table7",         cb_sinc_table7,         FLUID_INTERP_MAX, 7);

    return 0;
}
