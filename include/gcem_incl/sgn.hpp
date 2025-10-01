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

#ifndef _gcem_sgn_HPP
#define _gcem_sgn_HPP

/**
 * Compile-time sign function
 *
 * @param x a real-valued input
 * @return a value \f$ y \f$ such that \f[ y = \begin{cases} 1 \ &\text{ if } x > 0 \\ 0 \ &\text{ if } x = 0 \\ -1 \ &\text{ if } x < 0 \end{cases} \f]
 */

template<typename T>
constexpr
int
sgn(const T x)
noexcept
{
    return( // positive
            x > T(0) ?  1 :
            // negative
            x < T(0) ? -1 :
            // else
                0 );
}

#endif
