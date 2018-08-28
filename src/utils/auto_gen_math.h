#ifndef __AUTO_GEN_MATH_H__
#define __AUTO_GEN_MATH_H__

/* Taylor series to approxmate sine, i.e.:
(_x) \
- (((_x)*(_x)*(_x))/6) \
+ (((_x)*(_x)*(_x)*(_x)*(_x))/120) \
- (((_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x))/5040) \
+ (((_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x))/362880.0) \
- (((_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x))/39916800.0) \
+ (((_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x))/6227020800.0) \
- (((_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x))/1307674368000.0) \
)
*/
#define FSIN_REDUCED(_x)    ((_x) \
                             + ((((_x)*(_x)*(_x)) * ((_x)*(_x) - 20))/120) \
                             - ((((_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)) * ((_x)*(_x)*(_x)*(_x) - 110*(_x)*(_x) + 7920))/39916800.0) \
                             - ((((_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)*(_x)) * ((_x)*(_x) - 210))/1307674368000.0) \
                            )


#define FABS(_i)            (((_i) < 0) ? -(_i) : (_i))
#define FROUND(_i) ((int)((_i) + (((_i) >= 0.0f) ? 0.5f : -0.5f)))

// argument reduction needed:
// we cannot accept arguments of any arbitrary range
// x should be in range [-pi;pi], otherwise we become very inaccurate
// bring x back into that range
#define FSIN(_i)            FSIN_REDUCED((_i) - FROUND((_i) / (2*M_PI)) * (2*M_PI))
#define FCOS(_i)            FSIN((_i) + M_PI/2)

#ifdef ENABLE_CONST_TABLES
#define TABLE_CONST const
#define TABLE_INIT(x) x
#else
#define TABLE_CONST 
#define TABLE_INIT(x) 0
#endif

#endif /* __AUTO_GEN_MATH_H__ */
