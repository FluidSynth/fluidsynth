/*----------------------------------------------------------------------------
Test of possible attenuation reduction evaluated  by 
fluid_voice_get_lower_boundary_for_attenuation().

These tests use lower_bound returned by fluid_voice_get_lower_boundary_for_attenuation(voice)
with one modulator (simple or complex) at a time in the voice.
This leads to compute the minimum value contribution of this modulator (min_val_mod).
Then min_val_mod is compared to the expected minimun value of this modulator(min_val_expected).

The comparison is done 2 times:
1) min_val_mod is compared to theorical expected minimum value.
2) min_val_mod is comparaed to real expected minimum value computed by running the
   modulator in the voice and driving source CC value from (min (0) to max(127)
   before calling fluid_voice_calculate_modulator_contributions().

Tests must be done for each type of modulator (simple or complex).
For each type all test combinations should be done for:
 - sources (unipolar/bipolar).
 - and amount(positive/negative).
----------------------------------------------------------------------------*/
#include "test.h"
#include "fluidsynth.h"
#include "utils/fluid_sys.h"
#include "synth/fluid_voice.h"
#include "synth/fluid_chan.h"

//-----------------------------------------------------------------------------
// Externals
void fluid_voice_calculate_modulator_contributions(fluid_voice_t *voice);

fluid_real_t
fluid_voice_get_lower_boundary_for_attenuation(fluid_voice_t *voice);

/**
 * Compute attenuation reduction given by voice modulator(if possible)
 * by calling fluid_voice_get_lower_boundary_for_attenuation().
 * It is possible that the function cannot compute this if
 * initial_voice_attenuation isn't sufficient.
 * Then return the minimum value this modulator will supply.
 *
 * @param voice the voice that must contain the modulator at index 0.
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
    TEST_ASSERT(voice->mod_count > 0);

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

/**
 * - Initialize a simple modulator mod (source src1,src2,amount) and put it in voice.
 * - Compute attenuation reduction given by this simple modulator and
 *   return the minimum value this modulator will supply.
 *
 * @param voice, the voice to initialize.
 * @param mod, the simple modulator to initialize.
 * @param src1_cc, CC to initialize for sources src1.
 * @param src1_polarity, src1 polarity (FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR).
 * @param src1_cc_value, src1_cc value.
 * @param src2_cc, CC to initialize for source src2.
 * @param src2_polarity, src2 polarity (FLUID_MOD_UNIPOLAR, FLUID_MOD_BIPOLAR).
 * @param src2_value, src2_cc value.
 * @param amount, amount value.
 */
static fluid_real_t get_simple_mod_min_val(fluid_voice_t *voice, fluid_mod_t *mod,
                                    int src1_cc,  int src1_cc_value, int src1_polarity,
                                    int src2_cc, int src2_cc_value, int src2_polarity,
                                    double amount)
{
    fluid_real_t initial_voice_attenuation ; // cB

    // Initialize CC values in channel
    fluid_channel_set_cc(voice->channel, src1_cc, src1_cc_value);
    fluid_channel_set_cc(voice->channel, src2_cc, src2_cc_value);

    //initialise modulators sources and amount values.
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
     * For a simple modulator, initial_voice_attenuation must be > 2 * |amount|
     */
    initial_voice_attenuation = 2 * fabs(amount) + 1;
    return compute_possible_att_reduction(voice, initial_voice_attenuation);
}

/**
 * Update expected minimum value that a voice's simple modulator will supply for
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

    // Set src1_cc, src2cc value to 0, 0 and update expected_mod_min_val
    fluid_channel_set_cc(voice->channel, src1_cc, src1_cc_value);
    fluid_channel_set_cc(voice->channel, src2_cc, src2_cc_value);
    voice->gen[GEN_ATTENUATION].mod = 0; // reset mod input
    fluid_voice_calculate_modulator_contributions(voice);

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
 * Compute real expected minimum value a voice simple modulator will supply by
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
    TEST_ASSERT(voice->mod_count > 0);
    // Check that the modulator is a simple modulator
    TEST_ASSERT(fluid_get_num_mod(&voice->mod[0]) == 1);

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

// test modulators
int main(void)
{
    fluid_real_t  min_val_mod;
    fluid_real_t  min_val_expected;

    fluid_settings_t* set = new_fluid_settings();
    fluid_synth_t* synth = new_fluid_synth(set);
    fluid_channel_t *ch = new_fluid_channel(synth, 0);
    fluid_voice_t *v = new_fluid_voice(NULL, 22050);
    
    fluid_mod_t *mod0 = new_fluid_mod();

    v->channel = ch;

    // tests using one simple modulator:
	// CC20-->mod0-->GEN_ATTENUATION
	// CC21-->
    {    
        static const int src1_cc = 20;
        static const int src2_cc = 21;
        //---------------------------------------------------------------------
        //  src1      |  src2      |  amount sign | expected theorical minimum value
        //---------------------------------------------------------------------
        //  unipolar  |  unipolar  |  amount > 0  |  0.0
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation() (CC value:max)
        min_val_mod = get_simple_mod_min_val(v, mod0, src1_cc, 127, FLUID_MOD_UNIPOLAR, src2_cc, 127, FLUID_MOD_UNIPOLAR, 100.0);
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
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation() (CC value:max)
        min_val_mod = get_simple_mod_min_val(v, mod0, src1_cc, 127, FLUID_MOD_UNIPOLAR, src2_cc, 127, FLUID_MOD_BIPOLAR, 100.0);
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
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation() (CC value:max)
        min_val_mod = get_simple_mod_min_val(v, mod0, src1_cc, 127, FLUID_MOD_BIPOLAR, src2_cc, 127, FLUID_MOD_UNIPOLAR, 100.0);
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
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation() (CC value:max)
        min_val_mod = get_simple_mod_min_val(v, mod0, src1_cc, 127, FLUID_MOD_BIPOLAR, src2_cc, 127, FLUID_MOD_BIPOLAR, 100.0);
        min_val_expected = - fabs(fluid_mod_get_amount(mod0)); // expected theoritical minimun value
        TEST_ASSERT(min_val_mod == min_val_expected);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_simple_mod_min_val(v);
        TEST_ASSERT(min_val_mod == min_val_expected);

        //== same with amount < 0  ====================================================================
        //---------------------------------------------------------------------
        //  src1      |  src2      |  amount sign | expected theorical minimum value
        //---------------------------------------------------------------------
        //  unipolar  |  unipolar  |  amount < 0  |  -|amount|
        //---------------------------------------------------------------------
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation() (CC value:max)
        min_val_mod = get_simple_mod_min_val(v, mod0, src1_cc, 127, FLUID_MOD_UNIPOLAR, src2_cc, 127, FLUID_MOD_UNIPOLAR, -100.0);
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
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation() (CC value:max)
        min_val_mod = get_simple_mod_min_val(v, mod0, src1_cc, 127, FLUID_MOD_UNIPOLAR, src2_cc, 127, FLUID_MOD_BIPOLAR, -100.0);
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
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation() (CC value:max)
        min_val_mod = get_simple_mod_min_val(v, mod0, src1_cc, 127, FLUID_MOD_BIPOLAR, src2_cc, 127, FLUID_MOD_UNIPOLAR, -100.0);
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
        // get min_val value by calling fluid_voice_get_lower_boundary_for_attenuation() (CC value:max)
        min_val_mod = get_simple_mod_min_val(v, mod0, src1_cc, 127, FLUID_MOD_BIPOLAR, src2_cc, 127, FLUID_MOD_BIPOLAR, -100.0);
        min_val_expected = - fabs(fluid_mod_get_amount(mod0)); // expected theoritical minimun value
        TEST_ASSERT(min_val_mod == min_val_expected);

        // get expected real minimun value obtained by running the modulator in the voice
        min_val_expected = 	get_expected_simple_mod_min_val(v);
        TEST_ASSERT(min_val_mod == min_val_expected);
    }

    // tests using one complex modulator
    {
        // TODO
    }

    delete_fluid_mod(mod0);
    
    delete_fluid_voice(v);
    delete_fluid_channel(ch);
    delete_fluid_synth(synth);
    delete_fluid_settings(set);
    
    return EXIT_SUCCESS;
}
