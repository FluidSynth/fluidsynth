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

#include "gcem_tests.hpp"

// test misc functions

int main()
{
    
    GCEM_TEST_EXPECTED_VAL(gcem::sgn,  1,  1.5);
    GCEM_TEST_EXPECTED_VAL(gcem::sgn, -1, -1.5);

    GCEM_TEST_EXPECTED_VAL(gcem::internal::find_fraction, -0.5,  1.5);
    GCEM_TEST_EXPECTED_VAL(gcem::internal::find_fraction,  1.5, -1.5);

    GCEM_TEST_EXPECTED_VAL(gcem::internal::find_whole,  1,  1.5);
    GCEM_TEST_EXPECTED_VAL(gcem::internal::find_whole, -3, -1.5);

    GCEM_TEST_EXPECTED_VAL(gcem::internal::neg_zero, 0,  0.0);
    GCEM_TEST_EXPECTED_VAL(gcem::internal::neg_zero, 1,  -0.0);

    return 0;
}
