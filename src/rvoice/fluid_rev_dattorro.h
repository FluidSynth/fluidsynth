#pragma once

#include <array>

#include "fluid_rev.h"
#include "fluid_rev_filters.h"


// A reverbator based on Jon Dattorro's plate reverb.
// https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf
// pre = predelay
// AP = allpass diffuser / allpass in tank
// dly = delay line in tank
// bw = bandwidth one-pole lowpass on input (before diffuser)
// damp = damping one-pole lowpass in each tank loop
// decay = feedback gain (roomsize-controlled)
// taps = readout taps taken from various tank delay/AP buffers
// wet_gain = wet output gain (wet1/wet2 width-mix happens after tap sum)
// dry_gain = dry output gain (outside this unit in FluidSynth mixer)
//
// Note: This implementation is mono-in -> stereo-out (single input path).
/*-------------------------------------------------------------------------------
                                                                  (dry path mixed elsewhere)
mono in>--->(*DATTORRO_TRIM*)--->(pre dly)---->(bw LP)---->--->---+--------------------\
                                               ^                  |                     \
                                               |                  |                      \
                                          bw_state fb             |                       \
                                                                 \|/
                                                            -----   -----   -----   -----
                                                            |AP0|-> |AP1|-> |AP2|-> |AP3|    (input diffuser)
                                                            -----   -----   -----   -----
                                                                      |
                                                                     split
                                                                      |
                         +--------------------------------------------+--------------------------------------------+
                         |                                                                                         |
                         |                                                                                         |
                         |                                   LEFT TANK PATH                                        |   RIGHT TANK PATH
                         |                                                                                         |
                         |   (cross feedback from opposite side)                                                   |   (cross feedback from opposite side)
                         |     +-------------------------------+                                                   |     +-------------------------------+
                         |     |                               |                                                   |     |                               |
                         |     v                               |                                                   |     v                               |
                         |  /---\                              |                                                   |  /---\                              |
                         |  | + |<----- decay * last(tank_dly3)|                                                   |  | + |<----- decay * last(tank_dly1)|
                         |  \---/                              |                                                   |  \---/                              |
                         |     |                               |                                                   |     |                               |
                         |    -----     -------     (damp LP)  |                                                   |    -----     -------     (damp LP)  |
                         |    |AP4|--->|dly0|--->(damp_state_L)----->-----    -------                              |    |AP6|---> |dly2 |--->(damp_state_R)----->-----    ------
                         |    -----     -------                |     |AP5|--->|dly1|--->(to cross fb: last dly1)   |    -----     -------                |       |AP7|--->|dly3|--->(to cross fb: last dly3)
                         |                         ^           |     -----    -------                              |                         ^           |       -----    ------
                         |                         |           |                                                   |                         |           |
                         |                     decay * damp_L  |                                                   |                   decay * damp_R    |
                         |                         |           |                                                   |                         |           |
                         +-------------------------+-----------+                                                   +-------------------------+-----------+

   TAP READOUTS (stereo decorrelated output is produced by different tap combinations)
   --------------------------------------------------------------------------------
   left tap sum  =  +tap(dly2,t0) +tap(dly2,t1) -tap(AP7,t2) +tap(dly3,t3) -tap(dly0,t4) -tap(AP5,t5) -tap(dly1,t6)
   right tap sum =  +tap(dly0,t7) +tap(dly0,t8) -tap(AP5,t9) +tap(dly1,t10)-tap(dly2,t11)-tap(AP7,t12)-tap(dly3,t13)

   left wet  = left_tap_sum  * wet1  + right_tap_sum * wet2  ---> left out
   right wet = right_tap_sum * wet1  + left_tap_sum  * wet2  ---> right out
---------------------------------------------------------------------------------*/
struct fluid_revmodel_dattorro : public _fluid_revmodel_t
{
    fluid_real_t roomsize;
    fluid_real_t damp;
    fluid_real_t level;
    fluid_real_t wet1;
    fluid_real_t wet2;
    fluid_real_t width;
    fluid_real_t bandwidth;
    fluid_real_t decay;
    fluid_real_t cached_sample_rate;

    fluid_reverb_delay_line<float> predelay;
    // input diffusions
    fluid_reverb_allpass<float, FLUID_REVERB_ALLPASS_SCHROEDER> input_ap[4];
    // 2 decay diffusions left + 2 decay diffusions right
    fluid_reverb_allpass<float, FLUID_REVERB_ALLPASS_SCHROEDER> tank_ap[4];
    // delay between the decay diffusions in each tank, with damping filters in the feedback path
    fluid_reverb_delay_line<float> tank_delay[4];
    // readout tap positions in the delay lines and allpass filters for producing the output, 7 per channel
    std::array<int, 7*2> taps;

    explicit fluid_revmodel_dattorro(fluid_real_t sample_rate);
    ~fluid_revmodel_dattorro() override;

    void processmix(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out) override;
    void processreplace(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out) override;
    void reset() override;
    void set(int set, fluid_real_t roomsize, fluid_real_t damping,
             fluid_real_t width, fluid_real_t level) override;
    int samplerate_change(fluid_real_t sample_rate) override;

private:
    void setup();
    void update();

    template<bool MIX>
    void process(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out);
};

typedef struct fluid_revmodel_dattorro fluid_revmodel_dattorro_t;
