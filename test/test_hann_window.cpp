
#define _POSIX_C_SOURCE 199309L

#include "fluidsynth_priv.h"
#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef SINC_ORDER
#define SINC_ORDER 11
#endif
#define N SINC_ORDER

/* Tables, filled once */
static fluid_real_t C_tab[2 * N], S_tab[2 * N];
static fluid_real_t C2_tab[N], S2_tab[N];

static void init_tables(void)
{
    for (int i = 0; i < 2 * N; i++) {
        C_tab[i] = FLUID_COS(FLUID_M_PI * i / N);
        S_tab[i] = FLUID_SIN(FLUID_M_PI * i / N);
    }
    for (int i = 0; i < N; i++) {
        C2_tab[i] = FLUID_COS(2.0 * FLUID_M_PI * i / N);
        S2_tab[i] = FLUID_SIN(2.0 * FLUID_M_PI * i / N);
    }
}

/* 0: original */
static void hann_orig(fluid_real_t center, fluid_real_t* out)
{
    for (int i = 0; i < N; i++) {
        fluid_real_t i_shifted = (fluid_real_t)i - center;
        fluid_real_t arg = FLUID_M_PI * i_shifted;
        fluid_real_t value = FLUID_COS(arg / N);
        out[i] = value * value;
    }
}

/* 1: rotation recurrence on (FLUID_COS 2θ, FLUID_SIN 2θ) */
static void hann_rot(fluid_real_t center, fluid_real_t* out)
{
    const fluid_real_t d2 = 2.0 * FLUID_M_PI / N;
    const fluid_real_t cd = FLUID_COS(d2), sd = FLUID_SIN(d2);
    fluid_real_t a0 = -d2 * center / 2.0 * 2.0 / 2.0 * 2.0; /* = -d2*center */
    a0 = -d2 * center;
    fluid_real_t c = FLUID_COS(a0), s = FLUID_SIN(a0);
    for (int i = 0; i < N; i++) {
        out[i] = 0.5 * (1.0 + c);
        fluid_real_t cn = c * cd - s * sd;
        fluid_real_t sn = s * cd + c * sd;
        c = cn; s = sn;
    }
}

/* 2: Chebyshev recurrence */
static void hann_cheb(fluid_real_t center, fluid_real_t* out)
{
    const fluid_real_t d2 = 2.0 * FLUID_M_PI / N;
    const fluid_real_t k = 2.0 * FLUID_COS(d2);
    fluid_real_t xm = FLUID_COS(d2 * (-1.0 - center));
    fluid_real_t x = FLUID_COS(d2 * (0.0 - center));
    for (int i = 0; i < N; i++) {
        out[i] = 0.5 * (1.0 + x);
        fluid_real_t xn = k * x - xm;
        xm = x; x = xn;
    }
}

/* 3: tables period 2N, then square */
static void hann_tab(fluid_real_t center, fluid_real_t* out)
{
    const fluid_real_t cc = FLUID_COS(FLUID_M_PI * center / N);
    const fluid_real_t sc = FLUID_SIN(FLUID_M_PI * center / N);
    // Tables are 2*N long. The upper half is sign flipped, which doesn't matter, as we square the result below.
    // This way we only need to process half of the table.
    for (int i = 0; i < N; i++) {
        fluid_real_t v = C_tab[i] * cc + S_tab[i] * sc;
        out[i] = v * v;
    }
}

/* 4: half-angle tables, no square */
static void hann_tab2(fluid_real_t center, fluid_real_t* out)
{
    const fluid_real_t cc = FLUID_COS(2.0 * FLUID_M_PI * center / N);
    const fluid_real_t sc = FLUID_SIN(2.0 * FLUID_M_PI * center / N);
    for (int i = 0; i < N; i++)
        out[i] = 0.5 * (1.0 + C2_tab[i] * cc + S2_tab[i] * sc);
}

/* long double reference */
static void hann_ref(double center, long double* out)
{
    const long double pi = 3.141592653589793238462643383279502884L;
    for (int i = 0; i < N; i++) {
        long double v = cosl(pi * ((long double)i - (long double)center) / N);
        out[i] = v * v;
    }
}

typedef void (*hann_fn)(fluid_real_t, fluid_real_t*);

static const struct { const char* name; hann_fn fn; } methods[] = {
    { "original FLUID_COS()",  hann_orig },
    { "rotation recurrence",   hann_rot  },
    { "Chebyshev recurrence",  hann_cheb },
    { "table 2N + square",     hann_tab  },
    { "table half-angle",      hann_tab2 },
};
#define NM (int)(sizeof methods / sizeof methods[0])

#ifdef _WIN32
int fluid_clock_gettime(int df, struct timespec* spec)      //C-file part
{
    __int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
    wintime -= 116444736000000000i64;  //1jan1601 to 1jan1970
    spec->tv_sec = wintime / 10000000i64;           //seconds
    spec->tv_nsec = wintime % 10000000i64 * 100;      //nano-seconds
    return 0;
}
#else
#define fluid_clock_gettime clock_gettime
#endif

static double now_sec(void)
{
    struct timespec ts;
    fluid_clock_gettime(0, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(void)
{
    enum { NCENTERS = 1001, REPS = 20000 };
    fluid_real_t out[N];
    long double ref[N];
    double max_e[NM] = { 0 }, sum_e[NM] = { 0 };
    [[maybe_unused]] constexpr double expected_max_e_d[NM] = { 8e-16, 4e-16, 9e-16, 4e-16, 4e-16 };
    [[maybe_unused]] constexpr float expected_max_e_f[NM] = { 4e-7, 2e-7, 4e-7, 4e-7, 3e-7 };
    long cnt = 0;

    init_tables();

    /* ---- accuracy ---- */
    for (int t = 0; t < NCENTERS; t++) {
        double center = (double)t / (NCENTERS - 1);
        hann_ref(center, ref);
        for (int m = 0; m < NM; m++) {
            methods[m].fn(center, out);
            for (int i = 0; i < N; i++) {
                double e = FLUID_FABS((double)((long double)out[i] - ref[i]));
                if (e > max_e[m]) max_e[m] = e;
                sum_e[m] += e;
            }
        }
        cnt += N;
    }

    /* ---- speed ---- */
    double centers[NCENTERS];
    for (int t = 0; t < NCENTERS; t++) centers[t] = (double)t / (NCENTERS - 1);

    double ns_per_call[NM];
    volatile double sink = 0;
    for (int m = 0; m < NM; m++) {
        /* warm-up */
        for (int t = 0; t < NCENTERS; t++) methods[m].fn(centers[t], out);
        double t0 = now_sec();
        for (int r = 0; r < REPS / 10; r++)
            for (int t = 0; t < NCENTERS; t++) {
                methods[m].fn(centers[t], out);
                sink += out[r % N];   /* keep the work alive */
            }
        double t1 = now_sec();
        ns_per_call[m] = (t1 - t0) * 1e9 / ((double)(REPS / 10) * NCENTERS);
    }

    printf("N=%d, %d centers, accuracy vs long double reference\n\n", N, NCENTERS);
    printf("%-22s %12s %12s %12s %10s\n",
        "method", "max err", "mean err", "ns/window", "speedup");
    for (int m = 0; m < NM; m++)
        printf("%-22s %12.3e %12.3e %12.1f %9.2fx\n",
            methods[m].name, max_e[m], sum_e[m] / cnt,
            ns_per_call[m], ns_per_call[0] / ns_per_call[m]);

    for (int m = 0; m < NM; m++)
    {
        const fluid_real_t (&expected_max_error)[] = 
#if defined(WITH_FLOAT)
        expected_max_e_f
#else
        expected_max_e_d
#endif
;
        TEST_ASSERT(max_e[m] <= expected_max_error[m]);
        printf("Successfully asserted max error method '%s'\n", methods[m].name);
        
    }
    (void)sink;
    return 0;
}
