.. Copyright (c) 2016-2024 Keith O'Hara

   Distributed under the terms of the Apache License, Version 2.0.

   The full license is in the file LICENSE, distributed with this software.

Introduction
============

GCE-Math (\ **G**\ eneralized **C**\ onstant **E**\ xpression Math) is a templated C++ library enabling compile-time computation of mathematical functions.

* The library is written in C++11 ``constexpr`` format, and is C++11/14/17/20 compatible.
* Continued fraction and series expansions are implemented using recursive templates.
* The ``gcem::`` syntax is identical to that of the C++ standard library (``std::``).
* Tested and accurate to floating-point precision against the C++ standard library.
* Released under a permissive, non-GPL license.

Author: Keith O'Hara

License: Apache 2.0

Status
------

The library is actively maintained, and is still being extended. A list of features includes:

* basic library functions:
    - ``abs``, ``max``, ``min``, ``pow``, ``sqrt``, ``inv_sqrt``
    - ``ceil``, ``floor``, ``round``, ``trunc``, ``fmod``,
    - ``exp``, ``expm1``, ``log``, ``log1p``, ``log2``, ``log10`` and more
* trigonometric functions:
    - basic: ``cos``, ``sin``, ``tan``
    - inverse: ``acos``, ``asin``, ``atan``, ``atan2``
* hyperbolic (area) functions: 
    - ``cosh``, ``sinh``, ``tanh``, ``acosh``, ``asinh``, ``atanh``
* algorithms:
    - ``gcd``, ``lcm``
* special functions:
    - factorials and the binomial coefficient: ``factorial``, ``binomial_coef``
    - beta, gamma, and multivariate gamma functions: ``beta``, ``lbeta``, ``lgamma``, ``tgamma``, ``lmgamma``
    - the Gaussian error function and inverse error function: ``erf``, ``erf_inv``
    - (regularized) incomplete beta and incomplete gamma functions: ``incomplete_beta``, ``incomplete_gamma``
    - inverse incomplete beta and incomplete gamma functions: ``incomplete_beta_inv``, ``incomplete_gamma_inv``

General Syntax
--------------

GCE-Math functions are written as C++ templates with ``constexpr`` specifiers. For example, the `Gaussian error function <https://en.wikipedia.org/wiki/Error_function>`_ (``erf``) is defined as:

.. code:: cpp

    template<typename T>
    constexpr
    return_t<T>
    erf(const T x) noexcept;

A set of internal templated ``constexpr`` functions will implement a continued fraction expansion and return a value of type ``return_t<T>``. The output type ('``return_t<T>``') is generally determined by the input type, e.g., ``int``, ``float``, ``double``, ``long double``, etc; when ``T`` is an intergral type, the output will be upgraded to ``return_t<T> = double``, otherwise ``return_t<T> = T``. For types not covered by ``std::is_integral``, recasts should be used.


Contents
--------

.. toctree::
   :caption: EXAMPLES
   :maxdepth: 2

   examples

.. toctree::
   :titlesonly:
   :caption: API REFERENCE
   :maxdepth: 2
   
   api/math_index
