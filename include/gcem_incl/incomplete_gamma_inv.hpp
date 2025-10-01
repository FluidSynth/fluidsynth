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
 * inverse of the incomplete gamma function
 */

#ifndef _gcem_incomplete_gamma_inv_HPP
#define _gcem_incomplete_gamma_inv_HPP

namespace internal
{

template<typename T>
constexpr T incomplete_gamma_inv_decision(const T value, const T a, const T p, const T direc, const T lg_val, const int iter_count) noexcept;

//
// initial value for Halley

template<typename T>
constexpr
T
incomplete_gamma_inv_t_val_1(const T p)
noexcept
{   // a > 1.0
    return( p > T(0.5) ? sqrt(-2*log(T(1) - p)) : sqrt(-2*log(p)) );
}

template<typename T>
constexpr
T
incomplete_gamma_inv_t_val_2(const T a)
noexcept
{   // a <= 1.0
    return( T(1) - T(0.253) * a - T(0.12) * a*a );
}

//

template<typename T>
constexpr
T
incomplete_gamma_inv_initial_val_1_int_begin(const T t_val)
noexcept
{   // internal for a > 1.0
    return( t_val - (T(2.515517L) + T(0.802853L)*t_val + T(0.010328L)*t_val*t_val) \
                / (T(1) + T(1.432788L)*t_val + T(0.189269L)*t_val*t_val + T(0.001308L)*t_val*t_val*t_val) );
}

template<typename T>
constexpr
T
incomplete_gamma_inv_initial_val_1_int_end(const T value_inp, const T a)
noexcept
{   // internal for a > 1.0
    return max( T(1E-04), a*pow(T(1) - T(1)/(9*a) - value_inp/(3*sqrt(a)), 3) );
}

template<typename T>
constexpr
T
incomplete_gamma_inv_initial_val_1(const T a, const T t_val, const T sgn_term)
noexcept
{   // a > 1.0
    return incomplete_gamma_inv_initial_val_1_int_end(sgn_term*incomplete_gamma_inv_initial_val_1_int_begin(t_val), a);
}

template<typename T>
constexpr
T
incomplete_gamma_inv_initial_val_2(const T a, const T p, const T t_val)
noexcept
{   // a <= 1.0
    return( p < t_val ? \
             // if 
                pow(p/t_val,T(1)/a) : 
             // else
                T(1) - log(T(1) - (p - t_val)/(T(1) - t_val)) );
}

// initial value

template<typename T>
constexpr
T
incomplete_gamma_inv_initial_val(const T a, const T p)
noexcept
{
    return( a > T(1) ? \
             // if
                incomplete_gamma_inv_initial_val_1(a,
                    incomplete_gamma_inv_t_val_1(p),
                    p > T(0.5) ? T(-1) : T(1)) :
             // else
                incomplete_gamma_inv_initial_val_2(a,p,
                    incomplete_gamma_inv_t_val_2(a)));
}

//
// Halley recursion

template<typename T>
constexpr
T
incomplete_gamma_inv_err_val(const T value, const T a, const T p)
noexcept
{ // err_val = f(x)
    return( incomplete_gamma(a,value) - p );
}

template<typename T>
constexpr
T
incomplete_gamma_inv_deriv_1(const T value, const T a, const T lg_val)
noexcept
{ // derivative of the incomplete gamma function w.r.t. x
    return( exp( - value + (a - T(1))*log(value) - lg_val ) );
}

template<typename T>
constexpr
T
incomplete_gamma_inv_deriv_2(const T value, const T a, const T deriv_1)
noexcept
{ // second derivative of the incomplete gamma function w.r.t. x
    return( deriv_1*((a - T(1))/value - T(1)) );
}

template<typename T>
constexpr
T
incomplete_gamma_inv_ratio_val_1(const T value, const T a, const T p, const T deriv_1)
noexcept
{
    return( incomplete_gamma_inv_err_val(value,a,p) / deriv_1 );
}

template<typename T>
constexpr
T
incomplete_gamma_inv_ratio_val_2(const T value, const T a, const T deriv_1)
noexcept
{
    return( incomplete_gamma_inv_deriv_2(value,a,deriv_1) / deriv_1 );
}

template<typename T>
constexpr
T
incomplete_gamma_inv_halley(const T ratio_val_1, const T ratio_val_2)
noexcept
{
    return( ratio_val_1 / max( T(0.8), min( T(1.2), T(1) - T(0.5)*ratio_val_1*ratio_val_2 ) ) );
}

template<typename T>
constexpr
T
incomplete_gamma_inv_recur(const T value, const T a, const T p, const T deriv_1, const T lg_val, const int iter_count)
noexcept
{
    return incomplete_gamma_inv_decision(value, a, p,
                incomplete_gamma_inv_halley(incomplete_gamma_inv_ratio_val_1(value,a,p,deriv_1), 
                incomplete_gamma_inv_ratio_val_2(value,a,deriv_1)),
                lg_val, iter_count);
}

template<typename T>
constexpr
T
incomplete_gamma_inv_decision(const T value, const T a, const T p, const T direc, const T lg_val, const int iter_count)
noexcept
{
    // return( abs(direc) > GCEM_INCML_GAMMA_INV_TOL ? incomplete_gamma_inv_recur(value - direc, a, p, incomplete_gamma_inv_deriv_1(value,a,lg_val), lg_val) : value - direc );
    return( iter_count <= GCEM_INCML_GAMMA_INV_MAX_ITER ? \
            // if
                incomplete_gamma_inv_recur(value-direc,a,p,
                    incomplete_gamma_inv_deriv_1(value,a,lg_val),
                    lg_val,iter_count+1) :
            // else 
                value - direc );
}

template<typename T>
constexpr
T
incomplete_gamma_inv_begin(const T initial_val, const T a, const T p, const T lg_val)
noexcept
{
    return incomplete_gamma_inv_recur(initial_val,a,p,
                incomplete_gamma_inv_deriv_1(initial_val,a,lg_val), lg_val,1);
}

template<typename T>
constexpr
T
incomplete_gamma_inv_check(const T a, const T p)
noexcept
{
    return( // NaN check
            any_nan(a, p) ? \
                GCLIM<T>::quiet_NaN() :
            //
            GCLIM<T>::min() > p ? \
                T(0) :
            p > T(1) ? \
                GCLIM<T>::quiet_NaN() :
            GCLIM<T>::min() > abs(T(1) - p) ? \
                GCLIM<T>::infinity() :
            //
            GCLIM<T>::min() > a ? \
                T(0) :
            // else
                incomplete_gamma_inv_begin(incomplete_gamma_inv_initial_val(a,p),a,p,lgamma(a)) );
}

template<typename T1, typename T2, typename TC = common_return_t<T1,T2>>
constexpr
TC
incomplete_gamma_inv_type_check(const T1 a, const T2 p)
noexcept
{
    return incomplete_gamma_inv_check(static_cast<TC>(a),
                                      static_cast<TC>(p));
}

}

/**
 * Compile-time inverse incomplete gamma function
 *
 * @param a a real-valued, non-negative input.
 * @param p a real-valued input with values in the unit-interval.
 *
 * @return Computes the inverse incomplete gamma function, a value \f$ x \f$ such that 
 * \f[ f(x) := \frac{\gamma(a,x)}{\Gamma(a)} - p \f]
 * equal to zero, for a given \c p.
 * GCE-Math finds this root using Halley's method:
 * \f[ x_{n+1} = x_n - \frac{f(x_n)/f'(x_n)}{1 - 0.5 \frac{f(x_n)}{f'(x_n)} \frac{f''(x_n)}{f'(x_n)} } \f]
 * where
 * \f[ \frac{\partial}{\partial x} \left(\frac{\gamma(a,x)}{\Gamma(a)}\right) = \frac{1}{\Gamma(a)} x^{a-1} \exp(-x) \f]
 * \f[ \frac{\partial^2}{\partial x^2} \left(\frac{\gamma(a,x)}{\Gamma(a)}\right) = \frac{1}{\Gamma(a)} x^{a-1} \exp(-x) \left( \frac{a-1}{x} - 1 \right) \f]
 */

template<typename T1, typename T2>
constexpr
common_return_t<T1,T2>
incomplete_gamma_inv(const T1 a, const T2 p)
noexcept
{
    return internal::incomplete_gamma_inv_type_check(a,p);
}

#endif
