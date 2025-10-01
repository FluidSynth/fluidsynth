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

// g++-mp-7 -O3 -Wall -std=c++11 -fconstexpr-depth=20 -fconstexpr-steps=1271242 -I./../include log.cpp -o log.test -framework Accelerate

#define TEST_PRINT_PRECISION_1 6
#define TEST_PRINT_PRECISION_2 18

#include "gcem_tests.hpp"

int main()
{
    print_begin("log");

    //

    GCEM_TEST_COMPARE_VALS(gcem::log,std::log,  0.5L);
    GCEM_TEST_COMPARE_VALS(gcem::log,std::log,  0.00199900000000000208L);
    GCEM_TEST_COMPARE_VALS(gcem::log,std::log,  1.0L);
    GCEM_TEST_COMPARE_VALS(gcem::log,std::log,  1.5L);
    GCEM_TEST_COMPARE_VALS(gcem::log,std::log,  41.5L);
    GCEM_TEST_COMPARE_VALS(gcem::log,std::log,  123456789.5L);

    GCEM_TEST_COMPARE_VALS(gcem::log,std::log,  0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::log,std::log, -1.0L);

    GCEM_TEST_COMPARE_VALS(gcem::log,std::log,  1e-500L);
    GCEM_TEST_COMPARE_VALS(gcem::log,std::log,  std::numeric_limits<long double>::min());
    GCEM_TEST_COMPARE_VALS(gcem::log,std::log,  std::numeric_limits<double>::max());
    
    GCEM_TEST_COMPARE_VALS(gcem::log,std::log, -std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::log,std::log,  std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::log,std::log,  std::numeric_limits<long double>::quiet_NaN());

    //

    print_final("log");

    return 0;
}
