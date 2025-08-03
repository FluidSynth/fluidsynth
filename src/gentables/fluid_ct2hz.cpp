
#include "utils/fluid_conv_tables.h"

#ifdef WITH_FLOAT
using REAL = float;
#else
using REAL = double;
#endif

struct Ct2HzFunctor {
    static constexpr REAL calc(int i) {
        return i;
    }
};

// === Recursive Template Struct: ConstExprArr_impl ===
// This struct recursively instantiates itself, decrementing N and accumulating template parameters in Rest...
// Each instantiation pushes N onto the Rest... pack, so eventually the values are collected as N, N-1, ..., 1
// in Rest..., until we reach N = 0.
template<typename F, int N, int... Rest>
struct ConstExprArr_impl {
    static constexpr auto& value = ConstExprArr_impl<F, N - 1, N, Rest...>::value;
};

// === Partial Specialization for N==0 ===
// When N == 0, this specialization is used.
// It defines a static constant array value containing the values as calculated by the functor.
// At this point, Rest... actually contains all previous N values, so the array will be { 0, 1, 2, ..., N }
template<typename F, int... Rest>
struct ConstExprArr_impl<F, 0, Rest...> {
    static constexpr REAL value[] = { F::calc(0), F::calc(Rest)... };
};

// Out-of-class Definition of the Static Array (needed for linkage)
template<typename F, int... Rest>
constexpr REAL ConstExprArr_impl<F, 0, Rest...>::value[];

template<typename F, int N>
struct ConstExprArr {
    static_assert(N >= 0, "N must be at least 0");

    // invokes the Recursive Template Struct
    // N-1 because we want to exclude the last element, i.e. only from 0 to N-1
    static constexpr auto& value = ConstExprArr_impl<F, N-1>::value;

    ConstExprArr() = delete;
    ConstExprArr(const ConstExprArr&) = delete;
    ConstExprArr(ConstExprArr&&) = delete;
};


extern const constexpr auto fluid_ct2hz_tab_cpp = ConstExprArr<Ct2HzFunctor, FLUID_CENTS_HZ_SIZE>::value;


extern "C" const double* fluid_ct2hz_tab_cpp_ex = fluid_ct2hz_tab_cpp;
