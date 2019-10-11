/*----------------------------------------------------------------------------
 Test of possible attenuation reduction evaluated  by 
 fluid_voice_get_lower_boundary_for_attenuation().

 These tests use lower_bound returned by fluid_voice_get_lower_boundary_for_attenuation(voice)
 with one modulator (simple or complex) at a time in the voice.
 This leads to compute the minimum value contribution of this modulator (min_val_mod).
 Then min_val_mod is compared to the expected minimun value of this modulator(min_val_expected).

 Tests must be done for each type of modulator (simple or complex).
 For each type all test combinations should be done for:
 - sources (unipolar/bipolar).
 - and amount(positive/negative).

 1)For simple modulator the comparison is done 2 times:
 1.1) min_val_mod is compared to theorical expected minimum value.
 1.2) min_val_mod is compared to real expected minimum value computed by running the
    modulator in the voice and driving source CC value from (min (0) to max(127)
    before calling fluid_voice_calculate_modulator_contributions().(see note)

 2)For complex modulator, because there is no theorical expected minimum value
   the comparison is done only using same step that 1.2.


 Note about tests dependencies and precedence:
  These steps (1.2, 2.1) are dependent of fluid_voice_calculate_modulator_contributions()
  function. That means that any change in this function must be checked by
  running test_modulator_amount before running test_possible_att_reduction.
----------------------------------------------------------------------------*/
#include "test.h"
#include "fluidsynth.h"
#include "utils/fluid_sys.h"
#include "synth/fluid_voice.h"
#include "synth/fluid_chan.h"

//-----------------------------------------------------------------------------
/**
 * Compute attenuation reduction given by voice modulator(if possible)
 * by calling fluid_voice_get_lower_boundary_for_attenuation().
 * (It is possible that the function cannot compute this if
 * initial_voice_attenuation isn't sufficient).
 * Then return the minimum value this modulator will supply.
 *
 * @param voice the voice that must contain one modulator.
 * @param initial_voice_attenuation, must be greater than any possible
 * attenuation reduction.
 * @return the minimum value this modulator will supply.
 */
static fluid_real_t compute_possible_att_reduction(fluid_voice_t *voice, fluid_real_t initial_voice_attenuation)
{
    fluid_real_t  lower_bound;
    fluid_real_t  min_val_mod;
    fluid_real_t  current_val_mod;

    // Check that the voice contains one modulator
    TEST_ASSERT(fluid_voice_get_count_modulators(voice) == 1);

    // initialize  voice attenuation to a value greater than any possible attenuation
    // reduction.
    // This must ensure that il will be always possible to compute this reduction
    voice->attenuation = initial_voice_attenuation;  // In cB
    lower_bound = fluid_voice_get_lower_boundary_for_attenuation(voice);
    // Check to verify if it is possible to compute attenuation reduction.
    if (lower_bound <= 0.0f)
    {
        FLUID_LOG(FLUID_ERR, "-- cannot compute attenuation reduction.");
        FLUID_LOG(FLUID_ERR, "-- lower_bound must be > 0.0. Try to augment initial_voice_attenuation\n");
        TEST_ASSERT(lower_bound > 0.0);
    }
    // compute minimum value that mod will supply and return this.
    current_val_mod = fluid_mod_get_value(&voice->mod[0], voice);
    min_val_mod = lower_bound - voice->attenuation + current_val_mod;
#if 0
    printf("lower_bound:%f, voice_attenuation:%f, current_mod_val:%f, min_val_mod:%f\n",
            lower_bound, voice->attenuation,  current_val_mod, min_val_mod);
#endif
	return min_val_mod;
}

/*-- functions for simple modulator -----------------------------------------*/

/**
 * - Initialize a simple modulator mod (source src1,src2,amount) and put it in voice.
 * - Compute attenuation reduction given by this simple modulator and
 *   return the minimum value this modulator will supply.
 *
 * @param voice, the voice to initialize.
 * @param mod, the simple modulator to initialize.
 * @param src1_polarity, src1 polarity (FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR).
 * @param src2_polarity, src2 polarity (FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR).
 * @param amount, amount value.
 */
static fluid_real_t get_simple_mod_min_val(fluid_voice_t *voice, fluid_mod_t *mod,
                                    int src1_polarity,
                                    int src2_polarity,
                                    double amount)
{
    static const int src1_cc = 20;
    static const int src2_cc = 21;
    fluid_real_t initial_voice_attenuation ; // cB

    // Initialize CC values in channel
    fluid_channel_set_cc(voice->channel, src1_cc, 127);
    fluid_channel_set_cc(voice->channel, src2_cc, 127);

    //initialize modulators sources and amount values.
    fluid_mod_set_source1(mod, src1_cc, FLUID_MOD_CC | FLUID_MOD_LINEAR | src1_polarity | FLUID_MOD_POSITIVE);
    fluid_mod_set_source2(mod, src2_cc, FLUID_MOD_CC | FLUID_MOD_LINEAR | src2_polarity | FLUID_MOD_POSITIVE);
    fluid_mod_set_amount (mod, amount);
    fluid_mod_set_dest(mod, GEN_ATTENUATION);

    // Add one simple modulator using fluid_voice_add_mod().
    voice->mod_count = 0;             // clear voice modulator table.
    fluid_voice_add_mod(voice, mod, FLUID_VOICE_DEFAULT);

    /* Compute attenuation reduction by calling fluid_voice_get_lower_boundary_for_attenuation
     * It is possible that the function cannot compute this if initial_voice_attenuation 
     * isn't sufficient.
     * For a simple modulator, initial_voice_attenuation must be greater than: 2 * |amount|
     */
    initial_voice_attenuation = 2 * fabs(amount) + 1;
    return compute_possible_att_reduction(voice, initial_voice_attenuation);
}

/**
 * Update expected minimum value that a simple modulator will supply for
 * the given CC's values by calling
 * fluid_voice_calculate_modulator_contributions().
 *
 * @param voice the voice that must contains only one simple modulator.
 * @param src1_cc_value, value to set for CC on src1 input.
 * @param src2_cc_value, value to set for CC on src2 input.
 * @param expected_mod_min_val, expected min_val to update.
 * @return the expected minimum value updated this modulator will supply.
 */
static fluid_real_t update_expected_simple_mod_min_val(fluid_voice_t *voice,
                                                int src1_cc_value,
                                                int src2_cc_value,
                                                fluid_real_t expected_mod_min_val)
{
    // CC used by voice modulator source src1
    int src1_cc  = fluid_mod_get_source1(&voice->mod[0]);
    // CC used by voice modulator source src2
    int src2_cc  = fluid_mod_get_source2(&voice->mod[0]);

    // Set src1_cc, src2cc values
    fluid_channel_set_cc(voice->channel, src1_cc, src1_cc_value);
    fluid_channel_set_cc(voice->channel, src2_cc, src2_cc_value);

    // calculate generator mod value
    voice->gen[GEN_ATTENUATION].mod = 0; // reset mod input
    fluid_voice_calculate_modulator_contributions(voice);

    // update expected_mod_min_val
    if( voice->gen[GEN_ATTENUATION].mod < expected_mod_min_val)
    {
        expected_mod_min_val = voice->gen[GEN_ATTENUATION].mod;
    }
#if 0
    {
        int flags1 = fluid_mod_get_flags1(&voice->mod[0]);
        int flags2 = fluid_mod_get_flags2(&voice->mod[0]);
        printf("src1 polarity:%d, src2 polarity:%d\n", flags1 & FLUID_MOD_BIPOLAR, flags2 & FLUID_MOD_BIPOLAR);
        printf("src1_cc_value:%d, src2_cc_value:%d, gen[GEN_ATTENUATION].mod:%f, expected_mod_min_val:%f\n",
                src1_cc_value, src2_cc_value, voice->gen[GEN_ATTENUATION].mod, expected_mod_min_val);
    }
#endif
    return expected_mod_min_val;
}

/**
 * Compute real expected minimum value a simple modulator will supply by
 * changing CC value from min (0) to max (127) and then calling
 * fluid_voice_calculate_modulator_contributions().
 *
 * @param voice the voice that must contains only one simple modulator.
 * @return the expected minimum value this modulator will supply.
 */
static fluid_real_t get_expected_simple_mod_min_val(fluid_voice_t *voice)
{
    fluid_real_t expected_mod_min_val = 0.0;

    // Check that the voice contains one modulator
    TEST_ASSERT(fluid_voice_get_count_modulators(voice) == 1);
    // Check that the modulator is a simple modulator
    TEST_ASSERT(fluid_mod_get_linked_count(&voice->mod[0]) == 1);

    // Set src1_cc, src2_cc values to 0, 0 and update expected_mod_min_val
    expected_mod_min_val = update_expected_simple_mod_min_val(voice, 0, 0, expected_mod_min_val);

    // Set src1_cc, src2_cc values to 0, 127 and update expected_mod_min_val
    expected_mod_min_val = update_expected_simple_mod_min_val(voice, 0, 127, expected_mod_min_val);

    // Set src1_cc, src2_cc values to 127, 0 and update expected_mod_min_val
    expected_mod_min_val = update_expected_simple_mod_min_val(voice, 127, 0, expected_mod_min_val);

    // Set src1_cc, src2_cc values to 127, 127 and update expected_mod_min_val
    expected_mod_min_val = update_expected_simple_mod_min_val(voice, 127, 127, expected_mod_min_val);

    return expected_mod_min_val;
}

//-- functions for complex modulator ------------------------------------------
/**
 * - Initialize a complex modulator: (m2 + m1)-->m0-->GEN_ATTENUATION with
 *   m0: source1 linked, source2 CC,   destination GEN_ATTENUATION
 *   m1: source1 CC,     source2 none, destination m0
 *   m2: source1 CC,     source2 none, destination m0
 * - Put this modulator in voice.
 * - Compute attenuation reduction given by this complex modulator and
 *   return the minimum value this modulator will supply.
 *
 * @param voice, the voice to initialize.
 * @param m0, ending modulator member.
 * @param m1, member modulator linked to m0.
 * @param m2, member modulator linked to m0.
 *
 * @param m0_src2_polarity, source2 polarity of m0 (FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR).
 * @param m1_src1_polarity, source1 polarity of m1 (FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR).
 * @param m2_src1_polarity, source1 polarity of m2 (FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR).
 *
 * @param m0_amount, amount value of m0.
 * @param m1_amount, amount value of m1.
 * @param m2_amount, amount value of m2.
 */
static fluid_real_t get_complex_mod_min_val(fluid_voice_t *voice,
                      // modulators: m0, m1, m2
                      fluid_mod_t *m0, fluid_mod_t *m1, fluid_mod_t *m2,

                      // m0: source2, m1 source1, m2 source1
					  int m0_src2_polarity,  int m1_src1_polarity, int m2_src1_polarity,

                      // m0 amount, m1 amount, m2 amout
					  double m0_amount, double m1_amount, double m2_amount)
{
    static const int m0_src2_cc = 20;
    static const int m1_src1_cc = 21;
    static const int m2_src1_cc = 22;

	fluid_real_t initial_voice_attenuation ; // cB

    // Initialize CC values in channel
    fluid_channel_set_cc(voice->channel, m0_src2_cc, 127);
    fluid_channel_set_cc(voice->channel, m1_src1_cc, 127);
    fluid_channel_set_cc(voice->channel, m2_src1_cc, 127);

    //initialize modulator m0: sources , amount , destination values.
    fluid_mod_set_source1(m0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
    fluid_mod_set_source2(m0, m0_src2_cc, FLUID_MOD_CC | FLUID_MOD_LINEAR | m0_src2_polarity | FLUID_MOD_POSITIVE);
    fluid_mod_set_amount (m0, m0_amount);
    fluid_mod_set_dest(m0, GEN_ATTENUATION);

    //initialize modulator m1: sources , amount , destination values.
    fluid_mod_set_source1(m1, m1_src1_cc, FLUID_MOD_CC | FLUID_MOD_CONCAVE | m1_src1_polarity | FLUID_MOD_POSITIVE);
    fluid_mod_set_source2(m1, FLUID_MOD_NONE, FLUID_MOD_GC);
    fluid_mod_set_amount (m1, m1_amount);
    fluid_mod_set_dest   (m1, FLUID_MOD_LINK_DEST | 0);
        
    //initialize modulator m2: sources , amount , destination values.
    fluid_mod_set_source1(m2, m2_src1_cc, FLUID_MOD_CC | FLUID_MOD_LINEAR | m2_src1_polarity | FLUID_MOD_POSITIVE);
    fluid_mod_set_source2(m2, FLUID_MOD_NONE, FLUID_MOD_GC);
    fluid_mod_set_amount (m2, m2_amount);
    fluid_mod_set_dest   (m2, FLUID_MOD_LINK_DEST | 0);

    /* valid internal list of linked modulators members for a complex modulator (mod0,mod1,mod2).
       Modulators member ordering is expected equivalent as the one produced by
       fluid_mod_copy_linked_mod() implementing the following ordering rule:

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

    /* Compute attenuation reduction by calling fluid_voice_get_lower_boundary_for_attenuation
       It is possible that the function cannot compute this if initial_voice_attenuation 
       isn't sufficient.
       For this complex modulator, initial_voice_attenuation must be greater than:
	    (2 * |m1_amount|) + (2 * |m2_amount|) * (2 * |m0_amount|)
     */
    initial_voice_attenuation = (((2 * fabs(m1_amount)) + (2 * fabs(m2_amount)))
	                             * 2 * fabs(m0_amount)) + 1;
    return compute_possible_att_reduction(voice, initial_voice_attenuation);
}

/**
 * Update expected minimum value that a complex modulator will supply for
 * the given CC's values by calling
 * fluid_voice_calculate_modulator_contributions().
 *
 * @param voice the voice that must contains only one complex modulator.
 *
 * @param m0_src2_cc_value, value of m0_src2_cc.
 * @param m1_src1_cc_value, value of m1_src1_cc.
 * @param m2_src1_cc_value, value of m2_src1_cc.
 *
 * @param expected_mod_min_val, expected min_val to update.
 * @return the expected minimum value updated this modulator will supply.
 */
static fluid_real_t update_expected_complex_mod_min_val(fluid_voice_t *voice,
                                                int m0_src2_cc_value,
                                                int m1_src1_cc_value,
                                                int m2_src1_cc_value,
                                                fluid_real_t expected_mod_min_val)
{
    int i;
    // CC used by voice modulator m0 source src2
    int m0_src2_cc  = fluid_mod_get_source2(&voice->mod[0]);
    // CC used by voice modulator m1 source src1
    int m1_src1_cc  = fluid_mod_get_source1(&voice->mod[1]);
    // CC used by voice modulator m2 source src1
    int m2_src1_cc  = fluid_mod_get_source1(&voice->mod[2]);

    // Set m0_src2_cc, m1_src1_cc, m2_src1_cc  values
    fluid_channel_set_cc(voice->channel, m0_src2_cc, m0_src2_cc_value);
    fluid_channel_set_cc(voice->channel, m1_src1_cc, m1_src1_cc_value);
    fluid_channel_set_cc(voice->channel, m2_src1_cc, m2_src1_cc_value);
   
    // calculate generator mod value
    for(i = 0; i < voice->mod_count; i++)
    {
        voice->mod[i].link = 0.0; // reset link input
    }
	voice->gen[GEN_ATTENUATION].mod = 0; // reset mod input
    fluid_voice_calculate_modulator_contributions(voice);

    // update expected_mod_min_val
    if( voice->gen[GEN_ATTENUATION].mod < expected_mod_min_val)
    {
        expected_mod_min_val = voice->gen[GEN_ATTENUATION].mod;
    }
    return expected_mod_min_val;
}

/**
 * Compute real expected minimum value a complex modulator will supply by
 * changing CC value from min (0) to max (127) and then calling
 * fluid_voice_calculate_modulator_contributions().
 *
 * @param voice the voice that must contains only one complex modulator.
 * @return the expected minimum value this modulator will supply.
 */
static fluid_real_t get_expected_complex_mod_min_val(fluid_voice_t *voice)
{
    fluid_real_t expected_mod_min_val = 0.0;

    // Check that the voice contains one modulator
    TEST_ASSERT(fluid_voice_get_count_modulators(voice) == 1);
    // Check that the modulator is a complex modulator
    TEST_ASSERT(fluid_mod_get_linked_count(&voice->mod[0]) > 1);

    // Set m0_src2_cc, m1_src1_cc, m2_src1_cc  values to 0, 0, 0 and update expected_mod_min_val
    expected_mod_min_val = update_expected_complex_mod_min_val(voice, 0, 0, 0, expected_mod_min_val);

    // Set m0_src2_cc, m1_src1_cc, m2_src1_cc  values to 0, 0, 127 and update expected_mod_min_val
    expected_mod_min_val = update_expected_complex_mod_min_val(voice, 0, 0, 127, expected_mod_min_val);

    // Set m0_src2_cc, m1_src1_cc, m2_src1_cc  values to 0, 127, 0 and update expected_mod_min_val
    expected_mod_min_val = update_expected_complex_mod_min_val(voice, 0, 127, 0, expected_mod_min_val);

    // Set m0_src2_cc, m1_src1_cc, m2_src1_cc  values to 0, 127, 127 and update expected_mod_min_val
    expected_mod_min_val = update_expected_complex_mod_min_val(voice, 0, 127, 127, expected_mod_min_val);

    // Set m0_src2_cc, m1_src1_cc, m2_src1_cc  values to 127, 0, 0 and update expected_mod_min_val
    expected_mod_min_val = update_expected_complex_mod_min_val(voice, 127, 0, 0, expected_mod_min_val);

    // Set m0_src2_cc, m1_src1_cc, m2_src1_cc  values to 127, 0, 127 and update expected_mod_min_val
    expected_mod_min_val = update_expected_complex_mod_min_val(voice, 127, 0, 127, expected_mod_min_val);

    // Set m0_src2_cc, m1_src1_cc, m2_src1_cc  values to 127, 127, 0 and update expected_mod_min_val
    expected_mod_min_val = update_expected_complex_mod_min_val(voice, 127, 127, 0, expected_mod_min_val);

    // Set m0_src2_cc, m1_src1_cc, m2_src1_cc  values to 127, 127, 127 and update expected_mod_min_val
    expected_mod_min_val = update_expected_complex_mod_min_val(voice, 127, 127, 127, expected_mod_min_val);

    return expected_mod_min_val;
}


/* Main tests  --------------------------------------------------------------*/
int main(void)
{
    fluid_real_t  min_val_mod;
    fluid_real_t  min_val_expected;

    fluid_settings_t* set = new_fluid_settings();
    fluid_synth_t* synth = new_fluid_synth(set);
    fluid_channel_t *ch = new_fluid_channel(synth, 0);
    fluid_voice_t *v = new_fluid_voice(NULL, 22050);
    
    fluid_mod_t *mod0 = new_fluid_mod();
    fluid_mod_t *mod1 = new_fluid_mod();
    fluid_mod_t *mod2 = new_fluid_mod();

    v->channel = ch;

    // Tests using one simple modulator:
    //   CC20-->mod0-->GEN_ATTENUATION
    //   CC21-->
	printf("Tests using one simple modulator:\n");
    {    
        //---------------------------------------------------------------------
        //  src1      |  src2      |  amount sign | expected theorical minimum value
        //---------------------------------------------------------------------
        //  unipolar  |  unipolar  |  amount > 0  |  0.0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_simple_mod_min_val(v, mod0, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, 100.0);
        min_val_expected = 0.0; // expected theoritical minimun value
        TEST_ASSERT(min_val_mod == min_val_expected);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_simple_mod_min_val(v);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //---------------------------------------------------------------------
        //  src1      |  src2     |  amount sign | expected theorical minimum value
        //---------------------------------------------------------------------
        //  unipolar  |  bipolar  |  amount > 0  |  -|amount|
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_simple_mod_min_val(v, mod0, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, 100.0);
        min_val_expected = - fabs(fluid_mod_get_amount(mod0)); // expected theoritical minimun value
        TEST_ASSERT(min_val_mod == min_val_expected);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_simple_mod_min_val(v);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //---------------------------------------------------------------------
        //  src1      |  src2     |  amount sign | expected theorical minimum value
        //---------------------------------------------------------------------
        //  bipolar  |  unipolar  |  amount > 0  |  -|amount|
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_simple_mod_min_val(v, mod0, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, 100.0);
        min_val_expected = - fabs(fluid_mod_get_amount(mod0)); // expected theoritical minimun value
        TEST_ASSERT(min_val_mod == min_val_expected);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_simple_mod_min_val(v);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //---------------------------------------------------------------------
        //  src1      |  src2     |  amount sign | expected theorical minimum value
        //---------------------------------------------------------------------
        //  bipolar  |  bipolar   |  amount > 0  |  -|amount|
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_simple_mod_min_val(v, mod0, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, 100.0);
        min_val_expected = - fabs(fluid_mod_get_amount(mod0)); // expected theoritical minimun value
        TEST_ASSERT(min_val_mod == min_val_expected);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_simple_mod_min_val(v);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //-- same with amount < 0  --------------------------------------------
        //---------------------------------------------------------------------
        //  src1      |  src2      |  amount sign | expected theorical minimum value
        //---------------------------------------------------------------------
        //  unipolar  |  unipolar  |  amount < 0  |  -|amount|
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_simple_mod_min_val(v, mod0, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, -100.0);
        min_val_expected = - fabs(fluid_mod_get_amount(mod0)); // expected theoritical minimun value
        TEST_ASSERT(min_val_mod == min_val_expected);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_simple_mod_min_val(v);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //---------------------------------------------------------------------
        //  src1      |  src2     |  amount sign | expected theorical minimum value
        //---------------------------------------------------------------------
        //  unipolar  |  bipolar  |  amount < 0  |  -|amount|
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_simple_mod_min_val(v, mod0, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, -100.0);
        min_val_expected = - fabs(fluid_mod_get_amount(mod0)); // expected theoritical minimun value
        TEST_ASSERT(min_val_mod == min_val_expected);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_simple_mod_min_val(v);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //---------------------------------------------------------------------
        //  src1      |  src2     |  amount sign | expected theorical minimum value
        //---------------------------------------------------------------------
        //  bipolar   |  unipolar |  amount < 0  |  -|amount|
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_simple_mod_min_val(v, mod0, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, -100.0);
        min_val_expected = - fabs(fluid_mod_get_amount(mod0)); // expected theoritical minimun value
        TEST_ASSERT(min_val_mod == min_val_expected);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_simple_mod_min_val(v);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //---------------------------------------------------------------------
        //  src1      |  src2     |  amount sign | expected theorical minimum value
        //---------------------------------------------------------------------
        //  bipolar   |  bipolar  |  amount < 0  |  -|amount|
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_simple_mod_min_val(v, mod0, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, -100.0);
        min_val_expected = - fabs(fluid_mod_get_amount(mod0)); // expected theoritical minimun value
        TEST_ASSERT(min_val_mod == min_val_expected);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_simple_mod_min_val(v);
        TEST_ASSERT(min_val_mod == min_val_expected);
    }

    // Tests using one complex modulator
	//   CC20--------->mod0
	//   CC21-->mod1-->mod0-->GEN_ATTENUATION
	//   CC21-->mod2-->mod0
	printf("Tests using one complex modulator:\n");
    {
        //- Test 00/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | unipolar  |  > 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 00: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 01/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | unipolar  |  > 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 01: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 02/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | unipolar  |  > 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 02: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 03/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | unipolar  |  > 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 03: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 04/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | unipolar  |  < 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 04: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 05/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | unipolar  |  < 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 05: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 06/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | unipolar  |  < 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 06: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 07/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | unipolar  |  < 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 07: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 08/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | bipolar   |  > 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, 10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 08: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 09/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | bipolar   |  > 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, 10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 09: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 10/15 ---------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | bipolar   |  > 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, 10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 10: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 11/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | bipolar   |  > 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, 10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 11: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 12/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | bipolar   |  < 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, -10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 12: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 13/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | bipolar   |  < 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, -10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 13: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 14/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | bipolar   |  < 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, -10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 14: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 15/15 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | unipolar  | bipolar   |  < 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, -10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 15: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 16/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | unipolar  |  > 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 16: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 17/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | unipolar  |  > 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 17: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 18/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | unipolar  |  > 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 18: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 19/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | unipolar  |  > 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 19: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 20/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | unipolar  |  < 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 20: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 21/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | unipolar  |  < 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 21: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 22/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | unipolar  |  < 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 22: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 23/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | unipolar  |  < 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 23: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 24/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | bipolar   |  > 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, 10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 24: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 25/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | bipolar   |  > 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, 10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 25: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 26/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | bipolar   |  > 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, 10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 26: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 27/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | bipolar   |  > 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, 10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 27: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 28/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | bipolar   |  < 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, -10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 28: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 29/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | bipolar   |  < 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, -10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 29: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 30/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | bipolar   |  < 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, -10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 30: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 31/31 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | bipolar   |  < 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, -10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 31: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 32/63 --------------------------------------------------------

        //- Test 32/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | unipolar  |  > 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 32: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 33/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | unipolar  |  > 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 33: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 34/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | unipolar  |  > 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 34: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 35/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | unipolar  |  > 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 35: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 36/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | unipolar  |  < 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 36: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 37/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | unipolar  |  < 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 37: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 38/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | unipolar  |  < 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 38: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 39/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | unipolar  |  < 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 39: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 40/63 -------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | bipolar   |  > 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, 10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 40: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 41/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | bipolar   |  > 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, 10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 41: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 42/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | bipolar   |  > 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, 10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 42: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 43/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | bipolar   |  > 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, 10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 43: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 44/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | bipolar   |  < 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, -10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 44: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 45/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | bipolar   |  < 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, -10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 45: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 46/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | bipolar   |  < 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, -10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 46: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 47/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | unipolar  | bipolar   |  < 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR, -10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 47: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 48/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | unipolar  |  > 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 48: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 49/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | unipolar  |  > 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 49: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 50/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  unipolar | bipolar   | unipolar  |  > 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 50: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 51/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | unipolar  |  > 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, 10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 51: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 52/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | unipolar  |  < 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 52: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 53/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | unipolar  |  < 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 53: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 54/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | unipolar  |  < 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 54: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 55/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | unipolar  |  < 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_UNIPOLAR, -10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 55: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 56/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | bipolar   |  > 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, 10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 56: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 57/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | bipolar   |  > 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, 10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 57: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 58/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | bipolar   |  > 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, 10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 58: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 59/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | bipolar   |  > 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, 10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 59: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 60/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | bipolar   |  < 0      |  > 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, -10.0, 20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 60: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 61/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | bipolar   |  < 0      |  > 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, -10.0, 20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 61: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 62/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | bipolar   |  < 0      |  < 0      |  > 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, -10.0, -20.0, 30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 62: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //- Test 63/63 --------------------------------------------------------
        //  m0 src2  |  m1 src1  |  m2 src1  | m0 amount | m1 amount |m2 amount
        //---------------------------------------------------------------------
        //  bipolar  | bipolar   | bipolar   |  < 0      |  < 0      |  < 0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation()
        min_val_mod = get_complex_mod_min_val(v, mod0, mod1, mod2,
                               FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, FLUID_MOD_BIPOLAR, -10.0, -20.0, -30.0);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_complex_mod_min_val(v);
		printf(" Test 63: min_val_mod:%f, min_val_expected:%f\n", min_val_mod, min_val_expected);
        TEST_ASSERT(min_val_mod == min_val_expected);
	}

    delete_fluid_mod(mod0);
    delete_fluid_mod(mod1);
    delete_fluid_mod(mod2);
    
    delete_fluid_voice(v);
    delete_fluid_channel(ch);
    delete_fluid_synth(synth);
    delete_fluid_settings(set);
    
    return EXIT_SUCCESS;
}
