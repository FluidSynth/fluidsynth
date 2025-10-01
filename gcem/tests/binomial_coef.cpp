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

#include "gcem_tests.hpp"

int main()
{
    print_begin("binomial_coef");

    //

    GCEM_TEST_EXPECTED_VAL(gcem::binomial_coef,  1,  0,  0);
    GCEM_TEST_EXPECTED_VAL(gcem::binomial_coef,  0,  0,  1);
    GCEM_TEST_EXPECTED_VAL(gcem::binomial_coef,  1,  1,  0);
    GCEM_TEST_EXPECTED_VAL(gcem::binomial_coef,  1,  1,  1);
    GCEM_TEST_EXPECTED_VAL(gcem::binomial_coef, 10,  5,  2);
    GCEM_TEST_EXPECTED_VAL(gcem::binomial_coef, 45, 10,  8);
    GCEM_TEST_EXPECTED_VAL(gcem::binomial_coef, 10, 10,  9);
    GCEM_TEST_EXPECTED_VAL(gcem::binomial_coef,  1, 10, 10);

    //

    // GCEM_TEST_EXPECTED_VAL(gcem::binomial_coef, TEST_NAN, TEST_NAN,     1.0L);
    // GCEM_TEST_EXPECTED_VAL(gcem::binomial_coef, TEST_NAN,     1.0L, TEST_NAN);
    // GCEM_TEST_EXPECTED_VAL(gcem::binomial_coef, TEST_NAN, TEST_NAN, TEST_NAN);

    //

    print_final("binomial_coef");

    return 0;
}
