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
    print_begin("tgamma");

    //

    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma,  1.5L);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma,  2.7L);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma,  3.0L);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma,  4.0L);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma,  5.0L);

    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma, TEST_NAN);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma, 1.0);

    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma, 0.9);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma, 0.1);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma, 0.001);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma, 0.0);

    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma, -0.1);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma, -1);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma, -1.1);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma, -2);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma, -3);
    GCEM_TEST_COMPARE_VALS(gcem::tgamma,std::tgamma, -4);

    //

    print_final("tgamma");

    return 0;
}
