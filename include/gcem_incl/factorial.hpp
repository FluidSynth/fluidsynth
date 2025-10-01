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
 * compile-time factorial function
 */

#ifndef _gcem_factorial_HPP
#define _gcem_factorial_HPP

namespace internal
{

// T should be int, long int, unsigned int, etc.

template<typename T>
constexpr
T
factorial_table(const T x)
noexcept
{   // table for x! when x = {0, ..., 20}
    return( x == T(0)  ? T(1)     : x == T(1)  ? T(1)      :
            x == T(2)  ? T(2)     : x == T(3)  ? T(6)      :
            x == T(4)  ? T(24)    : x == T(5)  ? T(120)    :
            x == T(6)  ? T(720)   : x == T(7)  ? T(5040)   :
            x == T(8)  ? T(40320) : x == T(9)  ? T(362880) :
            //
            x == T(10) ? T(3628800)             : 
            x == T(11) ? T(39916800)            :
            x == T(12) ? T(479001600)           :
            x == T(13) ? T(6227020800)          : 
            x == T(14) ? T(87178291200)         : 
            x == T(15) ? T(1307674368000)       : 
            x == T(16) ? T(20922789888000)      :
            x == T(17) ? T(355687428096000)     :
            x == T(18) ? T(6402373705728000)    :
            x == T(19) ? T(121645100408832000)  :
                         T(2432902008176640000) );
}

template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
constexpr
T
factorial_recur(const T x)
noexcept
{
    return( x < T(21) ? \
                // if
                factorial_table(x) :
                // else (but overflow is almost guaranteed here)
                x * factorial_recur(x - 1) );
}

template<typename T, typename std::enable_if<!std::is_integral<T>::value>::type* = nullptr>
constexpr
T
factorial_recur(const T x)
noexcept
{
    return tgamma(x + 1);
}

}

/**
 * Compile-time factorial function
 *
 * @param x a real-valued input.
 * @return Computes the factorial value \f$ x! \f$. 
 * When \c x is an integral type (\c int, <tt>long int</tt>, etc.), a simple recursion method is used, along with table values.
 * When \c x is real-valued, <tt>factorial(x) = tgamma(x+1)</tt>.
 */

template<typename T>
constexpr
T
factorial(const T x)
noexcept
{
    return internal::factorial_recur(x);
}

#endif
