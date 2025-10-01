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
 * compile-time power function
 */

#ifndef _gcem_pow_HPP
#define _gcem_pow_HPP

namespace internal
{

template<typename T>
constexpr
T
pow_dbl(const T base, const T exp_term)
noexcept
{
    return exp(exp_term*log(base));
}

template<typename T1, typename T2, typename TC = common_t<T1,T2>, 
         typename std::enable_if<!std::is_integral<T2>::value>::type* = nullptr>
constexpr
TC
pow_check(const T1 base, const T2 exp_term)
noexcept
{
    return( base < T1(0) ? \
                GCLIM<TC>::quiet_NaN() :
            //
            pow_dbl(static_cast<TC>(base),static_cast<TC>(exp_term)) );
}

template<typename T1, typename T2, typename TC = common_t<T1,T2>, 
         typename std::enable_if<std::is_integral<T2>::value>::type* = nullptr>
constexpr
TC
pow_check(const T1 base, const T2 exp_term)
noexcept
{
    return pow_integral(base,exp_term);
}

}

/**
 * Compile-time power function
 *
 * @param base a real-valued input. 
 * @param exp_term a real-valued input.
 * @return Computes \c base raised to the power \c exp_term. In the case where \c exp_term is integral-valued, recursion by squaring is used, otherwise \f$ \text{base}^{\text{exp\_term}} = e^{\text{exp\_term} \log(\text{base})} \f$
 */

template<typename T1, typename T2>
constexpr
common_t<T1,T2>
pow(const T1 base, const T2 exp_term)
noexcept
{
    return internal::pow_check(base,exp_term);
}

#endif
