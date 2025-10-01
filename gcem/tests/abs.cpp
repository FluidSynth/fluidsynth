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

#define TEST_PRINT_PRECISION_1 6
#define TEST_PRINT_PRECISION_2 18

#include "gcem_tests.hpp"

int main()
{
    print_begin("abs");

    //

    GCEM_TEST_COMPARE_VALS(gcem::abs,std::abs, 0.0);
    GCEM_TEST_COMPARE_VALS(gcem::abs,std::abs,-0.0);
    GCEM_TEST_COMPARE_VALS(gcem::abs,std::abs, 1.0);
    GCEM_TEST_COMPARE_VALS(gcem::abs,std::abs,-1.0);
    GCEM_TEST_COMPARE_VALS(gcem::abs,std::abs, 0);
    GCEM_TEST_COMPARE_VALS(gcem::abs,std::abs, 1);
    GCEM_TEST_COMPARE_VALS(gcem::abs,std::abs,-1);

    GCEM_TEST_COMPARE_VALS(gcem::abs,std::abs, INT64_MIN);
    GCEM_TEST_COMPARE_VALS(gcem::abs,std::abs, INT64_MAX);

    GCEM_TEST_COMPARE_VALS(gcem::abs,std::abs, TEST_NAN);
    GCEM_TEST_COMPARE_VALS(gcem::abs,std::abs, TEST_NEGINF);
    GCEM_TEST_COMPARE_VALS(gcem::abs,std::abs, TEST_POSINF);

    //

    print_final("abs");

    return 0;
}
