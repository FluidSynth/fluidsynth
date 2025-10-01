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

#define TEST_PRINT_PRECISION_1 5
#define TEST_PRINT_PRECISION_2 18

#include "gcem_tests.hpp"

int main()
{
    print_begin("pow");

    //

    GCEM_TEST_COMPARE_VALS(gcem::pow,std::pow, 0.199900000000000208L, 3.5L);
    GCEM_TEST_COMPARE_VALS(gcem::pow,std::pow, 0.5L,  2.0L);
    GCEM_TEST_COMPARE_VALS(gcem::pow,std::pow, 1.5L,  0.99L);
    GCEM_TEST_COMPARE_VALS(gcem::pow,std::pow, 2.0L,  1.0L);
    GCEM_TEST_COMPARE_VALS(gcem::pow,std::pow, 41.5L, 7.0L);

    // int versions
    GCEM_TEST_COMPARE_VALS(gcem::pow,std::pow, 0.5L,  2L);
    GCEM_TEST_COMPARE_VALS(gcem::pow,std::pow, 41.5L, 7L);
    
    GCEM_TEST_COMPARE_VALS(gcem::pow,std::pow,  std::numeric_limits<long double>::quiet_NaN(), 2);
    GCEM_TEST_COMPARE_VALS(gcem::pow,std::pow,  2, std::numeric_limits<long double>::quiet_NaN());

    GCEM_TEST_COMPARE_VALS(gcem::pow,std::pow, std::numeric_limits<long double>::infinity(), 2);
    GCEM_TEST_COMPARE_VALS(gcem::pow,std::pow, std::numeric_limits<long double>::infinity(), - 2);
    GCEM_TEST_COMPARE_VALS(gcem::pow,std::pow, std::numeric_limits<long double>::infinity(), 0);

    //

    print_final("pow");

    return 0;
}
