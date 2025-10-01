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

#ifndef _gcem_copysign_HPP
#define _gcem_copysign_HPP

/**
 * Compile-time copy sign function
 *
 * @param x a real-valued input
 * @param y a real-valued input
 * @return replace the signbit of \c x with the signbit of \c y.
 */

template <typename T1, typename T2>
constexpr
T1
copysign(const T1 x, const T2 y)
noexcept
{
    return( signbit(x) != signbit(y) ? -x : x );
}

#endif
