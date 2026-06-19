/*

  Freeverb

  Written by Jezar at Dreampoint, June 2000
  http://www.dreampoint.co.uk
  This code is public domain

  Translated to C by Peter Hanappe, Mai 2001
*/

#include "fluid_rev_freeverb.h"

/***************************************************************
 *
 *                           REVERB
 */

/* Denormalising:
 *
 * We have a recursive filter. The output decays exponentially, if the input
 * stops. So the numbers get smaller and smaller... At some point, they reach
 * 'denormal' level. On some platforms this will lead to drastic spikes in the
 * CPU load. This is especially noticeable on some older Pentium (especially
 * Pentium 3) processors, but even more modern Intel Core processors still show
 * reduced performance with denormals. While there are compile-time switches to
 * treat denormals as zero for a lot of processors, those are not available or
 * effective on all platforms.
 *
 * The fix used here: Use a small DC-offset in the filter calculations.  Now
 * the signals converge not against 0, but against the offset.  The constant
 * offset is invisible from the outside world (i.e. it does not appear at the
 * output.  There is a very small turn-on transient response, which should not
 * cause problems.
 */
#define DC_OFFSET ((fluid_real_t)1e-8)

#define	fixedgain 0.015f
/* scale_wet_width is a compensation weight factor to get an output
   amplitude (wet) rather independent of the width setting.
    0: the output amplitude is fully dependent on the width setting.
   >0: the output amplitude is less dependent on the width setting.
   With a scale_wet_width of 0.2 the output amplitude is rather
   independent of width setting (see fluid_revmodel_update()).
 */
#define scale_wet_width 0.2f
#define scalewet 3.0f
#define scaledamp 1.0f
#define scaleroom 0.28f
#define offsetroom 0.7f
#define stereospread 23

/*
 These values assume 44.1KHz sample rate
 they will probably be OK for 48KHz sample rate
 but would need scaling for 96KHz (or other) sample rates.
 The values were obtained by listening tests.
*/
#define combtuningL1 1116
#define combtuningR1 (1116 + stereospread)
#define combtuningL2 1188
#define combtuningR2 (1188 + stereospread)
#define combtuningL3 1277
#define combtuningR3 (1277 + stereospread)
#define combtuningL4 1356
#define combtuningR4 (1356 + stereospread)
#define combtuningL5 1422
#define combtuningR5 (1422 + stereospread)
#define combtuningL6 1491
#define combtuningR6 (1491 + stereospread)
#define combtuningL7 1557
#define combtuningR7 (1557 + stereospread)
#define combtuningL8 1617
#define combtuningR8 (1617 + stereospread)
#define allpasstuningL1 556
#define allpasstuningR1 (556 + stereospread)
#define allpasstuningL2 441
#define allpasstuningR2 (441 + stereospread)
#define allpasstuningL3 341
#define allpasstuningR3 (341 + stereospread)
#define allpasstuningL4 225
#define allpasstuningR4 (225 + stereospread)

static void fluid_freeverb_revmodel_update(fluid_revmodel_freeverb_t *rev);
static void fluid_freeverb_revmodel_init(fluid_revmodel_freeverb_t *rev);
static void fluid_freeverb_set_revmodel_buffers(fluid_revmodel_freeverb_t *rev,
                                                fluid_real_t sample_rate);

fluid_revmodel_freeverb::fluid_revmodel_freeverb(fluid_real_t sample_rate)
{
    int i;

    fluid_freeverb_set_revmodel_buffers(this, sample_rate);

    /* Set default values */
    for(i = 0; i < numallpasses; i++)
    {
        allpassL[i].set_feedback(0.5f);
        allpassR[i].set_feedback(0.5f);
    }

    gain = fixedgain;
}

fluid_revmodel_freeverb::~fluid_revmodel_freeverb()
{
}

static void
fluid_freeverb_set_revmodel_buffers(fluid_revmodel_freeverb_t *rev,
                                    fluid_real_t sample_rate)
{

    float srfactor = sample_rate / 44100.0f;

    rev->combL[0].set_buffer(combtuningL1 * srfactor);
    rev->combR[0].set_buffer(combtuningR1 * srfactor);
    rev->combL[1].set_buffer(combtuningL2 * srfactor);
    rev->combR[1].set_buffer(combtuningR2 * srfactor);
    rev->combL[2].set_buffer(combtuningL3 * srfactor);
    rev->combR[2].set_buffer(combtuningR3 * srfactor);
    rev->combL[3].set_buffer(combtuningL4 * srfactor);
    rev->combR[3].set_buffer(combtuningR4 * srfactor);
    rev->combL[4].set_buffer(combtuningL5 * srfactor);
    rev->combR[4].set_buffer(combtuningR5 * srfactor);
    rev->combL[5].set_buffer(combtuningL6 * srfactor);
    rev->combR[5].set_buffer(combtuningR6 * srfactor);
    rev->combL[6].set_buffer(combtuningL7 * srfactor);
    rev->combR[6].set_buffer(combtuningR7 * srfactor);
    rev->combL[7].set_buffer(combtuningL8 * srfactor);
    rev->combR[7].set_buffer(combtuningR8 * srfactor);
    rev->allpassL[0].set_buffer(allpasstuningL1 * srfactor);
    rev->allpassR[0].set_buffer(allpasstuningR1 * srfactor);
    rev->allpassL[1].set_buffer(allpasstuningL2 * srfactor);
    rev->allpassR[1].set_buffer(allpasstuningR2 * srfactor);
    rev->allpassL[2].set_buffer(allpasstuningL3 * srfactor);
    rev->allpassR[2].set_buffer(allpasstuningR3 * srfactor);
    rev->allpassL[3].set_buffer(allpasstuningL4 * srfactor);
    rev->allpassR[3].set_buffer(allpasstuningR4 * srfactor);

    /* Clear all buffers */
    fluid_freeverb_revmodel_init(rev);
}


static void
fluid_freeverb_revmodel_init(fluid_revmodel_freeverb_t *rev)
{
    int i;

    for(i = 0; i < numcombs; i++)
    {
        rev->combL[i].fill_buffer(DC_OFFSET);
        rev->combR[i].fill_buffer(DC_OFFSET);
    }

    for(i = 0; i < numallpasses; i++)
    {
        rev->allpassL[i].fill_buffer(DC_OFFSET);
        rev->allpassR[i].fill_buffer(DC_OFFSET);
    }
}

static void
fluid_freeverb_revmodel_reset(fluid_revmodel_freeverb_t *rev)
{
    fluid_freeverb_revmodel_init(rev);
}

static void
fluid_freeverb_revmodel_update(fluid_revmodel_freeverb_t *rev)
{
    /* Recalculate internal values after parameter change */
    int i;

    /* The stereo amplitude equation (wet1 and wet2 below) have a
    tendency to produce high amplitude with high width values ( 1 < width < 100).
    This results in an unwanted noisy output clipped by the audio card.
    To avoid this dependency, we divide by (1 + rev->width * scale_wet_width)
    Actually, with a scale_wet_width of 0.2, (regardless of level setting),
    the output amplitude (wet) seems rather independent of width setting */
    fluid_real_t wet = (rev->level * scalewet) /
                       (1.0f + rev->width * scale_wet_width);

    /* wet1 and wet2 are used by the stereo effect controlled by the width setting
    for producing a stereo output from a monophonic reverb signal.
    Please see the note above about a side effect tendency */
    rev->wet1 = wet * (rev->width / 2.0f + 0.5f);
    rev->wet2 = wet * ((1.0f - rev->width) / 2.0f);

    for(i = 0; i < numcombs; i++)
    {
        rev->combL[i].set_feedback(rev->roomsize);
        rev->combR[i].set_feedback(rev->roomsize);
    }

    for(i = 0; i < numcombs; i++)
    {
        rev->combL[i].set_damp(rev->damp);
        rev->combR[i].set_damp(rev->damp);
    }
}

/**
 * Set one or more reverb parameters.
 * @param rev Reverb instance
 * @param set One or more flags from #fluid_revmodel_set_t indicating what
 *   parameters to set (#FLUID_REVMODEL_SET_ALL to set all parameters)
 * @param roomsize Reverb room size
 * @param damping Reverb damping
 * @param width Reverb width
 * @param level Reverb level
 */
static void
fluid_freeverb_revmodel_set(fluid_revmodel_freeverb_t *rev, int set,
                            fluid_real_t roomsize, fluid_real_t damping,
                            fluid_real_t width, fluid_real_t level)
{
    if(set & FLUID_REVMODEL_SET_ROOMSIZE)
    {
        /* With upper limit above 1.07, the output amplitude will grow
        exponentially. So, keeping this upper limit to 1.0 seems sufficient
        as it produces yet a long reverb time */
        fluid_clip(roomsize, 0.0f, 1.0f);
        rev->roomsize = (roomsize * scaleroom) + offsetroom;
    }

    if(set & FLUID_REVMODEL_SET_DAMPING)
    {
        rev->damp = damping * scaledamp;
    }

    if(set & FLUID_REVMODEL_SET_WIDTH)
    {
        rev->width = width;
    }

    if(set & FLUID_REVMODEL_SET_LEVEL)
    {
        fluid_clip(level, 0.0f, 1.0f);
        rev->level = level;
    }

    fluid_freeverb_revmodel_update(rev);
}

static int
fluid_freeverb_revmodel_samplerate_change(fluid_revmodel_freeverb_t *rev,
                                          fluid_real_t sample_rate)
{
    fluid_freeverb_set_revmodel_buffers(rev, sample_rate);

    return FLUID_OK;
}

void fluid_revmodel_freeverb::processmix(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out)
{
    process<true>(in, left_out, right_out);
}

void fluid_revmodel_freeverb::processreplace(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out)
{
    process<false>(in, left_out, right_out);
}

template<bool MIX>
void fluid_revmodel_freeverb::process(const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out)
{
    int i, k = 0;
    fluid_real_t outL, outR, input;

    for(k = 0; k < FLUID_BUFSIZE; k++)
    {

        outL = outR = 0;

        /* The original Freeverb code expects a stereo signal and 'input'
         * is set to the sum of the left and right input sample. Since
         * this code works on a mono signal, 'input' is set to twice the
         * input sample. */
        input = (2.0f * in[k] + DC_OFFSET) * this->gain;

        /* Accumulate comb filters in parallel */
        for(i = 0; i < numcombs; i++)
        {
            outL += this->combL[i].process(input);
            outR += this->combR[i].process(input);
        }

        /* Feed through allpasses in series */
        for(i = 0; i < numallpasses; i++)
        {
            outL = this->allpassL[i].process(outL);
            outR = this->allpassR[i].process(outR);
        }

        /* Remove the DC offset */
        outL -= DC_OFFSET;
        outR -= DC_OFFSET;

        fluid_real_t out_left = outL * this->wet1 + outR * this->wet2;
        fluid_real_t out_right = outR * this->wet1 + outL * this->wet2;

        if(MIX)
        {
            /* Calculate output MIXING with anything already there */
            left_out[k] += out_left;
            right_out[k] += out_right;
        }
        else
        {
            /* Calculate output REPLACING anything already there */
            left_out[k] = out_left;
            right_out[k] = out_right;
        }
    }
}

void fluid_revmodel_freeverb::reset()
{
    fluid_freeverb_revmodel_reset(this);
}

void fluid_revmodel_freeverb::set(int set, fluid_real_t roomsize, fluid_real_t damping,
                                  fluid_real_t width, fluid_real_t level)
{
    fluid_freeverb_revmodel_set(this, set, roomsize, damping, width, level);
}

int fluid_revmodel_freeverb::samplerate_change(fluid_real_t sample_rate)
{
    return fluid_freeverb_revmodel_samplerate_change(this, sample_rate);
}
