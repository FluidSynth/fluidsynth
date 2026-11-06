#include "test.h"
#include "fluidsynth_priv.h"
#include "fluidsynth.h"
#include <math.h>

enum { FRAME_COUNT = 256 * FLUID_BUFSIZE };

/* Cross the parallel-rendering threshold, finish voices, and submit more work
 * to sleeping workers. Compare every block with single-threaded synthesis. */
static void render_changing_voices(int cores, float *left, float *right)
{
    static const int block_sizes[] = {FLUID_BUFSIZE, 3 * FLUID_BUFSIZE, 4 * FLUID_BUFSIZE, 8 * FLUID_BUFSIZE};
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    int sfid, frame = 0, round, note;

    TEST_ASSERT(settings != NULL);
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.cpu-cores", cores));
    TEST_SUCCESS(fluid_settings_setint(settings, "audio.realtime-prio", 0));
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);
    sfid = fluid_synth_sfload(synth, TEST_SOUNDFONT, 1);
    TEST_ASSERT(sfid >= 0);
    TEST_SUCCESS(fluid_synth_program_select(synth, 0, sfid, 0, 127));

    for(round = 0; round < 64; ++round)
    {
        int block = block_sizes[round % FLUID_N_ELEMENTS(block_sizes)];

        if(round % 8 == 0)
        {
            /* Seven voices stay below the eight-voice parallel-rendering threshold;
             * 65 can activate all seven workers in the eight-core configuration. */
            int notes = round % 16 == 0 ? 7 : 65;

            TEST_SUCCESS(fluid_synth_all_sounds_off(synth, 0));
            for(note = 0; note < notes; ++note)
            {
                TEST_SUCCESS(fluid_synth_noteon(synth, 0, 32 + note, 100));
            }
            if(round == 0)
            {
                TEST_ASSERT(fluid_synth_get_active_voice_count(synth) > 0);
                TEST_ASSERT(fluid_synth_get_active_voice_count(synth) < 8);
            }
            else if(notes == 65)
            {
                TEST_ASSERT(fluid_synth_get_active_voice_count(synth) > 8);
            }
        }
        else if(round % 8 == 4)
        {
            TEST_SUCCESS(fluid_synth_all_notes_off(synth, 0));
        }
        else if(round % 8 == 6)
        {
            TEST_SUCCESS(fluid_synth_all_sounds_off(synth, 0));
        }

        TEST_SUCCESS(fluid_synth_write_float(synth, block, left, frame, 1,
                                             right, frame, 1));
        frame += block;
    }

    TEST_ASSERT(frame == FRAME_COUNT);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

int main(void)
{
    float reference_left[FRAME_COUNT], reference_right[FRAME_COUNT];
    float actual_left[FRAME_COUNT], actual_right[FRAME_COUNT];
    int cores, i, audible_samples = 0;

    render_changing_voices(1, reference_left, reference_right);
    for(i = 0; i < FRAME_COUNT; ++i)
    {
        audible_samples += reference_left[i] != 0 || reference_right[i] != 0;
    }
    TEST_ASSERT(audible_samples > 0);

    for(cores = 2; cores <= 8; cores *= 2)
    {
        render_changing_voices(cores, actual_left, actual_right);
        for(i = 0; i < FRAME_COUNT; ++i)
        {
            TEST_ASSERT(fabsf(reference_left[i] - actual_left[i]) < 1e-3f);
            TEST_ASSERT(fabsf(reference_right[i] - actual_right[i]) < 1e-3f);
        }
    }
    return EXIT_SUCCESS;
}
