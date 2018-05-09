
#include "test.h"
#include "fluidsynth.h"

// this test should make sure that sample rate changed are handled correctly
int main(void)
{
    fluid_synth_t *synth;
    fluid_settings_t *settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);
    
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.roomsize", 1.0));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.damp", 1.0));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.width", 1.0));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.level", 1.0));
    
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.chorus.nr", 99));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.level", 1.0));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.speed", 1.0));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.depth", 1.0));
  
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);    
    
    TEST_ASSERT(fluid_synth_get_reverb_roomsize(synth) == 1.0);
    TEST_ASSERT(fluid_synth_get_reverb_damp(synth) == 1.0);
    TEST_ASSERT(fluid_synth_get_reverb_width(synth) == 1.0);
    TEST_ASSERT(fluid_synth_get_reverb_level(synth) == 1.0);
    
    TEST_ASSERT(fluid_synth_get_chorus_nr(synth) == 99);
    TEST_ASSERT(fluid_synth_get_chorus_level(synth) == 1.0);
    TEST_ASSERT(fluid_synth_get_chorus_speed_Hz(synth) == 1.0);
    TEST_ASSERT(fluid_synth_get_chorus_depth_ms(synth) == 1.0);
    
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    
    return EXIT_SUCCESS;
}
