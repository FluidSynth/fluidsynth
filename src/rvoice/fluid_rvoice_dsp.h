#ifndef __GNUC__
#error This feature is supported only with GNU-C compiler
#endif

#define _X(_i)  ((double)(_i) / (double) FLUID_INTERP_MAX)
#define _X2(_i) (_X(_i) * _X(_i))

#define INTERP_COEFF_LINEAR_0(_i) ((fluid_real_t)(1.0 - _X(_i)))
#define INTERP_COEFF_LINEAR_1(_i) ((fluid_real_t)(      _X(_i)))

#define INTERP_COEFF_0(_i)  (fluid_real_t)(_X(_i) * (-0.5 + _X(_i) * (1 - 0.5 * _X(_i))))
#define INTERP_COEFF_1(_i)  (fluid_real_t)(1.0 + _X2(_i) * (1.5 * _X(_i) - 2.5))
#define INTERP_COEFF_2(_i)  (fluid_real_t)(_X(_i) * (0.5 + _X(_i) * (2.0 - 1.5 * _X(_i))))
#define INTERP_COEFF_3(_i)  (fluid_real_t)(0.5 * _X2(_i) * (_X(_i) - 1.0))

#define I_SHIFTED(_i, _i2)  ((double)(_i) - ((double)FLUID_INTERP_SINC_ORDER / 2.0) \
                            + (double)(_i2) / (double)FLUID_INTERP_MAX)

#if defined(WITH_FLOAT)
#define FSIN(_i)            __builtin_sinf(_i)
#define FCOS(_i)            __builtin_cosf(_i)
#else
#define FSIN(_i)            __builtin_sin(_i)
#define FCOS(_i)            __builtin_cos(_i)
#endif

#define SINC_TABLE(_i, _i2) ((__builtin_fabs(I_SHIFTED(_i, _i2)) > 0.000001) ? \
                             (fluid_real_t)(FSIN(I_SHIFTED(_i, _i2) * M_PI) / (2. * M_PI * I_SHIFTED(_i, _i2)) \
                             * (1.0 + FCOS(2.0 * M_PI * I_SHIFTED(_i, _i2) / FLUID_INTERP_SINC_ORDER))) : \
                             (fluid_real_t)1.0)

#define INTERP_COEFF_LINEAR(_i) \
    { INTERP_COEFF_LINEAR_0(_i), INTERP_COEFF_LINEAR_1(_i) }

#define INTERP_COEFF(_i) \
    { INTERP_COEFF_0(_i), INTERP_COEFF_1(_i), INTERP_COEFF_2(_i), INTERP_COEFF_3(_i) }
