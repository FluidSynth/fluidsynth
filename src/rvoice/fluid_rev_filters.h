/*
 * FluidSynth - Dedicated reverb filter building blocks
 *
 * Provides C++11 classes for allpass filters, comb filters, and delay lines
 * used by the reverb engines. The classes are header-only to allow easy reuse
 * across the different reverbs without changing their behavior.
 */

#pragma once

#ifndef FLUID_REV_FILTERS_H
#define FLUID_REV_FILTERS_H

#include <vector>
#include <stdexcept>

/** Algorithm variant used by the allpass filter. */
enum fluid_reverb_allpass_mode
{
    /** Freeverb-style allpass implementation, see https://ccrma.stanford.edu/~jos/Reverb/Freeverb_Allpass_Approximation.html */
    FLUID_REVERB_ALLPASS_FREEVERB,
    /** Standard Schroeder allpass implementation. */
    FLUID_REVERB_ALLPASS_SCHROEDER
};

/**
 * @brief One-pole damping low-pass filter state for delay lines.
 *
 * @tparam sample_t Floating point sample type (float or double).
 */
template<typename sample_t>
struct fluid_reverb_delay_damping
{
    /** Filter history value. */
    sample_t buffer;
    /** Feed-forward coefficient. */
    sample_t b0;
    /** Feedback coefficient. */
    sample_t a1;

    sample_t process(sample_t input)
    {
        /* Equivalent to buffer += b0 * (in - buffer) when a1 is maintained as (1 - b0). */
        this->buffer = this->b0 * input + this->a1 * this->buffer;
        return this->buffer;
    }

    void set_ff_coeff(sample_t b0)
    {
        /* Keep a1 = 1 - b0 so the one-pole filters maintain unity DC gain. */
        this->b0 = b0;
        this->a1 = 1.0f - b0;
    }

    void set_fb_coeff(sample_t a1)
    {
        this->a1 = a1;
        this->b0 = 1.0f - a1;
    }

    void set_coeffs(sample_t ff, sample_t fb)
    {
        this->b0 = ff;
        this->a1 = fb;
    }
};

/**
 * @brief Delay line used by reverb algorithms.
 *
 * @tparam sample_t Floating point sample type (float or double).
 */
template<typename sample_t, typename damping_t = fluid_reverb_delay_damping<sample_t> >
class fluid_reverb_delay_line
{
public:
    fluid_reverb_delay_line()
        : line(),
          line_in(0),
          line_out(0),
          damping(),
          coefficient(0),
          last_output(0)
    {
    }

    /** Allocate the delay buffer with the given length. */
    void set_buffer(int length)
    {
        if(length <= 0)
        {
            throw std::invalid_argument("Delay buffer length must be positive");
        }
        line_in = 0;
        line_out = 0;
        last_output = 0;
        line.resize(static_cast<size_t>(length), sample_t());
        // do not call shrink_to_fit() here since the buffer is inited by its biggest size and may be decrease again later in realtime, while avoiding unnecessary reallocations
    }

    /** Fill the delay buffer without changing indices. */
    void fill_buffer(sample_t value)
    {
        for(int i = 0; i < size(); ++i)
        {
            line[i] = value;
        }
    }

    /** Set the current read/write indices. */
    void set_positions(int in_pos, int out_pos)
    {
        line_in = in_pos;
        line_out = out_pos;
    }

    /** Set both read and write indices to the same position for single-tap use. */
    void set_single_tap_position(int index)
    {
        line_in = index;
        line_out = index;
    }

    /** Read the current sample at the output position (caller ensures valid index). */
    sample_t read() const
    {
        return line[line_out];
    }

    /** Write a sample at the output position (caller ensures valid index). */
    void write(sample_t value)
    {
        line[line_out] = value;
    }

    /** Advance the output position by one sample with wraparound. */
    void advance()
    {
        if(++line_out >= size())
        {
            line_out = 0;
        }
    }

    /**
     * Advance and keep read/write indices aligned for single-tap filters where
     * the read and write positions must remain identical.
     */
    void advance_single_tap()
    {
        advance();
        line_in = line_out;
    }

    /** Set the coefficient used by lexverb delay mixing. */
    void set_coefficient(sample_t value)
    {
        coefficient = value;
    }

    /** Return the coefficient used by lexverb delay mixing. */
    sample_t get_coefficient() const
    {
        return coefficient;
    }

    /** Set the cached output value. */
    void set_last_output(sample_t value)
    {
        last_output = value;
    }

    /** Return the most recently produced output sample. */
    sample_t get_last_output() const
    {
        return last_output;
    }

    /** Check if a buffer has been allocated. */
    bool has_buffer() const
    {
        return !this->line.empty();
    }

    int size() const
    {
        return static_cast<int>(this->line.size());
    }

    /**
     * Process a single sample through the delay line (read/write same position).
     *
     * @param input Input sample.
     * @return Delayed output sample.
     */
    sample_t process(sample_t input)
    {
        sample_t output = line[line_out];
        line[line_out] = input;

        advance_single_tap();

        last_output = output;
        return output;
    }

    /** Delay buffer storage. */
    std::vector<sample_t> line;
    /** Write index into the delay buffer. */
    int line_in;
    /**
     * Index into the delay buffer used for reading; single-tap operations also
     * write at this index, while multi-tap delays may keep line_in separate.
     */
    int line_out;
    /** Optional damping low-pass filter state. */
    damping_t damping;
    /** Optional coefficient for lexverb cross-feed. */
    sample_t coefficient;
    /** Last output sample produced by process(). */
    sample_t last_output;
};

/**
 * @brief Allpass filter stage for reverb processing.
 *
 * Reuses the shared fluid_reverb_delay_line storage to avoid duplicating delay
 * buffer bookkeeping across filter types.
 *
 * @tparam sample_t Floating point sample type (float or double).
 */
template<typename sample_t, enum fluid_reverb_allpass_mode mode = FLUID_REVERB_ALLPASS_SCHROEDER>
class fluid_reverb_allpass
{
public:
    /** Set the feedback coefficient controlling the allpass response. */
    void set_feedback(sample_t value)
    {
        feedback = value;
    }

    /** Get the feedback coefficient. */
    sample_t get_feedback() const
    {
        return feedback;
    }

    /** Allocate the delay buffer with the given length. */
    void set_buffer(int size)
    {
        delay.set_buffer(size);
        last_output = 0;
    }

    /** Fill the delay buffer without changing the current index. */
    void fill_buffer(sample_t value)
    {
        delay.fill_buffer(value);
    }

    /** Set the current delay buffer index (used when resetting state). */
    void set_index(int index)
    {
        /* Keep read/write indices aligned for the shared delay buffer. */
        delay.set_single_tap_position(index);
    }

    /** Set the cached output value (used for lexverb cross-feedback). */
    void set_last_output(sample_t value)
    {
        last_output = value;
    }

    /** Return the most recently produced output sample. */
    sample_t get_last_output() const
    {
        return last_output;
    }

    /** Check if a buffer has been allocated. */
    bool has_buffer() const
    {
        return delay.has_buffer();
    }

    /**
     * Process a single sample through the allpass filter.
     *
     * @param input Input sample.
     * @return Filtered output sample.
     */
    sample_t process(sample_t input)
    {
        sample_t bufout = delay.read();
        sample_t output;

        sample_t delay_in = input + (bufout * feedback);
        if(mode == FLUID_REVERB_ALLPASS_FREEVERB)
        {
            output = bufout - input;
        }
        else
        {
            output = bufout - (delay_in * feedback);
        }

        delay.write(delay_in);
        delay.advance_single_tap();
        last_output = output;
        return output;
    }

    /** Feedback coefficient (g) for the allpass filter. */
    sample_t feedback;
    /** Shared delay buffer storage for the filter. */
    fluid_reverb_delay_line<sample_t> delay;
    /** Last output sample produced by process(). */
    sample_t last_output;
};

/**
 * @brief Comb filter stage for reverb processing.
 *
 * Reuses the shared fluid_reverb_delay_line storage to avoid duplicating delay
 * buffer bookkeeping across filter types.
 *
 * @tparam sample_t Floating point sample type (float or double).
 */
template<typename sample_t>
class fluid_reverb_comb
{
public:
    /** Allocate the delay buffer with the given length. */
    void set_buffer(int size)
    {
        delay.set_buffer(size);
        filterstore = 0;
    }

    /** Fill the delay buffer without changing the current index. */
    void fill_buffer(sample_t value)
    {
        delay.fill_buffer(value);
    }

    /** Set the damping value (0..1) which controls the comb low pass. */
    void set_damp(sample_t value)
    {
        damp1 = value;
        damp2 = (sample_t)1 - value;
    }

    /** Return the current damping value. */
    sample_t get_damp() const
    {
        return damp1;
    }

    /** Set the feedback coefficient for the comb filter. */
    void set_feedback(sample_t value)
    {
        feedback = value;
    }

    /** Return the feedback coefficient. */
    sample_t get_feedback() const
    {
        return feedback;
    }

    /** Check if a buffer has been allocated. */
    bool has_buffer() const
    {
        return delay.has_buffer();
    }

    /**
     * Process a single sample through the comb filter.
     *
     * @param input Input sample.
     * @return Filtered output sample.
     */
    sample_t process(sample_t input)
    {
        sample_t output = delay.read();
        filterstore = (output * damp2) + (filterstore * damp1);
        delay.write(input + (filterstore * feedback));
        delay.advance_single_tap();
        return output;
    }

    /** Feedback coefficient (roomsize-dependent). */
    sample_t feedback;
    /** Internal low-pass filter storage. */
    sample_t filterstore;
    /** Damping coefficient (damp1) for the low-pass filter. */
    sample_t damp1;
    /** Complementary damping coefficient (damp2). */
    sample_t damp2;
    /** Shared delay buffer storage for the filter. */
    fluid_reverb_delay_line<sample_t> delay;
};

#endif
