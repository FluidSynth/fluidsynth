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

int main()
{
    std::cout << "\n*** begin is_odd test ***\n" << std::endl;

    //

    int run_val = 0;

    run_val += gcem::internal::is_odd(1);
    run_val += gcem::internal::is_odd(3);
    run_val += gcem::internal::is_odd(-5);
    run_val += gcem::internal::is_even(10UL);
    run_val += gcem::internal::is_odd(-400009L);
    run_val += gcem::internal::is_even(100000000L);

    if (run_val == 6)
        std::cout << "\033[32m All OK.\033[0m" << std::endl;
    else
        throw std::runtime_error("test fail");

    //

    std::cout << "\n*** end is_odd test ***\n" << std::endl;

    return 0;
}
