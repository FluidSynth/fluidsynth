
#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "utils/fluidsynth_priv.h"


// this test aims to make sure that sample data used by multiple synths is not freed
// once unloaded by its parent synth
int main(void)
{
    char *s;
    int id;
    fluid_sfont_t *sfont;
    
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth = new_fluid_synth(settings);
    
    TEST_ASSERT_NEQ(settings, NULL);
    TEST_ASSERT_NEQ(synth, NULL);
    
    // no sfont loaded
    TEST_ASSERT_EQ(fluid_synth_sfcount(synth), 0);
    
    TEST_SUCCESS(fluid_settings_dupstr(settings, "synth.default-soundfont", &s))
        
    TEST_ASSERT_NEQ(s[0], '\0');
    
    FLUID_LOG(FLUID_INFO, "Attempt to open %s", s);
    
    // load a sfont to synth
    TEST_SUCCESS(id = fluid_synth_sfload(synth, s, 1));
    // one sfont loaded
    TEST_ASSERT_EQ(fluid_synth_sfcount(synth), 1);    
    TEST_ASSERT_NEQ(sfont = fluid_synth_get_sfont_by_id(synth, id), NULL);
    
    // this is still the same filename as we've put in
    TEST_ASSERT_EQ(FLUID_STRCMP(s, fluid_sfont_get_name(sfont)), 0);
    TEST_ASSERT_EQ(fluid_sfont_get_id(sfont), id);
    
    // still the same id?
    TEST_ASSERT_EQ(fluid_synth_sfreload(synth, id), id);
    // one sfont loaded
    TEST_ASSERT_EQ(fluid_synth_sfcount(synth), 1);    
    TEST_ASSERT_NEQ(sfont = fluid_synth_get_sfont_by_id(synth, id), NULL);
    
    // still the same filename?
    TEST_ASSERT_EQ(FLUID_STRCMP(s, fluid_sfont_get_name(sfont)), 0);
    // correct id stored?
    TEST_ASSERT_EQ(fluid_sfont_get_id(sfont), id);
    
    // remove the sfont without deleting
    TEST_SUCCESS(fluid_synth_remove_sfont(synth, sfont));
    // no sfont loaded
    TEST_ASSERT_EQ(fluid_synth_sfcount(synth), 0);
    
    // re-add the sfont without deleting
    TEST_SUCCESS(id = fluid_synth_add_sfont(synth, sfont));
    // one sfont loaded
    TEST_ASSERT_EQ(fluid_synth_sfcount(synth), 1);
    
    // destroy the sfont
    TEST_SUCCESS(fluid_synth_sfunload(synth, id, 0));
    // no sfont loaded
    TEST_ASSERT_EQ(fluid_synth_sfcount(synth), 0);
    
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    
    return EXIT_SUCCESS;
}
