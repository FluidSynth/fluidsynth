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

// $CXX -O3 -Wall -std=c++11 -fconstexpr-depth=650 -fconstexpr-steps=1271242 -I./../include sqrt.cpp -o sqrt.test -framework Accelerate

#define TEST_PRINT_PRECISION_1 6
#define TEST_PRINT_PRECISION_2 18

#include "gcem_tests.hpp"

int main()
{
    print_begin("sqrt");

    //

    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt,  0.5L);
    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt,  0.00199900000000000208L);
    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt,  1.5L);
    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt,  2.0L);
    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt,  41.5L);
    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt,  123456789.5L);

    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt,  0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt, -1.0L);

    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt,  1e-500L);
    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt,  std::numeric_limits<long double>::min());
    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt,  std::numeric_limits<double>::max());
    
    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt, -std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt,  std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::sqrt,std::sqrt,  std::numeric_limits<long double>::quiet_NaN());

    // constexpr auto v = gcem::sqrt(std::numeric_limits<double>::max());

    //

    print_final("sqrt");

    return 0;
}
