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
    print_begin("atan2");

    //

    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,      0.0L,           0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,     -0.0L,           0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,      0.0L,          -0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,     -0.0L,          -0.0L);

    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,      0.2L,           0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,     -0.2L,           0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,      0.001L,         0.001L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,      0.49L,          0.49L);

    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,     -0.5L,          -0.5L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,      0.5L,          -0.5L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,     -0.5L,           0.5L);

    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,      9.6L,           8.4L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,      1.0L,           0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,      0.0L,           1.0L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,     -1.0L,           0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,      0.0L,          -1.0L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,      1.0L,           3.0L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,     -5.0L,           2.5L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,  -1000.0L,          -0.001L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,      0.1337L,  -123456.0L);

    //

    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2, TEST_NAN,     1.0L);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2,     1.0L, TEST_NAN);
    GCEM_TEST_COMPARE_VALS(gcem::atan2,std::atan2, TEST_NAN, TEST_NAN);

    //

    print_final("atan2");

    return 0;
}
