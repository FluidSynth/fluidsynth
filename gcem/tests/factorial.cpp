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

#define TEST_ERR_TOL 1e-02
#define TEST_PRINT_PRECISION_1 6
#define TEST_PRINT_PRECISION_2 18

#include "gcem_tests.hpp"

int main()
{
    print_begin("factorial");

    std::function<long double (long double)> std_fn  = [] (long double x) -> long double { return std::tgamma(x+1); };

    //

    // using long doubles -> tgamma
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 3.1L);
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 7.0L);
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 10.0L);
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 12.0L);
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 14.0L);
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 15.0L);

    // using long ints
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 5L);
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 9L);
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 11L);
#ifndef _WIN32
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 13L);
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 16L);
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 18L);
    GCEM_TEST_COMPARE_VALS(gcem::factorial,std_fn, 20L);
#endif

    //

    print_final("factorial");

    return 0;
}
