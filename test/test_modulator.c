
#include "test.h"
#include "fluidsynth.h"
#include "fluid_mod.h"


// this tests ensures that samples with invalid SfSampleType flag combinations are rejected
int main(void)
{

    static const fluid_real_t range1 = 128.0, max = 127.0/128.0;

    static const int mapping[] = {FLUID_MOD_SWITCH, FLUID_MOD_LINEAR, FLUID_MOD_CONCAVE, FLUID_MOD_CONVEX};
    unsigned int i;
    fluid_real_t v1, tmp;

    fluid_mod_t* mod = new_fluid_mod();
    fluid_mod_set_source2(mod, 0, 0);
    fluid_mod_set_dest(mod, GEN_ATTENUATION);
    fluid_mod_set_amount(mod, 1);

    // No secondary source given, result must be one
    tmp = range1;
    v1 = fluid_mod_get_source_value(mod->src2, mod->flags2, &tmp, NULL);
    TEST_ASSERT(tmp == range1);
    TEST_ASSERT(v1 == range1);
    v1 = fluid_mod_transform_source_value(mod, v1, mod->flags2, range1, TRUE);
    TEST_ASSERT(v1 == 1.0f);

    for(i = 0; i < FLUID_N_ELEMENTS(mapping); i++)
    {
        {
            fluid_mod_set_source1(mod,
                            FLUID_MOD_VELOCITY,
                            FLUID_MOD_GC
                            | mapping[i]
                            | FLUID_MOD_UNIPOLAR
                            | FLUID_MOD_POSITIVE
                            );

            v1 = fluid_mod_transform_source_value(mod, 0, mod->flags1, range1, TRUE);
            TEST_ASSERT(v1 == 0.0f);

            v1 = fluid_mod_transform_source_value(mod, 64, mod->flags1, range1, TRUE);
            tmp = ((mod->flags1 & FLUID_MOD_SWITCH) == FLUID_MOD_SWITCH) ? 1.0f : 0.5f;
            TEST_ASSERT(v1 == tmp);
            
            v1 = fluid_mod_transform_source_value(mod, 127, mod->flags1, range1, TRUE);
            tmp = ((mod->flags1 & FLUID_MOD_SWITCH) == FLUID_MOD_SWITCH) ? 1.0f : max;
            TEST_ASSERT(v1 == tmp);
        }

        {
            fluid_mod_set_source1(mod,
                            FLUID_MOD_VELOCITY,
                            FLUID_MOD_GC
                            | mapping[i]
                            | FLUID_MOD_UNIPOLAR
                            | FLUID_MOD_NEGATIVE
                            );

            v1 = fluid_mod_transform_source_value(mod, 127, mod->flags1, range1, TRUE);
            TEST_ASSERT(v1 == 0.0f);

            v1 = fluid_mod_transform_source_value(mod, 64, mod->flags1, range1, TRUE);
            tmp = ((mod->flags1 & FLUID_MOD_SWITCH) == FLUID_MOD_SWITCH) ? 0.0f : 0.5f;
            TEST_ASSERT(v1 == tmp);
            
            v1 = fluid_mod_transform_source_value(mod, 0, mod->flags1, range1, TRUE);
            tmp = ((mod->flags1 & FLUID_MOD_SWITCH) == FLUID_MOD_SWITCH) ? 1.0f : max;
            TEST_ASSERT(v1 == tmp);
        }

        {
            fluid_mod_set_source1(mod,
                            FLUID_MOD_VELOCITY,
                            FLUID_MOD_GC
                            | mapping[i]
                            | FLUID_MOD_BIPOLAR
                            | FLUID_MOD_POSITIVE
                            );

            v1 = fluid_mod_transform_source_value(mod, 0, mod->flags1, range1, TRUE);
            TEST_ASSERT(v1 == -1.0f);

            v1 = fluid_mod_transform_source_value(mod, 64, mod->flags1, range1, TRUE);
            tmp = ((mod->flags1 & FLUID_MOD_SWITCH) == FLUID_MOD_SWITCH) ? 1.0f : 0.0f;
            TEST_ASSERT(v1 == tmp);
            
            v1 = fluid_mod_transform_source_value(mod, 127, mod->flags1, range1, TRUE);
            tmp = ((mod->flags1 & FLUID_MOD_SWITCH) == FLUID_MOD_SWITCH) ? 1.0f : max;
            TEST_ASSERT(v1 == tmp);
        }

        {
            fluid_mod_set_source1(mod,
                            FLUID_MOD_VELOCITY,
                            FLUID_MOD_GC
                            | mapping[i]
                            | FLUID_MOD_BIPOLAR
                            | FLUID_MOD_NEGATIVE
                            );

            v1 = fluid_mod_transform_source_value(mod, 127, mod->flags1, range1, TRUE);
            TEST_ASSERT(v1 == -1.0f);

            v1 = fluid_mod_transform_source_value(mod, 64, mod->flags1, range1, TRUE);
            tmp = ((mod->flags1 & FLUID_MOD_SWITCH) == FLUID_MOD_SWITCH) ? -1.0f : 0.0f;
            TEST_ASSERT(v1 == tmp);
            
            v1 = fluid_mod_transform_source_value(mod, 0, mod->flags1, range1, TRUE);
            tmp = ((mod->flags1 & FLUID_MOD_SWITCH) == FLUID_MOD_SWITCH) ? 1.0f : max;
            TEST_ASSERT(v1 == tmp);
        }
    }

    delete_fluid_mod(mod);

    return EXIT_SUCCESS;
}
