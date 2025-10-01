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
 * compile-time incomplete beta function
 *
 * see eq. 18.5.17a in the Handbook of Continued Fractions for Special Functions
 */

#ifndef _gcem_incomplete_beta_HPP
#define _gcem_incomplete_beta_HPP

namespace internal
{

template<typename T>
constexpr T incomplete_beta_cf(const T a, const T b, const T z, const T c_j, const T d_j, const T f_j, const int depth) noexcept;

//
// coefficients; see eq. 18.5.17b

template<typename T>
constexpr
T
incomplete_beta_coef_even(const T a, const T b, const T z, const int k)
noexcept
{
    return( -z*(a + k)*(a + b + k)/( (a + 2*k)*(a + 2*k + T(1)) ) );
}

template<typename T>
constexpr
T
incomplete_beta_coef_odd(const T a, const T b, const T z, const int k)
noexcept
{
    return( z*k*(b - k)/((a + 2*k - T(1))*(a + 2*k)) );
}

template<typename T>
constexpr
T
incomplete_beta_coef(const T a, const T b, const T z, const int depth)
noexcept
{
    return( !is_odd(depth) ? incomplete_beta_coef_even(a,b,z,depth/2) :
                             incomplete_beta_coef_odd(a,b,z,(depth+1)/2) );
}

//
// update formulae for the modified Lentz method

template<typename T>
constexpr
T
incomplete_beta_c_update(const T a, const T b, const T z, const T c_j, const int depth)
noexcept
{
    return( T(1) + incomplete_beta_coef(a,b,z,depth)/c_j );
}

template<typename T>
constexpr
T
incomplete_beta_d_update(const T a, const T b, const T z, const T d_j, const int depth)
noexcept
{
    return( T(1) / (T(1) + incomplete_beta_coef(a,b,z,depth)*d_j) );
}

//
// convergence-type condition

template<typename T>
constexpr
T
incomplete_beta_decision(const T a, const T b, const T z, const T c_j, const T d_j, const T f_j, const int depth)
noexcept
{
    return( // tolerance check
                abs(c_j*d_j - T(1)) < GCEM_INCML_BETA_TOL ? f_j*c_j*d_j :
            // max_iter check
                depth < GCEM_INCML_BETA_MAX_ITER ? \
                    // if
                        incomplete_beta_cf(a,b,z,c_j,d_j,f_j*c_j*d_j,depth+1) :
                    // else 
                        f_j*c_j*d_j );
}

template<typename T>
constexpr
T
incomplete_beta_cf(const T a, const T b, const T z, const T c_j, const T d_j, const T f_j, const int depth)
noexcept
{
    return  incomplete_beta_decision(a,b,z,
                incomplete_beta_c_update(a,b,z,c_j,depth),
                incomplete_beta_d_update(a,b,z,d_j,depth),
                f_j,depth);
}

//
// x^a (1-x)^{b} / (a beta(a,b)) * cf

template<typename T>
constexpr
T
incomplete_beta_begin(const T a, const T b, const T z)
noexcept
{
    return  ( (exp(a*log(z) + b*log(T(1)-z) - lbeta(a,b)) / a) * \
                incomplete_beta_cf(a,b,z,T(1), 
                    incomplete_beta_d_update(a,b,z,T(1),0),
                    incomplete_beta_d_update(a,b,z,T(1),0),1)
            );
}

template<typename T>
constexpr
T
incomplete_beta_check(const T a, const T b, const T z)
noexcept
{
    return( // NaN check
            any_nan(a, b, z) ? \
                GCLIM<T>::quiet_NaN() :
            // indistinguishable from zero
            GCLIM<T>::min() > z ? \
                T(0) :
            // parameter check for performance
            (a + T(1))/(a + b + T(2)) > z ? \
                incomplete_beta_begin(a,b,z) :
                T(1) - incomplete_beta_begin(b,a,T(1) - z) );
}

template<typename T1, typename T2, typename T3, typename TC = common_return_t<T1,T2,T3>>
constexpr
TC
incomplete_beta_type_check(const T1 a, const T2 b, const T3 p)
noexcept
{
    return incomplete_beta_check(static_cast<TC>(a),
                                 static_cast<TC>(b),
                                 static_cast<TC>(p));
}

}

/**
 * Compile-time regularized incomplete beta function
 *
 * @param a a real-valued, non-negative input.
 * @param b a real-valued, non-negative input.
 * @param z a real-valued, non-negative input.
 *
 * @return computes the regularized incomplete beta function,
 * \f[ \frac{\text{B}(z;\alpha,\beta)}{\text{B}(\alpha,\beta)} = \frac{1}{\text{B}(\alpha,\beta)}\int_0^z t^{a-1} (1-t)^{\beta-1} dt \f]
 * using a continued fraction representation, found in the Handbook of Continued Fractions for Special Functions, and a modified Lentz method.
 * \f[ \frac{\text{B}(z;\alpha,\beta)}{\text{B}(\alpha,\beta)} = \frac{z^{\alpha} (1-t)^{\beta}}{\alpha \text{B}(\alpha,\beta)} \dfrac{a_1}{1 + \dfrac{a_2}{1 + \dfrac{a_3}{1 + \dfrac{a_4}{1 + \ddots}}}} \f]
 * where \f$ a_1 = 1 \f$ and
 * \f[ a_{2m+2} = - \frac{(\alpha + m)(\alpha + \beta + m)}{(\alpha + 2m)(\alpha + 2m + 1)}, \ m \geq 0 \f]
 * \f[ a_{2m+1} = \frac{m(\beta - m)}{(\alpha + 2m - 1)(\alpha + 2m)}, \ m \geq 1 \f]
 * The Lentz method works as follows: let \f$ f_j \f$ denote the value of the continued fraction up to the first \f$ j \f$ terms; \f$ f_j \f$ is updated as follows:
 * \f[ c_j = 1 + a_j / c_{j-1}, \ \ d_j = 1 / (1 + a_j d_{j-1}) \f]
 * \f[ f_j = c_j d_j f_{j-1} \f]
 */

template<typename T1, typename T2, typename T3>
constexpr
common_return_t<T1,T2,T3>
incomplete_beta(const T1 a, const T2 b, const T3 z)
noexcept
{
    return internal::incomplete_beta_type_check(a,b,z);
}

#endif
