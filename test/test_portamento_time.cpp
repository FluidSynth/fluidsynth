
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

int main(void)
{
    auto *settings = new_fluid_settings();
    auto *synth = new_fluid_synth(settings);
    auto *chan = new_fluid_channel(synth, 0);

    print_portamento_time7(chan);

    delete_fluid_channel(chan);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    return EXIT_SUCCESS;
}
