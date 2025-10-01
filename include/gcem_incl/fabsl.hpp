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

#ifndef _gcem_fabsl_HPP
#define _gcem_fabsl_HPP

/**
 * Compile-time floating-point absolute value function
 *
 * @param x a real-valued input.
 * @return the absolute value of \c x, \f$ |x| \f$, where the return type is a floating point number (long double only).
 */

template<typename T>
constexpr
long double
fabsl(const T x)
noexcept
{
    return gcem::abs( static_cast<long double>(x) );
}

#endif
