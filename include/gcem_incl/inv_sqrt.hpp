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
 * compile-time inverse-square-root function
 */

#ifndef _gcem_inv_sqrt_HPP
#define _gcem_inv_sqrt_HPP

namespace internal
{

template<typename T>
constexpr
T
inv_sqrt_recur(const T x, const T xn, const int count)
noexcept
{
    return( abs( xn - T(1)/(x*xn) ) / (T(1) + xn) < GCLIM<T>::min() ? \
            // if
                xn :
            count < GCEM_INV_SQRT_MAX_ITER ? \
            // else
                inv_sqrt_recur(x, T(0.5)*(xn + T(1)/(x*xn)), count+1) :
                xn );
}

template<typename T>
constexpr
T
inv_sqrt_check(const T x)
noexcept
{
    return( is_nan(x) ? \
                GCLIM<T>::quiet_NaN() :
            //
            x < T(0) ? \
                GCLIM<T>::quiet_NaN() :
            //
            is_posinf(x) ? \
                T(0) :
            // indistinguishable from zero or one
            GCLIM<T>::min() > abs(x) ? \
                GCLIM<T>::infinity() :
            GCLIM<T>::min() > abs(T(1) - x) ? \
                x :
            // else
            inv_sqrt_recur(x, x/T(2), 0) );
}

}


/**
 * Compile-time inverse-square-root function
 *
 * @param x a real-valued input.
 * @return Computes \f$ 1 / \sqrt{x} \f$ using a Newton-Raphson approach.
 */

template<typename T>
constexpr
return_t<T>
inv_sqrt(const T x)
noexcept
{
    return internal::inv_sqrt_check( static_cast<return_t<T>>(x) );
}

#endif
