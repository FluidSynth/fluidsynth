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
    print_begin("lcm");

    //

    GCEM_TEST_EXPECTED_VAL(gcem::lcm,12,3,4);
    GCEM_TEST_EXPECTED_VAL(gcem::lcm,60,12,15);
    GCEM_TEST_EXPECTED_VAL(gcem::lcm,1377,17,81);

    //

    print_final("lcm");

    return 0;
}
