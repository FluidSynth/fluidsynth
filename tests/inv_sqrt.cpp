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

template<typename T>
constexpr
T
std_test_fn(const T x)
{
    return T(1) / std::sqrt(x);
}

int main()
{
    print_begin("sqrt");

    //

    GCEM_TEST_COMPARE_VALS(gcem::inv_sqrt, std_test_fn,  0.5L);
    GCEM_TEST_COMPARE_VALS(gcem::inv_sqrt, std_test_fn,  0.00199900000000000208L);
    GCEM_TEST_COMPARE_VALS(gcem::inv_sqrt, std_test_fn,  1.5L);
    GCEM_TEST_COMPARE_VALS(gcem::inv_sqrt, std_test_fn,  2.0L);
    GCEM_TEST_COMPARE_VALS(gcem::inv_sqrt, std_test_fn,  41.5L);
    GCEM_TEST_COMPARE_VALS(gcem::inv_sqrt, std_test_fn,  0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::inv_sqrt, std_test_fn, -1.0L);
    
    GCEM_TEST_COMPARE_VALS(gcem::inv_sqrt, std_test_fn, -std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::inv_sqrt, std_test_fn,  std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::inv_sqrt, std_test_fn,  std::numeric_limits<long double>::quiet_NaN());

    //

    print_final("sqrt");

    return 0;
}
