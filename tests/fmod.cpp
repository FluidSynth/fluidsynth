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

#define TEST_PRINT_PRECISION_1 2
#define TEST_PRINT_PRECISION_2 2

#include "gcem_tests.hpp"

int main()
{
    print_begin("fmod");

    //

    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod,   0.0,   0.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod,  -0.0,   0.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod,   0.0,  -0.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod,   5.3,   2.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod,  -5.3,   2.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod,   5.3,  -2.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod,  -5.3,  -2.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod,  18.5,   4.2);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, -18.5,   4.2);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod,  18.5,  -4.2);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, -18.5,  -4.2);

    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, -std::numeric_limits<long double>::infinity(),  0.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, -std::numeric_limits<long double>::infinity(), -1.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, -std::numeric_limits<long double>::infinity(),  1.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, -std::numeric_limits<long double>::infinity(),  std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, -std::numeric_limits<long double>::infinity(), -std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, -std::numeric_limits<long double>::infinity(),  std::numeric_limits<long double>::quiet_NaN());

    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, std::numeric_limits<long double>::infinity(),  0.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, std::numeric_limits<long double>::infinity(), -1.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, std::numeric_limits<long double>::infinity(),  1.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, std::numeric_limits<long double>::infinity(),  std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, std::numeric_limits<long double>::infinity(), -std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, std::numeric_limits<long double>::infinity(),  std::numeric_limits<long double>::quiet_NaN());

    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, std::numeric_limits<long double>::quiet_NaN(),  0.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, std::numeric_limits<long double>::quiet_NaN(), -1.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, std::numeric_limits<long double>::quiet_NaN(),  1.0);
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, std::numeric_limits<long double>::quiet_NaN(),  std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, std::numeric_limits<long double>::quiet_NaN(), -std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::fmod,std::fmod, std::numeric_limits<long double>::quiet_NaN(),  std::numeric_limits<long double>::quiet_NaN());

    //

    print_final("fmod");

    return 0;
}
