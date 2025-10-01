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
 * compile-time mantissa function
 */

#ifndef _gcem_mantissa_HPP
#define _gcem_mantissa_HPP

namespace internal
{

template<typename T>
constexpr
T
mantissa(const T x)
noexcept
{
    return( x < T(1) ? \
                mantissa(x * T(10)) : 
            x > T(10) ? \
                mantissa(x / T(10)) :
            // else
                x );
}

}

#endif
