
#ifndef _FLUID_RVOICE_DSP_H
#define _FLUID_RVOICE_DSP_H

#define FLUID_INTERP_MAX         (256)

#define SINC_INTERP_ORDER 7	/* 7th order constant */


void fluid_rvoice_dsp_config(void);
void fluid_rvoice_dsp_config_LOCAL(fluid_real_t interp_coeff_linear[FLUID_INTERP_MAX][2],
                                   fluid_real_t interp_coeff[FLUID_INTERP_MAX][4],
                                   fluid_real_t sinc_table7[FLUID_INTERP_MAX][SINC_INTERP_ORDER]);

#endif
