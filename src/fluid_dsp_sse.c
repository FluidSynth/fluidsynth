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
	/* !!! SSE interpolation is less efficient that normal interpolation.  */

		
	/* Initialize amplitude increase */
	sse_b->sf[0] = sse_b->sf[1] = sse_b->sf[2] = sse_b->sf[3] = 4.*dsp_amp_incr;
	
	/* Initialize amplitude => xmm7
	 * The amplitude is kept in xmm7 throughout the whole process
	 */
	
	sse_a->sf[0]=sse_a->sf[1]=sse_a->sf[2]=sse_a->sf[3]=dsp_amp;
	sse_a->sf[1] += dsp_amp_incr;
	sse_a->sf[2] += 2.* dsp_amp_incr;
	sse_a->sf[3] += 3.* dsp_amp_incr;
	movaps_m2r(*sse_a,xmm7);
	
	/* Where to store the result */
	sse_dest=(sse_t*)&dsp_buf[0];
	for (dsp_i = 0; dsp_i < FLUID_BUFSIZE; dsp_i += 4) { 
		/* Note / fixme: The coefficients are first copied to
		 * sse_c, then to the xmm register.  Can't get it through
		 * the compiler differently... */
	
		/* Load the four source samples for the 1st output sample */
		dsp_phase_index = fluid_phase_index(dsp_phase); 
		sse_a->sf[0]=(fluid_real_t)dsp_data[dsp_phase_index];
		sse_a->sf[1]=(fluid_real_t)dsp_data[dsp_phase_index+1];
		sse_a->sf[2]=(fluid_real_t)dsp_data[dsp_phase_index+2];
		sse_a->sf[3]=(fluid_real_t)dsp_data[dsp_phase_index+3];
		movaps_m2r(*sse_a,xmm0);
		*sse_c=interp_coeff_sse[fluid_phase_fract_to_tablerow(dsp_phase)];
		mulps_m2r(*sse_c,xmm0);
	
		fluid_phase_incr(dsp_phase, dsp_phase_incr); 
	
		/* Load the four source samples for the 2nd output sample */
		dsp_phase_index = fluid_phase_index(dsp_phase); 
		sse_a->sf[0]=(fluid_real_t)dsp_data[dsp_phase_index];
		sse_a->sf[1]=(fluid_real_t)dsp_data[dsp_phase_index+1];
		sse_a->sf[2]=(fluid_real_t)dsp_data[dsp_phase_index+2];
		sse_a->sf[3]=(fluid_real_t)dsp_data[dsp_phase_index+3];
		movaps_m2r(*sse_a,xmm1);
		*sse_c=interp_coeff_sse[fluid_phase_fract_to_tablerow(dsp_phase)];
		mulps_m2r(*sse_c,xmm1);
	
		fluid_phase_incr(dsp_phase, dsp_phase_incr); 
	
		/* Load the four source samples for the 3rd output sample */
		dsp_phase_index = fluid_phase_index(dsp_phase); 
		sse_a->sf[0]=(fluid_real_t)dsp_data[dsp_phase_index];
		sse_a->sf[1]=(fluid_real_t)dsp_data[dsp_phase_index+1];
		sse_a->sf[2]=(fluid_real_t)dsp_data[dsp_phase_index+2];
		sse_a->sf[3]=(fluid_real_t)dsp_data[dsp_phase_index+3];
		movaps_m2r(*sse_a,xmm2);
		*sse_c=interp_coeff_sse[fluid_phase_fract_to_tablerow(dsp_phase)];
		mulps_m2r(*sse_c,xmm2);
	
		fluid_phase_incr(dsp_phase, dsp_phase_incr); 
	
		/* Load the four source samples for the 4th output sample */
		dsp_phase_index = fluid_phase_index(dsp_phase); 
		sse_a->sf[0]=(fluid_real_t)dsp_data[dsp_phase_index];
		sse_a->sf[1]=(fluid_real_t)dsp_data[dsp_phase_index+1];
		sse_a->sf[2]=(fluid_real_t)dsp_data[dsp_phase_index+2];
		sse_a->sf[3]=(fluid_real_t)dsp_data[dsp_phase_index+3];
		movaps_m2r(*sse_a,xmm3);
		*sse_c=interp_coeff_sse[fluid_phase_fract_to_tablerow(dsp_phase)];
		mulps_m2r(*sse_c,xmm3);
	
		fluid_phase_incr(dsp_phase, dsp_phase_incr); 
	
#if 0	    
		/*Testcase for horizontal add */
		sse_a->sf[0]=0.1;sse_a->sf[1]=0.01;sse_a->sf[2]=0.001;sse_a->sf[3]=0.0001;
		movaps_m2r(*sse_a,xmm0);
		sse_a->sf[0]=0.2;sse_a->sf[1]=0.02;sse_a->sf[2]=0.002;sse_a->sf[3]=0.0002;
		movaps_m2r(*sse_a,xmm1);
		sse_a->sf[0]=0.3;sse_a->sf[1]=0.03;sse_a->sf[2]=0.003;sse_a->sf[3]=0.0003;
		movaps_m2r(*sse_a,xmm2);
		sse_a->sf[0]=0.4;sse_a->sf[1]=0.04;sse_a->sf[2]=0.004;sse_a->sf[3]=0.0004;
		movaps_m2r(*sse_a,xmm3);
#endif /* #if 0 */
	
#if 1    
		/* Horizontal add 
		 * xmm4[0]:=xmm0[0]+xmm1[0]+xmm2[0]+xmm3[0]
		 * xmm4[1]:=xmm0[1]+xmm1[1]+xmm2[1]+xmm3[1]
		 * etc.
		 * The only register, which is unused, is xmm7.
		 */
		movaps_r2r(xmm0,xmm5);
		movaps_r2r(xmm2,xmm6);
		movlhps_r2r(xmm1,xmm5);
		movlhps_r2r(xmm3,xmm6);
		movhlps_r2r(xmm0,xmm1);
		movhlps_r2r(xmm2,xmm3);
		addps_r2r(xmm1,xmm5);
		addps_r2r(xmm3,xmm6);
		movaps_r2r(xmm5,xmm4);
		shufps_r2r(xmm6,xmm5,0xDD);
		shufps_r2r(xmm6,xmm4,0x88);
		addps_r2r(xmm5,xmm4);
	    
/* 	movaps_r2m(xmm4,*sse_a); */
/* 	printf("xmm4 (Result): %f %f %f %f\n",  */
/* 	       sse_a->sf[0], sse_a->sf[1],  */
/* 	       sse_a->sf[2], sse_a->sf[3]); */
#else
		/* Add using normal FPU */
		movaps_r2m(xmm0,*sse_a);
		sse_c->sf[0]=sse_a->sf[0]+sse_a->sf[1]+sse_a->sf[2]+sse_a->sf[3];
		movaps_r2m(xmm1,*sse_a);
		sse_c->sf[1]=sse_a->sf[0]+sse_a->sf[1]+sse_a->sf[2]+sse_a->sf[3];
		movaps_r2m(xmm2,*sse_a);
		sse_c->sf[2]=sse_a->sf[0]+sse_a->sf[1]+sse_a->sf[2]+sse_a->sf[3];
		movaps_r2m(xmm3,*sse_a);
		sse_c->sf[3]=sse_a->sf[0]+sse_a->sf[1]+sse_a->sf[2]+sse_a->sf[3];
		movaps_m2r(*sse_c,xmm4);
#endif /* #if 1 */
		/* end horizontal add. Result in xmm6. */

	
		/* Multiply xmm4 with amplitude */
		mulps_r2r(xmm7,xmm4);
	
		/* Store the result */
		movaps_r2m(xmm4,*sse_dest); // ++
	
		/* Advance the position in the output buffer */
		sse_dest++;
	
		/* Change the amplitude */
		addps_m2r(*sse_b,xmm7);
	
	} /* for dsp_i in steps of four */    
      
	movaps_r2m(xmm7,*sse_a);
      
	/* Retrieve the last amplitude value. */
	dsp_amp=sse_a->sf[3];
      

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


/* The following optimization will process a whole buffer using the
 * SSE extension of the Pentium processor.
 */

if (voice->amp_left != 0.0) {
	sse_a->sf[0]=voice->amp_left;
	sse_a->sf[1]=voice->amp_left;
	sse_a->sf[2]=voice->amp_left;
	sse_a->sf[3]=voice->amp_left;
	movaps_m2r(*sse_a,xmm0);
	
	sse_src=(sse_t*)dsp_buf;
	sse_dest=(sse_t*)&dsp_left_buf[0];
	for (dsp_i = 0; dsp_i < FLUID_BUFSIZE; dsp_i+=4) { 
		movaps_m2r(*sse_src,xmm4);	  /* Load original sample */
		mulps_r2r(xmm0,xmm4);             /* Gain */
		sse_src++;
		addps_m2r(*sse_dest,xmm4);   /* Mix with buf */
		movaps_r2m(xmm4,*sse_dest);  /* Store in buf */
		sse_dest++;
	}
}

if (voice->amp_right != 0.0){
	sse_a->sf[0]=voice->amp_right;
	sse_a->sf[1]=voice->amp_right;
	sse_a->sf[2]=voice->amp_right;
	sse_a->sf[3]=voice->amp_right;
	movaps_m2r(*sse_a,xmm0);
      
	sse_src=(sse_t*)dsp_buf;
	sse_dest=(sse_t*)&dsp_right_buf[0];
	for (dsp_i = 0; dsp_i < FLUID_BUFSIZE; dsp_i+=4) { 
		movaps_m2r(*sse_src,xmm4);	  /* Load original sample */
		sse_src++;
		mulps_r2r(xmm0,xmm4);             /* Gain */
		addps_m2r(*sse_dest,xmm4);   /* Mix with buf */
		movaps_r2m(xmm4,*sse_dest);  /* Store in buf */
		sse_dest++;
	}
}
  
/* reverb send. Buffer may be NULL. */ 
if (dsp_reverb_buf && voice->amp_reverb != 0.0){
	sse_a->sf[0]=voice->amp_reverb;
	sse_a->sf[1]=voice->amp_reverb;
	sse_a->sf[2]=voice->amp_reverb;
	sse_a->sf[3]=voice->amp_reverb;
	movaps_m2r(*sse_a,xmm0);
    
	sse_src=(sse_t*)dsp_buf;
	sse_dest=(sse_t*)&dsp_reverb_buf[0];
	for (dsp_i = 0; dsp_i < FLUID_BUFSIZE; dsp_i+=4) { 
		movaps_m2r(*sse_src,xmm4);	  /* Load original sample */
		sse_src++;
		mulps_r2r(xmm0,xmm4);             /* Gain */
		addps_m2r(*sse_dest,xmm4);   /* Mix with buf */
		movaps_r2m(xmm4,*sse_dest);  /* Store in buf */
		sse_dest++;
	}
}
  
/* chorus send. Buffer may be NULL. */ 
if (dsp_chorus_buf && voice->amp_chorus != 0){
	sse_a->sf[0]=voice->amp_chorus;
	sse_a->sf[1]=voice->amp_chorus;
	sse_a->sf[2]=voice->amp_chorus;
	sse_a->sf[3]=voice->amp_chorus;
	movaps_m2r(*sse_a,xmm0);
    
	sse_src=(sse_t*)dsp_buf;
	sse_dest=(sse_t*)&dsp_chorus_buf[0];
	for (dsp_i = 0; dsp_i < FLUID_BUFSIZE; dsp_i+=4) { 
		movaps_m2r(*sse_src,xmm4);	  /* Load original sample */
		sse_src++;
		mulps_r2r(xmm0,xmm4);             /* Gain */
		addps_m2r(*sse_dest,xmm4);   /* Mix with buf */
		movaps_r2m(xmm4,*sse_dest);  /* Store in buf */
		sse_dest++;
	}
}
