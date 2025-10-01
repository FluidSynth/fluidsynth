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

#define TEST_ERR_TOL 1e-12

#define TEST_PRINT_PRECISION_1 6
#define TEST_PRINT_PRECISION_2 18

#include "gcem_tests.hpp"

int main()
{
    print_begin("erf_inv");

    //

    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv, -3.0L,  -0.999977909503001415L);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv, -2.5L,  -0.999593047982555041L);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv, -2.11L, -0.997154845031177802L);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv, -2.05L, -0.996258096044456873L);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv, -1.3L,  -0.934007944940652437L);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv,  0.0L,   0.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv,  1.3L,   0.934007944940652437L);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv,  2.05L,  0.996258096044456873L);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv,  2.11L,  0.997154845031177802L);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv,  2.5L,   0.999593047982555041L);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv,  3.0L,   0.999977909503001415L);

    //

    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv, TEST_NAN, TEST_NAN);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv, TEST_NAN, -1.1);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv, TEST_NAN,  1.1);

    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv, TEST_POSINF, 1);
    GCEM_TEST_EXPECTED_VAL(gcem::erf_inv, TEST_NEGINF, -1);

    //

    print_final("erf_inv");

    return 0;
}
