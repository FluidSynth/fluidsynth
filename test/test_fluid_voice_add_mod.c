/*----------------------------------------------------------------------------
 Test of API fluid_voice_add_mod()function for adding simple modulators
 in a voice in mode:
 FLUID_VOICE_DEFAULT, FLUID_VOICE_ADD, FLUID_VOICE_OVERWRITE.

 Test of fluid_voice_add_mod_local()function for adding complex modulators
 in a voice in mode:
 FLUID_VOICE_DEFAULT, FLUID_VOICE_ADD, FLUID_VOICE_OVERWRITE.
----------------------------------------------------------------------------*/
#include "test.h"
#include "fluidsynth.h"
#include "utils/fluid_sys.h"
#include "synth/fluid_voice.h"
#include "synth/fluid_chan.h"

//----------------------------------------------------------------------------
static int fluid_compare_simple_mod_structure(fluid_mod_t *mod1, fluid_mod_t *mod2);
static int fluid_compare_complex_mod_structure(fluid_mod_t *cm1, fluid_mod_t *cm2,
                                               double amount2_mul);

//---- Simples modulators ---------------------------------------------------
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

/*----------------------------------------------------------------------------
 1)Adding simple modulators in a voice
   Tests 1_1 to 1_5 add a simple modulator in a voice in mode
   FLUID_VOICE_DEFAULT, FLUID_VOICE_ADD, FLUID_VOICE_OVERWRITE
   using fluid_voice_add_mod().

 2)Adding complex modulators in a voice
   Tests 2_1 to 2_4 add a complex modulator in a voice in mode
   FLUID_VOICE_DEFAULT, FLUID_VOICE_ADD, FLUID_VOICE_OVERWRITE
   using fluid_voice_add_mod_local().
------------------------------------------------------------------------------*/
int main(void)
{
    fluid_voice_t * voice;
    int voice_mod_count; // number of initial modulators
    int voice_mod_count1_1,  voice_mod_count1_2, voice_mod_count1_3;
    int voice_mod_count1_4,  voice_mod_count1_5;
    int voice_mod_count2_1,  voice_mod_count2_2, voice_mod_count2_3;
    int voice_mod_count2_4;

    // members of complex modulator cm0
    fluid_mod_t *cm0_0 = new_fluid_mod();
    fluid_mod_t *cm0_1 = new_fluid_mod();
    fluid_mod_t *cm0_2 = new_fluid_mod();

    // members of complex modulator cm1
    fluid_mod_t *cm1_0 = new_fluid_mod();
    fluid_mod_t *cm1_1 = new_fluid_mod();
    fluid_mod_t *cm1_2 = new_fluid_mod();

    voice = new_fluid_voice(NULL, 22050);
    TEST_ASSERT(voice != NULL);
    voice->channel = NULL;
    voice->mod_count = 0;

    fluid_voice_print_mod(voice); // just to see that there are no modulators
    voice_mod_count = fluid_voice_get_count_modulators(voice);
    TEST_ASSERT(voice_mod_count == 0);

    //-------------------------------------------------------------------------
    // Tests using simple modulators
    //-------------------------------------------------------------------------
    printf("\nTest 1_1: Add a valid new simple modulator to voice in mode FLUID_VOICE_DEFAULT\n");
    {
        fluid_mod_t * mod_out;

        // add a valid new simple modulator in mode FLUID_VOICE_DEFAULT
        fluid_voice_add_mod(voice, mod1_simple_in, FLUID_VOICE_DEFAULT);
        fluid_voice_print_mod(voice);

        // Check if the new modulator has been correctly added in mode
        // FLUID_VOICE_DEFAULT. Voice count of modulators must be voice_mod_count
        // plus one.
        voice_mod_count1_1 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count1_1 == voice_mod_count + 1);

        /* Compare new modulator in voice (mod_out) with modulator
           mod1_simple_in. Both must have equal structure
        */
        mod_out = fluid_voice_get_modulator(voice, voice_mod_count1_1 - 1);
        TEST_ASSERT(mod_out != NULL);

        TEST_ASSERT(fluid_compare_simple_mod_structure(mod_out, mod1_simple_in) == TRUE);
    }

    //-------------------------------------------------------------------------
    printf("\nTest 1_2: Add a valid identical simple modulator to voice in mode FLUID_VOICE_ADD\n");
    {
        fluid_mod_t * mod_out;

        // add a valid identical simple modulator in mode FLUID_VOICE_ADD
        fluid_voice_add_mod(voice, mod1_simple_in, FLUID_VOICE_ADD);
        fluid_voice_print_mod(voice);

        // Check if the identical modulator has been correctly added in mode
        // FLUID_VOICE_ADD. Voice count of modulators must be the same as for
        // test 1_1.
        voice_mod_count1_2 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count1_2 == voice_mod_count1_1);

        /* Compare added modulator in voice (mod_out) with modulator mod1_simple_in.
           - mod_out must be identical to mod1_simple_in
           - mod_out->amount must be: 2 * cm0->amount
        */
        mod_out = fluid_voice_get_modulator(voice, voice_mod_count1_2 - 1);
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
        fluid_voice_print_mod(voice);

        // Check if the identical modulator has been correctly added in mode
        // FLUID_VOICE_OVERWRITE. Voice count of modulators must be the same as for
        // test 1_2.
        voice_mod_count1_3 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count1_3 == voice_mod_count1_2);

        /* Compare added modulator in voice (mod_out) with modulator
           mod1_simple_in. Both must have equal structure
        */
        mod_out = fluid_voice_get_modulator(voice, voice_mod_count1_3 - 1);
        TEST_ASSERT(mod_out != NULL);
        // compare mod_out and mod1_simple_in
        TEST_ASSERT(fluid_compare_simple_mod_structure(mod_out, mod1_simple_in) == TRUE);
    }

    //-------------------------------------------------------------------------
    printf("\nTest 1_4: Add a valid not identical simple modulator to voice in mode FLUID_VOICE_ADD\n");
    {
        fluid_mod_t * mod_out;

        // add a valid identical simple modulator in mode FLUID_VOICE_ADD
        fluid_voice_add_mod(voice, mod2_simple_in, FLUID_VOICE_ADD);
        fluid_voice_print_mod(voice);

        // Check if the modulator hasn' been found identical and not
        // addded in mode FLUID_VOICE_ADD. Voice count of modulators must be the
        // same as for test 1_3 plus one.
        voice_mod_count1_4 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count1_4 == voice_mod_count1_3 + 1);

        /* Compare added modulator in voice (mod_out) with modulator mod1_simple_in. */
        mod_out = fluid_voice_get_modulator(voice, voice_mod_count1_4 - 1);
        TEST_ASSERT(mod_out != NULL);
        // compare mod_out and mod2_simple_in
        TEST_ASSERT(fluid_compare_simple_mod_structure(mod_out, mod2_simple_in) == TRUE);
    }

    //-------------------------------------------------------------------------
    printf("\nTest 1_5: Add an invalid simple modulator to voice in mode FLUID_VOICE_DEFAULT\n");
    {
        // add a valid new simple modulator in mode FLUID_VOICE_DEFAULT
        fluid_voice_add_mod(voice, mod3_simple_in, FLUID_VOICE_DEFAULT);
        fluid_voice_print_mod(voice);
        
        // Check that the new modulator hasn't been added in mode FLUID_VOICE_DEFAULT.
        // Voice count of modulators must be the same as for test 1_4.
        voice_mod_count1_5 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count1_5 == voice_mod_count1_4);
    }

    //-------------------------------------------------------------------------
    // Tests using 2 complex modulators cm0, cm1 (cm1 not identic to cm0).
	// CC20-->cm0_1-->cm0_0-->GEN_FILTERFC     CC20-->cm1_1-->cm1_0-->GEN_FILTERFC
	// CC21-->cm0_2-->cm0_0                    CC22-->cm1_2-->cm1_0
    //-------------------------------------------------------------------------
	{
        // Build complex modulator cm0, cm1

        // Initialize cm0_0: link-->cm0_0-->GEN_FILTERFC
        fluid_mod_set_source1(cm0_0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(cm0_0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (cm0_0, 1.0);
        fluid_mod_set_dest   (cm0_0, GEN_FILTERFC);
        cm0_0->next = cm0_1;

        // Initialize cm0_1: CC20-->cm0_1-->cm0_0
        fluid_mod_set_source1(cm0_1, 20, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(cm0_1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (cm0_1, 2.0);
        fluid_mod_set_dest   (cm0_1, FLUID_MOD_LINK_DEST | 0);
        cm0_1->next = cm0_2;

        // Initialize cm0_2: CC21-->cm0_2-->cm0_0
        fluid_mod_set_source1(cm0_2, 21, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(cm0_2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (cm0_2, 3.0);
        fluid_mod_set_dest   (cm0_2, FLUID_MOD_LINK_DEST | 0);

        // Initialize cm1_0: link-->cm1_0-->GEN_FILTERFC
        fluid_mod_set_source1(cm1_0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(cm1_0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (cm1_0, 4.0);
        fluid_mod_set_dest   (cm1_0, GEN_FILTERFC);
        cm1_0->next = cm1_1;

        // Initialize cm1_1: CC20-->cm1_1-->cm1_0
        fluid_mod_set_source1(cm1_1, 20, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(cm1_1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (cm1_1, 5.0);
        fluid_mod_set_dest   (cm1_1, FLUID_MOD_LINK_DEST | 0);
        cm1_1->next = cm1_2;

        // Initialize cm1_2: CC22-->cm1_2-->cm1_0
        fluid_mod_set_source1(cm1_2, 22, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(cm1_2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (cm1_2, 6.0);
        fluid_mod_set_dest   (cm1_2, FLUID_MOD_LINK_DEST | 0);
    }

    printf("\nTest 2_1: Add a new complex modulator to voice in mode FLUID_VOICE_DEFAULT\n");
    {
        fluid_mod_t * mod_out;

        // add a new complex modulator cm0 in mode FLUID_VOICE_DEFAULT
        fluid_voice_add_mod_local(voice, cm0_0, FLUID_VOICE_DEFAULT, FLUID_NUM_MOD);
        fluid_voice_print_mod(voice);

        // Check if the new modulator has been correctly added in mode
        // FLUID_VOICE_DEFAULT. Voice count of modulators must be voice_mod_count1_5
        // plus one.
        voice_mod_count2_1 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count2_1 == voice_mod_count1_5 + 1);

        /* Compare new modulator in voice (mod_out) with modulator
           cm0_0. Both must have equal structure
        */
        mod_out = fluid_voice_get_modulator(voice, voice_mod_count2_1 - 1);
        TEST_ASSERT(mod_out != NULL);
        TEST_ASSERT(fluid_compare_complex_mod_structure(mod_out, cm0_0, 1.0) == TRUE);
    }

    //-------------------------------------------------------------------------
    printf("\nTest 2_2: Add an identical complex to voice in mode FLUID_VOICE_ADD\n");
    {
        fluid_mod_t * mod_out;

        // add an identical complex modulator in mode FLUID_VOICE_ADD
        fluid_voice_add_mod_local(voice, cm0_0, FLUID_VOICE_ADD, FLUID_NUM_MOD);
        fluid_voice_print_mod(voice);

        // Check if the identical modulator has been correctly added in mode
        // FLUID_VOICE_ADD. Voice count of modulators must be the same as for
        // test 2_1.
        voice_mod_count2_2 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count2_2 == voice_mod_count2_1);

        /* Compare added modulator in voice (mod_out) with modulator cm_0.
           - mod_out must be identical to cm0
           - mod_out members's amount must be: 2 * cm0 members's amount
        */
        mod_out = fluid_voice_get_modulator(voice, voice_mod_count2_2 - 1);
        TEST_ASSERT(mod_out != NULL);
        // compare mod_out and cm0
        TEST_ASSERT(fluid_compare_complex_mod_structure(mod_out, cm0_0, 2.0) == TRUE);
    }

    //-------------------------------------------------------------------------
    printf("\nTest 2_3: Add an identical complex modulator to voice in mode FLUID_VOICE_OVERWRITE\n");
    {
        fluid_mod_t * mod_out;

        // add an identical complex modulator in mode FLUID_VOICE_OVERWRITE
        fluid_voice_add_mod_local(voice, cm0_0, FLUID_VOICE_OVERWRITE, FLUID_NUM_MOD);
        fluid_voice_print_mod(voice);

        // Check if the identical modulator has been correctly added in mode
        // FLUID_VOICE_OVERWRITE. Voice count of modulators must be the same as for
        // test 2_2.
        voice_mod_count2_3 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count2_3 == voice_mod_count2_2);

        /* Compare added modulator in voice (mod_out) with modulator
           cm0. Both must have equal structure
        */
        mod_out = fluid_voice_get_modulator(voice, voice_mod_count2_3 - 1);
        TEST_ASSERT(mod_out != NULL);
        // compare mod_out and cm0
        TEST_ASSERT(fluid_compare_complex_mod_structure(mod_out, cm0_0, 1.0) == TRUE);
    }

    //-------------------------------------------------------------------------
    printf("\nTest 2_4: Add a not identical complex modulator to voice in mode FLUID_VOICE_ADD\n");
    {
        fluid_mod_t * mod_out;

        // add a not identical complex modulator in mode FLUID_VOICE_ADD
        fluid_voice_add_mod_local(voice, cm1_0, FLUID_VOICE_ADD, FLUID_NUM_MOD);
        fluid_voice_print_mod(voice);

        // Check if the modulator hasn' been found identical and not
        // addded in mode FLUID_VOICE_ADD. Voice count of modulators must be the
        // same as for test 2_3 plus one.
        voice_mod_count2_4 = fluid_voice_get_count_modulators(voice);
        TEST_ASSERT(voice_mod_count2_4 == voice_mod_count2_3 + 1);

        /* Compare added modulator in voice (mod_out) with modulator cm1.*/
        mod_out = fluid_voice_get_modulator(voice, voice_mod_count2_4 - 1);
        TEST_ASSERT(mod_out != NULL);
        // compare mod_out and cm1
        TEST_ASSERT(fluid_compare_complex_mod_structure(mod_out, cm1_0, 1.0) == TRUE);
    }

	//freeing
    delete_fluid_mod(cm0_0);
    delete_fluid_mod(cm0_1);
    delete_fluid_mod(cm0_2);

    delete_fluid_mod(cm1_0);
    delete_fluid_mod(cm1_1);
    delete_fluid_mod(cm1_2);

    delete_fluid_voice(voice);
    return EXIT_SUCCESS;
}

/**
 * Compare 2 simple modulators structure
 * Return TRUE if mod1 fluid_mod_t fields are equal to mod1 fluid_mod_t fields.
 * @param mod1, mod2 pointers on fluid_mod_t structures.
 */
static int fluid_compare_simple_mod_structure(fluid_mod_t *mod1, fluid_mod_t *mod2)
{
    return ( fluid_mod_test_identity(mod1, mod2) &&
            (mod1->amount == mod2->amount));
}

/**
 * Compare 2 complex modulators structures.
 * Return TRUE if cm1's members  are equal to cm2's members
 * fluid_mod_t fields.
 * @param cm1, cm2 pointers on complex modulators.
 * @param amount2_mul, multiplier applied to cm2 members amount during comparison.
 * (cm1 members's amount is compared to cm0 members's amount * amount2_mul)
 */
static int fluid_compare_complex_mod_structure(fluid_mod_t *cm1, fluid_mod_t *cm2,
                                               double amount2_mul)
{
    int offset1, offset2;
    int count1 = fluid_get_num_mod(cm1);
    int count2 = fluid_get_num_mod(cm2);

    // compare members count and ending modulators
    if ((count1 != count2)
        || ! fluid_mod_test_identity(cm1, cm2)
        || cm1->amount != (cm2->amount * amount2_mul))
    {
        return FALSE;
    }

    // now compare branches
	cm1 = cm1->next;
	cm2 = cm2->next;

    offset1 = cm1->dest &~FLUID_MOD_LINK_DEST;
    offset2 = cm2->dest &~FLUID_MOD_LINK_DEST;
    while(cm1)
    {
        if( cm1->dest - offset1 != cm2->dest - offset2
           || cm1->src1 != cm2->src1
           || cm1->src2 != cm2->src2
           || cm1->flags1 != cm2->flags1
           || cm1->flags2 != cm2->flags2
           || cm1->amount != (cm2->amount * amount2_mul))
        {
            return FALSE;
        }
        cm1 = cm1->next;
        cm2 = cm2->next;
    }
    return TRUE;
}
