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
    print_begin("rounding");

    //

    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor,  0.0);
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor, -0.0);
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor,  4.2);
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor,  4.5);
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor,  4.7);
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor,  5.0);
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor, -4.2);
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor, -4.7);
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor, -5.0);
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor, 42e32);
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor, -42e32);

    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor,  std::numeric_limits<float>::max());
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor, -std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor,  std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::floor,std::floor,  std::numeric_limits<long double>::quiet_NaN());

    //

    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil,  0.0);
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil, -0.0);
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil,  4.2);
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil,  4.5);
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil,  4.7);
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil,  5.0);
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil, -4.2);
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil, -4.7);
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil, -5.0);
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil, 42e32);
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil, -42e32);

    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil,  std::numeric_limits<float>::max());
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil, -std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil,  std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::ceil,std::ceil,  std::numeric_limits<long double>::quiet_NaN());

    //

    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc,  0.0);
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc, -0.0);
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc,  4.2);
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc,  4.5);
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc,  4.7);
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc,  5.0);
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc, -4.2);
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc, -4.7);
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc, -5.0);
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc, 42e32);
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc, -42e32);

    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc,  std::numeric_limits<float>::max());
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc, -std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc,  std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::trunc,std::trunc,  std::numeric_limits<long double>::quiet_NaN());

    //

    GCEM_TEST_COMPARE_VALS(gcem::round,std::round,  0.0);
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round, -0.0);
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round,  4.2);
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round,  4.5);
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round,  4.7);
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round,  5.0);
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round, -4.2);
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round, -4.5);
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round, -4.7);
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round, -5.0);
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round, 42e32);
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round, -42e32);

    GCEM_TEST_COMPARE_VALS(gcem::round,std::round,  std::numeric_limits<float>::max());
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round, -std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round,  std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::round,std::round,  std::numeric_limits<long double>::quiet_NaN());

    //

    print_final("rounding");

    return 0;
}
