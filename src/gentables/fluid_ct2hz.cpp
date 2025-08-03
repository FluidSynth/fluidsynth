
#include "utils/fluid_conv_tables.h"

#ifdef WITH_FLOAT
using REAL = float;
#else
using REAL = double;
#endif

struct Ct2HzFunctor
{
    static constexpr REAL calc(int i)
    {
        return 6.875L /** constexpr_pow(2.0L, static_cast<REAL>(i) / 1200.0L)*/;
    }
};

struct IsBoundary
{
    static constexpr bool chunk10(int N)
    {
        return (N >= 0) && (N % 10 == 0);
    }
};

// === Recursive Template Struct: ConstExprArr_impl ===
// This struct recursively instantiates itself, decrementing N and accumulating template parameters in Rest...
// Each instantiation pushes N onto the Rest... pack, so eventually the values are collected as N, N-1, ..., 1
// in Rest..., until we reach N = 0.
template<typename F, bool NisMult10, int N, int... Rest>
struct ConstExprArr_impl
{
    static constexpr auto &value = ConstExprArr_impl<F, IsBoundary::chunk10(N-1), N - 1, N, Rest...>::value;
};

// === Partial Specialization for N==0 ===
// When N == 0, this specialization is used.
// It defines a static constant array value containing the values as calculated by the functor.
// At this point, Rest... actually contains all previous N values, so the array will be { 0, 1, 2, ..., N }
template<typename F, int... Rest>
struct ConstExprArr_impl<F, true, 0, Rest...>
{
    static constexpr REAL value[] = { F::calc(0), F::calc(Rest)... };
};

// === Partial Specialization whenever N is positive and a multiple of 10 ===
// The sole purpose of this specialization is to allow compilation with MSVC. MSVC has a hard-coded recursive template instantiation
// limit of 500. By chunking the recursion in steps of 10 whenever N is a multiple of 10, we drastically reduce the template
// recursion depth, allowing the code to compile for MSVC.
template<typename F, int N, int... Rest>
struct ConstExprArr_impl<F, true, N, Rest...>
{
    static constexpr auto &value =
    ConstExprArr_impl<F, IsBoundary::chunk10(N - 10), N - 10, N - 9, N - 8, N - 7, N - 6, N - 5, N - 4, N - 3, N - 2, N - 1, N, Rest...>::value;
};

// Out-of-class Definition of the Static Array (needed for linkage)
template<typename F, int... Rest>
constexpr REAL ConstExprArr_impl<F, true, 0, Rest...>::value[];

template<typename F, int N>
struct ConstExprArr
{
    static_assert(N > 0, "N must be divisable by 10");

    // invokes the Recursive Template Struct
    // N-1 because we want to exclude the last element, i.e. only from 0 to N-1
    static constexpr auto &value = ConstExprArr_impl<F, IsBoundary::chunk10(N - 1), N - 1>::value;

    ConstExprArr() = delete;
    ConstExprArr(const ConstExprArr &) = delete;
    ConstExprArr(ConstExprArr &&) = delete;
};


extern const constexpr auto fluid_ct2hz_tab_cpp = ConstExprArr<Ct2HzFunctor, FLUID_CENTS_HZ_SIZE>::value;


extern "C" const double *fluid_ct2hz_tab_cpp_ex = fluid_ct2hz_tab_cpp;
