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

#include <cmath>
#include <ios>
#include <iostream>
#include <iomanip>
#include <string>
#include <functional>
#include <cstdint> // INT64_MIN and INT64_MAX

#include "gcem.hpp"

//
// tolerance

int GCEM_TEST_NUMBER = 0;

#ifndef TEST_VAL_TYPES
    #define TEST_VAL_TYPES long double
    #define TEST_VAL_TYPES_V 1
#endif

#ifndef TEST_ERR_TOL
#ifdef _WIN32
    #define TEST_ERR_TOL 1e-10
#else
    #define TEST_ERR_TOL 1e-14
#endif
#endif

#ifdef TEST_VAL_TYPES_V
#define PRINT_ERR(err_val)                  \
{                                           \
    printf(" error value = %LE.", err_val); \
}
#else
#define PRINT_ERR(err_val)                  \
{                                           \
    printf(" error value = %d.", err_val);  \
}
#endif

#ifdef TEST_VAL_TYPES_V
    #define VAL_IS_INF(val) std::isinf(static_cast<long double>(val))
    #define VAL_IS_NAN(val) std::isnan(static_cast<long double>(val))
#else
    #define VAL_IS_INF(val) false
    #define VAL_IS_NAN(val) false
#endif

#ifndef TEST_NAN
    #define TEST_NAN gcem::GCLIM<double>::quiet_NaN()
#endif

#ifndef TEST_POSINF
    #define TEST_POSINF gcem::GCLIM<double>::infinity()
#endif

#ifndef TEST_NEGINF
    #define TEST_NEGINF -gcem::GCLIM<double>::infinity()
#endif

//

#ifndef TEST_PRINT_LEVEL
    #define TEST_PRINT_LEVEL 3
#endif

#ifndef TEST_PRINT_PRECISION_1
    #define TEST_PRINT_PRECISION_1 2
#endif

#ifndef TEST_PRINT_PRECISION_2
    #define TEST_PRINT_PRECISION_2 5
#endif

#ifndef GCEM_UNUSED_PAR
    #define GCEM_UNUSED_PAR(x) (void)(x)
#endif

//
// printing pass

#ifndef TRAVIS_CLANG_CXX

#define TEST_PASS_PRINT_FINISH                                                                      \
    if (print_level > 1 && !VAL_IS_NAN(err_val)) {                                                  \
        std::cout << std::setprecision(3) << std::scientific                                        \
                  << "    - error value = " << err_val << "\n";                                     \
    }                                                                                               \
                                                                                                    \
    std::cout << std::defaultfloat;                                                                 \
    std::cout << std::endl;                                                                         \

#else

#define TEST_PASS_PRINT_FINISH                                                                      \
    if (print_level > 1 && !VAL_IS_NAN(err_val)) {                                                  \
        std::cout << "    - error value = " << err_val << "\n";                                     \
    }                                                                                               \
                                                                                                    \
    std::cout << std::endl;                                                                         \

#endif

//

template<typename T1, typename T2, typename T3>
inline
void
print_test_pass(std::string fn_name, const int print_level, 
                int print_precision_1, int print_precision_2,
                const T1 f_val, const T2 err_val, 
                const T3 par_1)
{
    std::cout << "[\033[32mOK\033[0m] ";
    std::cout << std::setiosflags(std::ios::fixed)
              << std::setprecision(print_precision_1) << fn_name
              << "(" << par_1 << ") = "
              << std::setprecision(print_precision_2) << f_val << "\n";

    TEST_PASS_PRINT_FINISH
}

template<typename T1, typename T2, typename T3, typename T4>
inline
void
print_test_pass(std::string fn_name, const int print_level, 
                int print_precision_1, int print_precision_2,
                const T1 f_val, const T2 err_val, 
                const T3 par_1, const T4 par_2)
{
    std::cout << "[\033[32mOK\033[0m] ";
    std::cout << std::setiosflags(std::ios::fixed)
              << std::setprecision(print_precision_1) << fn_name
              << "(" << par_1 << "," << par_2 << ") = "
              << std::setprecision(print_precision_2) << f_val << "\n";

    TEST_PASS_PRINT_FINISH
}

template<typename T1, typename T2, typename T3, typename T4, typename T5>
inline
void
print_test_pass(std::string fn_name, const int print_level, 
                int print_precision_1, int print_precision_2,
                const T1 f_val, const T2 err_val, 
                const T3 par_1, const T4 par_2, const T5 par_3)
{
    std::cout << "[\033[32mOK\033[0m] ";
    std::cout << std::setiosflags(std::ios::fixed)
              << std::setprecision(print_precision_1) << fn_name
              << "(" << par_1 << "," << par_2 << "," << par_3 << ") = "
              << std::setprecision(print_precision_2) << f_val << "\n";

    TEST_PASS_PRINT_FINISH
}

//
// printing fail

template<typename T1, typename T2, typename T3>
inline
void
print_test_fail(std::string fn_name, int test_number, const int print_level, 
                int print_precision_1, int print_precision_2,
                const T1 f_val, const T2 expected_val, 
                const T3 par_1)
{
    GCEM_UNUSED_PAR(print_level);
    GCEM_UNUSED_PAR(print_precision_1);
    GCEM_UNUSED_PAR(print_precision_2);

    std::cerr << "\033[31m Test failed!\033[0m\n";
    std::cerr << "  - Test number: " << test_number << "\n";
    std::cerr << "  - Function Call:  " << fn_name
              << "(" << par_1 << ");\n";
    std::cerr << "  - Expected value: " << expected_val << "\n";
    std::cerr << "  - Actual value:   " << f_val << "\n";
    std::cerr << std::endl;

    throw std::runtime_error("test fail");
}

template<typename T1, typename T2, typename T3, typename T4>
inline
void
print_test_fail(std::string fn_name, int test_number, const int print_level, 
                int print_precision_1, int print_precision_2,
                const T1 f_val, const T2 expected_val, 
                const T3 par_1, const T4 par_2)
{
    GCEM_UNUSED_PAR(print_level);
    GCEM_UNUSED_PAR(print_precision_1);
    GCEM_UNUSED_PAR(print_precision_2);

    std::cerr << "\033[31m Test failed!\033[0m\n";
    std::cerr << "  - Test number: " << test_number << "\n";
    std::cerr << "  - Function Call:  " << fn_name
              << "(" << par_1 << "," << par_2 << ");\n";
    std::cerr << "  - Expected value: " << expected_val << "\n";
    std::cerr << "  - Actual value:   " << f_val << "\n";
    std::cerr << std::endl;

    throw std::runtime_error("test fail");
}

template<typename T1, typename T2, typename T3, typename T4, typename T5>
inline
void
print_test_fail(std::string fn_name, int test_number, const int print_level, 
                int print_precision_1, int print_precision_2,
                const T1 f_val, const T2 expected_val, 
                const T3 par_1, const T4 par_2, const T5 par_3)
{
    GCEM_UNUSED_PAR(print_level);
    GCEM_UNUSED_PAR(print_precision_1);
    GCEM_UNUSED_PAR(print_precision_2);

    std::cerr << "\033[31m Test failed!\033[0m\n";
    std::cerr << "  - Test number: " << test_number << "\n";
    std::cerr << "  - Function Call:  " << fn_name
              << "(" << par_1 << "," << par_2 << "," << par_3 << ");\n";
    std::cerr << "  - Expected value: " << expected_val << "\n";
    std::cerr << "  - Actual value:   " << f_val << "\n";
    std::cerr << std::endl;

    throw std::runtime_error("test fail");
}

//
// macros

#define GCEM_TEST_EXPECTED_VAL(gcem_fn, expected_val, ...)                                          \
{                                                                                                   \
    ++GCEM_TEST_NUMBER;                                                                             \
    std::string fn_name = #gcem_fn;                                                                 \
                                                                                                    \
    auto f_val = gcem_fn(__VA_ARGS__);                                                              \
                                                                                                    \
    auto err_val = std::abs(f_val - expected_val) / (1 + std::abs(expected_val));                   \
                                                                                                    \
    bool test_success = false;                                                                      \
                                                                                                    \
    if (VAL_IS_NAN(expected_val) && VAL_IS_NAN(f_val)) {                                            \
        test_success = true;                                                                        \
    } else if(!VAL_IS_NAN(f_val) && VAL_IS_INF(f_val) && f_val == expected_val) {                   \
        test_success = true;                                                                        \
    } else if(err_val < TEST_ERR_TOL) {                                                             \
        test_success = true;                                                                        \
    } else {                                                                                        \
        print_test_fail(fn_name,GCEM_TEST_NUMBER,TEST_PRINT_LEVEL,                                  \
                        TEST_PRINT_PRECISION_1,TEST_PRINT_PRECISION_2,                              \
                        f_val,expected_val,__VA_ARGS__);                                            \
    }                                                                                               \
                                                                                                    \
    if (test_success && TEST_PRINT_LEVEL > 0)                                                       \
    {                                                                                               \
        print_test_pass(fn_name,TEST_PRINT_LEVEL,                                                   \
                        TEST_PRINT_PRECISION_1,TEST_PRINT_PRECISION_2,                              \
                        f_val,err_val,__VA_ARGS__);                                                 \
    }                                                                                               \
}

#define GCEM_TEST_COMPARE_VALS(gcem_fn, std_fn, ...)                                                \
{                                                                                                   \
    ++GCEM_TEST_NUMBER;                                                                             \
                                                                                                    \
    std::string fn_name = #gcem_fn;                                                                 \
                                                                                                    \
    auto gcem_fn_val = gcem_fn(__VA_ARGS__);                                                        \
    auto std_fn_val  = std_fn(__VA_ARGS__);                                                         \
                                                                                                    \
    auto err_val = std::abs(gcem_fn_val - std_fn_val) / (1 + std::abs(std_fn_val));                 \
                                                                                                    \
    bool test_success = false;                                                                      \
                                                                                                    \
    if (VAL_IS_NAN(gcem_fn_val) && VAL_IS_NAN(std_fn_val)) {                                        \
        test_success = true;                                                                        \
    } else if(!VAL_IS_NAN(gcem_fn_val) && VAL_IS_INF(gcem_fn_val) && gcem_fn_val == std_fn_val) {   \
        test_success = true;                                                                        \
    } else if(err_val < TEST_ERR_TOL) {                                                             \
        test_success = true;                                                                        \
    } else {                                                                                        \
        print_test_fail(fn_name,GCEM_TEST_NUMBER,TEST_PRINT_LEVEL,                                  \
                        TEST_PRINT_PRECISION_1,TEST_PRINT_PRECISION_2,                              \
                        gcem_fn_val,std_fn_val,__VA_ARGS__);                                        \
    }                                                                                               \
                                                                                                    \
    if (test_success && TEST_PRINT_LEVEL > 0)                                                       \
    {                                                                                               \
        print_test_pass(fn_name,TEST_PRINT_LEVEL,                                                   \
                        TEST_PRINT_PRECISION_1,TEST_PRINT_PRECISION_2,                              \
                        gcem_fn_val,err_val,__VA_ARGS__);                                           \
    }                                                                                               \
}

//
// begin and end print

inline
void 
finish_print_line(int k)
{
    for (int i=0; i < 80 - k; ++i) {
        printf(" ");
    }
    printf("#");
}

inline
void 
print_begin(std::string fn_name)
{
    // GCEM_UNUSED_PAR(fn_name);
    if (TEST_PRINT_LEVEL > 0) {
    std::cout << 
        "\n################################################################################\n";
    std::cout << 
        "\n\033[36m~~~~ Begin tests for: " << fn_name << "\033[0m\n";
    std::cout << std::endl;
    }
}

inline
void 
print_final(std::string fn_name)
{
    std::cout.precision(1);

    std::cout << 
        "################################################################################\n" <<
        "#                                TESTS SUMMARY                                 #\n";
    
    //

    std::cout << 
        "#  - Test suite: " << fn_name;
        finish_print_line(14 + int(fn_name.length()) + 4);
    std::cout << std::endl;

    //

    std::cout << 
        "#  - Number of tests: " << GCEM_TEST_NUMBER;
        finish_print_line(20 + (GCEM_TEST_NUMBER > 9 ? 1 : 0) + 4);
    std::cout << std::endl;

    //

    std::cout << 
        "#  - Error tolerance: " << std::scientific << TEST_ERR_TOL;
        finish_print_line(30);
    std::cout << std::endl;

    //

    std::cout << 
        "#                                   [\033[32mPASS\033[0m]                                     #\n" <<
        "################################################################################\n";
    //

    std::cout << std::endl;
}
