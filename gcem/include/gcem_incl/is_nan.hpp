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
 * compile-time check if a float is NaN-valued
 */

#ifndef _gcem_is_nan_HPP
#define _gcem_is_nan_HPP

namespace internal
{

// future: consider using __builtin_isnan(__x)

template<typename T>
constexpr
bool
is_nan(const T x)
noexcept
{
    return x != x;
}

template<typename T1, typename T2>
constexpr
bool
any_nan(const T1 x, const T2 y)
noexcept
{
    return( is_nan(x) || is_nan(y) );
}

template<typename T1, typename T2>
constexpr
bool
all_nan(const T1 x, const T2 y)
noexcept
{
    return( is_nan(x) && is_nan(y) );
}

template<typename T1, typename T2, typename T3>
constexpr
bool
any_nan(const T1 x, const T2 y, const T3 z)
noexcept
{
    return( is_nan(x) || is_nan(y) || is_nan(z) );
}

template<typename T1, typename T2, typename T3>
constexpr
bool
all_nan(const T1 x, const T2 y, const T3 z)
noexcept
{
    return( is_nan(x) && is_nan(y) && is_nan(z) );
}

}

#endif
