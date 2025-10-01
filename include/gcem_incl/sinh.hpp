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
 * compile-time hyperbolic sine function
 */

#ifndef _gcem_sinh_HPP
#define _gcem_sinh_HPP

namespace internal
{

template<typename T>
constexpr
T
sinh_check(const T x)
noexcept
{
    return( // NaN check
            is_nan(x) ? \
                GCLIM<T>::quiet_NaN() :
            // indistinguishable from zero
            GCLIM<T>::min() > abs(x) ? \
                T(0) :
            // else
                (exp(x) - exp(-x))/T(2) );
}

}

/**
 * Compile-time hyperbolic sine function
 *
 * @param x a real-valued input.
 * @return the hyperbolic sine function using \f[ \sinh(x) = \frac{\exp(x) - \exp(-x)}{2} \f]
 */

template<typename T>
constexpr
return_t<T>
sinh(const T x)
noexcept
{
    return internal::sinh_check( static_cast<return_t<T>>(x) );
}

#endif
