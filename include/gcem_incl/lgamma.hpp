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
 * compile-time log-gamma function
 * 
 * for coefficient values, see:
 * http://my.fit.edu/~gabdo/gamma.txt
 */

#ifndef _gcem_lgamma_HPP
#define _gcem_lgamma_HPP

namespace internal
{

// P. Godfrey's coefficients:
//
//  0.99999999999999709182
//  57.156235665862923517
// -59.597960355475491248
//  14.136097974741747174
//  -0.49191381609762019978
//    .33994649984811888699e-4
//    .46523628927048575665e-4
//   -.98374475304879564677e-4
//    .15808870322491248884e-3
//   -.21026444172410488319e-3
//    .21743961811521264320e-3
//   -.16431810653676389022e-3
//    .84418223983852743293e-4
//   -.26190838401581408670e-4
//    .36899182659531622704e-5

constexpr
long double
lgamma_coef_term(const long double x)
noexcept
{
    return(     0.99999999999999709182L             + 57.156235665862923517L      / (x+1)  \
             - 59.597960355475491248L      / (x+2)  + 14.136097974741747174L      / (x+3)  \
             -  0.49191381609762019978L    / (x+4)  +   .33994649984811888699e-4L / (x+5)  \
             +   .46523628927048575665e-4L / (x+6)  -   .98374475304879564677e-4L / (x+7)  \
             +   .15808870322491248884e-3L / (x+8)  -   .21026444172410488319e-3L / (x+9)  \
             +   .21743961811521264320e-3L / (x+10) -   .16431810653676389022e-3L / (x+11) \
             +   .84418223983852743293e-4L / (x+12) -   .26190838401581408670e-4L / (x+13) \
             +   .36899182659531622704e-5L / (x+14) );
}

template<typename T>
constexpr
T
lgamma_term_2(const T x)
noexcept
{ //
    return( T(GCEM_LOG_SQRT_2PI) + log(T(lgamma_coef_term(x))) );
}

template<typename T>
constexpr
T
lgamma_term_1(const T x)
noexcept
{   // note: 607/128 + 0.5 = 5.2421875
    return( (x + T(0.5))*log(x + T(5.2421875L)) - (x + T(5.2421875L)) );
}

template<typename T>
constexpr
T
lgamma_begin(const T x)
noexcept
{   // returns lngamma(x+1)
    return( lgamma_term_1(x) + lgamma_term_2(x) );
}

template<typename T>
constexpr
T
lgamma_check(const T x)
noexcept
{
    return( // NaN check
            is_nan(x) ? \
                GCLIM<T>::quiet_NaN() :
            // indistinguishable from one or <= zero
            GCLIM<T>::min() > abs(x - T(1)) ? \
                T(0) :
            GCLIM<T>::min() > x ? \
                GCLIM<T>::infinity() :
            // else
                lgamma_begin(x - T(1)) );
}

}

/**
 * Compile-time log-gamma function
 *
 * @param x a real-valued input.
 * @return computes the log-gamma function
 * \f[ \ln \Gamma(x) = \ln \int_0^\infty y^{x-1} \exp(-y) dy \f]
 * using a polynomial form:
 * \f[ \Gamma(x+1) \approx (x+g+0.5)^{x+0.5} \exp(-x-g-0.5) \sqrt{2 \pi} \left[ c_0 + \frac{c_1}{x+1} + \frac{c_2}{x+2} + \cdots + \frac{c_n}{x+n} \right] \f]
 * where the value \f$ g \f$ and the coefficients \f$ (c_0, c_1, \ldots, c_n) \f$
 * are taken from Paul Godfrey, whose note can be found here: http://my.fit.edu/~gabdo/gamma.txt
 */

template<typename T>
constexpr
return_t<T>
lgamma(const T x)
noexcept
{
    return internal::lgamma_check( static_cast<return_t<T>>(x) );
}

#endif
