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
 * compile-time error function
 */

#ifndef _gcem_erf_HPP
#define _gcem_erf_HPP

namespace internal
{

// see
// http://functions.wolfram.com/GammaBetaErf/Erf/10/01/0007/

#if __cplusplus >= 201402L // C++14 version

template<typename T>
constexpr
T
erf_cf_large_recur(const T x, const int depth_end)
noexcept
{
    int depth = GCEM_ERF_MAX_ITER - 1;
    T res = x;

    while (depth > depth_end - 1) {
        res = x + 2 * depth / res;

        --depth;
    }

    return res;
}

#else // C++11 version

template<typename T>
constexpr
T
erf_cf_large_recur(const T x, const int depth)
noexcept
{
    return( depth < GCEM_ERF_MAX_ITER ? \
            // if
                x + 2 * depth / erf_cf_large_recur(x,depth+1) :
            // else
                x );
}

#endif

template<typename T>
constexpr
T
erf_cf_large_main(const T x)
noexcept
{
    return( T(1) - T(2) * ( exp(-x*x) / T(GCEM_SQRT_PI) ) \
                / erf_cf_large_recur(T(2)*x,1) );
}

// see
// http://functions.wolfram.com/GammaBetaErf/Erf/10/01/0005/

#if __cplusplus >= 201402L // C++14 version

template<typename T>
constexpr
T
erf_cf_small_recur(const T xx, const int depth_end)
noexcept
{
    int depth = GCEM_ERF_MAX_ITER - 1;
    T res = T(2*(depth+1) - 1) - 2 * xx;

    while (depth > depth_end - 1) {
        res = T(2*depth - 1) - 2 * xx + 4 * depth * xx / res;

        --depth;
    }

    return res;
}

#else // C++11 version

template<typename T>
constexpr
T
erf_cf_small_recur(const T xx, const int depth)
noexcept
{
    return( depth < GCEM_ERF_MAX_ITER ? \
            // if
                (2*depth - T(1)) - 2 * xx \
                    + 4 * depth * xx / erf_cf_small_recur(xx,depth+1) :
            // else
                (2*depth - T(1)) - 2*xx );
}

#endif

template<typename T>
constexpr
T
erf_cf_small_main(const T x)
noexcept
{
    return( T(2) * x * ( exp(-x*x) / T(GCEM_SQRT_PI) ) \
                / erf_cf_small_recur(x*x,1) );
}

//

template<typename T>
constexpr
T
erf_begin(const T x)
noexcept
{
    return( x > T(2.1) ? \
            // if
                erf_cf_large_main(x) :
            // else
                erf_cf_small_main(x) );
}

template<typename T>
constexpr
T
erf_check(const T x)
noexcept
{
    return( // NaN check
            is_nan(x) ? \
                GCLIM<T>::quiet_NaN() :
            // +/-Inf
            is_posinf(x) ? \
                T(1) :
            is_neginf(x) ? \
                - T(1) :
            // indistinguishable from zero
            GCLIM<T>::min() > abs(x) ? \
                T(0) :
            // else
                x < T(0) ? \
                    - erf_begin(-x) : 
                      erf_begin( x) );
}

}

/**
 * Compile-time Gaussian error function
 *
 * @param x a real-valued input.
 * @return computes the Gaussian error function
 * \f[ \text{erf}(x) = \frac{2}{\sqrt{\pi}} \int_0^x \exp( - t^2) dt \f]
 * using a continued fraction representation:
 * \f[ \text{erf}(x) = \frac{2x}{\sqrt{\pi}} \exp(-x^2) \dfrac{1}{1 - 2x^2 + \dfrac{4x^2}{3 - 2x^2 + \dfrac{8x^2}{5 - 2x^2 + \dfrac{12x^2}{7 - 2x^2 + \ddots}}}} \f]
 */

template<typename T>
constexpr
return_t<T>
erf(const T x)
noexcept
{
    return internal::erf_check( static_cast<return_t<T>>(x) );
}

#endif
