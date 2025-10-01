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

#ifndef _gcem_lbeta_HPP
#define _gcem_lbeta_HPP

/**
 * Compile-time log-beta function
 *
 * @param a a real-valued input.
 * @param b a real-valued input.
 * @return the log-beta function using \f[ \ln \text{B}(\alpha,\beta) := \ln \int_0^1 t^{\alpha - 1} (1-t)^{\beta - 1} dt = \ln \Gamma(\alpha) + \ln \Gamma(\beta) - \ln \Gamma(\alpha + \beta) \f]
 * where \f$ \Gamma \f$ denotes the gamma function.
 */

template<typename T1, typename T2>
constexpr
common_return_t<T1,T2>
lbeta(const T1 a, const T2 b)
noexcept
{
    return( (lgamma(a) + lgamma(b)) - lgamma(a+b) );
}

#endif
