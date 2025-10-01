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

#ifndef _gcem_expm1_HPP
#define _gcem_expm1_HPP

namespace internal
{

template<typename T>
constexpr
T
expm1_compute(const T x)
noexcept
{
    // return x * ( T(1) + x * ( T(1)/T(2) + x * ( T(1)/T(6) + x * ( T(1)/T(24) +  x/T(120) ) ) ) ); // O(x^6)
    return x + x * ( x/T(2) + x * ( x/T(6) + x * ( x/T(24) +  x*x/T(120) ) ) ); // O(x^6)
}

template<typename T>
constexpr
T
expm1_check(const T x)
noexcept
{
    return( // NaN check
            is_nan(x) ? \
                GCLIM<T>::quiet_NaN() :
            //
            abs(x) > T(1e-04) ? \
            // if
                exp(x) - T(1) :
            // else    
                expm1_compute(x) );
}

}

/**
 * Compile-time exponential-minus-1 function
 *
 * @param x a real-valued input.
 * @return \f$ \exp(x) - 1 \f$ using \f[ \exp(x) = \sum_{k=0}^\infty \dfrac{x^k}{k!} \f] 
 */

template<typename T>
constexpr
return_t<T>
expm1(const T x)
noexcept
{
    return internal::expm1_check( static_cast<return_t<T>>(x) );
}

#endif
