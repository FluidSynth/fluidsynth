/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *  
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */


/* Purpose: 
 * Low-level voice processing:
 *
 * - interpolates (obtains values between the samples of the original waveform data)
 * - filters (applies a lowpass filter with variable cutoff frequency and quality factor)
 * - mixes the processed sample to left and right output using the pan setting
 * - sends the processed sample to chorus and reverb
 *
 *
 * This file does -not- generate an object file.
 * Instead, it is #included in several places in fluid_voice.c.
 * The motivation for this is
 * - Calling it as a subroutine may be time consuming, especially with optimization off
 * - The previous implementation as a macro was clumsy to handle
 *
 *
 * Fluid_voice.c sets a couple of variables before #including this:
 * - dsp_data: Pointer to the original waveform data
 * - dsp_left_buf: The generated signal goes here, left channel
 * - dsp_right_buf: right channel
 * - dsp_reverb_buf: Send to reverb unit
 * - dsp_chorus_buf: Send to chorus unit
 * - dsp_start: Start processing at this output buffer index
 * - dsp_end: End processing just before this output buffer index
 * - dsp_a1: Coefficient for the filter
 * - dsp_a2: same
 * - dsp_b0: same
 * - dsp_b1: same
 * - dsp_b2: same
 * - dsp_filter_flag: Set, the filter is needed (many sound fonts don't use
 *                    the filter at all. If it is left at its default setting
 *                    of roughly 20 kHz, there is no need to apply filterling.)
 * - dsp_interp_method: Which interpolation method to use.
 * - voice holds the voice structure
 *
 * Some variables are set and modified:
 * - dsp_phase: The position in the original waveform data.
 *              This has an integer and a fractional part (between samples).
 * - dsp_phase_incr: For each output sample, the position in the original
 *              waveform advances by dsp_phase_incr. This also has an integer
 *              part and a fractional part.
 *              If a sample is played at root pitch (no pitch change), 
 *              dsp_phase_incr is integer=1 and fractional=0.
 * - dsp_amp: The current amplitude envelope value.
 * - dsp_amp_incr: The changing rate of the amplitude envelope.
 *
 * A couple of variables are used internally, their results are discarded:
 * - dsp_i: Index through the output buffer
 * - dsp_phase_fractional: The fractional part of dsp_phase
 * - dsp_coeff: A table of four coefficients, depending on the fractional phase.
 *              Used to interpolate between samples.
 * - dsp_process_buffer: Holds the processed signal between stages
 * - dsp_centernode: delay line for the IIR filter
 * - dsp_hist1: same
 * - dsp_hist2: same
 * 
 */

/* Purpose: 
 * zap_almost_zero will return a number, as long as its
 * absolute value is over a certain threshold.  Otherwise 0.  See
 * fluid_rev.c for documentation (denormal numbers)
 */

# if defined(WITH_FLOAT)
# define zap_almost_zero(_sample) \
  ((((*(unsigned int*)&(_sample))&0x7f800000) < 0x08000000)? 0.0f : (_sample))
# else
/* 1e-20 was chosen as an arbitrary (small) threshold. */
#define zap_almost_zero(_sample) ((abs(_sample) < 1e-20)? 0.0f : (_sample))
#endif

/* Interpolation (find a value between two samples of the original waveform) */

if ((fluid_phase_fract(dsp_phase) == 0) 
    && (fluid_phase_fract(dsp_phase_incr) == 0) 
    && (fluid_phase_index(dsp_phase_incr) == 1)) {

	/* Check for a special case: The current phase falls directly on an
	 * original sample.  Also, the stepsize per output sample is exactly
	 * one sample, no fractional part.  In other words: The sample is
	 * played back at normal phase and root pitch.  => No interpolation
	 * needed.
	 */
	for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {  
		/* Mix to the buffer and advance the phase by one sample */
		dsp_buf[dsp_i] = dsp_amp * dsp_data[fluid_phase_index_plusplus(dsp_phase)];
		dsp_amp += dsp_amp_incr;
	}

} else {

	/* wave table interpolation: Choose the interpolation method */ 
    
	switch(dsp_interp_method){
	case FLUID_INTERP_NONE:
		/* No interpolation. Just take the sample, which is closest to
		 * the playback pointer.  Questionable quality, but very
		 * efficient. */
    
		for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {  
			dsp_phase_index = fluid_phase_index(dsp_phase); 
			dsp_buf[dsp_i] = dsp_amp * dsp_data[dsp_phase_index];
      
			/* increment phase and amplitude */ 
			fluid_phase_incr(dsp_phase, dsp_phase_incr); 
			dsp_amp += dsp_amp_incr;
		};
		break;

	case FLUID_INTERP_LINEAR:
		/* Straight line interpolation. */
		for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {  
			dsp_coeff = &interp_coeff_linear[fluid_phase_fract_to_tablerow(dsp_phase)];  
			dsp_phase_index = fluid_phase_index(dsp_phase); 
			dsp_buf[dsp_i] = (dsp_amp * 
					  (dsp_coeff->a0 * dsp_data[dsp_phase_index] 
					   + dsp_coeff->a1 * dsp_data[dsp_phase_index+1]));
      
			/* increment phase and amplitude */ 
			fluid_phase_incr(dsp_phase, dsp_phase_incr); 
			dsp_amp += dsp_amp_incr;
		};
		break;

	case FLUID_INTERP_4THORDER:
	default:
		/* Default interpolation loop using floats */
      
		for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {  
			dsp_coeff = &interp_coeff[fluid_phase_fract_to_tablerow(dsp_phase)];  
	
			dsp_phase_index = fluid_phase_index(dsp_phase); 
			dsp_buf[dsp_i] = (dsp_amp * 
					  (dsp_coeff->a0 * dsp_data[dsp_phase_index] 
					   + dsp_coeff->a1 * dsp_data[dsp_phase_index+1] 
					   + dsp_coeff->a2 * dsp_data[dsp_phase_index+2] 
					   + dsp_coeff->a3 * dsp_data[dsp_phase_index+3]));
	
			/* increment phase and amplitude */ 
			fluid_phase_incr(dsp_phase, dsp_phase_incr); 
			dsp_amp += dsp_amp_incr;
		}
		break;


	case FLUID_INTERP_7THORDER:
		for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {
			int fract = fluid_phase_fract_to_tablerow(dsp_phase);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_buf[dsp_i] = (dsp_amp * 
					  (sinc_table7[0][fract] * (fluid_real_t) dsp_data[dsp_phase_index] 
					   + sinc_table7[1][fract] * (fluid_real_t) dsp_data[dsp_phase_index+1]
					   + sinc_table7[2][fract] * (fluid_real_t) dsp_data[dsp_phase_index+2]
					   + sinc_table7[3][fract] * (fluid_real_t) dsp_data[dsp_phase_index+3]
					   + sinc_table7[4][fract] * (fluid_real_t) dsp_data[dsp_phase_index+4]
					   + sinc_table7[5][fract] * (fluid_real_t) dsp_data[dsp_phase_index+5]
					   + sinc_table7[6][fract] * (fluid_real_t) dsp_data[dsp_phase_index+6]));
      
			/* increment phase and amplitude */ 
			fluid_phase_incr(dsp_phase, dsp_phase_incr); 
			dsp_amp += dsp_amp_incr;
		}
		break;

	} /* switch interpolation method */
} /* If interpolation is needed */

/* filter (implement the voice filter according to Soundfont standard) */ 
if (dsp_use_filter_flag) {

	/* Check for denormal number (too close to zero) once in a
	 * while. This is not a big concern here - why would someone play a
	 * sample with an empty tail? */
	dsp_hist1 = zap_almost_zero(dsp_hist1);
  
	/* Two versions of the filter loop. One, while the filter is
	 * changing towards its new setting. The other, if the filter
	 * doesn't change.
	 */
  
	if (dsp_filter_coeff_incr_count > 0) {
		/* The increment is added to each filter coefficient
		   filter_coeff_incr_count times. */

		for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {
			/* The filter is implemented in Direct-II form. */ 
			dsp_centernode = dsp_buf[dsp_i] - dsp_a1 * dsp_hist1 - dsp_a2 * dsp_hist2;
			dsp_buf[dsp_i] = dsp_b02 * (dsp_centernode + dsp_hist2) + dsp_b1 * dsp_hist1;
			dsp_hist2 = dsp_hist1;  
			dsp_hist1 = dsp_centernode;  
      
			if (dsp_filter_coeff_incr_count-- > 0){
				dsp_a1 += dsp_a1_incr;
				dsp_a2 += dsp_a2_incr;
				dsp_b02 += dsp_b02_incr;
				dsp_b1 += dsp_b1_incr;
			}
		} /* for dsp_i */

	} else {

		/* The filter parameters are constant.  This is duplicated to save
		 * time. */
    
		for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {
			/* The filter is implemented in Direct-II form. */ 
			dsp_centernode = dsp_buf[dsp_i] - dsp_a1 * dsp_hist1 - dsp_a2 * dsp_hist2;
			dsp_buf[dsp_i] = dsp_b02 * (dsp_centernode + dsp_hist2) + dsp_b1 * dsp_hist1;
			dsp_hist2 = dsp_hist1;  
			dsp_hist1 = dsp_centernode;  
		}
	} /* if filter is fixed */
} /* if filter is enabled */



/* pan (Copy the signal to the left and right output buffer) The voice
 * panning generator has a range of -500 .. 500.  If it is centered,
 * it's close to 0.  voice->amp_left and voice->amp_right are then the
 * same, and we can save one multiplication per voice and sample.
   */
if ((-0.5 < voice->pan) && (voice->pan < 0.5)) {
    
	/* The voice is centered. Use voice->amp_left twice. */ 
	for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {
		float v = voice->amp_left * dsp_buf[dsp_i];
		dsp_left_buf[dsp_i] += v;
		dsp_right_buf[dsp_i] += v;
	}

} else {

	/* The voice is not centered. For stereo samples, one of the
	 * amplitudes will be zero. */
	if (voice->amp_left != 0.0){
		for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {
			dsp_left_buf[dsp_i] += voice->amp_left * dsp_buf[dsp_i];
		}
	}
	if (voice->amp_right != 0.0){
		for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {
			dsp_right_buf[dsp_i] += voice->amp_right * dsp_buf[dsp_i];
		}
	}
}

/* reverb send. Buffer may be NULL. */ 
if ((dsp_reverb_buf != NULL) && (voice->amp_reverb != 0.0)) {
	for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {
		dsp_reverb_buf[dsp_i] += voice->amp_reverb * dsp_buf[dsp_i];
	}
}

/* chorus send. Buffer may be NULL. */ 
if ((dsp_chorus_buf != NULL) && (voice->amp_chorus != 0)) {
	for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {
		dsp_chorus_buf[dsp_i] += voice->amp_chorus * dsp_buf[dsp_i];
	}
}
