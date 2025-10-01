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

#ifndef _WIN32
    #define TEST_ERR_TOL 1e-12
#endif

#define TEST_PRINT_PRECISION_1 3
#define TEST_PRINT_PRECISION_2 18

#include "gcem_tests.hpp"

int main()
{
    print_begin("incomplete_gamma_inv");

    //

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,        1.0L,    2.0L, 0.26424111765711527644L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,        1.0L,    1.5L, 0.42759329552912134220L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,        3.0L,    2.0L, 0.80085172652854419439L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,        5.0L,    2.0L, 0.95957231800548714595L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,        9.0L,    2.0L, 0.99876590195913317327L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv, TEST_POSINF,    0.0L,                    1.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,        0.0L,    2.0L,                    0.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,        0.0L,    0.0L,                    0.0L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,       11.0L,   11.5L, 0.47974821959920432857L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,       18.0L,   15.5L, 0.75410627202774938027L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,       18.0L,   19.0L, 0.43775501395596266851L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,       19.0L,   20.0L, 0.43939261060849132967L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,       39.0L,   38.0L, 0.58504236243630125536L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,       55.0L,   56.0L, 0.46422093592347296598L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,       99.0L,   98.0L, 0.55342727426212001696L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,      101.0L,  102.0L, 0.47356929040257916830L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,      297.0L,  298.0L, 0.48457398488934400049L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,      301.0L,  302.0L, 0.48467673854389853316L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,      497.0L,  498.0L, 0.48807306199561317772L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,      501.0L,  502.0L, 0.48812074555706047585L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,      799.0L,  798.0L, 0.51881664512582725823L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,      800.0L,  801.0L, 0.49059834200005758564L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,      999.0L,  997.0L, 0.52943655952003709775L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv,     1001.0L, 1005.0L, 0.45389544705967349580L);

    //

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv, TEST_NAN, TEST_NAN, TEST_NAN);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv, TEST_NAN, TEST_NAN,     0.5L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma_inv, TEST_NAN,     2.0L, TEST_NAN);

    //

    print_final("incomplete_gamma_inv");

    return 0;
}
