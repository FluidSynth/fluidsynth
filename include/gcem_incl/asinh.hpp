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
 * compile-time inverse hyperbolic sine function
 */

#ifndef _gcem_asinh_HPP
#define _gcem_asinh_HPP

namespace internal
{

template<typename T>
constexpr
T
asinh_compute(const T x)
noexcept
{
    return( // NaN check
            is_nan(x) ? \
                GCLIM<T>::quiet_NaN() :
            // indistinguishable from zero
            GCLIM<T>::min() > abs(x) ? \
                T(0) :
            // else
                log( x + sqrt(x*x + T(1)) ) );
}

}

/**
 * Compile-time inverse hyperbolic sine function
 *
 * @param x a real-valued input.
 * @return the inverse hyperbolic sine function using \f[ \text{asinh}(x) = \ln \left( x + \sqrt{x^2 + 1} \right) \f]
 */

template<typename T>
constexpr
return_t<T>
asinh(const T x)
noexcept
{
    return internal::asinh_compute( static_cast<return_t<T>>(x) );
}


#endif
