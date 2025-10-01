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

#define TEST_ERR_TOL 1e-12

#define TEST_PRINT_PRECISION_1 3
#define TEST_PRINT_PRECISION_2 18

#include "gcem_tests.hpp"

int main()
{
    print_begin("incomplete_gamma");

    //

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.26424111765711527644L,    2.0L,    1.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.42759329552912134220L,    1.5L,    1.0L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.80085172652854419439L,    2.0L,    3.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.95957231800548714595L,    2.0L,    5.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.99876590195913317327L,    2.0L,    9.0L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma,                    1.0L,    0.0L,    9.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma,                    0.0L,    2.0L,    0.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma,                    0.0L,    0.0L,    0.0L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.47974821959920432857L,   11.5L,   11.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.75410627202774938027L,   15.5L,   18.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.43775501395596266851L,   19.0L,   18.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.43939261060849132967L,   20.0L,   19.0L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.58504236243630125536L,   38.0L,   39.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.46422093592347296598L,   56.0L,   55.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.55342727426212001696L,   98.0L,   99.0L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.47356929040257916830L,  102.0L,  101.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.48457398488934400049L,  298.0L,  297.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.48467673854389853316L,  302.0L,  301.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.48807306199561317772L,  498.0L,  497.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.48812074555706047585L,  502.0L,  501.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.51881664512582725823L,  798.0L,  799.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.49059834200005758564L,  801.0L,  800.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.52943655952003709775L,  997.0L,  999.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.45389544705967349580L, 1005.0L, 1001.0L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.99947774194996708008L,   3.0L,   12.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.99914335878922466705L,   5.0L,   15.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.99998305525606989708L,   5.0L,   20.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.99999973309165746116L,   5.0L,   25.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.99999999995566213329L,   5.0L,   35.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 1.0L                   ,   5.0L,   50.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 1.0L                   ,   5.0L,  500.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.99791274095086501816L,   9.0L,   20.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.99999999914307291515L,   9.0L,   40.0L);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.88153558847098478779L,  11.0L,   15.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.98918828117334733907L,  11.0L,   20.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.99941353837024693441L,  11.0L,   25.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.99999933855955824846L,  11.0L,   35.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 0.99999999999354982627L,  11.0L,   50.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, 1.0L                   ,  11.0L,  500.0L);

    //

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, TEST_NAN, -0.1, 0.0);

    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, TEST_NAN, TEST_NAN, TEST_NAN);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, TEST_NAN, TEST_NAN,     3.0L);
    GCEM_TEST_EXPECTED_VAL(gcem::incomplete_gamma, TEST_NAN,     2.0L, TEST_NAN);

    //

    print_final("incomplete_gamma");

    return 0;
}
