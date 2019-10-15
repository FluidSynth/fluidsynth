/*----------------------------------------------------------------------------
These tests check fluid_voice_calculate_modulator_contributions() function.

Test of value of linked/simple modulator.
 For simple modulator:
  out = src1  * src2 * amount.
  With src1 = map(CC)
 For a src1 linked modulator:
  out = link * src2 * amount
  With link being the summing node input drived  by the output of
  one or more modulators.

 Note about about test dependency and precedence:
 See test_possible_att_reduction.c and test_fluid_voice_modulate.c.
----------------------------------------------------------------------------*/
#include "test.h"
#include "fluidsynth.h"
#include "utils/fluid_sys.h"
#include "synth/fluid_voice.h"
#include "synth/fluid_chan.h"

int float_equal(fluid_real_t x, fluid_real_t y)
{
	return ( x == y );
}

// test modulators (amount, source, linked modulators...)
int main(void)
{
    static const int CC = 20;
    
    fluid_settings_t* set = new_fluid_settings();
    fluid_synth_t* synth = new_fluid_synth(set);
    fluid_channel_t *ch = new_fluid_channel(synth, 0);
    fluid_voice_t *v = new_fluid_voice(NULL, 22050);
    
    fluid_mod_t *mod0 = new_fluid_mod();
    fluid_mod_t *mod1 = new_fluid_mod();
    fluid_mod_t *mod2 = new_fluid_mod();
    fluid_mod_t *mod3 = new_fluid_mod();

    fluid_gen_init(&v->gen[0], NULL);
    
    fluid_channel_set_cc(ch, CC, 127);
    v->channel = ch;
    v->mod_count = 0;
    
    // set up a valid list of complex modulators with members (mod0,mod1,mod2)
    {
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod1, CC,
                                FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0);
        
        fluid_mod_set_source1(mod2, CC, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);

        /* valid internal list of linked modulators members for a complex modulator (mod0,mod1,mod2).
            Modulator member ordering is expected to be equivalent to the one produced by
            fluid_mod_copy_linked_mod()
        */
        mod0->next = mod1;
        mod1->next = mod2;

        fluid_voice_add_mod_local(v, mod0, FLUID_VOICE_DEFAULT, FLUID_NUM_MOD);
        fluid_voice_calculate_modulator_contributions(v);
        
        TEST_ASSERT(float_equal(v->mod[0].link, fluid_mod_get_amount(mod1) + fluid_mod_get_amount(mod2)));
        TEST_ASSERT(float_equal(v->gen[GEN_FILTERFC].mod, v->mod[0].link * fluid_mod_get_amount(mod0)));
    }
    
    // same list, with additional mod3
    {
        v->gen[GEN_FILTERFC].mod = 0; // reset mod input
        v->mod_count = 0;             // clear voice modulator table.
        
        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        
        fluid_mod_set_source1(mod3, CC, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 50);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 1); // link to mod1

        /* valid internal list of linked modulators members for a complex modulator (mod0,mod1,mod2).
            Modulator member ordering is expected to be equivalent to the one produced by
            fluid_mod_copy_linked_mod()
        */
        mod0->next = mod1;
        mod1->next = mod3;
        mod3->next = mod2;

        fluid_voice_add_mod_local(v, mod0, FLUID_VOICE_DEFAULT, FLUID_NUM_MOD);
        fluid_voice_calculate_modulator_contributions(v);
        
        TEST_ASSERT(float_equal(v->mod[1].link, fluid_mod_get_amount(mod3)));
        TEST_ASSERT(float_equal(v->mod[0].link, fluid_mod_get_amount(mod2) + fluid_mod_get_amount(mod1) * v->mod[1].link));
        TEST_ASSERT(float_equal(v->gen[GEN_FILTERFC].mod, v->mod[0].link * fluid_mod_get_amount(mod0)));
    }

    // Same test using simple modulator: CC->mod0->GEN_FILTERFC
    {
        v->gen[GEN_FILTERFC].mod = 0; // reset mod input
        v->mod_count = 0;             // clear voice modulator table.

        fluid_mod_set_source1(mod0, CC, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);

        // Add one complex modulator using fluid_voice_add_mod().
        fluid_voice_add_mod(v, mod0, FLUID_VOICE_DEFAULT);

        fluid_voice_calculate_modulator_contributions(v);

        TEST_ASSERT(float_equal(v->gen[GEN_FILTERFC].mod, fluid_mod_get_amount(mod0)));
    }

    delete_fluid_mod(mod0);
    delete_fluid_mod(mod1);
    delete_fluid_mod(mod2);
    delete_fluid_mod(mod3);
    
    delete_fluid_voice(v);
    delete_fluid_channel(ch);
    delete_fluid_synth(synth);
    delete_fluid_settings(set);
    
    return EXIT_SUCCESS;
}
