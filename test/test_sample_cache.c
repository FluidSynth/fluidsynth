
#include "test.h"
#include "fluidsynth.h" // use local fluidsynth header


// this test aims to make sure that sample data used by multiple synths is not freed
// once unloaded by its parent synth
int main(void)
{
    enum { FRAMES = 1024 };
    float buf[FRAMES * 2];
    char *s;
    
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth1 = new_fluid_synth(settings),
                  *synth2 = new_fluid_synth(settings);
    
    TEST_ASSERT_NEQ(settings, NULL);
    TEST_ASSERT_NEQ(synth1, NULL);
    TEST_ASSERT_NEQ(synth2, NULL);
    
    TEST_SUCCESS(fluid_settings_dupstr(settings, "synth.default-soundfont", &s))
        
    TEST_ASSERT_NEQ(s[0], '\0');
    
    printf("Attempt to open %s\n", s);
    
    // load a sfont to synth1
    TEST_SUCCESS(fluid_synth_sfload(synth1, s, 1));
    
    // load the same font to synth2 and start a note
    TEST_SUCCESS(fluid_synth_sfload(synth2, s, 1));
    TEST_SUCCESS(fluid_synth_noteon(synth2, 0, 60, 127));
        
    // render some audio
    TEST_SUCCESS(fluid_synth_write_float(synth2, FRAMES, buf, 0, 2, buf, 1, 2));
    
    // delete the synth that owns the soundfont
    delete_fluid_synth(synth1);
    
    // render again with the unloaded sfont and hope no segfault happens
    TEST_SUCCESS(fluid_synth_write_float(synth2, FRAMES, buf, 0, 2, buf, 1, 2));
        
    delete_fluid_synth(synth2);
    delete_fluid_settings(settings);
    
    return EXIT_SUCCESS;
}
