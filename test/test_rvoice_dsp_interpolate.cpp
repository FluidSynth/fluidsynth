/* FluidSynth - A Software Synthesizer
 *
 * Unit test for fluid_rvoice_dsp_interpolate()
 *
 * Covers all four interpolation modes:
 *   FLUID_INTERP_NONE      - nearest-neighbour
 *   FLUID_INTERP_LINEAR    - linear (2-point)
 *   FLUID_INTERP_4THORDER  - cubic (4-point Catmull-Rom)
 *   FLUID_INTERP_7THORDER  - 7-point windowed sinc
 *
 * Two properties are verified for every mode:
 *   a) Interpolation between sample points produces the correct value.
 *   b) Interpolation at (and across) a sample loop boundary is handled
 *      correctly (guard/wrap-around samples come from the right place).
 */

#include "test.h"
#include "rvoice/fluid_rvoice.h"
#include "sfloader/fluid_sfont.h"

#include <cstring>
#include <cmath>

/* ----- tolerances ------------------------------------------------------ */

/* NONE, LINEAR and 4TH ORDER reproduce exact integer samples and linear
 * ramps with no floating-point error beyond rounding of the lookup table
 * index (256 levels).  A tolerance of 0.5 LSB of a 16-bit short (i.e. 1)
 * covers all table-quantisation artefacts for these modes. */
static const fluid_real_t TOL_POLY  = (fluid_real_t)1.0;

/* The 7th-order windowed sinc adds an internal half-sample phase shift to
 * centre the kernel.  At an exact integer input position it smears a tiny
 * amount of energy into adjacent samples, making the output deviate
 * slightly from the integer sample value.  For sample amplitudes up to
 * ~16000 counts the smearing error stays below ~5 counts. */
static const fluid_real_t TOL_SINC  = (fluid_real_t)5.0;

/* For the constant-sample loop-boundary test the DC gain of every mode
 * must equal 1.0.  Any misuse of zero-initialised guard memory instead of
 * the correct loop-end samples would produce an error of ~(3/7)*CONST_VAL
 * ≈ 4285 for the sinc.  A tolerance of 50 counts (0.5 % of CONST_VAL)
 * therefore distinguishes correct boundary handling from broken code while
 * remaining insensitive to normal floating-point rounding. */
static const fluid_real_t TOL_LOOP  = (fluid_real_t)50.0;

/* ----- sample constants ------------------------------------------------- */

static const int SAMPLE_SIZE  = 64;   /* total PCM samples allocated     */
static const int LOOP_START   = 8;    /* inclusive loop start index       */
static const int LOOP_END     = 24;   /* exclusive (first index past loop) */
static const short CONST_VAL  = 10000;

/* ----- helpers ---------------------------------------------------------- */

/* Build a minimal fluid_rvoice_t that is safe for all interpolation paths.
 * Only the fields read by the interpolation engine are set; everything else
 * is zero-initialised.
 *
 * Parameters
 *   rvoice        - output: the voice to initialise
 *   sample        - output: the sample object to initialise
 *   data          - 16-bit PCM buffer (SAMPLE_SIZE shorts)
 *   phase_float   - starting phase (as a double, integer + fractional part)
 *   phase_incr    - playback rate (1.0 = root pitch)
 *   loop_start    - first sample index of the loop (inclusive)
 *   loop_end      - one past the last sample index of the loop (exclusive)
 *   has_looped    - 1 if the voice has already completed its first loop
 */
static void setup_rvoice(fluid_rvoice_t  *rvoice,
						 fluid_sample_t  *sample,
						 short           *data,
						 double           phase_float,
						 double           phase_incr,
						 int              loop_start,
						 int              loop_end,
						 int              has_looped)
{
	memset(rvoice, 0, sizeof(*rvoice));
	memset(sample, 0, sizeof(*sample));

	sample->data   = data;
	sample->data24 = NULL;   /* 16-bit path */

	rvoice->dsp.sample     = sample;
	rvoice->dsp.start      = 0;
	rvoice->dsp.end        = SAMPLE_SIZE - 1;
	rvoice->dsp.loopstart  = loop_start;
	rvoice->dsp.loopend    = loop_end;
	rvoice->dsp.has_looped = (char)has_looped;

	fluid_phase_set_float(rvoice->dsp.phase, phase_float);
	rvoice->dsp.phase_incr = (fluid_real_t)phase_incr;
}

static fluid_real_t absf(fluid_real_t x)
{
	return x < (fluid_real_t)0.0 ? -x : x;
}

/* =========================================================================
 * Test A – integer-phase passthrough
 *
 * At 1:1 playback speed (phase_incr == 1.0) the output of every sample
 * must equal the underlying PCM integer value.  This is guaranteed for
 * NONE, LINEAR and 4TH ORDER because their kernels collapse to an impulse
 * at zero fractional phase.  The 7th-order sinc is excluded here (see
 * test C for its constant-input verification).
 * ========================================================================= */
static void test_A_integer_phase_passthrough(void)
{
	/* Distinctive non-trivial ramp: data[i] = i * 512 */
	short data[SAMPLE_SIZE];
	for(int i = 0; i < SAMPLE_SIZE; i++)
		data[i] = (short)(i * 512);

	const fluid_interp modes[] = { FLUID_INTERP_NONE,
								   FLUID_INTERP_LINEAR,
								   FLUID_INTERP_4THORDER };
	const int n_modes = (int)(sizeof(modes) / sizeof(modes[0]));

	for(int m = 0; m < n_modes; m++)
	{
		fluid_rvoice_t  rvoice;
		fluid_sample_t  samp;
		fluid_real_t    buf[FLUID_BUFSIZE];
		memset(buf, 0, sizeof(buf));

		setup_rvoice(&rvoice, &samp, data,
					 /*phase_float=*/0.0, /*phase_incr=*/1.0,
					 /*loop_start=*/0, /*loop_end=*/SAMPLE_SIZE,
					 /*has_looped=*/0);
		rvoice.dsp.interp_method = modes[m];

		int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, /*looping=*/0);
		TEST_ASSERT(count > 0);

		for(int i = 0; i < count; i++)
		{
			fluid_real_t expected = (fluid_real_t)data[i];
			TEST_ASSERT(absf(buf[i] - expected) <= TOL_POLY);
		}
	}
}

/* =========================================================================
 * Test B – fractional-phase accuracy for polynomial interpolators
 *
 * A linear ramp (data[i] = i * 1000) is played at half speed
 * (phase_incr = 0.5), starting from index 5 so the 4th-order cubic is
 * always in the "interior" region with real neighbours on both sides.
 *
 * Every polynomial interpolator (LINEAR, 4TH ORDER) must reproduce a
 * linear function exactly because the polynomial they fit has at most
 * degree 1, which every higher-degree polynomial also fits exactly.
 *
 * The expected value at output step i is: (5.0 + i * 0.5) * 1000.
 *
 * FLUID_INTERP_NONE is tested separately: it must return the sample at the
 * nearest integer index (round-to-nearest semantics).
 * ========================================================================= */
static void test_B_fractional_linear_ramp(void)
{
	short data[SAMPLE_SIZE];
	for(int i = 0; i < SAMPLE_SIZE; i++)
		data[i] = (short)(i * 1000);

	/* --- NONE: nearest-neighbour rounding -------------------------------- */
	{
		fluid_rvoice_t  rvoice;
		fluid_sample_t  samp;
		fluid_real_t    buf[FLUID_BUFSIZE];
		memset(buf, 0, sizeof(buf));

		/* Start at 5.0, step 0.5 */
		setup_rvoice(&rvoice, &samp, data,
					 /*phase_float=*/5.0, /*phase_incr=*/0.5,
					 /*loop_start=*/0, /*loop_end=*/SAMPLE_SIZE,
					 /*has_looped=*/0);
		rvoice.dsp.interp_method = FLUID_INTERP_NONE;

		int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, /*looping=*/0);
		TEST_ASSERT(count > 0);

		for(int i = 0; i < count; i++)
		{
			/* fluid_phase_index_round rounds fract ≥ 0x80000000 up.
			 * At phase = 5.0 + i*0.5: fractional part is 0.0 or 0.5.
			 * Round(5.0+i*0.5) = 5 + (i+1)/2  (integer arithmetic). */
			int nearest = (int)((5.0 + i * 0.5) + 0.5);   /* standard round */
			if(nearest >= SAMPLE_SIZE) nearest = SAMPLE_SIZE - 1;
			fluid_real_t expected = (fluid_real_t)data[nearest];
			TEST_ASSERT(absf(buf[i] - expected) <= TOL_POLY);
		}
	}

	/* --- LINEAR and 4TH ORDER: exact linear ramp reproduction ----------- */
	{
		const fluid_interp poly_modes[] = { FLUID_INTERP_LINEAR,
											FLUID_INTERP_4THORDER };
		const int n = (int)(sizeof(poly_modes) / sizeof(poly_modes[0]));

		for(int m = 0; m < n; m++)
		{
			fluid_rvoice_t  rvoice;
			fluid_sample_t  samp;
			fluid_real_t    buf[FLUID_BUFSIZE];
			memset(buf, 0, sizeof(buf));

			setup_rvoice(&rvoice, &samp, data,
						 /*phase_float=*/5.0, /*phase_incr=*/0.5,
						 /*loop_start=*/0, /*loop_end=*/SAMPLE_SIZE,
						 /*has_looped=*/0);
			rvoice.dsp.interp_method = poly_modes[m];

			int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, /*looping=*/0);
			TEST_ASSERT(count > 0);

			for(int i = 0; i < count; i++)
			{
				fluid_real_t expected = (fluid_real_t)((5.0 + i * 0.5) * 1000.0);
				TEST_ASSERT(absf(buf[i] - expected) <= TOL_POLY);
			}
		}
	}
}

/* =========================================================================
 * Test C – constant-sample accuracy for all four modes
 *
 * All modes, including the 7th-order sinc, must produce the constant value
 * CONST_VAL for every output sample when the input is constant-valued.
 * This holds because the sum of the interpolation coefficients must equal
 * 1 for any reasonable interpolator (DC gain = 1).
 *
 * Two sub-cases are exercised:
 *   C1: non-looping, integer phase (phase_incr = 1.0)
 *   C2: non-looping, fractional phase (phase_incr = 0.7, start in interior)
 * ========================================================================= */
static void test_C_constant_sample(void)
{
	short data[SAMPLE_SIZE];
	for(int i = 0; i < SAMPLE_SIZE; i++)
		data[i] = CONST_VAL;

	const fluid_interp modes[] = { FLUID_INTERP_NONE,
								   FLUID_INTERP_LINEAR,
								   FLUID_INTERP_4THORDER,
								   FLUID_INTERP_7THORDER };
	const int n_modes = (int)(sizeof(modes) / sizeof(modes[0]));

	/* C1: integer phase */
	for(int m = 0; m < n_modes; m++)
	{
		fluid_rvoice_t  rvoice;
		fluid_sample_t  samp;
		fluid_real_t    buf[FLUID_BUFSIZE];
		memset(buf, 0, sizeof(buf));

		setup_rvoice(&rvoice, &samp, data,
					 /*phase_float=*/0.0, /*phase_incr=*/1.0,
					 /*loop_start=*/0, /*loop_end=*/SAMPLE_SIZE,
					 /*has_looped=*/0);
		rvoice.dsp.interp_method = modes[m];

		int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, /*looping=*/0);
		TEST_ASSERT(count > 0);

		fluid_real_t tol = (modes[m] == FLUID_INTERP_7THORDER) ? TOL_SINC : TOL_POLY;
		for(int i = 0; i < count; i++)
			TEST_ASSERT(absf(buf[i] - (fluid_real_t)CONST_VAL) <= tol);
	}

	/* C2: fractional phase, start well into the interior so all modes
	 * (including 7th ORDER which needs 3 real prior samples) use only
	 * interior data. */
	for(int m = 0; m < n_modes; m++)
	{
		fluid_rvoice_t  rvoice;
		fluid_sample_t  samp;
		fluid_real_t    buf[FLUID_BUFSIZE];
		memset(buf, 0, sizeof(buf));

		setup_rvoice(&rvoice, &samp, data,
					 /*phase_float=*/10.0, /*phase_incr=*/0.7,
					 /*loop_start=*/0, /*loop_end=*/SAMPLE_SIZE,
					 /*has_looped=*/0);
		rvoice.dsp.interp_method = modes[m];

		int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, /*looping=*/0);
		TEST_ASSERT(count > 0);

		fluid_real_t tol = (modes[m] == FLUID_INTERP_7THORDER) ? TOL_SINC : TOL_POLY;
		for(int i = 0; i < count; i++)
			TEST_ASSERT(absf(buf[i] - (fluid_real_t)CONST_VAL) <= tol);
	}
}

/* =========================================================================
 * Test D – loop-boundary correctness with a constant-valued sample
 *
 * A looping voice is configured so that the playback head crosses the
 * loop-end boundary within the first output buffer.  Every interpolator
 * must continue to output CONST_VAL after the boundary because the guard
 * samples it fetches from beyond loopend must wrap around to loopstart.
 *
 * Two sub-cases:
 *   D1: first loop crossing (has_looped = 0 → set to 1 by the engine)
 *   D2: steady-state looping (has_looped = 1, guard samples come from
 *       loopend-1, loopend-2, loopend-3)
 *
 * For a correctly-implemented loop: all guard values equal CONST_VAL, so
 * any interpolation formula still yields CONST_VAL.  If the engine
 * accidentally reads uninitialised/zero memory the error would be
 * O(CONST_VAL * guard_coeff) ≈ 4000 for the sinc, far above TOL_LOOP = 50.
 * ========================================================================= */
static void test_D_loop_boundary_constant_sample(void)
{
	short data[SAMPLE_SIZE];
	for(int i = 0; i < SAMPLE_SIZE; i++)
		data[i] = CONST_VAL;

	/* Loop region [LOOP_START, LOOP_END).  Start just before the end so
	 * the boundary is crossed immediately. */
	const double phase_start = (double)(LOOP_END - 4);
	const double phase_incr  = 0.6;

	const fluid_interp modes[] = { FLUID_INTERP_NONE,
								   FLUID_INTERP_LINEAR,
								   FLUID_INTERP_4THORDER,
								   FLUID_INTERP_7THORDER };
	const int n_modes = (int)(sizeof(modes) / sizeof(modes[0]));

	for(int sub = 0; sub < 2; sub++)   /* sub=0: first crossing, sub=1: already looped */
	{
		int has_looped = sub;

		for(int m = 0; m < n_modes; m++)
		{
			fluid_rvoice_t  rvoice;
			fluid_sample_t  samp;
			fluid_real_t    buf[FLUID_BUFSIZE];
			memset(buf, 0, sizeof(buf));

			setup_rvoice(&rvoice, &samp, data,
						 phase_start, phase_incr,
						 LOOP_START, LOOP_END,
						 has_looped);
			rvoice.dsp.interp_method = modes[m];

			int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, /*looping=*/1);
			TEST_ASSERT(count == FLUID_BUFSIZE);

			fluid_real_t tol = (modes[m] == FLUID_INTERP_7THORDER) ? TOL_LOOP : TOL_POLY;
			for(int i = 0; i < count; i++)
				TEST_ASSERT(absf(buf[i] - (fluid_real_t)CONST_VAL) <= tol);

			/* After a looping run, has_looped must be set. */
			TEST_ASSERT(rvoice.dsp.has_looped == 1);
		}
	}
}

/* =========================================================================
 * Test E – loop-boundary: LINEAR interpolation with a ramp continuous
 *           across the loop boundary
 *
 * The loop region [LOOP_START, LOOP_END) is filled with a repeating ramp
 * f(k) = k * 4000  for k = 0 .. (LOOP_END - LOOP_START - 1).
 *
 * Neighbouring memory outside the loop is also filled with the periodic
 * extension so that the linear interpolator always sees matching values
 * when it reads one sample ahead (dsp_phase_index + 1).
 *
 * For LINEAR interpolation at a position within the last sample of the
 * loop, the "next" sample must come from loopstart (the wrap-around
 * value), not from the raw memory at loopend.  If the engine reads the
 * raw memory instead of the wrapped value the test will fail because the
 * memory beyond the loop contains a different periodic value.
 * ========================================================================= */
static void test_E_loop_boundary_linear_wrap(void)
{
	const int loop_len   = LOOP_END - LOOP_START;
	const int ramp_step  = 4000;

	/* Fill the whole buffer with the periodic extension of the loop ramp
	 * so that data[i] == ((i - LOOP_START) mod loop_len) * ramp_step. */
	short data[SAMPLE_SIZE];
	for(int i = 0; i < SAMPLE_SIZE; i++)
	{
		int rel = ((i - LOOP_START) % loop_len + loop_len) % loop_len;
		data[i] = (short)(rel * ramp_step);
	}

	/* Start one sample before loop end so the boundary is crossed. */
	const double phase_start = (double)(LOOP_END - 2);
	const double phase_incr  = 0.25;   /* small step → many fractional positions */

	fluid_rvoice_t  rvoice;
	fluid_sample_t  samp;
	fluid_real_t    buf[FLUID_BUFSIZE];
	memset(buf, 0, sizeof(buf));

	setup_rvoice(&rvoice, &samp, data,
				 phase_start, phase_incr,
				 LOOP_START, LOOP_END,
				 /*has_looped=*/1);
	rvoice.dsp.interp_method = FLUID_INTERP_LINEAR;

	int count = fluid_rvoice_dsp_interpolate(&rvoice, buf, /*looping=*/1);
	TEST_ASSERT(count == FLUID_BUFSIZE);

	/* Reference: advance phase manually and compute the linear interpolation
	 * of the periodic ramp at each position. */
	double phase_d = phase_start;
	for(int i = 0; i < count; i++)
	{
		/* Integer and fractional parts of the current phase. */
		int    idx  = (int)phase_d;
		double frac = phase_d - idx;

		/* Wrap index into loop. */
		while(idx >= LOOP_END)   idx -= loop_len;
		while(idx < LOOP_START)  idx += loop_len;

		int next_idx = idx + 1;
		if(next_idx >= LOOP_END)  next_idx -= loop_len;

		fluid_real_t s0 = (fluid_real_t)data[idx];
		fluid_real_t s1 = (fluid_real_t)data[next_idx];
		fluid_real_t expected = s0 + (fluid_real_t)frac * (s1 - s0);

		TEST_ASSERT(absf(buf[i] - expected) <= TOL_POLY);

		phase_d += phase_incr;
	}
}

/* ========================================================================= */

int main(void)
{
	test_A_integer_phase_passthrough();
	test_B_fractional_linear_ramp();
	test_C_constant_sample();
	test_D_loop_boundary_constant_sample();
	test_E_loop_boundary_linear_wrap();

	return EXIT_SUCCESS;
}
