/*----------------------------------------------------------------------------
Test of API fluid_voice_add_mod()function.

Adding simple modulators in a voice in mode:
FLUID_VOICE_DEFAULT, FLUID_VOICE_ADD, FLUID_VOICE_OVERWRITE.
----------------------------------------------------------------------------*/
#include "test.h"
#include "fluidsynth.h"
#include "utils/fluid_sys.h"
#include "synth/fluid_voice.h"
#include "synth/fluid_chan.h"

//----------------------------------------------------------------------------
static int fluid_compare_mod_structure(fluid_mod_t *mod1, fluid_mod_t *mod2);

//----------------------------------------------------------------------------
/*-- modulators to add to voice */
/* valid simple modulator (i.e unlinked): sources cc,  sources cc 3 valid */
fluid_mod_t mod1_simple_in[] =
{
    {
        // dest         , src1             , flags1      , src2          , flags2
        GEN_ATTENUATION , 3                , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
        // amount, link, next
        10.0      , 0.0 , NULL
    }
};

/* valid simple modulator (i.e unlinked): sources cc,  sources cc 4 valid */
fluid_mod_t mod2_simple_in[] =
{
    {
        // dest         , src1             , flags1      , src2          , flags2
        GEN_ATTENUATION , 4                , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
        // amount, link, next
        10.0      , 0.0 , NULL
    }
};

/* invalid simple modulator (i.e unlinked): sources cc,  sources cc 0 invalid */
fluid_mod_t mod3_simple_in[] =
{
    {
        // dest         , src1              , flags1      , src2          , flags2
        2               , 0                 , FLUID_MOD_CC, FLUID_MOD_NONE, FLUID_MOD_GC,
        // amount, link, next
        10.0      , 0.0 , NULL
    }
};

//----------------------------------------------------------------------------
// Adding simple modulators in a voice
// These tests add a simple modulator in a voice in mode 
// FLUID_VOICE_DEFAULT, FLUID_VOICE_ADD, FLUID_VOICE_OVERWRITE.
int main(void)
{
    fluid_voice_t * voice;
    int voice_mod_count; // numbers of default modulator
    int voice_mod_count1,  voice_mod_count2, voice_mod_count3;
    int voice_mod_count4,  voice_mod_count5;

    voice = new_fluid_voice(NULL, 22050);
    TEST_ASSERT(voice != NULL);
    fluid_print_voice_mod(voice);
    voice_mod_count = fluid_voice_get_count_modulators(voice);
    TEST_ASSERT(voice_mod_count == 0);

    //-------------------------------------------------------------------------
    printf("\nTest 1_1: Add a valid new simple modulator to voice in mode FLUID_VOICE_DEFAULT\n");
    {
        fluid_mod_t * mod_out;

        // add a valid new simple modulator in mode FLUID_VOICE_DEFAULT
        fluid_voice_add_mod(voice, mod1_simple_in, FLUID_VOICE_DEFAULT);
        fluid_print_voice_mod(voice);

        // Check if the new modulator has been correctly added in mode
        // FLUID_VOICE_DEFAULT. Voice count of modulators must be voice_mod_count
        // plus one.
        voice_mod_count1 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count1 == voice_mod_count + 1);

        /* Compare new modulator in voice (mod_out) with modulator
           mod1_simple_in. Both must have equal structure
        */
        mod_out = fluid_voice_get_modulator(voice, voice_mod_count1 - 1);
        TEST_ASSERT(mod_out != NULL);

        TEST_ASSERT(fluid_compare_mod_structure(mod_out, mod1_simple_in) == TRUE);
    }

    //-------------------------------------------------------------------------
    printf("\nTest 1_2: Add a valid identical simple modulator to voice in mode FLUID_VOICE_ADD\n");
    {
        fluid_mod_t * mod_out;

        // add a valid identical simple modulator in mode FLUID_VOICE_ADD
        fluid_voice_add_mod(voice, mod1_simple_in, FLUID_VOICE_ADD);
        fluid_print_voice_mod(voice);

        // Check if the identical modulator has been correctly added in mode
        // FLUID_VOICE_ADD. Voice count of modulators must be the same as for
        // test 1_1.
        voice_mod_count2 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count2 == voice_mod_count1);

        /* Compare added modulator in voice (mod_out) with modulator mod1_simple_in.
           - mod_out must be identical to mod1_simple_in
           - mod_out->amount must be: 2 * mod1_simple_in->amount 
        */
        mod_out = fluid_voice_get_modulator(voice, voice_mod_count2 - 1);
        TEST_ASSERT(mod_out != NULL);
        // compare mod_out and mod1_simple_in
        TEST_ASSERT(fluid_mod_test_identity(mod_out, mod1_simple_in) == TRUE);
        TEST_ASSERT(mod_out->amount == (2 * mod1_simple_in->amount));
    }

    //-------------------------------------------------------------------------
    printf("\nTest 1_3: Add a valid identical simple modulator to voice in mode FLUID_VOICE_OVERWRITE\n");
    {
        fluid_mod_t * mod_out;

        // add a valid identical simple modulator in mode FLUID_VOICE_OVERWRITE
        fluid_voice_add_mod(voice, mod1_simple_in, FLUID_VOICE_OVERWRITE);
        fluid_print_voice_mod(voice);

        // Check if the identical modulator has been correctly added in mode
        // FLUID_VOICE_OVERWRITE. Voice count of modulators must be the same as for
        // test 1_2.
        voice_mod_count3 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count3 == voice_mod_count2);

        /* Compare added modulator in voice (mod_out) with modulator
           mod1_simple_in. Both must have equal structure
        */
        mod_out = fluid_voice_get_modulator(voice, voice_mod_count3 - 1);
        TEST_ASSERT(mod_out != NULL);
        // compare mod_out and mod1_simple_in
        TEST_ASSERT(fluid_compare_mod_structure(mod_out, mod1_simple_in) == TRUE);
    }

    //-------------------------------------------------------------------------
    printf("\nTest 1_4: Add a valid not identical simple modulator to voice in mode FLUID_VOICE_ADD\n");
    {
        fluid_mod_t * mod_out;

        // add a valid identical simple modulator in mode FLUID_VOICE_ADD
        fluid_voice_add_mod(voice, mod2_simple_in, FLUID_VOICE_ADD);
        fluid_print_voice_mod(voice);

        // Check if the identical modulator has been correctly added in mode
        // FLUID_VOICE_ADD. Voice count of modulators must be the same as for
        // test 1_3 plus one.
        voice_mod_count4 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count4 == voice_mod_count3 + 1);

        /* Compare added modulator in voice (mod_out) with modulator mod1_simple_in.
           - mod_out must be identical to mod1_simple_in
           - mod_out->amount must be: 2 * mod1_simple_in->amount 
        */
        mod_out = fluid_voice_get_modulator(voice, voice_mod_count4 - 1);
        TEST_ASSERT(mod_out != NULL);
        // compare mod_out and mod2_simple_in
        TEST_ASSERT(fluid_compare_mod_structure(mod_out, mod2_simple_in) == TRUE);
    }

    //-------------------------------------------------------------------------
    printf("\nTest 1_5: Add an invalid simple modulator to voice in mode FLUID_VOICE_DEFAULT\n");
    {
        // add a valid new simple modulator in mode FLUID_VOICE_DEFAULT
        fluid_voice_add_mod(voice, mod3_simple_in, FLUID_VOICE_DEFAULT);
        fluid_print_voice_mod(voice);
        
        // Check if the new modulator has been correctly added in mode
        // FLUID_VOICE_DEFAULT. Voice count of modulators must be voice_mod_count
        // plus one.
        voice_mod_count5 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count5 == voice_mod_count4);
    }

    //freeing
    delete_fluid_voice(voice);
    return EXIT_SUCCESS;
}

/**
 * Return TRUE if mod1 fluid_mod_t fields are equal to mod1 fluid_mod_t fields.
 * param mod1, mod2 pointer on fluid_mod_t structures.
 */
static int fluid_compare_mod_structure(fluid_mod_t *mod1, fluid_mod_t *mod2)
{
    return ( fluid_mod_test_identity(mod1, mod2) &&
            (mod1->amount == mod2->amount));
}
