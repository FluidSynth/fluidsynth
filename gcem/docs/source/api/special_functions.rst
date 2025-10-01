.. Copyright (c) 2016-2024 Keith O'Hara

   Distributed under the terms of the Apache License, Version 2.0.

   The full license is in the file LICENSE, distributed with this software.

Special functions
=================

**Table of contents**

.. contents:: :local:

----

Binomial function
-----------------

.. _binom-func-ref:
.. doxygenfunction:: binomial_coef(const T1, const T2)
   :project: gcem

.. _lbinom-ref:
.. doxygenfunction:: log_binomial_coef(const T1, const T2)
   :project: gcem

----

Beta function
-------------

.. _beta-function-reference:
.. doxygenfunction:: beta(const T1, const T2)
   :project: gcem

.. _lbeta-func-ref:
.. doxygenfunction:: lbeta(const T1, const T2)
   :project: gcem

----

Gamma function
--------------

.. _tgamma-func-ref:
.. doxygenfunction:: tgamma(const T)
   :project: gcem

.. _lgamma-func-ref:
.. doxygenfunction:: lgamma(const T)
   :project: gcem

.. _lmgamma-func-ref:
.. doxygenfunction:: lmgamma(const T1, const T2)
   :project: gcem

----

Incomplete integral functions
-----------------------------

.. _erf-function-reference:
.. doxygenfunction:: erf(const T)
   :project: gcem

.. _ib-func-ref:
.. doxygenfunction:: incomplete_beta(const T1, const T2, const T3)
   :project: gcem

.. _ig-func-ref:
.. doxygenfunction:: incomplete_gamma(const T1, const T2)
   :project: gcem

----

Inverse incomplete integral functions
-------------------------------------

.. _erf_inv-func-ref:
.. doxygenfunction:: erf_inv(const T)
   :project: gcem

.. _iib-ref:
.. doxygenfunction:: incomplete_beta_inv(const T1, const T2, const T3)
   :project: gcem

.. _iig-ref:
.. doxygenfunction:: incomplete_gamma_inv(const T1, const T2)
   :project: gcem
