
#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include "fluid_mod.h"
#include "fluid_sfont.h"

unsigned int count_modulator_list(fluid_mod_t *mod)
{
    unsigned int i = 0;
    while(mod)
    {
        mod = mod->next;
        i++;
    }
    return i;
}

void synth_global_default_mods(fluid_synth_t *synth)
{
    // Custom Modulators borrowed from https://github.com/spessasus/spessasynth_core/blob/6ce2aa37cec01c6345d08c816298706dc2e8d69d/src/soundbank/basic_soundbank/modulator.ts#L304
    // ... might turn out to be useful
    fluid_mod_t mod[6] = {0};
    fluid_mod_t *my_mod = &mod[0];

    unsigned int n = count_modulator_list(synth->default_mod);
    TEST_ASSERT(n == 11);

    // Poly pressure to vibrato
    fluid_mod_set_source1(my_mod,
                            FLUID_MOD_KEYPRESSURE,
                            FLUID_MOD_GC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
    fluid_mod_set_source2(my_mod, FLUID_MOD_NONE, 0);
    fluid_mod_set_dest(my_mod, GEN_VIBLFOTOPITCH);
    fluid_mod_set_amount(my_mod, 50);
    TEST_SUCCESS(fluid_synth_add_default_mod(synth, my_mod, FLUID_SYNTH_OVERWRITE));

    my_mod = &mod[1];
    // Cc 92 (tremolo) to modLFO volume
    fluid_mod_set_source1(my_mod,
                            EFFECTS_DEPTH2,
                            FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
    fluid_mod_set_source2(my_mod, FLUID_MOD_NONE, 0);
    fluid_mod_set_dest(my_mod, GEN_MODLFOTOVOL);
    fluid_mod_set_amount(my_mod, 24);
    TEST_SUCCESS(fluid_synth_add_default_mod(synth, my_mod, FLUID_SYNTH_OVERWRITE));

    my_mod = &mod[2];
    // Cc 73 (attack time) to volEnv attack
    fluid_mod_set_source1(my_mod,
                            SOUND_CTRL4,
                            FLUID_MOD_CC | FLUID_MOD_CONVEX | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
    fluid_mod_set_source2(my_mod, FLUID_MOD_NONE, 0);
    fluid_mod_set_dest(my_mod, GEN_VOLENVATTACK);
    fluid_mod_set_amount(my_mod, 6000);
    TEST_SUCCESS(fluid_synth_add_default_mod(synth, my_mod, FLUID_SYNTH_OVERWRITE));

    my_mod = &mod[3];
    // Cc 72 (release time) to volEnv release
    fluid_mod_set_source1(my_mod,
                            SOUND_CTRL3,
                            FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
    fluid_mod_set_source2(my_mod, FLUID_MOD_NONE, 0);
    fluid_mod_set_dest(my_mod, GEN_VOLENVRELEASE);
    fluid_mod_set_amount(my_mod, 3600);
    TEST_SUCCESS(fluid_synth_add_default_mod(synth, my_mod, FLUID_SYNTH_OVERWRITE));

    my_mod = &mod[4];
    // Cc 74 (brightness) to filterFc
    fluid_mod_set_source1(my_mod,
                            SOUND_CTRL5,
                            FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
    fluid_mod_set_source2(my_mod, FLUID_MOD_NONE, 0);
    fluid_mod_set_dest(my_mod, GEN_FILTERFC);
    fluid_mod_set_amount(my_mod, 6000);
    TEST_SUCCESS(fluid_synth_add_default_mod(synth, my_mod, FLUID_SYNTH_OVERWRITE));

    my_mod = &mod[5];
    // Cc 71 (filter Q) to filter Q (default resonant modulator)
    fluid_mod_set_source1(my_mod,
                            SOUND_CTRL2,
                            FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
    fluid_mod_set_source2(my_mod, FLUID_MOD_NONE, 0);
    fluid_mod_set_dest(my_mod, GEN_FILTERQ);
    fluid_mod_set_amount(my_mod, 250);
    TEST_SUCCESS(fluid_synth_add_default_mod(synth, my_mod, FLUID_SYNTH_OVERWRITE));

    n = count_modulator_list(synth->default_mod);
    TEST_ASSERT(n == 11+6);

    TEST_SUCCESS(fluid_synth_remove_default_mod(synth, &default_vel2att_mod));
    TEST_SUCCESS(fluid_synth_remove_default_mod(synth, &default_vel2filter_mod));
    TEST_SUCCESS(fluid_synth_remove_default_mod(synth, &default_at2viblfo_mod));
    TEST_SUCCESS(fluid_synth_remove_default_mod(synth, &default_mod2viblfo_mod));
    TEST_SUCCESS(fluid_synth_remove_default_mod(synth, &default_att_mod));
    TEST_SUCCESS(fluid_synth_remove_default_mod(synth, &default_pan_mod));
    TEST_SUCCESS(fluid_synth_remove_default_mod(synth, &default_expr_mod));
    TEST_SUCCESS(fluid_synth_remove_default_mod(synth, &default_reverb_mod));
    TEST_SUCCESS(fluid_synth_remove_default_mod(synth, &default_chorus_mod));
    TEST_SUCCESS(fluid_synth_remove_default_mod(synth, &default_pitch_bend_mod));
    TEST_SUCCESS(fluid_synth_remove_default_mod(synth, &custom_balance_mod));

    n = count_modulator_list(synth->default_mod);
    TEST_ASSERT(n == 6);

    for(n=0; n< FLUID_N_ELEMENTS(mod); n++)
    {
        TEST_SUCCESS(fluid_synth_remove_default_mod(synth, &mod[n]));
    }

    TEST_ASSERT(synth->default_mod == NULL);
    n = count_modulator_list(synth->default_mod);
    TEST_ASSERT(n == 0);

    n = fluid_synth_remove_default_mod(synth, &default_pitch_bend_mod);
    TEST_ASSERT(n == FLUID_FAILED);

    TEST_SUCCESS(fluid_synth_add_default_mod(synth, &default_vel2att_mod, FLUID_SYNTH_ADD));

    n = count_modulator_list(synth->default_mod);
    TEST_ASSERT(n == 1);
    TEST_ASSERT(fluid_mod_test_identity(synth->default_mod, &default_vel2att_mod));

    n = fluid_synth_remove_default_mod(synth, &default_pitch_bend_mod);
    TEST_ASSERT(n == FLUID_FAILED);

    n = count_modulator_list(synth->default_mod);
    TEST_ASSERT(n == 1);
}

void sfont_default_mods(fluid_sfont_t *sfont)
{
    int res, i;
    fluid_mod_t mods[3] = {0};
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

    res = fluid_sfont_set_default_mod(sfont, (void*)0xE6AL, 0);
    TEST_SUCCESS(res);
    TEST_ASSERT(sfont->default_mod_list == NULL);
}

// this test should make sure that sample rate changed are handled correctly
int main(void)
{
    fluid_mod_t *mod;
    fluid_sfont_t *sfont;
    fluid_settings_t *settings;
    fluid_synth_t *synth;

    mod = new_fluid_mod();
    TEST_ASSERT(mod != NULL);

    sfont = new_fluid_sfont_local(NULL, NULL, NULL, NULL, NULL);
    TEST_ASSERT(sfont != NULL);

    settings = new_fluid_settings();
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
