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
    print_begin("exp");

    //

    GCEM_TEST_COMPARE_VALS(gcem::exp,std::exp, -40.0L);
    GCEM_TEST_COMPARE_VALS(gcem::exp,std::exp, -4.0L);
    GCEM_TEST_COMPARE_VALS(gcem::exp,std::exp,  0.0001L);
    GCEM_TEST_COMPARE_VALS(gcem::exp,std::exp,  1.0L);
    GCEM_TEST_COMPARE_VALS(gcem::exp,std::exp,  1.75L);
    GCEM_TEST_COMPARE_VALS(gcem::exp,std::exp,  1.9991L);
    GCEM_TEST_COMPARE_VALS(gcem::exp,std::exp,  2.1L);
    GCEM_TEST_COMPARE_VALS(gcem::exp,std::exp,  4.0L);
    
    GCEM_TEST_COMPARE_VALS(gcem::exp,std::exp, -std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::exp,std::exp,  std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::exp,std::exp,  std::numeric_limits<long double>::quiet_NaN());

    //

    print_final("exp");

    return 0;
}
