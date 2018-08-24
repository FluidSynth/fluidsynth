
#ifndef _FLUID_RVOICE_DSP_H
#define _FLUID_RVOICE_DSP_H

#define FLUID_INTERP_MAX         (256)

#define SINC_INTERP_ORDER 7	/* 7th order constant */

#define _X(_i)  ((double)(_i) / (double) FLUID_INTERP_MAX)
#define _X2(_i) (_X(_i) * _X(_i))

#define INTERP_COEFF_LINEAR_0(_i) ((fluid_real_t)(1.0 - _X(_i)))
#define INTERP_COEFF_LINEAR_1(_i) ((fluid_real_t)(      _X(_i)))

#define INTERP_COEFF_0(_i)  (fluid_real_t)(_X(_i) * (-0.5 + _X(_i) * (1 - 0.5 * _X(_i))))
#define INTERP_COEFF_1(_i)  (fluid_real_t)(1.0 + _X2(_i) * (1.5 * _X(_i) - 2.5))
#define INTERP_COEFF_2(_i)  (fluid_real_t)(_X(_i) * (0.5 + _X(_i) * (2.0 - 1.5 * _X(_i))))
#define INTERP_COEFF_3(_i)  (fluid_real_t)(0.5 * _X2(_i) * (_X(_i) - 1.0))

#define I_SHIFTED(_i, _i2)  ((double)(_i) - ((double)SINC_INTERP_ORDER / 2.0) \
                            + (double)(_i2) / (double)FLUID_INTERP_MAX)

#define SINC_TABLE(_i, _i2) ((FABS(I_SHIFTED(_i, _i2)) > 0.000001) ? \
                             (fluid_real_t)(FSIN(I_SHIFTED(_i, _i2) * M_PI) / (I_SHIFTED(_i, _i2) * M_PI) \
                             * 0.5 * (1.0 + FCOS(2.0 * M_PI * I_SHIFTED(_i, _i2) / SINC_INTERP_ORDER))) : \
                             (fluid_real_t)1.0)

#define INTERP_COEFF_LINEAR(_i) \
    { INTERP_COEFF_LINEAR_0(_i), INTERP_COEFF_LINEAR_1(_i) }

#define INTERP_COEFF(_i) \
    { INTERP_COEFF_0(_i), INTERP_COEFF_1(_i), INTERP_COEFF_2(_i), INTERP_COEFF_3(_i) }
    
#define INTERP_COEFF_SINC(_i) \
    { SINC_TABLE(0,((FLUID_INTERP_MAX-1)-(_i))),\
      SINC_TABLE(1,((FLUID_INTERP_MAX-1)-(_i))),\
      SINC_TABLE(2,((FLUID_INTERP_MAX-1)-(_i))),\
      SINC_TABLE(3,((FLUID_INTERP_MAX-1)-(_i))),\
      SINC_TABLE(4,((FLUID_INTERP_MAX-1)-(_i))),\
      SINC_TABLE(5,((FLUID_INTERP_MAX-1)-(_i))),\
      SINC_TABLE(6,((FLUID_INTERP_MAX-1)-(_i))) \
    }


void fluid_rvoice_dsp_config(void);
void fluid_rvoice_dsp_config_LOCAL(fluid_real_t interp_coeff_linear[FLUID_INTERP_MAX][2],
                                   fluid_real_t interp_coeff[FLUID_INTERP_MAX][4],
                                   fluid_real_t sinc_table7[FLUID_INTERP_MAX][SINC_INTERP_ORDER]);

#endif
