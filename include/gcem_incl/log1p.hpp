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
 * compile-time natural logarithm(x+1) function
 */

#ifndef _gcem_log1p_HPP
#define _gcem_log1p_HPP

namespace internal
{

// see:
// http://functions.wolfram.com/ElementaryFunctions/Log/06/01/04/01/0003/


template<typename T>
constexpr
T
log1p_compute(const T x)
noexcept
{
    // return x * ( T(1) + x * ( -T(1)/T(2) +  x * ( T(1)/T(3) +  x * ( -T(1)/T(4) + x/T(5) ) ) ) ); // O(x^6)
    return x + x * ( - x/T(2) +  x * ( x/T(3) +  x * ( -x/T(4) + x*x/T(5) ) ) ); // O(x^6)
}

template<typename T>
constexpr
T
log1p_check(const T x)
noexcept
{
    return( // NaN check
            is_nan(x) ? \
                GCLIM<T>::quiet_NaN() :
            //
            abs(x) > T(1e-04) ? \
            // if
                log(T(1) + x) :
            // else    
                log1p_compute(x) );
}

}

/**
 * Compile-time natural-logarithm-plus-1 function
 *
 * @param x a real-valued input.
 * @return \f$ \log_e(x+1) \f$ using \f[ \log(x+1) = \sum_{k=1}^\infty \dfrac{(-1)^{k-1}x^k}{k}, \ \ |x| < 1 \f] 
 */

template<typename T>
constexpr
return_t<T>
log1p(const T x)
noexcept
{
    return internal::log1p_check( static_cast<return_t<T>>(x) );
}

#endif
