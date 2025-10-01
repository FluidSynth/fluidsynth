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

#define TEST_PRINT_PRECISION_1 3
#define TEST_PRINT_PRECISION_2 10

#include "gcem_tests.hpp"

int main()
{
    print_begin("erf");

    //

    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf, -3.0L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf, -2.5L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf, -2.11L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf, -2.099L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf, -2.0L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf, -1.3L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf, -0.7L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf, -0.1L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf,  0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf,  0.1L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf,  0.7L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf,  1.3L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf,  1.3L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf,  2.0L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf,  2.099L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf,  2.11L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf,  2.5L);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf,  3.0L);

    //

    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf, TEST_NAN);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf, TEST_POSINF);
    GCEM_TEST_COMPARE_VALS(gcem::erf, std::erf, TEST_NEGINF);

    //

    print_final("erf");

    return 0;
}
