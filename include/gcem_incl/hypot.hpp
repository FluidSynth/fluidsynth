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
 * compile-time Pythagorean addition function
 */

// see: https://en.wikipedia.org/wiki/Pythagorean_addition

#ifndef _gcem_hypot_HPP
#define _gcem_hypot_HPP

namespace internal
{

template<typename T>
constexpr
T
hypot_compute(const T x, const T ydx)
noexcept
{
    return abs(x) * sqrt( T(1) + (ydx * ydx) );
}

template<typename T>
constexpr
T
hypot_vals_check(const T x, const T y)
noexcept
{
    return( any_nan(x, y) ? \
                GCLIM<T>::quiet_NaN() :
            //
            any_inf(x,y) ? \
                GCLIM<T>::infinity() :
            // indistinguishable from zero or one
            GCLIM<T>::min() > abs(x) ? \
                abs(y) :
            GCLIM<T>::min() > abs(y) ? \
                abs(x) :
            // else
            hypot_compute(x, y/x) );
}

template<typename T1, typename T2, typename TC = common_return_t<T1,T2>>
constexpr
TC
hypot_type_check(const T1 x, const T2 y)
noexcept
{
    return hypot_vals_check(static_cast<TC>(x),static_cast<TC>(y));
}

}

/**
 * Compile-time Pythagorean addition function
 *
 * @param x a real-valued input.
 * @param y a real-valued input.
 * @return Computes \f$ x \oplus y = \sqrt{x^2 + y^2} \f$.
 */

template<typename T1, typename T2>
constexpr
common_return_t<T1,T2>
hypot(const T1 x, const T2 y)
noexcept
{
    return internal::hypot_type_check(x,y);
}

#endif
