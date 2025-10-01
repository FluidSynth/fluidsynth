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
 * compile-time exponential function
 */

#ifndef _gcem_exp_HPP
#define _gcem_exp_HPP

namespace internal
{

// see https://en.wikipedia.org/wiki/Euler%27s_continued_fraction_formula

#if __cplusplus >= 201402L // C++14 version

template<typename T>
constexpr
T
exp_cf_recur(const T x, const int depth_end)
noexcept
{
    int depth = GCEM_EXP_MAX_ITER_SMALL - 1;
    T res = T(1);

    while (depth > depth_end - 1) {
        res = T(1) + x/T(depth - 1) - x/depth/res;

        --depth;
    }

    return res;
}

#else // C++11 version

template<typename T>
constexpr
T
exp_cf_recur(const T x, const int depth)
noexcept
{
    return( depth < GCEM_EXP_MAX_ITER_SMALL ? \
            // if
                T(1) + x/T(depth - 1) - x/depth/exp_cf_recur(x,depth+1) : 
             // else
                T(1) );
}

#endif

template<typename T>
constexpr
T
exp_cf(const T x)
noexcept
{
    return( T(1) / (T(1) - x / exp_cf_recur(x,2)) );
}

template<typename T>
constexpr
T
exp_split(const T x)
noexcept
{
    return( static_cast<T>(pow_integral(GCEM_E,find_whole(x))) * exp_cf(find_fraction(x)) );
}

template<typename T>
constexpr
T
exp_check(const T x)
noexcept
{
    return( is_nan(x) ? \
                GCLIM<T>::quiet_NaN() :
            //
            is_neginf(x) ? \
                T(0) :
            // indistinguishable from zero
            GCLIM<T>::min() > abs(x) ? \
                T(1) : 
            //
            is_posinf(x) ? \
                GCLIM<T>::infinity() :
            //
            abs(x) < T(2) ? \
                exp_cf(x) : \
                exp_split(x) );
}

}

/**
 * Compile-time exponential function
 *
 * @param x a real-valued input.
 * @return \f$ \exp(x) \f$ using \f[ \exp(x) = \dfrac{1}{1-\dfrac{x}{1+x-\dfrac{\frac{1}{2}x}{1 + \frac{1}{2}x - \dfrac{\frac{1}{3}x}{1 + \frac{1}{3}x - \ddots}}}} \f] 
 * The continued fraction argument is split into two parts: \f$ x = n + r \f$, where \f$ n \f$ is an integer and \f$ r \in [-0.5,0.5] \f$.
 */

template<typename T>
constexpr
return_t<T>
exp(const T x)
noexcept
{
    return internal::exp_check( static_cast<return_t<T>>(x) );
}

#endif
