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

#ifndef _gcem_pow_integral_HPP
#define _gcem_pow_integral_HPP

namespace internal
{

template<typename T1, typename T2>
constexpr T1 pow_integral_compute(const T1 base, const T2 exp_term) noexcept;

// integral-valued powers using method described in 
// https://en.wikipedia.org/wiki/Exponentiation_by_squaring

template<typename T1, typename T2>
constexpr
T1
pow_integral_compute_recur(const T1 base, const T1 val, const T2 exp_term)
noexcept
{
    return( exp_term > T2(1) ? \
                (is_odd(exp_term) ? \
                    pow_integral_compute_recur(base*base,val*base,exp_term/2) :
                    pow_integral_compute_recur(base*base,val,exp_term/2)) :
                (exp_term == T2(1) ? val*base : val) );
}

template<typename T1, typename T2, typename std::enable_if<std::is_signed<T2>::value>::type* = nullptr>
constexpr
T1
pow_integral_sgn_check(const T1 base, const T2 exp_term)
noexcept
{
    return( exp_term < T2(0) ? \
            //
                T1(1) / pow_integral_compute(base, - exp_term) : 
            //
                pow_integral_compute_recur(base,T1(1),exp_term) );
}

template<typename T1, typename T2, typename std::enable_if<!std::is_signed<T2>::value>::type* = nullptr>
constexpr
T1
pow_integral_sgn_check(const T1 base, const T2 exp_term)
noexcept
{
    return( pow_integral_compute_recur(base,T1(1),exp_term) );
}

template<typename T1, typename T2>
constexpr
T1
pow_integral_compute(const T1 base, const T2 exp_term)
noexcept
{
    return( exp_term == T2(3) ? \
                base*base*base :
            exp_term == T2(2) ? \
                base*base :
            exp_term == T2(1) ? \
                base :
            exp_term == T2(0) ? \
                T1(1) :
            // check for overflow
            exp_term == GCLIM<T2>::min() ? \
                T1(0) :
            exp_term == GCLIM<T2>::max() ? \
                GCLIM<T1>::infinity() :
            // else
            pow_integral_sgn_check(base,exp_term) );
}

template<typename T1, typename T2, typename std::enable_if<std::is_integral<T2>::value>::type* = nullptr>
constexpr
T1
pow_integral_type_check(const T1 base, const T2 exp_term)
noexcept
{
    return pow_integral_compute(base,exp_term);
}

template<typename T1, typename T2, typename std::enable_if<!std::is_integral<T2>::value>::type* = nullptr>
constexpr
T1
pow_integral_type_check(const T1 base, const T2 exp_term)
noexcept
{
    // return GCLIM<return_t<T1>>::quiet_NaN();
    return pow_integral_compute(base,static_cast<llint_t>(exp_term));
}

//
// main function

template<typename T1, typename T2>
constexpr
T1
pow_integral(const T1 base, const T2 exp_term)
noexcept
{
    return internal::pow_integral_type_check(base,exp_term);
}

}

#endif
