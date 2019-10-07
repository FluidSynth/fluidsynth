/*----------------------------------------------------------------------------
 Test of modulation contribution produced by fluid_voice_modulate().

 These tests use the modulation value (mod_modulation) produced by
 fluid_voice_modulate() with one modulator (simple or complex) at a time in the
 voice.
 Then mod_modulation is compared to the expected modulation value
 (mod_modulation_expected).

 The comparison is done 2 times:
 1) mod_modulation is compared to theorical expected modulation value.
 2) mod_modulation is compared to the modulation computed by running
   fluid_voice_calculate_modulator_contributions()

 Tests must be done for each type of modulator (simple or complex).
----------------------------------------------------------------------------*/
#include "test.h"
#include "fluidsynth.h"
#include "utils/fluid_sys.h"
#include "synth/fluid_voice.h"
#include "synth/fluid_chan.h"

//-----------------------------------------------------------------------------
// Externals
void fluid_voice_calculate_modulator_contributions(fluid_voice_t *voice);

int fluid_voice_modulate(fluid_voice_t* voice, int cc, int ctrl);

void fluid_voice_add_mod_local(fluid_voice_t *voice, fluid_mod_t *mod, int mode,
                               int check_limit_count);

//-----------------------------------------------------------------------------
/**
 * Compute expected modulation a  modulator will supply by
 * calling fluid_voice_calculate_modulator_contributions().
 *
 * @param voice, the voice that must contains only one  modulator
 * (simple or complex).
 * @return the expected modulation this modulator will supply.
 */
static fluid_real_t get_expected_mod_modulation(fluid_voice_t *voice)
{
    int i;
    // Check that the voice contains one modulator
    TEST_ASSERT(fluid_voice_get_count_modulators(voice) == 1);

    // calculate generator mod value
    for(i = 0; i < voice->mod_count; i++)
    {
        voice->mod[i].link = 0.0; // reset link input
    }
    voice->gen[GEN_FILTERFC].mod = 0; // reset mod input
    fluid_voice_calculate_modulator_contributions(voice);

    return  voice->gen[GEN_FILTERFC].mod; // return  mod input}
}

/*-- functions for simple modulator -----------------------------------------*/

/**
 * - Initialize a simple modulator (mod = bipo(cc20) * amount) and put it in voice.
 * - Compute modulation given by this simple modulator by calling fluid_voice_modulate()
 *   and return this value.
 *
 * @param voice, the voice to initialize.
 * @param mod, the simple modulator to initialize.
 * @param src1_cc_value, src1_cc value.
 * @param amount, amount value.
 */
static fluid_real_t get_simple_mod_modulation(fluid_voice_t *voice, fluid_mod_t *mod,
                                    int src1_cc_value,
                                    double amount)
{
    static const int src1_cc = 20;

    // Initialize CC values in channel
    fluid_channel_set_cc(voice->channel, src1_cc, src1_cc_value);

    //initialize modulators sources and amount values.
    fluid_mod_set_source1(mod, src1_cc, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
    fluid_mod_set_source2(mod, FLUID_MOD_NONE, FLUID_MOD_GC);
    fluid_mod_set_amount (mod, amount);
    fluid_mod_set_dest(mod, GEN_FILTERFC);

    // Add one simple modulator using fluid_voice_add_mod().
    voice->mod_count = 0;             // clear voice modulator table.
    fluid_voice_add_mod(voice, mod, FLUID_VOICE_DEFAULT);

    /* Compute modulation by calling fluid_voice_modulate() */
    TEST_ASSERT(fluid_voice_modulate(voice, 1, src1_cc) == FLUID_OK);

    return  voice->gen[GEN_FILTERFC].mod; // return  mod input
}

//-- functions for complex modulator ------------------------------------------
/**
 * - Initialize a complex modulator: (m2 + m1)-->m0-->GEN_ATTENUATION with
 *   m0: source1 linked,   source2 unip, CC20,   destination GEN_FILTERFC
 *   m1: source1 bip, CC21,     source2 none,         destination m0
 *   m2: source1 bip, CC22,     source2 none,         destination m0
 * - Put this modulator in voice.
 * - Compute modulation given by this complex modulator by calling
 *   fluid_voice_modulate() and return this value.
 *
 * @param voice, the voice to initialize.
 * @param m0, ending modulator member.
 * @param m1, member modulator linked to m0.
 * @param m2, member modulator linked to m0.
 *
 * @param m0_src2_cc_value, cc value on source2 of m0 (0..127).
 * @param m1_src1_cc_value, cc value on source1 of m1 (0..127).
 * @param m2_src1_cc_value, cc value on source1 of m2 (0..127).
 *
 * @param m0_amount, amount value of m0.
 * @param m1_amount, amount value of m1.
 * @param m2_amount, amount value of m2.
 */
static fluid_real_t get_complex_mod_modulation(fluid_voice_t *voice,
                      // modulators: m0, m1, m2
                      fluid_mod_t *m0, fluid_mod_t *m1, fluid_mod_t *m2,

                      // m0: cc value,       m1 cc value,          m2 cc value
                      int m0_src2_cc_value,  int m1_src1_cc_value, int m2_src1_cc_value,

                      // m0 amount,     m1 amount,        m2 amout
                      double m0_amount, double m1_amount, double m2_amount)
{
    static const int m0_src2_cc = 20;
    static const int m1_src1_cc = 21;
    static const int m2_src1_cc = 22;

    fluid_real_t initial_voice_attenuation ; // cB

    // Initialize CC values in channel
    fluid_channel_set_cc(voice->channel, m0_src2_cc, m0_src2_cc_value);
    fluid_channel_set_cc(voice->channel, m1_src1_cc, m1_src1_cc_value);
    fluid_channel_set_cc(voice->channel, m2_src1_cc, m2_src1_cc_value);

    //initialize modulator m0: sources , amount , destination values.
    fluid_mod_set_source1(m0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
    fluid_mod_set_source2(m0, m0_src2_cc, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
    fluid_mod_set_amount (m0, m0_amount);
    fluid_mod_set_dest(m0, GEN_FILTERFC);

    //initialize modulator m1: sources , amount , destination values.
    fluid_mod_set_source1(m1, m1_src1_cc, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
    fluid_mod_set_source2(m1, FLUID_MOD_NONE, FLUID_MOD_GC);
    fluid_mod_set_amount (m1, m1_amount);
    fluid_mod_set_dest   (m1, FLUID_MOD_LINK_DEST | 0);
        
    //initialize modulator m2: sources , amount , destination values.
    fluid_mod_set_source1(m2, m2_src1_cc, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
    fluid_mod_set_source2(m2, FLUID_MOD_NONE, FLUID_MOD_GC);
    fluid_mod_set_amount (m2, m2_amount);
    fluid_mod_set_dest   (m2, FLUID_MOD_LINK_DEST | 0);

    /* valid internal list of linked modulators members for a complex modulator (mod0,mod1,mod2).
       Modulators member ordering is expected equivalent as the one produced by
       fluid_list_copy_linked_mod() implementing the following ordering rule:

       If any member mx has src1 linked it must be immediatley followed by a
       member whose destination field is mx. This rule ensures:
       1) That at synthesis time (noteon or CC modulation), any modulator mod_src
          (connected to another modulators mod_dst) are computed before this modulator mod_dst.
       2) The ordering is previsible in a way making test identity possible
          between two complex modulators (in fluid_linked_branch_test_identity()).
       Note that for the current test, only point (1) is relevant.
    */
    m0->next = m1;
    m1->next = m2;

    // Add one complex modulator using fluid_voice_add_mod_local().
    // fluid_voice_add_mod_local() is able to add a simple or complex modulator.
    // (API fluid_voice_add_mod() is only able to add a simple modulator.)
    voice->mod_count = 0;             // clear voice modulator table.
    fluid_voice_add_mod_local(voice, m0, FLUID_VOICE_DEFAULT, FLUID_NUM_MOD);

    /* Compute modulation by calling fluid_voice_modulate() */
    TEST_ASSERT(fluid_voice_modulate(voice, 1, m1_src1_cc) == FLUID_OK);

    return  voice->gen[GEN_FILTERFC].mod; // return  mod input
}

/* Main tests  --------------------------------------------------------------*/
int main(void)
{
    fluid_real_t  mod_modulation;
    fluid_real_t  mod_modulation_expected;

    fluid_settings_t* set = new_fluid_settings();
    fluid_synth_t* synth = new_fluid_synth(set);
    fluid_channel_t *ch = new_fluid_channel(synth, 0);
    fluid_sample_t *s = new_fluid_sample();

    // for modulation we need a voice belonging to the synthesizer
    fluid_voice_t *v = fluid_synth_alloc_voice(synth, s, 0, 0, 0);

    fluid_mod_t *mod0 = new_fluid_mod();
    fluid_mod_t *mod1 = new_fluid_mod();
    fluid_mod_t *mod2 = new_fluid_mod();

    TEST_ASSERT(v != NULL);

    fluid_gen_init(&v->gen[0], NULL);
    v->channel = ch;
    v->mod_count = 0;

    // Tests using one simple modulator:
    //   bip(CC20)-->mod0-->GEN_FILTERFC
    printf("Tests using one simple modulator:\n");
    {    
        //---------------------------------------------------------------------
        // Test 0 -- mod_modulation = b(cc20) * a0 
        // get modulation value by calling fluid_voice_modulate()
        mod_modulation = get_simple_mod_modulation(v, mod0, 0, 100.0);
        mod_modulation_expected = -100.0; // expected theoritical modulation
        printf(" Test 0.0: mod_modulation:%f, mod_modulation theoritical:%f\n",
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);

        // get expected modulation value obtained by running the modulator in the voice
        mod_modulation_expected = get_expected_mod_modulation(v);
        printf(" Test 0.1: mod_modulation:%f, mod_modulation_expected:%f\n\n", 
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);

        //---------------------------------------------------------------------
        // Test 1 -- mod_modulation = b(cc20) * a0 
        // get modulation value by calling fluid_voice_modulate()
        mod_modulation = get_simple_mod_modulation(v, mod0, 127, 100.0);
        mod_modulation_expected = 100.0; // expected theoritical modulation
        printf(" Test 1.0: mod_modulation:%f, mod_modulation theoritical:%f\n",
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);

        // get expected modulation value obtained by running the modulator in the voice
        mod_modulation_expected = get_expected_mod_modulation(v);
        printf(" Test 1.1: mod_modulation:%f, mod_modulation_expected:%f\n\n", 
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);
    }

    // Tests using one complex modulator
    //   uni(CC20)--------->mod0
    //   bip(CC21)-->mod1-->mod0-->GEN_FILTERFC
    //   bip(CC21)-->mod2-->mod0
	printf("Tests using one complex modulator:\n");
    {
        //---------------------------------------------------------------------
        // Test 0 mod_modulation = u(cc20) * a0 * ((b(cc21) * a1) + (b(cc22) * a2))
        // get modulation value by calling fluid_voice_modulate()
        mod_modulation = get_complex_mod_modulation(v, mod0, mod1, mod2,
                               // cc20, cc21, cc22, a0,   a1,   a2
                                  0,    0,    0,    10.0, 20.0, 30.0);
        // expected theoritical minimun value: 0 because cc20 value is 0
        mod_modulation_expected = 0.0;
        printf(" Test 0.0: mod_modulation:%f, mod_modulation theoritical:%f\n",
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);

        // get expected modulation value obtained by running the modulator in the voice
        mod_modulation_expected = get_expected_mod_modulation(v);
        printf(" Test 0.1: mod_modulation:%f, mod_modulation_expected:%f\n\n", 
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);

        //---------------------------------------------------------------------
        // Test 1 mod_modulation = u(cc20) * a0 * ((b(cc21) * a1) + (b(cc22) * a2))
        // get modulation value by calling fluid_voice_modulate()
        mod_modulation = get_complex_mod_modulation(v, mod0, mod1, mod2, 
                               // cc20, cc21, cc22, a0,   a1,   a2
                                  127,  0,    0,    10.0, 20.0, 30.0);
        // expected theoritical minimun value: -500 because cc20 value is 127
        mod_modulation_expected = -500;
        printf(" Test 1.0: mod_modulation:%f, mod_modulation theoritical:%f\n",
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);

        // get expected modulation value obtained by running the modulator in the voice
        mod_modulation_expected = get_expected_mod_modulation(v);
        printf(" Test 1.1: mod_modulation:%f, mod_modulation_expected:%f\n\n",
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);

        //---------------------------------------------------------------------
        // Test 2 mod_modulation = u(cc20) * a0 * ((b(cc21) * a1) + (b(cc22) * a2))
        // get modulation value by calling fluid_voice_modulate()
        mod_modulation = get_complex_mod_modulation(v, mod0, mod1, mod2, 
                               // cc20, cc21, cc22, a0,   a1,   a2
                                  127,  127,  0,    10.0, 20.0, 30.0);
        // expected theoritical minimun value: -100
        mod_modulation_expected = -100;
        printf(" Test 2.0: mod_modulation:%f, mod_modulation theoritical:%f\n",
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);

        // get expected modulation value obtained by running the modulator in the voice
        mod_modulation_expected = get_expected_mod_modulation(v);
        printf(" Test 2.1: mod_modulation:%f, mod_modulation_expected:%f\n\n",
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);

        //---------------------------------------------------------------------
        // Test 3 mod_modulation = u(cc20) * a0 * ((b(cc21) * a1) + (b(cc22) * a2))
        // get modulation value by calling fluid_voice_modulate()
        mod_modulation = get_complex_mod_modulation(v, mod0, mod1, mod2, 
                               // cc20, cc21, cc22, a0,   a1,   a2
                                  127,  0,    127,  10.0, 20.0, 30.0);
        // expected theoritical minimun value: -100
        mod_modulation_expected = 100;
        printf(" Test 3.0: mod_modulation:%f, mod_modulation theoritical:%f\n", 
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);

        // get expected modulation value obtained by running the modulator in the voice
        mod_modulation_expected = get_expected_mod_modulation(v);
        printf(" Test 3.1: mod_modulation:%f, mod_modulation_expected:%f\n\n",
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);

        //---------------------------------------------------------------------
        // Test 4 mod_modulation = u(cc20) * a0 * ((b(cc21) * a1) + (b(cc22) * a2))
        // get modulation value by calling fluid_voice_modulate()
        mod_modulation = get_complex_mod_modulation(v, mod0, mod1, mod2, 
                               // cc20, cc21, cc22, a0,   a1,   a2
                                  127,  127,  127,  10.0, 20.0, 30.0);
        // expected theoritical minimun value: -100
        mod_modulation_expected = 500;
        printf(" Test 4.0: mod_modulation:%f, mod_modulation theoritical:%f\n",
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);

        // get expected modulation value obtained by running the modulator in the voice
        mod_modulation_expected = get_expected_mod_modulation(v);
        printf(" Test 4.1: mod_modulation:%f, mod_modulation_expected:%f\n\n",
                 mod_modulation, mod_modulation_expected);
        TEST_ASSERT(mod_modulation == mod_modulation_expected);
	}

    delete_fluid_mod(mod0);
    delete_fluid_mod(mod1);
    delete_fluid_mod(mod2);

    delete_fluid_sample(s);
    delete_fluid_channel(ch);
    delete_fluid_synth(synth);
    delete_fluid_settings(set);
    
    return EXIT_SUCCESS;
}
