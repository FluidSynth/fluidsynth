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
    print_begin("fabsl");

    // note: we use std::fabs instead of std::fabsl due to
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79700

    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, 0.0);
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs,-0.0);
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, 1.0);
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs,-1.0);

    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, 0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs,-0.0L);
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, 1.0L);
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs,-1.0L);

    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, std::numeric_limits<double>::lowest());
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, std::numeric_limits<double>::min());
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, std::numeric_limits<double>::max());

    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, std::numeric_limits<long double>::lowest());
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, std::numeric_limits<long double>::min());
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, std::numeric_limits<long double>::max());

    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, std::numeric_limits<double>::quiet_NaN());
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, std::numeric_limits<double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, -std::numeric_limits<double>::infinity());

    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, std::numeric_limits<long double>::quiet_NaN());
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, std::numeric_limits<long double>::infinity());
    GCEM_TEST_COMPARE_VALS(gcem::fabsl,std::fabs, -std::numeric_limits<long double>::infinity());

    //

    print_final("fabsl");

    return 0;
}
