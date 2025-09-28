
#include "test.h"
#include "fluidsynth.h"
#include "fluid_mod.h"

#ifndef TRUE
#define TRUE 1
#endif

// Range shall be set to 7bit resolution, i.e. 128
static const fluid_real_t Range = 128.0;

fluid_real_t get_mod_max(const fluid_mod_t *mod)
{
    // The maximum mapped position is always 127/128, see section 9.5.3 of SF2.4
    // For switches however, we keep sticking to fluidsynth historical limits of +-1.0
    return ((mod->flags1 & FLUID_MOD_MAP_MASK) == FLUID_MOD_SWITCH) ? 1.0f : (Range-1)/Range;
}

// this tests ensures that samples with invalid SfSampleType flag combinations are rejected
int main(void)
{
    static const int mapping[] = {FLUID_MOD_SWITCH, FLUID_MOD_LINEAR, FLUID_MOD_CONCAVE, FLUID_MOD_CONVEX};
    unsigned int i;
    fluid_real_t v1, tmp;

    fluid_mod_t* mod = new_fluid_mod();
    fluid_mod_set_source2(mod, 0, 0);
    fluid_mod_set_dest(mod, GEN_ATTENUATION);
    fluid_mod_set_amount(mod, 1);

    // No secondary source given, result must be one
    tmp = Range;
    v1 = fluid_mod_get_source_value(mod->src2, mod->flags2, &tmp, NULL);
    TEST_ASSERT(tmp == Range);
    TEST_ASSERT(v1 == Range);
    v1 = fluid_mod_transform_source_value(mod, v1, mod->flags2, Range, TRUE);
    TEST_ASSERT(v1 == 1.0f);

    for(i = 0; i < FLUID_N_ELEMENTS(mapping); i++)
    {
        {
            static const fluid_real_t mid = 64.0/128.0;
            fluid_mod_set_source1(mod,
                            FLUID_MOD_VELOCITY,
                            FLUID_MOD_GC
                            | mapping[i]
                            | FLUID_MOD_UNIPOLAR
                            | FLUID_MOD_POSITIVE
                            );

            v1 = fluid_mod_transform_source_value(mod, 0, mod->flags1, Range, TRUE);
            TEST_ASSERT(v1 == 0.0f);

            // skip midpoint validation for concave and convex since we're not checking correctness of concave and convex implementations here
            if(mapping[i] != FLUID_MOD_CONCAVE && mapping[i] != FLUID_MOD_CONVEX)
            {
                v1 = fluid_mod_transform_source_value(mod, 64, mod->flags1, Range, TRUE);
                tmp = ((mod->flags1 & FLUID_MOD_MAP_MASK) == FLUID_MOD_SWITCH) ? 1.0f : mid;
                TEST_ASSERT(v1 == tmp);
            }

            v1 = fluid_mod_transform_source_value(mod, 127, mod->flags1, Range, TRUE);
            tmp = get_mod_max(mod);
            TEST_ASSERT(v1 == tmp);
        }

        {
            static const fluid_real_t mid = (64-1)/128.0;
            fluid_mod_set_source1(mod,
                            FLUID_MOD_VELOCITY,
                            FLUID_MOD_GC
                            | mapping[i]
                            | FLUID_MOD_UNIPOLAR
                            | FLUID_MOD_NEGATIVE
                            );

            v1 = fluid_mod_transform_source_value(mod, 127, mod->flags1, Range, TRUE);
            TEST_ASSERT(v1 == 0.0f);

            if(mapping[i] != FLUID_MOD_CONCAVE && mapping[i] != FLUID_MOD_CONVEX)
            {
                v1 = fluid_mod_transform_source_value(mod, 64, mod->flags1, Range, TRUE);
                tmp = ((mod->flags1 & FLUID_MOD_MAP_MASK) == FLUID_MOD_SWITCH) ? 0.0f : mid;
                TEST_ASSERT(v1 == tmp);
            }
            
            v1 = fluid_mod_transform_source_value(mod, 0, mod->flags1, Range, TRUE);
            tmp = get_mod_max(mod);
            TEST_ASSERT(v1 == tmp);
        }

        {
            static const fluid_real_t mid = 0;
            fluid_mod_set_source1(mod,
                            FLUID_MOD_VELOCITY,
                            FLUID_MOD_GC
                            | mapping[i]
                            | FLUID_MOD_BIPOLAR
                            | FLUID_MOD_POSITIVE
                            );

            v1 = fluid_mod_transform_source_value(mod, 0, mod->flags1, Range, TRUE);
            TEST_ASSERT(v1 == -1.0f);

            if(mapping[i] != FLUID_MOD_CONCAVE && mapping[i] != FLUID_MOD_CONVEX)
            {
                v1 = fluid_mod_transform_source_value(mod, 64, mod->flags1, Range, TRUE);
                tmp = ((mod->flags1 & FLUID_MOD_MAP_MASK) == FLUID_MOD_SWITCH) ? 1.0f : mid;
                TEST_ASSERT(v1 == tmp);
            }

            v1 = fluid_mod_transform_source_value(mod, 127, mod->flags1, Range, TRUE);
            tmp = get_mod_max(mod);
            TEST_ASSERT(v1 == tmp);
        }

        {
            static const fluid_real_t mid = -1/64.0;
            fluid_mod_set_source1(mod,
                            FLUID_MOD_VELOCITY,
                            FLUID_MOD_GC
                            | mapping[i]
                            | FLUID_MOD_BIPOLAR
                            | FLUID_MOD_NEGATIVE
                            );

            v1 = fluid_mod_transform_source_value(mod, 127, mod->flags1, Range, TRUE);
            TEST_ASSERT(v1 == -1.0f);

            if(mapping[i] != FLUID_MOD_CONCAVE && mapping[i] != FLUID_MOD_CONVEX)
            {
                v1 = fluid_mod_transform_source_value(mod, 64, mod->flags1, Range, TRUE);
                tmp = ((mod->flags1 & FLUID_MOD_MAP_MASK) == FLUID_MOD_SWITCH) ? -1.0f : mid;
                TEST_ASSERT(v1 == tmp);
            }

            v1 = fluid_mod_transform_source_value(mod, 0, mod->flags1, Range, TRUE);
            tmp = get_mod_max(mod);
            TEST_ASSERT(v1 == tmp);
        }
    }

    delete_fluid_mod(mod);

    return EXIT_SUCCESS;
}
