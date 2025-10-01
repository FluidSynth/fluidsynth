/*################################################################################
  ##
  ##   Copyright (C) 2016-2024 Keith O'Hara
  ##
  ##   This file is part of the GCE-Math C++ library.
  ##
  ##   Licensed under the Apache License, Version 2.0 (the "License");
  ##   you may not use this file except in compliance with the License.
  ##   You may obtain a copy of the License at
  ##
  ##       http://www.apache.org/licenses/LICENSE-2.0
  ##
  ##   Unless required by applicable law or agreed to in writing, software
  ##   distributed under the License is distributed on an "AS IS" BASIS,
  ##   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  ##   See the License for the specific language governing permissions and
  ##   limitations under the License.
  ##
  ################################################################################*/

/* 
 * compile-time (regularized) incomplete gamma function
 */

#ifndef _gcem_incomplete_gamma_HPP
#define _gcem_incomplete_gamma_HPP

namespace internal
{

// 50 point Gauss-Legendre quadrature

template<typename T>
constexpr
T
incomplete_gamma_quad_inp_vals(const T lb, const T ub, const int counter)
noexcept
{
    return (ub-lb) * gauss_legendre_50_points[counter] / T(2) + (ub + lb) / T(2);
}

template<typename T>
constexpr
T
incomplete_gamma_quad_weight_vals(const T lb, const T ub, const int counter)
noexcept
{
    return (ub-lb) * gauss_legendre_50_weights[counter] / T(2);
}

template<typename T>
constexpr
T
incomplete_gamma_quad_fn(const T x, const T a, const T lg_term)
noexcept
{
    return exp( -x + (a-T(1))*log(x) - lg_term );
}

template<typename T>
constexpr
T
incomplete_gamma_quad_recur(const T lb, const T ub, const T a, const T lg_term, const int counter)
noexcept
{
    return( counter < 49 ? \
            // if 
                incomplete_gamma_quad_fn(incomplete_gamma_quad_inp_vals(lb,ub,counter),a,lg_term) \
                    * incomplete_gamma_quad_weight_vals(lb,ub,counter) \
                    + incomplete_gamma_quad_recur(lb,ub,a,lg_term,counter+1) :
            // else
                incomplete_gamma_quad_fn(incomplete_gamma_quad_inp_vals(lb,ub,counter),a,lg_term) \
                    * incomplete_gamma_quad_weight_vals(lb,ub,counter) );
}

template<typename T>
constexpr
T
incomplete_gamma_quad_lb(const T a, const T z)
noexcept
{
    return( a > T(1000) ? max(T(0),min(z,a) - 11*sqrt(a)) : // break integration into ranges
            a > T(800)  ? max(T(0),min(z,a) - 11*sqrt(a)) :
            a > T(500)  ? max(T(0),min(z,a) - 10*sqrt(a)) :
            a > T(300)  ? max(T(0),min(z,a) - 10*sqrt(a)) : 
            a > T(100)  ? max(T(0),min(z,a) -  9*sqrt(a)) :
            a > T(90)   ? max(T(0),min(z,a) -  9*sqrt(a)) :
            a > T(70)   ? max(T(0),min(z,a) -  8*sqrt(a)) :
            a > T(50)   ? max(T(0),min(z,a) -  7*sqrt(a)) :
            a > T(40)   ? max(T(0),min(z,a) -  6*sqrt(a)) :
            a > T(30)   ? max(T(0),min(z,a) -  5*sqrt(a)) :
            // else
                max(T(0),min(z,a)-4*sqrt(a)) );
}

template<typename T>
constexpr
T
incomplete_gamma_quad_ub(const T a, const T z)
noexcept
{
    return( a > T(1000) ? min(z, a + 10*sqrt(a)) :
            a > T(800)  ? min(z, a + 10*sqrt(a)) :
            a > T(500)  ? min(z, a + 9*sqrt(a))  :
            a > T(300)  ? min(z, a + 9*sqrt(a))  : 
            a > T(100)  ? min(z, a + 8*sqrt(a))  :
            a > T(90)   ? min(z, a + 8*sqrt(a))  :
            a > T(70)   ? min(z, a + 7*sqrt(a))  :
            a > T(50)   ? min(z, a + 6*sqrt(a))  :
            // else
                min(z, a + 5*sqrt(a)) );
}

template<typename T>
constexpr
T
incomplete_gamma_quad(const T a, const T z)
noexcept
{
    return incomplete_gamma_quad_recur(incomplete_gamma_quad_lb(a,z), incomplete_gamma_quad_ub(a,z), a,lgamma(a),0);
}

// reverse cf expansion
// see: https://functions.wolfram.com/GammaBetaErf/Gamma2/10/0003/

template<typename T>
constexpr
T
incomplete_gamma_cf_2_recur(const T a, const T z, const int depth)
noexcept
{
    return( depth < 100 ? \
            // if
                (1 + (depth-1)*2 - a + z) + depth*(a - depth)/incomplete_gamma_cf_2_recur(a,z,depth+1) :
            // else
                (1 + (depth-1)*2 - a + z) );
}

template<typename T>
constexpr
T
incomplete_gamma_cf_2(const T a, const T z)
noexcept
{   // lower (regularized) incomplete gamma function
    return( T(1.0) - exp(a*log(z) - z - lgamma(a)) / incomplete_gamma_cf_2_recur(a,z,1) );
}

// cf expansion
// see: http://functions.wolfram.com/GammaBetaErf/Gamma2/10/0009/

template<typename T>
constexpr
T
incomplete_gamma_cf_1_coef(const T a, const T z, const int depth)
noexcept
{
    return( is_odd(depth) ? - (a - 1 + T(depth+1)/T(2)) * z : T(depth)/T(2) * z );
}

template<typename T>
constexpr
T
incomplete_gamma_cf_1_recur(const T a, const T z, const int depth)
noexcept
{
    return( depth < GCEM_INCML_GAMMA_MAX_ITER ? \
            // if
                (a + depth - 1) + incomplete_gamma_cf_1_coef(a,z,depth)/incomplete_gamma_cf_1_recur(a,z,depth+1) :
            // else
                (a + depth - 1) );
}

template<typename T>
constexpr
T
incomplete_gamma_cf_1(const T a, const T z)
noexcept
{   // lower (regularized) incomplete gamma function
    return( exp(a*log(z) - z - lgamma(a)) / incomplete_gamma_cf_1_recur(a,z,1) );
}

//

template<typename T>
constexpr
T
incomplete_gamma_check(const T a, const T z)
noexcept
{
    return( // NaN check
            any_nan(a, z) ? \
                GCLIM<T>::quiet_NaN() :
            //
            a < T(0) ? \
                GCLIM<T>::quiet_NaN() :
            //
            GCLIM<T>::min() > z ? \
                T(0) : 
            //
            GCLIM<T>::min() > a ? \
                T(1) : 
            // cf or quadrature
            (a < T(10)) && (z - a < T(10)) ?
                incomplete_gamma_cf_1(a,z) :
            (a < T(10)) || (z/a > T(3)) ?
                incomplete_gamma_cf_2(a,z) :
            // else
                incomplete_gamma_quad(a,z) );
}

template<typename T1, typename T2, typename TC = common_return_t<T1,T2>>
constexpr
TC
incomplete_gamma_type_check(const T1 a, const T2 p)
noexcept
{
    return incomplete_gamma_check(static_cast<TC>(a),
                                  static_cast<TC>(p));
}

}

/**
 * Compile-time regularized lower incomplete gamma function
 *
 * @param a a real-valued, non-negative input.
 * @param x a real-valued, non-negative input.
 *
 * @return the regularized lower incomplete gamma function evaluated at (\c a, \c x),
 * \f[ \frac{\gamma(a,x)}{\Gamma(a)} = \frac{1}{\Gamma(a)} \int_0^x t^{a-1} \exp(-t) dt \f]
 * When \c a is not too large, the value is computed using the continued fraction representation of the upper incomplete gamma function, \f$ \Gamma(a,x) \f$, using
 * \f[ \Gamma(a,x) = \Gamma(a) - \dfrac{x^a\exp(-x)}{a - \dfrac{ax}{a + 1 + \dfrac{x}{a + 2 - \dfrac{(a+1)x}{a + 3 + \dfrac{2x}{a + 4 - \ddots}}}}} \f]
 * where \f$ \gamma(a,x) \f$ and \f$ \Gamma(a,x) \f$ are connected via
 * \f[ \frac{\gamma(a,x)}{\Gamma(a)} + \frac{\Gamma(a,x)}{\Gamma(a)} = 1 \f]
 * When \f$ a > 10 \f$, a 50-point Gauss-Legendre quadrature scheme is employed.
 */

template<typename T1, typename T2>
constexpr
common_return_t<T1,T2>
incomplete_gamma(const T1 a, const T2 x)
noexcept
{
    return internal::incomplete_gamma_type_check(a,x);
}

#endif
