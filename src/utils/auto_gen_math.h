#ifndef __AUTO_GEN_MATH_H__
#define __AUTO_GEN_MATH_H__

/* Bhaskara I's sine approximation formula.
 */

#define AUTO_GEN_SIN(_x)    (((_x) < M_PI) ? \
                            16*(_x)*(M_PI-(_x))/(5*M_PI*M_PI-4*(_x)*(M_PI-(_x))) : \
                            -16*(2*M_PI-(_x))*(M_PI-(2*M_PI-(_x)))/(5*M_PI*M_PI-4*(2*M_PI-(_x))*(M_PI-(2*M_PI-(_x)))))


#define AUTO_GEN_COS(_x)    (((_x) < M_PI/2) ? \
                             (M_PI*M_PI-4*(_x)*(_x))/(M_PI*M_PI+(_x)*(_x)) : \
                             ((_x) < 3*M_PI/2) ? \
                             -(M_PI*M_PI-4*(M_PI-(_x))*(M_PI-(_x)))/(M_PI*M_PI+(M_PI-(_x))*(M_PI-(_x))) : \
                             (M_PI*M_PI-4*((_x)-2*M_PI)*((_x)-2*M_PI))/(M_PI*M_PI+((_x)-2*M_PI)*((_x)-2*M_PI)))


#endif /* __AUTO_GEN_MATH_H__ */
