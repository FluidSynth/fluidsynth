
#pragma once


#include "fluid_sys.h"
#include "fluid_rev.h"
#include "fluid_rev_filters.h"

// A reverbator inspired by Lexicon reverbs.
// AP = allpass filter
// dly = delay line
// lrgain = left to right feedback coefficient gain
// rlgain = right to left feedback coefficient gain
// wet_gain = wet output gain
// dry_gain = dry output gain
/*-------------------------------------------------------------------------------
                                                                 |\ dry_gain
 left in>---->---------------------------------------------------| >------|
          |                                                      |/     /---\
          |                                                             | + |--> left out
          |    -----   -----   /---\   -----   -----   -----     |\     \---/
          |-->-|AP0|->-|AP1|->-| + |->-|AP2|->-|AP3|->-|AP4|--->-| >------|
               -----   -----   \---/   -----   -----   -----  |  |/ wet_gain
                                  \  lrgain/|   ------        |
                                   \  /---< |---|dly0|---<----|
                                    \/     \|   ------
                                    /\     /|   ------
                                   /  \---< |---|dly1|---<----|
                                  /  rlgain\|   ------        |
               -----   -----   /---\   -----   -----   -----  |  |\ wet_gain   
          |-->-|AP5|->-|AP6|->-| + |->-|AP7|->-|AP8|->-|AP9|--->-| >------|
          |    -----   -----   \---/   -----   -----   -----     |/     /---\
          |                                                             | + |--> right out
          |                                                      |\     \---/
right in>---->---------------------------------------------------| >------|
                                                                 |/ dry_gain
---------------------------------------------------------------------------------*/

constexpr const int NUM_OF_AP_SECTS = 10;
constexpr const int NUM_OF_DELAY_SECTS = 2;
constexpr const int NUM_OF_SECTS = NUM_OF_AP_SECTS + NUM_OF_DELAY_SECTS;

constexpr const struct {float length; float coef; } LEX_REVERB_PARMS[ NUM_OF_SECTS ] =
{
			{ 50.00f /*ms*/,0.750f },		/* AP0 */
			{ 44.50f /*ms*/,0.720f },		/* AP1 */
			{ 37.37f /*ms*/,0.691f },		/* AP2 */
			{ 24.85f /*ms*/,0.649f },		/* AP3 */
			{ 19.31f /*ms*/,0.662f },		/* AP4 */
			{ 49.60f /*ms*/,0.750f },		/* AP5 */
			{ 45.13f /*ms*/,0.720f },		/* AP6 */
			{ 35.25f /*ms*/,0.691f },		/* AP7 */
			{ 28.17f /*ms*/,0.649f },		/* AP8 */
			{ 15.59f /*ms*/,0.646f },		/* AP9 */
			{  8.71f /*ms*/,0.646f },		/* left into right delay, lrgain */
			{ 12.05f /*ms*/,0.666f }   	    /* right into left delay, rlgain */
};

struct fluid_revmodel_lexverb : public _fluid_revmodel_t
{
    fluid_real_t roomsize;
    fluid_real_t damp;
    fluid_real_t level;
    fluid_real_t wet1;
    fluid_real_t wet2;
    fluid_real_t width;
    fluid_real_t cached_sample_rate;

    fluid_reverb_allpass<float, FLUID_REVERB_ALLPASS_SCHROEDER> ap[NUM_OF_AP_SECTS];
    fluid_reverb_delay_line<float> dl[NUM_OF_DELAY_SECTS];

    explicit fluid_revmodel_lexverb(fluid_real_t sample_rate);
    ~fluid_revmodel_lexverb() override;

    void processmix(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out) override;
    void processreplace(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out) override;

    void reset() override;
    void set(int set, fluid_real_t roomsize, fluid_real_t damping,
             fluid_real_t width, fluid_real_t level) override;
    int samplerate_change(fluid_real_t sample_rate) override;

private:
    template<bool MIX>
    void process(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out);
};

typedef struct fluid_revmodel_lexverb fluid_revmodel_lexverb_t;
