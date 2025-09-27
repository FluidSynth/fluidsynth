
#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include <string.h>

// static const int CHANNELS=16;
enum { SAMPLES=1024 };

static int smpl;

void synth_global_default_mods(fluid_synth_t *synth)
{
}

void sfont_default_mods(fluid_sfont_t *sfont)
{
    int res, i;
    fluid_mod_t mods[3];
    fluid_mod_t *def_mod, *my_mod;

    TEST_ASSERT(sfont->default_mod_list == NULL);
    res = fluid_sfont_get_default_mod(sfont, &def_mod);
    TEST_ASSERT(res == 0);
    TEST_ASSERT(def_mod != NULL);
    fluid_free(def_mod);
    def_mod = NULL;

    res = fluid_sfont_set_default_mod(sfont, NULL, 0);
    TEST_SUCCESS(res);
    TEST_ASSERT(sfont->default_mod_list == NULL);

    my_mod = &mods[0];
    fluid_mod_set_source1(my_mod,
                              FLUID_MOD_VELOCITY,
                              FLUID_MOD_GC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE);
    fluid_mod_set_source2(my_mod, FLUID_MOD_NONE, 0);
    fluid_mod_set_dest(my_mod, GEN_ATTENUATION);
    fluid_mod_set_amount(my_mod, 960 * 0.4);

    my_mod = &mods[1];
    fluid_mod_set_source1(my_mod,
                            7,
                            FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE);

    my_mod = &mods[2];
    fluid_mod_set_source1(my_mod,
                            11,
                            FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE);

    res = fluid_sfont_set_default_mod(sfont, mods, FLUID_N_ELEMENTS(mods));
    TEST_SUCCESS(res);
    TEST_ASSERT(sfont->default_mod_list != NULL);

    res = fluid_sfont_get_default_mod(sfont, &def_mod);
    TEST_ASSERT(res == 3);
    TEST_ASSERT(def_mod != NULL);

    for(i=0; i<res; i++)
    {
        TEST_ASSERT(fluid_mod_test_identity(&def_mod[i], &mods[i]));
    }

    fluid_free(def_mod);
    def_mod = NULL;

    res = fluid_sfont_set_default_mod(sfont, NULL, 0);
    TEST_SUCCESS(res);
    TEST_ASSERT(sfont->default_mod_list == NULL);
}

// this test should make sure that sample rate changed are handled correctly
int main(void)
{
    int off=0;
    fluid_mod_t *mod;
    fluid_sfont_t *sfont;
    fluid_synth_t *synth;

    mod = new_fluid_mod();
    TEST_ASSERT(mod != NULL);

    sfont = new_fluid_sfont_local(NULL, NULL, NULL, NULL, NULL);
    TEST_ASSERT(sfont != NULL);

    fluid_settings_t *settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    synth_global_default_mods(synth);
    sfont_default_mods(sfont);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    delete_fluid_sfont(sfont);
    delete_fluid_mod(mod);

    return EXIT_SUCCESS;
}
