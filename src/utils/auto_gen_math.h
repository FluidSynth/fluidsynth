#ifndef __AUTO_GEN_MATH_H__
#define __AUTO_GEN_MATH_H__

/* Taylor series to approxmate sine. */
#define AUTO_GEN_SIN(_x)    ((_x) \
                             - (((_x)*(_x)*(_x))/6) \
                             + (((_x)*(_x)*(_x)*(_x)*(_x))/120) \
                             - (((_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x))/5040) \
                             + (((_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x))/362880.0) \
                             - (((_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x))/39916800.0) \
                             + (((_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x))/6227020800.0) \
                             - (((_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x))/1307674368000.0) \
                            )

#define FABS(_i)            (((_i) < 0) ? -(_i) : (_i))
#define FROUND(_i) ((int)((_i) + (((_i) >= 0.0f) ? 0.5f : -0.5f)))

#define FSIN_REDUCED(_i)            AUTO_GEN_SIN(_i)

// argument reduction needed:
// we cannot accept arguments of any arbitrary range
// x should be in range [-pi;pi], otherwise we become very inaccurate
// bring x back into that range
#define FSIN(_i)            FSIN_REDUCED((_i) - FROUND((_i) / (2*M_PI)) * (2*M_PI))
#define FCOS(_i)            FSIN((_i) + M_PI/2)

#endif /* __AUTO_GEN_MATH_H__ */
