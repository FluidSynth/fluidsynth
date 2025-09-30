
#include "test.h"
#include "utils/fluid_conv.h"
#include "synth/fluid_chan.h"

#include <iostream>

// this test makes sure FLUID_SNPRINTF uses a proper C99 compliant implementation

void print_portamento_time7(fluid_channel_t *chan)
{
    for(int i = 0; i<128; i++)
    {
        fluid_channel_set_cc(chan, PORTAMENTO_TIME_MSB, i);
        auto ms = fluid_channel_portamentotime_with_mode(chan, FLUID_PORTAMENTO_TIME_MODE_XG_GS, false);
        std::cout << "CC5: " << i << " ^= " << ms/1000.0 << " s" << std::endl;
    }
}

void test_portamento_time7(fluid_channel_t *chan, enum fluid_portamento_time_mode time_mode, int lsb_seen)
{
    fluid_channel_set_cc(chan, PORTAMENTO_TIME_LSB, 80);
    fluid_channel_set_cc(chan, PORTAMENTO_TIME_MSB, 0);
    auto ms = fluid_channel_portamentotime_with_mode(chan, time_mode, lsb_seen);
    TEST_ASSERT(0 == ms);

    fluid_channel_set_cc(chan, PORTAMENTO_TIME_MSB, 85);
    ms = fluid_channel_portamentotime_with_mode(chan, time_mode, lsb_seen);
    TEST_ASSERT(5*1000 == ms);

    fluid_channel_set_cc(chan, PORTAMENTO_TIME_MSB, 127);
    ms = fluid_channel_portamentotime_with_mode(chan, time_mode, lsb_seen);
    TEST_ASSERT(480*1000 == ms);
}

void test_portamento_time_lin(fluid_channel_t *chan, enum fluid_portamento_time_mode time_mode, int lsb_seen)
{
    fluid_channel_set_cc(chan, PORTAMENTO_TIME_LSB, 0);
    fluid_channel_set_cc(chan, PORTAMENTO_TIME_MSB, 8);
    auto ms = fluid_channel_portamentotime_with_mode(chan, time_mode, lsb_seen);
    TEST_ASSERT(1024 == ms);

    fluid_channel_set_cc(chan, PORTAMENTO_TIME_MSB, 0);
    fluid_channel_set_cc(chan, PORTAMENTO_TIME_LSB, 8);
    ms = fluid_channel_portamentotime_with_mode(chan, time_mode, lsb_seen);
    TEST_ASSERT(8 == ms);

    fluid_channel_set_cc(chan, PORTAMENTO_TIME_MSB, 0);
    fluid_channel_set_cc(chan, PORTAMENTO_TIME_LSB, 127);
    ms = fluid_channel_portamentotime_with_mode(chan, time_mode, lsb_seen);
    TEST_ASSERT(127 == ms);

    fluid_channel_set_cc(chan, PORTAMENTO_TIME_MSB, 0);
    fluid_channel_set_cc(chan, PORTAMENTO_TIME_LSB, 0);
    ms = fluid_channel_portamentotime_with_mode(chan, time_mode, lsb_seen);
    TEST_ASSERT(0 == ms);
}

int main(void)
{
    auto *settings = new_fluid_settings();
    auto *synth = new_fluid_synth(settings);
    auto *chan = new_fluid_channel(synth, 0);

    print_portamento_time7(chan);
    test_portamento_time7(chan, FLUID_PORTAMENTO_TIME_MODE_XG_GS, false);
    test_portamento_time7(chan, FLUID_PORTAMENTO_TIME_MODE_XG_GS, true);
    test_portamento_time7(chan, FLUID_PORTAMENTO_TIME_MODE_AUTO, false);

    test_portamento_time_lin(chan, FLUID_PORTAMENTO_TIME_MODE_LINEAR, false);
    test_portamento_time_lin(chan, FLUID_PORTAMENTO_TIME_MODE_LINEAR, true);
    test_portamento_time_lin(chan, FLUID_PORTAMENTO_TIME_MODE_AUTO, true);

    delete_fluid_channel(chan);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    return EXIT_SUCCESS;
}
