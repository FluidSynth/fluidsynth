#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "fluid_phase.h"

int main (int argc, char *argv[])
{
    FILE *fp;
    int   i;
 
    // make sure we have enough arguments
    if (argc < 2)
        return 1;
  
    // open the output file
    fp = fopen(argv[1],"w");
    if (!fp)
        return 2;

    // Emit warning header
    fprintf(fp, "/* THIS FILE HAS BEEN AUTOMATICALLY GENERATED. DO NOT EDIT. */\n\n");

    // Create macro for filter order interpolator declaration
    fprintf(fp, "#define SINC_TABLE7(_i) \\\n    { \\\n");
    for (i = 0; i < FLUID_INTERP_SINC_ORDER; i++)
    {
        fprintf(fp, "        SINC_TABLE(_i, %u)%c \\\n",
                i, (i < (FLUID_INTERP_SINC_ORDER-1)) ? ',' : ' ');
    }
    fprintf(fp, "    }\n\n");
 
    /* Linear interpolation table (2 coefficients centered on 1st) */
    fprintf(fp, "static const fluid_real_t interp_coeff_linear[FLUID_INTERP_MAX][2] = {\n");
    for (i = 0; i < FLUID_INTERP_MAX; i++)
    {
        fprintf(fp, "    INTERP_COEFF_LINEAR(%u)%c\n",
                i, (i < (FLUID_INTERP_MAX-1)) ? ',' : ' ');
    }
    fprintf(fp, "};\n\n");

    /* 4th order (cubic) interpolation table (4 coefficients centered on 2nd) */
    fprintf(fp, "static const fluid_real_t interp_coeff[FLUID_INTERP_MAX][4] = {\n");
    for (i = 0; i < FLUID_INTERP_MAX; i++)
    {
        fprintf(fp, "    INTERP_COEFF(%u)%c\n",
                i, (i < (FLUID_INTERP_MAX-1)) ? ',' : ' ');
    }
    fprintf(fp, "};\n\n");

    /* 7th order interpolation (7 coefficients centered on 3rd) */
    fprintf(fp, "static fluid_real_t sinc_table7[FLUID_INTERP_MAX][FLUID_INTERP_SINC_ORDER] = {\n");
    for (i = 0; i < FLUID_INTERP_MAX; i++)
    {
        fprintf(fp, "    SINC_TABLE7(%u)%c\n",
                i, (i < (FLUID_INTERP_MAX-1)) ? ',' : ' ');
    }
    fprintf(fp, "};\n\n");

    fclose(fp);

    return 0;
}
