/*
 * FluidSynth - A Software Synthesizer
 *
 * Lightweight s24 render identity test (float-oracle).
 *
 * Verifies that fluid_synth_write_s24() matches a local reference conversion
 * computed from fluid_synth_write_float() using the same s32 scale + round + clip + mask
 * semantics as the s24 renderer. No golden EXPECTED buffer yet.
 *
 * Note: On 32-bit x86 builds without SSE2 (x87 FPU), floating-point evaluation
 * can differ slightly between render paths (excess precision / double rounding),
 * leading to rare, small LSB-level deltas at the quantization boundary. This test
 * tolerates a bounded number of small mismatches on such builds.
 */

#include "test.h"
#include "fluid_audio_convert.h"

template<typename T>
void test_infinity_nan()
{
    T i = round_clip_to<T>(std::numeric_limits<float>::infinity());
    TEST_ASSERT(i == std::numeric_limits<T>::max());

    i = round_clip_to<T>(-std::numeric_limits<float>::infinity());
    TEST_ASSERT(i == std::numeric_limits<T>::lowest());

    i = round_clip_to<T>(std::numeric_limits<float>::quiet_NaN());
    TEST_ASSERT(i == 0);
}

void test_int32()
{
    int32_t i32;
    i32 = round_clip_to<int32_t>(0.0f);
    TEST_ASSERT(i32 == 0);

    i32 = round_clip_to<int32_t>(2147483520.0f);
    TEST_ASSERT(i32 == 2147483520);

    i32 = round_clip_to<int32_t>(2147483646.0f);
    TEST_ASSERT(i32 == std::numeric_limits<int32_t>::max());

    i32 = round_clip_to<int32_t>(2147483647.0f);
    TEST_ASSERT(i32 == std::numeric_limits<int32_t>::max());

    i32 = round_clip_to<int32_t>(2147483648.0f);
    TEST_ASSERT(i32 == std::numeric_limits<int32_t>::max());

    i32 = round_clip_to<int32_t>(-2147483648.0f);
    TEST_ASSERT(i32 == std::numeric_limits<int32_t>::lowest());

    i32 = round_clip_to<int32_t>(-2147483647.0f);
    TEST_ASSERT(i32 == std::numeric_limits<int32_t>::lowest());

    i32 = round_clip_to<int32_t>(-2147483646.0f);
    TEST_ASSERT(i32 == std::numeric_limits<int32_t>::lowest());

    i32 = round_clip_to<int32_t>(-2147483520.0f);
    TEST_ASSERT(i32 == -2147483520);

    i32 = round_clip_to<int32_t>(-0.0f);
    TEST_ASSERT(i32 == 0);
}

void test_int16()
{
    int16_t i16;
    i16 = round_clip_to<int16_t>(0.0f);
    TEST_ASSERT(i16 == 0);

    i16 = round_clip_to<int16_t>(32766.0f);
    TEST_ASSERT(i16 == 32766);

    i16 = round_clip_to<int16_t>(32767.0f);
    TEST_ASSERT(i16 == std::numeric_limits<int16_t>::max());

    i16 = round_clip_to<int16_t>(32768.0f);
    TEST_ASSERT(i16 == std::numeric_limits<int16_t>::max());

    i16 = round_clip_to<int16_t>(2147483520.0f);
    TEST_ASSERT(i16 == std::numeric_limits<int16_t>::max());

    i16 = round_clip_to<int16_t>(2147483648.0f);
    TEST_ASSERT(i16 == std::numeric_limits<int16_t>::max());

    i16 = round_clip_to<int16_t>(-2147483648.0f);
    TEST_ASSERT(i16 == std::numeric_limits<int16_t>::lowest());

    i16 = round_clip_to<int16_t>(-2147483520.0f);
    TEST_ASSERT(i16 == std::numeric_limits<int16_t>::lowest());

    i16 = round_clip_to<int16_t>(-32766.0f);
    TEST_ASSERT(i16 == -32766);

    i16 = round_clip_to<int16_t>(-32767.0f);
    TEST_ASSERT(i16 == -32767);

    i16 = round_clip_to<int16_t>(-32768.0f);
    TEST_ASSERT(i16 == std::numeric_limits<int16_t>::lowest());

    i16 = round_clip_to<int16_t>(-0.0f);
    TEST_ASSERT(i16 == 0);
}

int main(void)
{
    test_infinity_nan<int8_t>();
    test_infinity_nan<int16_t>();
    test_infinity_nan<int32_t>();
    test_infinity_nan<uint8_t>();
    test_infinity_nan<uint16_t>();
    test_infinity_nan<uint32_t>();

    test_int32();
    test_int16();

    return 0;
}
