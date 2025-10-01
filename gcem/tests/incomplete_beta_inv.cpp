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

#define TEST_PRINT_PRECISION_1 3
#define TEST_PRINT_PRECISION_2 18

#include "gcem_tests.hpp"

int main()
{
    print_begin("incomplete_beta_inv");

    //

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta_inv, 0.085977260425697907276L,  0.9L,  0.9L,  0.1L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta_inv, 0.81533908558467627081L,   0.9L,  0.9L,  0.8L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta_inv, 0.80000000000000004441L,   1.0L,  1.0L,  0.8L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta_inv, 0.71285927458325959449L,   2.0L,  2.0L,  0.8L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta_inv, 0.78768287172204876079L,   3.0L,  2.0L,  0.8L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta_inv, 0.58245357452433332845L,   2.0L,  3.0L,  0.8L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta_inv, 0.064038139102833388505L,  3.0L,  2.0L,  0.001L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta_inv, 0.43293107714773171324L,   2.0L,  2.0L,  0.4L);

    //

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta_inv, TEST_NAN,  TEST_NAN,      3.0L,      0.8L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta_inv, TEST_NAN,      3.0L,  TEST_NAN,      0.8L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta_inv, TEST_NAN,      3.0L,      3.0L,  TEST_NAN);

    //

    print_final("incomplete_beta_inv");

    return 0;
}
