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
 * compile-time arctangent function
 */

// see
// http://functions.wolfram.com/ElementaryFunctions/ArcTan/10/0001/
// http://functions.wolfram.com/ElementaryFunctions/ArcTan/06/01/06/01/0002/

#ifndef _gcem_atan_HPP
#define _gcem_atan_HPP

namespace internal
{

// Series

template<typename T>
constexpr
T
atan_series_order_calc(const T xx, const T x_pow, const uint_t order)
noexcept
{
    return( T(1)/( T((order-1)*4 - 1) * x_pow ) \
              - T(1)/( T((order-1)*4 + 1) * x_pow * xx) );
}

#if __cplusplus >= 201402L // C++14 version

template<typename T>
constexpr
T
atan_series_order(const T x, const T x_pow, const uint_t order_begin, const uint_t max_order)
noexcept
{
    // run in reverse order to sum smallest numbers first

    if (max_order == 1) {
        return GCEM_HALF_PI - T(1)/x_pow; // use x_pow to avoid a warning
    }

    T xx = x*x;
    T res = atan_series_order_calc(xx, pow(x,4*max_order-5), max_order);

    uint_t depth = max_order - 1;

    while (depth > order_begin) {
        res += atan_series_order_calc(xx, pow(x,4*depth-5), depth);

        --depth;
    }

    res += GCEM_HALF_PI - T(1)/x;

    return res;
}

#else // C++11 version

template<typename T>
constexpr
T
atan_series_order(const T x, const T x_pow, const uint_t order, const uint_t max_order)
noexcept
{
    return( max_order == 1 ? \
                T(GCEM_HALF_PI) - T(1)/x :
            order == 1 ? \
                T(GCEM_HALF_PI) - T(1)/x + atan_series_order(x*x,pow(x,3),order+1,max_order) :
            // NOTE: x changes to x*x for order > 1
            order < max_order ? \
                atan_series_order_calc(x,x_pow,order) \
                    + atan_series_order(x,x_pow*x*x,order+1,max_order) :
            // order == max_order
                atan_series_order_calc(x,x_pow,order) );
}

#endif

template<typename T>
constexpr
T
atan_series_main(const T x)
noexcept
{
    return( x < T(3)    ? atan_series_order(x,x,1U,10U) :  // O(1/x^39)
            x < T(4)    ? atan_series_order(x,x,1U,9U)  :  // O(1/x^35)
            x < T(5)    ? atan_series_order(x,x,1U,8U)  :  // O(1/x^31)
            x < T(7)    ? atan_series_order(x,x,1U,7U)  :  // O(1/x^27)
            x < T(11)   ? atan_series_order(x,x,1U,6U)  :  // O(1/x^23)
            x < T(25)   ? atan_series_order(x,x,1U,5U)  :  // O(1/x^19)
            x < T(100)  ? atan_series_order(x,x,1U,4U)  :  // O(1/x^15)
            x < T(1000) ? atan_series_order(x,x,1U,3U)  :  // O(1/x^11)
                          atan_series_order(x,x,1U,2U) );  // O(1/x^7)
}

// CF

#if __cplusplus >= 201402L // C++14 version

template<typename T>
constexpr
T
atan_cf_recur(const T xx, const uint_t depth_begin, const uint_t max_depth)
noexcept
{
    uint_t depth = max_depth - 1;
    T res = T(2*(depth+1) - 1);

    while (depth > depth_begin - 1) {
        res = T(2*depth - 1) + T(depth*depth) * xx / res;

        --depth;
    }

    return res;
}

#else // C++11 version

template<typename T>
constexpr
T
atan_cf_recur(const T xx, const uint_t depth, const uint_t max_depth)
noexcept
{
    return( depth < max_depth ? \
            // if
                T(2*depth - 1) + T(depth*depth) * xx / atan_cf_recur(xx,depth+1,max_depth) :
            // else
                T(2*depth - 1) );
}

#endif

template<typename T>
constexpr
T
atan_cf_main(const T x)
noexcept
{
    return( x < T(0.5) ? x/atan_cf_recur(x*x, 1U, 15U ) : 
            x < T(1)   ? x/atan_cf_recur(x*x, 1U, 25U ) : 
            x < T(1.5) ? x/atan_cf_recur(x*x, 1U, 35U ) : 
            x < T(2)   ? x/atan_cf_recur(x*x, 1U, 45U ) : 
                         x/atan_cf_recur(x*x, 1U, 52U ) );
}

// choose between series expansion and continued fraction

template<typename T>
constexpr
T
atan_begin(const T x)
noexcept
{
    return( x > T(2.5) ? atan_series_main(x) : atan_cf_main(x) );
}

// check input

template<typename T>
constexpr
T
atan_check(const T x)
noexcept
{
    return( // NaN check
            is_nan(x) ? \
                GCLIM<T>::quiet_NaN() :
            // indistinguishable from zero
            GCLIM<T>::min() > abs(x) ? \
                T(0) :
            // negative or positive
            x < T(0) ? \
                - atan_begin(-x) :
                  atan_begin( x) );
}

}

/**
 * Compile-time arctangent function
 *
 * @param x a real-valued input.
 * @return the inverse tangent function using \f[ \text{atan}(x) = \dfrac{x}{1 + \dfrac{x^2}{3 + \dfrac{4x^2}{5 + \dfrac{9x^2}{7 + \ddots}}}} \f]
 */

template<typename T>
constexpr
return_t<T>
atan(const T x)
noexcept
{
    return internal::atan_check( static_cast<return_t<T>>(x) );
}

#endif
