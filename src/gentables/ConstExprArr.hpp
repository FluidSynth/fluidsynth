
#pragma once

#include "fluidsynth_priv.h"


struct IsBoundary
{
    // Checks whether N is a multiple of 10
    static constexpr bool chunk10(int N)
    {
        return (N >= 0) && (N % 10 == 0);
    }
};

// === Recursive Template Struct: ConstExprArr_impl ===
// This struct recursively instantiates itself, decrementing N and accumulating template parameters
// in Rest... Each instantiation pushes N onto the Rest... pack, so eventually the values are
// collected as 1, 2, ..., N-1, N in Rest..., until we reach N = 0.
template<typename F, bool NisMult10, int N, int... Rest> struct ConstExprArr_impl
{
    static constexpr auto &value = ConstExprArr_impl<F, IsBoundary::chunk10(N - 1), N - 1, N, Rest...>::value;
};

// === Partial Specialization for N==0 ===
// When N == 0, this specialization is used.
// It defines a static constant array value containing the values as calculated by the functor.
// At this point, Rest... actually contains all previous N values, so the array will be { 0, 1, 2, ..., N }
template<typename F, int... Rest> struct ConstExprArr_impl<F, true, 0, Rest...>
{
    static constexpr fluid_real_t value[] = { F::calc(0), F::calc(Rest)... };
};

// === Partial Specialization whenever N is positive and a multiple of 10 ===
// The sole purpose of this specialization is to allow compilation with MSVC. MSVC has a hard-coded
// recursive template instantiation limit of 500. By chunking the recursion in steps of 10 whenever
// N is a multiple of 10, we drastically reduce the template recursion depth, allowing the code to
// compile for MSVC.
template<typename F, int N, int... Rest> struct ConstExprArr_impl<F, true, N, Rest...>
{
    static constexpr auto &value =
    ConstExprArr_impl<F, IsBoundary::chunk10(N - 10), N - 10, N - 9, N - 8, N - 7, N - 6, N - 5, N - 4, N - 3, N - 2, N - 1, N, Rest...>::value;
};

// Out-of-class Definition of the Static Array (needed for linkage)
template<typename F, int... Rest> constexpr fluid_real_t ConstExprArr_impl<F, true, 0, Rest...>::value[];

template<typename F, int N> struct ConstExprArr
{
    static_assert(N > 0, "N must be greater 0");

    // invokes the Recursive Template Struct
    // N-1 because we want to exclude the last element, i.e. only from 0 to N-1
    static constexpr auto &value = ConstExprArr_impl<F, IsBoundary::chunk10(N - 1), N - 1>::value;

    ConstExprArr() = delete;
    ConstExprArr(const ConstExprArr &) = delete;
    ConstExprArr(ConstExprArr &&) = delete;
};
