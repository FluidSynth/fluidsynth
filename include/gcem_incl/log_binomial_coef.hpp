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

#ifndef _gcem_log_binomial_coef_HPP
#define _gcem_log_binomial_coef_HPP

namespace internal
{

template<typename T>
constexpr
T
log_binomial_coef_compute(const T n, const T k)
noexcept
{
    return( lgamma(n+1) - (lgamma(k+1) + lgamma(n-k+1)) );
}

template<typename T1, typename T2, typename TC = common_return_t<T1,T2>>
constexpr
TC
log_binomial_coef_type_check(const T1 n, const T2 k)
noexcept
{
    return log_binomial_coef_compute(static_cast<TC>(n),static_cast<TC>(k));
}

}

/**
 * Compile-time log binomial coefficient
 *
 * @param n integral-valued input.
 * @param k integral-valued input.
 * @return computes the log Binomial coefficient
 * \f[ \ln \frac{n!}{k!(n-k)!} = \ln \Gamma(n+1) - [ \ln \Gamma(k+1) + \ln \Gamma(n-k+1) ] \f]
 */

template<typename T1, typename T2>
constexpr
common_return_t<T1,T2>
log_binomial_coef(const T1 n, const T2 k)
noexcept
{
    return internal::log_binomial_coef_type_check(n,k);
}

#endif