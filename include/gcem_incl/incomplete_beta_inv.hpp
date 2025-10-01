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
 * inverse of the incomplete beta function
 */

#ifndef _gcem_incomplete_beta_inv_HPP
#define _gcem_incomplete_beta_inv_HPP

namespace internal
{

template<typename T>
constexpr T incomplete_beta_inv_decision(const T value, const T alpha_par, const T beta_par, const T p,
                                         const T direc, const T lb_val, const int iter_count) noexcept;

//
// initial value for Halley

//
// a,b > 1 case

template<typename T>
constexpr
T
incomplete_beta_inv_initial_val_1_tval(const T p)
noexcept
{   // a > 1.0
    return( p > T(0.5) ? \
            // if
                sqrt(-T(2)*log(T(1) - p)) :
            // else
                sqrt(-T(2)*log(p)) );
}

template<typename T>
constexpr
T
incomplete_beta_inv_initial_val_1_int_begin(const T t_val)
noexcept
{   // internal for a > 1.0
    return( t_val - ( T(2.515517) + T(0.802853)*t_val + T(0.010328)*t_val*t_val ) \
                / ( T(1) + T(1.432788)*t_val + T(0.189269)*t_val*t_val + T(0.001308)*t_val*t_val*t_val ) );
}

template<typename T>
constexpr
T
incomplete_beta_inv_initial_val_1_int_ab1(const T alpha_par, const T beta_par)
noexcept
{
    return( T(1)/(2*alpha_par - T(1)) + T(1)/(2*beta_par - T(1)) );
}

template<typename T>
constexpr
T
incomplete_beta_inv_initial_val_1_int_ab2(const T alpha_par, const T beta_par)
noexcept
{
    return( T(1)/(2*beta_par - T(1)) - T(1)/(2*alpha_par - T(1)) );
}

template<typename T>
constexpr
T
incomplete_beta_inv_initial_val_1_int_h(const T ab_term_1)
noexcept
{
    return( T(2) / ab_term_1 );
}

template<typename T>
constexpr
T
incomplete_beta_inv_initial_val_1_int_w(const T value, const T ab_term_2, const T h_term)
noexcept
{
    // return( value * sqrt(h_term + lambda)/h_term - ab_term_2*(lambda + 5.0/6.0 -2.0/(3.0*h_term)) );
    return( value * sqrt(h_term + (value*value - T(3))/T(6))/h_term \
                - ab_term_2*((value*value - T(3))/T(6) + T(5)/T(6) - T(2)/(T(3)*h_term)) );
}

template<typename T>
constexpr
T
incomplete_beta_inv_initial_val_1_int_end(const T alpha_par, const T beta_par, const T w_term)
noexcept
{
    return( alpha_par / (alpha_par + beta_par*exp(2*w_term)) );
}

template<typename T>
constexpr
T
incomplete_beta_inv_initial_val_1(const T alpha_par, const T beta_par, const T t_val, const T sgn_term)
noexcept
{   // a > 1.0
    return  incomplete_beta_inv_initial_val_1_int_end( alpha_par, beta_par,
                incomplete_beta_inv_initial_val_1_int_w(
                    sgn_term*incomplete_beta_inv_initial_val_1_int_begin(t_val),
                    incomplete_beta_inv_initial_val_1_int_ab2(alpha_par,beta_par),
                    incomplete_beta_inv_initial_val_1_int_h(
                        incomplete_beta_inv_initial_val_1_int_ab1(alpha_par,beta_par)
                    )
                )
            );
}

//
// a,b else

template<typename T>
constexpr
T
incomplete_beta_inv_initial_val_2_s1(const T alpha_par, const T beta_par)
noexcept
{
    return( pow(alpha_par/(alpha_par+beta_par),alpha_par) / alpha_par );
}

template<typename T>
constexpr
T
incomplete_beta_inv_initial_val_2_s2(const T alpha_par, const T beta_par)
noexcept
{
    return( pow(beta_par/(alpha_par+beta_par),beta_par) / beta_par );
}

template<typename T>
constexpr
T
incomplete_beta_inv_initial_val_2(const T alpha_par, const T beta_par, const T p, const T s_1, const T s_2)
noexcept
{
    return( p <= s_1/(s_1 + s_2) ? pow(p*(s_1+s_2)*alpha_par,T(1)/alpha_par) :
                                    T(1) - pow(p*(s_1+s_2)*beta_par,T(1)/beta_par) );
}

// initial value

template<typename T>
constexpr
T
incomplete_beta_inv_initial_val(const T alpha_par, const T beta_par, const T p)
noexcept
{
    return( (alpha_par > T(1) && beta_par > T(1)) ?
            // if
                incomplete_beta_inv_initial_val_1(alpha_par,beta_par,
                    incomplete_beta_inv_initial_val_1_tval(p),
                    p < T(0.5) ? T(1) : T(-1) ) :
            // else
                p > T(0.5) ?
                    // if
                       T(1) - incomplete_beta_inv_initial_val_2(beta_par,alpha_par,T(1) - p,
                                    incomplete_beta_inv_initial_val_2_s1(beta_par,alpha_par),
                                    incomplete_beta_inv_initial_val_2_s2(beta_par,alpha_par)) :
                    // else
                       incomplete_beta_inv_initial_val_2(alpha_par,beta_par,p,
                            incomplete_beta_inv_initial_val_2_s1(alpha_par,beta_par),
                            incomplete_beta_inv_initial_val_2_s2(alpha_par,beta_par))
            );
}

//
// Halley recursion

template<typename T>
constexpr
T
incomplete_beta_inv_err_val(const T value, const T alpha_par, const T beta_par, const T p)
noexcept
{   // err_val = f(x)
    return( incomplete_beta(alpha_par,beta_par,value) - p );
}

template<typename T>
constexpr
T
incomplete_beta_inv_deriv_1(const T value, const T alpha_par, const T beta_par, const T lb_val)
noexcept
{   // derivative of the incomplete beta function w.r.t. x
    return( // indistinguishable from zero or one
            GCLIM<T>::min() > abs(value) ? \
                T(0) :
            GCLIM<T>::min() > abs(T(1) - value) ? \
                T(0) :
            // else
                exp( (alpha_par - T(1))*log(value) + (beta_par - T(1))*log(T(1) - value) - lb_val ) );
}

template<typename T>
constexpr
T
incomplete_beta_inv_deriv_2(const T value, const T alpha_par, const T beta_par, const T deriv_1)
noexcept
{ // second derivative of the incomplete beta function w.r.t. x
    return( deriv_1*((alpha_par - T(1))/value - (beta_par - T(1))/(T(1) - value)) );
}

template<typename T>
constexpr
T
incomplete_beta_inv_ratio_val_1(const T value, const T alpha_par, const T beta_par, const T p, const T deriv_1)
noexcept
{
    return( incomplete_beta_inv_err_val(value,alpha_par,beta_par,p) / deriv_1 );
}

template<typename T>
constexpr
T
incomplete_beta_inv_ratio_val_2(const T value, const T alpha_par, const T beta_par, const T deriv_1)
noexcept
{
    return( incomplete_beta_inv_deriv_2(value,alpha_par,beta_par,deriv_1) / deriv_1 );
}

template<typename T>
constexpr
T
incomplete_beta_inv_halley(const T ratio_val_1, const T ratio_val_2)
noexcept
{
    return( ratio_val_1 / max( T(0.8), min( T(1.2), T(1) - T(0.5)*ratio_val_1*ratio_val_2 ) ) );
}

template<typename T>
constexpr
T
incomplete_beta_inv_recur(const T value, const T alpha_par, const T beta_par, const T p, const T deriv_1,
                          const T lb_val, const int iter_count)
noexcept
{
    return( // derivative = 0
            GCLIM<T>::min() > abs(deriv_1) ? \
                incomplete_beta_inv_decision( value, alpha_par, beta_par, p, T(0), lb_val,
                    GCEM_INCML_BETA_INV_MAX_ITER+1) :
            // else
            incomplete_beta_inv_decision( value, alpha_par, beta_par, p,
               incomplete_beta_inv_halley(
                   incomplete_beta_inv_ratio_val_1(value,alpha_par,beta_par,p,deriv_1),
                   incomplete_beta_inv_ratio_val_2(value,alpha_par,beta_par,deriv_1)
               ), lb_val, iter_count) );
}

template<typename T>
constexpr
T
incomplete_beta_inv_decision(const T value, const T alpha_par, const T beta_par, const T p, const T direc,
                             const T lb_val, const int iter_count)
noexcept
{
    return( iter_count <= GCEM_INCML_BETA_INV_MAX_ITER ?
            // if
                incomplete_beta_inv_recur(value-direc,alpha_par,beta_par,p,
                    incomplete_beta_inv_deriv_1(value,alpha_par,beta_par,lb_val),
                    lb_val, iter_count+1) :
            // else
                value - direc );
}

template<typename T>
constexpr
T
incomplete_beta_inv_begin(const T initial_val, const T alpha_par, const T beta_par, const T p, const T lb_val)
noexcept
{
    return incomplete_beta_inv_recur(initial_val,alpha_par,beta_par,p,
               incomplete_beta_inv_deriv_1(initial_val,alpha_par,beta_par,lb_val),
               lb_val,1);
}

template<typename T>
constexpr
T
incomplete_beta_inv_check(const T alpha_par, const T beta_par, const T p)
noexcept
{
    return( // NaN check
            any_nan(alpha_par, beta_par, p) ? \
                GCLIM<T>::quiet_NaN() :
            // indistinguishable from zero or one
            GCLIM<T>::min() > p ? \
                T(0) :
            GCLIM<T>::min() > abs(T(1) - p) ? \
                T(1) :
            // else
                incomplete_beta_inv_begin(incomplete_beta_inv_initial_val(alpha_par,beta_par,p),
                    alpha_par,beta_par,p,lbeta(alpha_par,beta_par)) );
}

template<typename T1, typename T2, typename T3, typename TC = common_t<T1,T2,T3>>
constexpr
TC
incomplete_beta_inv_type_check(const T1 a, const T2 b, const T3 p)
noexcept
{
    return incomplete_beta_inv_check(static_cast<TC>(a),
                                     static_cast<TC>(b),
                                     static_cast<TC>(p));
}

}

/**
 * Compile-time inverse incomplete beta function
 *
 * @param a a real-valued, non-negative input.
 * @param b a real-valued, non-negative input.
 * @param p a real-valued input with values in the unit-interval.
 *
 * @return Computes the inverse incomplete beta function, a value \f$ x \f$ such that 
 * \f[ f(x) := \frac{\text{B}(x;\alpha,\beta)}{\text{B}(\alpha,\beta)} - p \f]
 * equal to zero, for a given \c p.
 * GCE-Math finds this root using Halley's method:
 * \f[ x_{n+1} = x_n - \frac{f(x_n)/f'(x_n)}{1 - 0.5 \frac{f(x_n)}{f'(x_n)} \frac{f''(x_n)}{f'(x_n)} } \f]
 * where
 * \f[ \frac{\partial}{\partial x} \left(\frac{\text{B}(x;\alpha,\beta)}{\text{B}(\alpha,\beta)}\right) = \frac{1}{\text{B}(\alpha,\beta)} x^{\alpha-1} (1-x)^{\beta-1} \f]
 * \f[ \frac{\partial^2}{\partial x^2} \left(\frac{\text{B}(x;\alpha,\beta)}{\text{B}(\alpha,\beta)}\right) = \frac{1}{\text{B}(\alpha,\beta)} x^{\alpha-1} (1-x)^{\beta-1} \left( \frac{\alpha-1}{x} - \frac{\beta-1}{1 - x} \right) \f]
 */

template<typename T1, typename T2, typename T3>
constexpr
common_t<T1,T2,T3>
incomplete_beta_inv(const T1 a, const T2 b, const T3 p)
noexcept
{
    return internal::incomplete_beta_inv_type_check(a,b,p);
}

#endif
