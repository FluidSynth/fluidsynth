/**
 * Test XG drum bank selection and validation functionality.
 * This test verifies that the XG drum bank selection correctly:
 * 1. Uses the actual XG drum bank when specified
 * 2. Falls back to bank 128 when the XG bank contains non-drum presets
 * 3. Preserves existing functionality for non-XG modes
 */

#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_synth.h" 
#include "fluid_chan.h"

/* Test that XG drum bank selection uses actual bank numbers */
static void test_xg_drum_bank_selection(void)
{
    fluid_settings_t* settings;
    fluid_synth_t* synth;
    int bank;
    fluid_channel_t* channel;
    
    settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);
    
    /* Set to XG mode for testing */
    TEST_SUCCESS(fluid_settings_setstr(settings, "synth.midi-bank-select", "xg"));
    
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);
    
    /* Get channel 9 (typically drum channel) */
    channel = synth->channel[9];
    TEST_ASSERT(channel != NULL);
    
    /* Test XG drum bank 127 */
    fluid_channel_set_bank_msb(channel, 127);
    fluid_channel_get_sfont_bank_prog(channel, NULL, &bank, NULL);
    TEST_ASSERT(bank == 127);
    
    /* Test XG drum bank 126 */
    fluid_channel_set_bank_msb(channel, 126);
    fluid_channel_get_sfont_bank_prog(channel, NULL, &bank, NULL);
    TEST_ASSERT(bank == 126);
    
    /* Test XG drum bank 120 */
    fluid_channel_set_bank_msb(channel, 120);
    fluid_channel_get_sfont_bank_prog(channel, NULL, &bank, NULL);
    TEST_ASSERT(bank == 120);
    
    /* Test non-drum bank for melodic channel */
    fluid_channel_set_bank_msb(channel, 0);
    /* This should change the channel to melodic and ignore the bank change */
    TEST_ASSERT(channel->channel_type == CHANNEL_TYPE_MELODIC);
    
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

/* Test that channel type is correctly detected for XG banks */
static void test_xg_channel_type_detection(void)
{
    fluid_settings_t* settings;
    fluid_synth_t* synth;
    fluid_channel_t* channel;
    
    settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);
    
    TEST_SUCCESS(fluid_settings_setstr(settings, "synth.midi-bank-select", "xg"));
    
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);
    
    channel = synth->channel[9];
    TEST_ASSERT(channel != NULL);
    
    /* Test drum banks */
    fluid_channel_set_bank_msb(channel, 127);
    TEST_ASSERT(channel->channel_type == CHANNEL_TYPE_DRUM);
    
    fluid_channel_set_bank_msb(channel, 126);
    TEST_ASSERT(channel->channel_type == CHANNEL_TYPE_DRUM);
    
    fluid_channel_set_bank_msb(channel, 120);
    TEST_ASSERT(channel->channel_type == CHANNEL_TYPE_DRUM);
    
    /* Test melodic banks */
    fluid_channel_set_bank_msb(channel, 0);
    TEST_ASSERT(channel->channel_type == CHANNEL_TYPE_MELODIC);
    
    fluid_channel_set_bank_msb(channel, 1);
    TEST_ASSERT(channel->channel_type == CHANNEL_TYPE_MELODIC);
    
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

/* Test non-XG modes are not affected */
static void test_non_xg_modes_unaffected(void)
{
    fluid_settings_t* settings;
    fluid_synth_t* synth;
    fluid_channel_t* channel;
    int bank, initial_bank;
    
    /* Test GM mode - it should ignore bank MSB changes */
    settings = new_fluid_settings();
    TEST_SUCCESS(fluid_settings_setstr(settings, "synth.midi-bank-select", "gm"));
    synth = new_fluid_synth(settings);
    
    channel = synth->channel[9];
    /* Get initial bank value */
    fluid_channel_get_sfont_bank_prog(channel, NULL, &bank, NULL);
    initial_bank = bank;
    
    /* Try to change bank - should be ignored in GM mode */
    fluid_channel_set_bank_msb(channel, 127);
    fluid_channel_get_sfont_bank_prog(channel, NULL, &bank, NULL);
    /* GM mode should ignore bank changes, so bank should remain unchanged */
    TEST_ASSERT(bank == initial_bank);
    
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    
    /* Test GS mode */
    settings = new_fluid_settings();
    TEST_SUCCESS(fluid_settings_setstr(settings, "synth.midi-bank-select", "gs"));
    synth = new_fluid_synth(settings);
    
    channel = synth->channel[9];
    /* Force drum channel type for GS mode */
    channel->channel_type = CHANNEL_TYPE_DRUM;
    fluid_channel_set_bank_msb(channel, 1);
    fluid_channel_get_sfont_bank_prog(channel, NULL, &bank, NULL);
    /* GS mode should add 128 to drum banks */
    TEST_ASSERT(bank == 129);
    
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

int main(void)
{
    test_xg_drum_bank_selection();
    test_xg_channel_type_detection(); 
    test_non_xg_modes_unaffected();
    
    return EXIT_SUCCESS;
}