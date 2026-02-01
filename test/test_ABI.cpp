/*
  Prints sizeof(...) and alignof(...) for common fundamental/intrinsic types.

  Build:
    c++ -std=c++11 -Wall -Wextra -pedantic -O2 print_type_sizes_align.cpp -o print_type_sizes_align

  Run:
    ./print_type_sizes_align
*/

#include <iostream>
#include <iomanip>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <cwchar>

template <typename T>
void print_type(const char* name) {
    std::cout << std::left << std::setw(28) << name
              << " sizeof=" << std::setw(2) << sizeof(T)
              << " alignof=" << std::setw(2) << alignof(T)
              << "\n";
}

int main() {
    std::cout << "sizeof(...) and alignof(...) in bytes on this platform:\n\n";

    // Fundamental character / boolean types
    print_type<char>("char");
    print_type<signed char>("signed char");
    print_type<unsigned char>("unsigned char");
    print_type<wchar_t>("wchar_t");
    print_type<bool>("bool");

    // C++11 Unicode character types
    print_type<char16_t>("char16_t");
    print_type<char32_t>("char32_t");

    // Integers
    print_type<short>("short");
    print_type<unsigned short>("unsigned short");
    print_type<int>("int");
    print_type<unsigned int>("unsigned int");
    print_type<long>("long");
    print_type<unsigned long>("unsigned long");
    print_type<long long>("long long");
    print_type<unsigned long long>("unsigned long long");

    // Floating point
    print_type<float>("float");
    print_type<double>("double");
    print_type<long double>("long double");

    // Pointers / size-related
    print_type<void*>("void*");
    print_type<char*>("char*");
    print_type<std::size_t>("std::size_t");
    print_type<std::ptrdiff_t>("std::ptrdiff_t");

    std::cout << "\n<stdint> fixed-width integer types:\n\n";
    print_type<std::int8_t>("std::int8_t");
    print_type<std::uint8_t>("std::uint8_t");
    print_type<std::int16_t>("std::int16_t");
    print_type<std::uint16_t>("std::uint16_t");
    print_type<std::int32_t>("std::int32_t");
    print_type<std::uint32_t>("std::uint32_t");
    print_type<std::int64_t>("std::int64_t");
    print_type<std::uint64_t>("std::uint64_t");

    std::cout << "\n<stdint> least / fast / pointer integer types:\n\n";
    print_type<std::int_least8_t>("std::int_least8_t");
    print_type<std::uint_least8_t>("std::uint_least8_t");
    print_type<std::int_fast8_t>("std::int_fast8_t");
    print_type<std::uint_fast8_t>("std::uint_fast8_t");

    print_type<std::int_least16_t>("std::int_least16_t");
    print_type<std::uint_least16_t>("std::uint_least16_t");
    print_type<std::int_fast16_t>("std::int_fast16_t");
    print_type<std::uint_fast16_t>("std::uint_fast16_t");

    print_type<std::int_least32_t>("std::int_least32_t");
    print_type<std::uint_least32_t>("std::uint_least32_t");
    print_type<std::int_fast32_t>("std::int_fast32_t");
    print_type<std::uint_fast32_t>("std::uint_fast32_t");

    print_type<std::int_least64_t>("std::int_least64_t");
    print_type<std::uint_least64_t>("std::uint_least64_t");
    print_type<std::int_fast64_t>("std::int_fast64_t");
    print_type<std::uint_fast64_t>("std::uint_fast64_t");

    print_type<std::intptr_t>("std::intptr_t");
    print_type<std::uintptr_t>("std::uintptr_t");

    // (Optional) show max alignment type
    std::cout << "\nMax fundamental alignment-related type:\n\n";
    print_type<std::max_align_t>("std::max_align_t");

    return 0;
}
