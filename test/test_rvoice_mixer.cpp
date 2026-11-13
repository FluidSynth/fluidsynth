#include "test.h"
#include "../src/rvoice/fluid_rvoice_mixer.cpp"

/* Check gain mixing at a block offset, including samples outside the mixed range. */
int main(void)
{
    const struct
    {
        int count;
        fluid_real_t current_amp, target_amp;
    } cases[] =
    {
        {1, 0.5, 0.5},
        {FLUID_BUFSIZE - 1, 0.5, 0.5},
        {FLUID_BUFSIZE, 0.5, 0.5},
        {FLUID_BUFSIZE + 1, 0.5, 0.5},
        {FLUID_BUFSIZE - 1, 0.25, 0.75},
        {FLUID_BUFSIZE + 1, 0.75, 0.25},
        {FLUID_BUFSIZE + 1, 0.5, 0}
    };
    alignas(FLUID_DEFAULT_ALIGNMENT) fluid_real_t source[3 * FLUID_BUFSIZE];
    alignas(FLUID_DEFAULT_ALIGNMENT) fluid_real_t output[3 * FLUID_BUFSIZE];
    fluid_real_t *destinations[] = {output};

    for(int c = 0; c < (int)FLUID_N_ELEMENTS(cases); c++)
    {
        fluid_rvoice_buffers_t buffers = {};
        buffers.count = 1;
        buffers.bufs[0].mapping = 0;
        buffers.bufs[0].current_amp = cases[c].current_amp;
        buffers.bufs[0].target_amp = cases[c].target_amp;
        for(int i = 0; i < (int)FLUID_N_ELEMENTS(source); i++)
        {
            source[i] = (fluid_real_t)(i + 1) / FLUID_BUFSIZE;
            output[i] = 1;
        }

        fluid_rvoice_buffers_mix(&buffers, source, 1, cases[c].count,
                                 destinations, FLUID_N_ELEMENTS(destinations));

        for(int i = 0; i < (int)FLUID_N_ELEMENTS(output); i++)
        {
            int sample = i - FLUID_BUFSIZE;
            fluid_real_t expected = 1;
            if(sample >= 0 && sample < cases[c].count)
            {
                int ramp_sample = sample < FLUID_BUFSIZE ? sample : FLUID_BUFSIZE;
                fluid_real_t amp = cases[c].current_amp
                    + (cases[c].target_amp - cases[c].current_amp) * ramp_sample / FLUID_BUFSIZE;
                expected += amp * source[i];
            }
            TEST_ASSERT(output[i] == expected);
        }
        TEST_ASSERT(buffers.bufs[0].current_amp == cases[c].target_amp);
    }
    return EXIT_SUCCESS;
}
