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

template<typename T>
constexpr
int
std_test_fn(const T x)
{
    return static_cast<int>(std::signbit(x));
}

int main()
{
    print_begin("signbit");

    //

    GCEM_TEST_COMPARE_VALS(gcem::signbit,std_test_fn,  1.0);
    GCEM_TEST_COMPARE_VALS(gcem::signbit,std_test_fn, -1.0);

    GCEM_TEST_COMPARE_VALS(gcem::signbit,std_test_fn,  0.0);
    GCEM_TEST_COMPARE_VALS(gcem::signbit,std_test_fn, -0.0);
    
    GCEM_TEST_COMPARE_VALS(gcem::signbit,std_test_fn, TEST_POSINF);
    GCEM_TEST_COMPARE_VALS(gcem::signbit,std_test_fn, TEST_NEGINF);
    GCEM_TEST_COMPARE_VALS(gcem::signbit,std_test_fn, TEST_NAN);

    //

    print_final("signbit");

    return 0;
}
