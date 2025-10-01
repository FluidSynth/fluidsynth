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
 * compile-time tangent function
 */

#ifndef _gcem_tan_HPP
#define _gcem_tan_HPP

namespace internal
{

template<typename T>
constexpr
T
tan_series_exp_long(const T z)
noexcept
{   // this is based on a fourth-order expansion of tan(z) using Bernoulli numbers
    return( - 1/z + (z/3 + (pow_integral(z,3)/45 + (2*pow_integral(z,5)/945 + pow_integral(z,7)/4725))) );
}

template<typename T>
constexpr
T
tan_series_exp(const T x)
noexcept
{
    return( GCLIM<T>::min() > abs(x - T(GCEM_HALF_PI)) ? \
            // the value tan(pi/2) is somewhat of a convention;
            // technically the function is not defined at EXACTLY pi/2,
            // but this is floating point pi/2
                T(1.633124e+16) :
            // otherwise we use an expansion around pi/2
                tan_series_exp_long(x - T(GCEM_HALF_PI))
            );
}

template<typename T>
constexpr
T
tan_cf_recur(const T xx, const int depth, const int max_depth)
noexcept
{
    return( depth < max_depth ? \
            // if
                T(2*depth - 1) - xx/tan_cf_recur(xx,depth+1,max_depth) :
            // else
                T(2*depth - 1) );
}

template<typename T>
constexpr
T
tan_cf_main(const T x)
noexcept
{
    return( (x > T(1.55) && x < T(1.60)) ? \
                tan_series_exp(x) : // deals with a singularity at tan(pi/2)
            //
            x > T(1.4) ? \
                x/tan_cf_recur(x*x,1,45) :
            x > T(1)   ? \
                x/tan_cf_recur(x*x,1,35) :
            // else
                x/tan_cf_recur(x*x,1,25) );
}

template<typename T>
constexpr
T
tan_begin(const T x, const int count = 0)
noexcept
{   // tan(x) = tan(x + pi)
    return( x > T(GCEM_PI) ? \
            // if
                count > 1 ? GCLIM<T>::quiet_NaN() : // protect against undefined behavior
                tan_begin( x - T(GCEM_PI) * internal::floor_check(x/T(GCEM_PI)), count+1 ) :
            // else 
                tan_cf_main(x) );
}

template<typename T>
constexpr
T
tan_check(const T x)
noexcept
{
    return( // NaN check
            is_nan(x) ? \
                GCLIM<T>::quiet_NaN() :
            // indistinguishable from zero 
            GCLIM<T>::min() > abs(x) ? \
                T(0) :
            // else
                x < T(0) ? \
                    - tan_begin(-x) : 
                      tan_begin( x) );
}

}

/**
 * Compile-time tangent function
 *
 * @param x a real-valued input.
 * @return the tangent function using
 * \f[ \tan(x) = \dfrac{x}{1 - \dfrac{x^2}{3 - \dfrac{x^2}{5 - \ddots}}} \f]
 * To deal with a singularity at \f$ \pi / 2 \f$, the following expansion is employed:
 * \f[ \tan(x) = - \frac{1}{x-\pi/2} - \sum_{k=1}^\infty \frac{(-1)^k 2^{2k} B_{2k}}{(2k)!} (x - \pi/2)^{2k - 1} \f]
 * where \f$ B_n \f$ is the n-th Bernoulli number.
 */

template<typename T>
constexpr
return_t<T>
tan(const T x)
noexcept
{
    return internal::tan_check( static_cast<return_t<T>>(x) );
}

#endif
