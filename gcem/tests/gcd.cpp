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

#define TEST_VAL_TYPES int

#include "gcem_tests.hpp"

int main()
{
    print_begin("gcd");

    //

    GCEM_TEST_EXPECTED_VAL(gcem::gcd,6,12,18);
    GCEM_TEST_EXPECTED_VAL(gcem::gcd,2,-4,14);
    GCEM_TEST_EXPECTED_VAL(gcem::gcd,14,42,56);

    //

    print_final("gcd");

    return 0;
}
