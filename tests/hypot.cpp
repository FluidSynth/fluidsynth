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
    print_begin("hypot");

    //

    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,      0.0L,           0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,     -0.0L,           0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,      0.0L,          -0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,     -0.0L,          -0.0L);

    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,      0.2L,           0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,     -0.2L,           0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,      0.001L,         0.001L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,      0.49L,          0.49L);

    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,     -0.5L,          -0.5L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,      0.5L,          -0.5L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,     -0.5L,           0.5L);

    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,      9.6L,           8.4L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,      1.0L,           0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,      0.0L,           1.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,     -1.0L,           0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,      0.0L,          -1.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,      1.0L,           3.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,     -5.0L,           2.5L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,  -1000.0L,          -0.001L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,      0.1337L,  -123456.0L);

    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot, TEST_POSINF,        2.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,        2.0L, TEST_POSINF);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot, TEST_POSINF, TEST_POSINF);

    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot, TEST_NEGINF,        2.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,        2.0L, TEST_NEGINF);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot, TEST_NEGINF, TEST_NEGINF);

    //

    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot, TEST_NAN,     1.0L);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot,     1.0L, TEST_NAN);
    GCEM_TEST_COMPARE_VALS(gcem::hypot,std::hypot, TEST_NAN, TEST_NAN);

    //

    print_final("hypot");

    return 0;
}
