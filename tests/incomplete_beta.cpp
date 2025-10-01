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
    print_begin("incomplete_beta");

    //

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta, 0.11464699677582491921L,     0.9L,  0.9L,  0.1L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta, 0.78492840804657726395L,     0.9L,  0.9L,  0.8L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta, 0.80000000000000004441L,     1.0L,  1.0L,  0.8L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta, 0.89600000000000001865L,     2.0L,  2.0L,  0.8L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta, 0.81920000000000003926L,     3.0L,  2.0L,  0.8L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta, 0.97279999999999999805L,     2.0L,  3.0L,  0.8L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta, 3.9970000000000084279e-09L,  3.0L,  2.0L,  0.001L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta, 0.35200000000000003508L,     2.0L,  2.0L,  0.4L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta, TEST_NAN, TEST_NAN, TEST_NAN, TEST_NAN);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta, TEST_NAN, TEST_NAN,     2.0L,     0.8L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta, TEST_NAN,     2.0L, TEST_NAN,     0.8L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_beta, TEST_NAN,     2.0L,     2.0L, TEST_NAN);

    //

    print_final("incomplete_beta");

    return 0;
}
