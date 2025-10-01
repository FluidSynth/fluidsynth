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

#ifndef _gcem_signbit_HPP
#define _gcem_signbit_HPP

/**
 * Compile-time sign bit detection function
 *
 * @param x a real-valued input
 * @return return true if \c x is negative, otherwise return false.
 */

template <typename T>
constexpr 
bool
signbit(const T x)
noexcept
{
#ifdef _MSC_VER
    return( (x == T(0)) ? (_fpclass(x) == _FPCLASS_NZ) : (x < T(0)) );
#else
    return GCEM_SIGNBIT(x);
#endif
}

#endif
