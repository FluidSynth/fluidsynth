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

#include "fluid_rvoice_mixer.h"
#include "fluid_rvoice.h"
#include "fluid_sys.h"
#include "fluid_rev.h"
#include "fluid_chorus.h"

#define SYNTH_REVERB_CHANNEL 0
#define SYNTH_CHORUS_CHANNEL 1

typedef struct _fluid_mixer_buffers_t fluid_mixer_buffers_t;

struct _fluid_mixer_buffers_t {
  fluid_rvoice_mixer_t* mixer; /**< Owner of object */
#ifdef ENABLE_MIXER_THREADS
  fluid_thread_t* thread;     /**< Thread object */
#endif

  fluid_rvoice_t** finished_voices; /* List of voices who have finished */
  int finished_voice_count;

  int ready;             /**< Atomic: buffers are ready for mixing */

  int buf_blocks;             /**< Number of blocks allocated in the buffers */

  int buf_count;
  fluid_real_t** left_buf;
  fluid_real_t** right_buf;

  int fx_buf_count;
  fluid_real_t** fx_left_buf;
  fluid_real_t** fx_right_buf;
};

typedef struct _fluid_mixer_fx_t fluid_mixer_fx_t;

struct _fluid_mixer_fx_t {
  fluid_revmodel_t* reverb; /**< Reverb unit */
  fluid_chorus_t* chorus; /**< Chorus unit */
  int with_reverb;        /**< Should the synth use the built-in reverb unit? */
  int with_chorus;        /**< Should the synth use the built-in chorus unit? */
  int mix_fx_to_out;      /**< Should the effects be mixed in with the primary output? */
};

struct _fluid_rvoice_mixer_t {
  fluid_mixer_fx_t fx;

  fluid_mixer_buffers_t buffers; /**< Used by mixer only: own buffers */
  void (*remove_voice_callback)(void*, fluid_rvoice_t*); /**< Used by mixer only: Receive this callback every time a voice is removed */
  void* remove_voice_callback_userdata;

  fluid_rvoice_t** rvoices; /**< Read-only: Voices array, sorted so that all nulls are last */
  int polyphony; /**< Read-only: Length of voices array */
  int active_voices; /**< Read-only: Number of non-null voices */
  int current_blockcount;      /**< Read-only: how many blocks to process this time */

#ifdef ENABLE_MIXER_THREADS
//  int sleeping_threads;        /**< Atomic: number of threads currently asleep */
//  int active_threads;          /**< Atomic: number of threads in the thread loop */
  int threads_should_terminate; /**< Atomic: Set to TRUE when threads should terminate */
  int current_rvoice;           /**< Atomic: for the threads to know next voice to  */
  fluid_cond_t* wakeup_threads; /**< Signalled when the threads should wake up */
  fluid_cond_mutex_t* wakeup_threads_m; /**< wakeup_threads mutex companion */
  fluid_cond_t* thread_ready; /**< Signalled from thread, when the thread has a buffer ready for mixing */
  fluid_cond_mutex_t* thread_ready_m; /**< thread_ready mutex companion */

  int thread_count;            /**< Number of extra mixer threads for multi-core rendering */
  fluid_mixer_buffers_t* threads;    /**< Array of mixer threads (thread_count in length) */
#endif
};

static FLUID_INLINE void 
fluid_rvoice_mixer_process_fx(fluid_rvoice_mixer_t* mixer)
{
  int i;
  if (mixer->fx.with_reverb) {
    if (mixer->fx.mix_fx_to_out) {
      for (i=0; i < mixer->current_blockcount * FLUID_BUFSIZE; i += FLUID_BUFSIZE)
        fluid_revmodel_processmix(mixer->fx.reverb, 
                                  &mixer->buffers.fx_left_buf[SYNTH_REVERB_CHANNEL][i],
				  &mixer->buffers.left_buf[0][i],
				  &mixer->buffers.right_buf[0][i]);
    } 
    else {
      for (i=0; i < mixer->current_blockcount * FLUID_BUFSIZE; i += FLUID_BUFSIZE)
        fluid_revmodel_processreplace(mixer->fx.reverb, 
                                  &mixer->buffers.fx_left_buf[SYNTH_REVERB_CHANNEL][i],
				  &mixer->buffers.fx_left_buf[SYNTH_REVERB_CHANNEL][i],
				  &mixer->buffers.fx_right_buf[SYNTH_REVERB_CHANNEL][i]);
    }
  }

  if (mixer->fx.with_chorus) {
    if (mixer->fx.mix_fx_to_out) {
      for (i=0; i < mixer->current_blockcount * FLUID_BUFSIZE; i += FLUID_BUFSIZE)
        fluid_chorus_processmix(mixer->fx.chorus, 
                                &mixer->buffers.fx_left_buf[SYNTH_CHORUS_CHANNEL][i],
			        &mixer->buffers.left_buf[0][i],
				&mixer->buffers.right_buf[0][i]);
    } 
    else {
      for (i=0; i < mixer->current_blockcount * FLUID_BUFSIZE; i += FLUID_BUFSIZE)
        fluid_chorus_processreplace(mixer->fx.chorus, 
                                &mixer->buffers.fx_left_buf[SYNTH_CHORUS_CHANNEL][i],
				&mixer->buffers.fx_left_buf[SYNTH_CHORUS_CHANNEL][i],
				&mixer->buffers.fx_right_buf[SYNTH_CHORUS_CHANNEL][i]);
    }
  }

}

/**
 * During rendering, rvoices might be finished. Set this callback
 * for getting a callback any time the rvoice is finished.
 */
void fluid_rvoice_mixer_set_finished_voices_callback(
  fluid_rvoice_mixer_t* mixer,
  void (*func)(void*, fluid_rvoice_t*),
  void* userdata)
{
  mixer->remove_voice_callback_userdata = userdata;
  mixer->remove_voice_callback = func;
}



/**
 * Synthesize one voice and add to buffer.
 * NOTE: If return value is less than blockcount*FLUID_BUFSIZE, that means 
 * voice has been finished, removed and possibly replaced with another voice.
 * @return Number of samples written 
 */
static int
fluid_mix_one(fluid_rvoice_t* rvoice, fluid_real_t** bufs, unsigned int bufcount, int blockcount)
{
  int i, result = 0;

  fluid_real_t local_buf[FLUID_BUFSIZE*blockcount];

  for (i=0; i < blockcount; i++) {
    int s = fluid_rvoice_write(rvoice, &local_buf[FLUID_BUFSIZE*i]);
    if (s == -1) {
      s = FLUID_BUFSIZE; /* Voice is quiet, TODO: optimize away memset/mix */
      FLUID_MEMSET(&local_buf[FLUID_BUFSIZE*i], 0, FLUID_BUFSIZE*sizeof(fluid_real_t*));
    } 
    result += s;
    if (s < FLUID_BUFSIZE) {
      break;
    }
  }
  fluid_rvoice_buffers_mix(&rvoice->buffers, local_buf, result, bufs, bufcount);

  return result;
}

/**
 * Glue to get fluid_rvoice_buffers_mix what it wants
 * Note: Make sure outbufs has buf_count + fx_buf_count elements before calling
 */
static FLUID_INLINE int 
fluid_mixer_buffers_prepare(fluid_mixer_buffers_t* buffers, fluid_real_t** outbufs)
{
  fluid_real_t* reverb_buf, *chorus_buf;
  int i;

  /* Set up the reverb / chorus buffers only, when the effect is
   * enabled on synth level.  Nonexisting buffers are detected in the
   * DSP loop. Not sending the reverb / chorus signal saves some time
   * in that case. */
  reverb_buf = buffers->mixer->fx.with_reverb ? buffers->fx_left_buf[SYNTH_REVERB_CHANNEL] : NULL;
  chorus_buf = buffers->mixer->fx.with_chorus ? buffers->fx_left_buf[SYNTH_CHORUS_CHANNEL] : NULL;
  outbufs[buffers->buf_count*2 + SYNTH_REVERB_CHANNEL] = reverb_buf;
  outbufs[buffers->buf_count*2 + SYNTH_CHORUS_CHANNEL] = chorus_buf;

      /* The output associated with a MIDI channel is wrapped around
       * using the number of audio groups as modulo divider.  This is
       * typically the number of output channels on the 'sound card',
       * as long as the LADSPA Fx unit is not used. In case of LADSPA
       * unit, think of it as subgroups on a mixer.
       *
       * For example: Assume that the number of groups is set to 2.
       * Then MIDI channel 1, 3, 5, 7 etc. go to output 1, channels 2,
       * 4, 6, 8 etc to output 2.  Or assume 3 groups: Then MIDI
       * channels 1, 4, 7, 10 etc go to output 1; 2, 5, 8, 11 etc to
       * output 2, 3, 6, 9, 12 etc to output 3.
       */

  for (i = 0; i < buffers->buf_count; i++) {
    outbufs[i*2] = buffers->left_buf[i];
    outbufs[i*2+1] = buffers->right_buf[i];
  }
  return buffers->buf_count*2 + 2;
}


static FLUID_INLINE void
fluid_finish_rvoice(fluid_mixer_buffers_t* buffers, fluid_rvoice_t* rvoice)
{
  if (buffers->finished_voice_count < buffers->mixer->polyphony)
    buffers->finished_voices[buffers->finished_voice_count++] = rvoice;
  else
    FLUID_LOG(FLUID_ERR, "Exceeded finished voices array, try increasing polyphony");
}

static void   
fluid_mixer_buffer_process_finished_voices(fluid_mixer_buffers_t* buffers)
{
  int i,j;
  for (i=0; i < buffers->finished_voice_count; i++) {
    fluid_rvoice_t* v = buffers->finished_voices[i];
    int* av = &buffers->mixer->active_voices; 
    for (j=0; j < *av; j++) {
      if (v == buffers->mixer->rvoices[j]) {
        (*av)--;
        /* Pack the array */
        if (j < *av) 
          buffers->mixer->rvoices[j] = buffers->mixer->rvoices[*av];
      }
    }
    if (buffers->mixer->remove_voice_callback)
      buffers->mixer->remove_voice_callback(
        buffers->mixer->remove_voice_callback_userdata, v);
  }
  buffers->finished_voice_count = 0;
}

static FLUID_INLINE void fluid_rvoice_mixer_process_finished_voices(fluid_rvoice_mixer_t* mixer)
{
#ifdef ENABLE_MIXER_THREADS  
  int i;
  for (i=0; i < thread_count; i++)
    fluid_mixer_buffer_process_finished_voices(&mixer->threads[i]);
#endif
  fluid_mixer_buffer_process_finished_voices(&mixer->buffers);
}

int 
fluid_rvoice_mixer_add_voice(fluid_rvoice_mixer_t* mixer, fluid_rvoice_t* voice)
{
  if (mixer->active_voices >= mixer->polyphony) {
    FLUID_LOG(FLUID_WARN, "Trying to exceed polyphony in fluid_rvoice_mixer_add_voice");
    return FLUID_FAILED;
  }
  mixer->rvoices[mixer->active_voices++] = voice;
  return FLUID_OK;
}

/**
 * Update polyphony - max number of voices (NOTE: not hard real-time capable)
 * @return FLUID_OK or FLUID_FAILED
 */
int 
fluid_rvoice_mixer_set_polyphony(fluid_rvoice_mixer_t* handler, int value)
{
  void* newptr;
  if (handler->active_voices > value) 
    return FLUID_FAILED;
  if (handler->buffers.finished_voice_count > value) 
    return FLUID_FAILED;

  newptr = FLUID_REALLOC(handler->rvoices, value * sizeof(fluid_rvoice_t*));
  if (newptr == NULL) 
    return FLUID_FAILED;
  handler->rvoices = newptr;

  newptr = FLUID_REALLOC(handler->buffers.finished_voices, value * sizeof(fluid_rvoice_t*));
  if (newptr == NULL) 
    return FLUID_FAILED;
  handler->buffers.finished_voices = newptr;

  handler->polyphony = value;
  return FLUID_OK;
}


static FLUID_INLINE void fluid_render_loop_singlethread(fluid_rvoice_mixer_t* mixer)
{
  int i;
  int scount = mixer->current_blockcount * FLUID_BUFSIZE;
  fluid_real_t* bufs[mixer->buffers.buf_count + mixer->buffers.fx_buf_count];
  int bufcount = fluid_mixer_buffers_prepare(&mixer->buffers, bufs);
  for (i=0; i < mixer->active_voices; i++) {
    int s = fluid_mix_one(mixer->rvoices[i], bufs, bufcount, mixer->current_blockcount);
    if (s < scount) {
      fluid_finish_rvoice(&mixer->buffers, mixer->rvoices[i]);
    }
  }
}


static FLUID_INLINE void
fluid_mixer_buffers_zero(fluid_mixer_buffers_t* buffers)
{
  int i;
  int size = buffers->mixer->current_blockcount * FLUID_BUFSIZE * sizeof(fluid_real_t);
  /* TODO: Optimize by only zero out the buffers we actually use later on. */
  for (i=0; i < buffers->buf_count; i++) {
    FLUID_MEMSET(buffers->left_buf[i], 0, size);
    FLUID_MEMSET(buffers->right_buf[i], 0, size);
  }
  for (i=0; i < buffers->fx_buf_count; i++) {
    FLUID_MEMSET(buffers->fx_left_buf[i], 0, size);
    FLUID_MEMSET(buffers->fx_right_buf[i], 0, size);
  }
}


/**
 * Synthesize audio into buffers
 * @param blockcount number of blocks to render, each having FLUID_BUFSIZE samples 
 * @return number of blocks rendered
 */
int 
fluid_rvoice_mixer_render(fluid_rvoice_mixer_t* mixer, int blockcount)
{
  mixer->current_blockcount = blockcount > mixer->buffers.buf_blocks ? 
      mixer->buffers.buf_blocks : blockcount;

  // Zero buffers
  fluid_mixer_buffers_zero(&mixer->buffers);
  
  // If no additional threads, just render all voices in a simple loop
#ifdef ENABLE_MIXER_THREADS
  if (thread_count > 0)
    fluid_render_loop_multithread(mixer);
  else
#endif
    fluid_render_loop_singlethread(mixer);
    
  // If additional threads: Prepare voice list
  // Signal threads to wake up
  // while threads are running: 
  // If thread is finished, mix it in
  // Otherwise get a voice and render it
  // If no voices, wait for mixes

  // Process reverb & chorus
  fluid_rvoice_mixer_process_fx(mixer);

  // Call the callback and pack active voice array
  fluid_rvoice_mixer_process_finished_voices(mixer);

  return mixer->current_blockcount;
}

static int 
fluid_mixer_buffers_init(fluid_mixer_buffers_t* buffers, fluid_rvoice_mixer_t* mixer)
{
  int i, samplecount;
  
  buffers->mixer = mixer;
  buffers->buf_count = buffers->mixer->buffers.buf_count;
  buffers->fx_buf_count = buffers->mixer->buffers.fx_buf_count;
  buffers->buf_blocks = buffers->mixer->buffers.buf_blocks;
  samplecount = FLUID_BUFSIZE * buffers->buf_count;
  
 
  /* Left and right audio buffers */

  buffers->left_buf = FLUID_ARRAY(fluid_real_t*, buffers->buf_count);
  buffers->right_buf = FLUID_ARRAY(fluid_real_t*, buffers->buf_count);

  if ((buffers->left_buf == NULL) || (buffers->right_buf == NULL)) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }

  FLUID_MEMSET(buffers->left_buf, 0, buffers->buf_count * sizeof(fluid_real_t*));
  FLUID_MEMSET(buffers->right_buf, 0, buffers->buf_count * sizeof(fluid_real_t*));

  for (i = 0; i < buffers->buf_count; i++) {

    buffers->left_buf[i] = FLUID_ARRAY(fluid_real_t, samplecount);
    buffers->right_buf[i] = FLUID_ARRAY(fluid_real_t, samplecount);

    if ((buffers->left_buf[i] == NULL) || (buffers->right_buf[i] == NULL)) {
      FLUID_LOG(FLUID_ERR, "Out of memory");
      return 0;
    }
  }

  /* Effects audio buffers */

  buffers->fx_left_buf = FLUID_ARRAY(fluid_real_t*, buffers->fx_buf_count);
  buffers->fx_right_buf = FLUID_ARRAY(fluid_real_t*, buffers->fx_buf_count);

  if ((buffers->fx_left_buf == NULL) || (buffers->fx_right_buf == NULL)) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }

  FLUID_MEMSET(buffers->fx_left_buf, 0, buffers->fx_buf_count * sizeof(fluid_real_t*));
  FLUID_MEMSET(buffers->fx_right_buf, 0, buffers->fx_buf_count * sizeof(fluid_real_t*));

  for (i = 0; i < buffers->fx_buf_count; i++) {
    buffers->fx_left_buf[i] = FLUID_ARRAY(fluid_real_t, samplecount);
    buffers->fx_right_buf[i] = FLUID_ARRAY(fluid_real_t, samplecount);

    if ((buffers->fx_left_buf[i] == NULL) || (buffers->fx_right_buf[i] == NULL)) {
      FLUID_LOG(FLUID_ERR, "Out of memory");
      return 0;
    }
  }
  return 1;
}

/**
 * Note: Not hard real-time capable (calls malloc)
 */
void 
fluid_rvoice_mixer_set_samplerate(fluid_rvoice_mixer_t* mixer, fluid_real_t samplerate)
{
  int i;
  if (mixer->fx.chorus)
    delete_fluid_chorus(mixer->fx.chorus);
  mixer->fx.chorus = new_fluid_chorus(samplerate);
  for (i=0; i < mixer->active_voices; i++)
    fluid_rvoice_set_output_rate(mixer->rvoices[i], samplerate);
}


/**
 * @param buf_count number of primary stereo buffers
 * @param fx_buf_count number of stereo effect buffers
 */
fluid_rvoice_mixer_t* 
new_fluid_rvoice_mixer(int buf_count, int fx_buf_count)
{
  fluid_rvoice_mixer_t* mixer = FLUID_NEW(fluid_rvoice_mixer_t);
  if (mixer == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  FLUID_MEMSET(mixer, 0, sizeof(fluid_rvoice_mixer_t));
  mixer->buffers.buf_count = buf_count;
  mixer->buffers.fx_buf_count = fx_buf_count;
  mixer->buffers.buf_blocks = FLUID_MIXER_MAX_BUFFERS_DEFAULT;
  
  /* allocate the reverb module */
  mixer->fx.reverb = new_fluid_revmodel();
  mixer->fx.chorus = new_fluid_chorus(44100); /* FIXME: Hardcoded sample rate */
  if (mixer->fx.reverb == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    delete_fluid_rvoice_mixer(mixer);
    return NULL;
  }

  
  if (!fluid_mixer_buffers_init(&mixer->buffers, mixer)) {
    delete_fluid_rvoice_mixer(mixer);
    return NULL;
  }
  return mixer;
}

static void
fluid_mixer_buffers_free(fluid_mixer_buffers_t* buffers)
{
  int i;
  
  /* free all the sample buffers */
  if (buffers->left_buf != NULL) {
    for (i = 0; i < buffers->buf_count; i++) {
      if (buffers->left_buf[i] != NULL) {
	FLUID_FREE(buffers->left_buf[i]);
      }
    }
    FLUID_FREE(buffers->left_buf);
  }

  if (buffers->right_buf != NULL) {
    for (i = 0; i < buffers->buf_count; i++) {
      if (buffers->right_buf[i] != NULL) {
	FLUID_FREE(buffers->right_buf[i]);
      }
    }
    FLUID_FREE(buffers->right_buf);
  }

  if (buffers->fx_left_buf != NULL) {
    for (i = 0; i < buffers->fx_buf_count; i++) {
      if (buffers->fx_left_buf[i] != NULL) {
	FLUID_FREE(buffers->fx_left_buf[i]);
      }
    }
    FLUID_FREE(buffers->fx_left_buf);
  }

  if (buffers->fx_right_buf != NULL) {
    for (i = 0; i < buffers->fx_buf_count; i++) {
      if (buffers->fx_right_buf[i] != NULL) {
	FLUID_FREE(buffers->fx_right_buf[i]);
      }
    }
    FLUID_FREE(buffers->fx_right_buf);
  }  
}

void delete_fluid_rvoice_mixer(fluid_rvoice_mixer_t* mixer)
{
  if (!mixer) 
    return;
  fluid_mixer_buffers_free(&mixer->buffers);
  if (mixer->fx.reverb)
    delete_fluid_revmodel(mixer->fx.reverb);
  if (mixer->fx.chorus)
    delete_fluid_chorus(mixer->fx.chorus);
  FLUID_FREE(mixer);
}


void fluid_rvoice_mixer_set_reverb_enabled(fluid_rvoice_mixer_t* mixer, int on)
{
  mixer->fx.with_reverb = on;
}

void fluid_rvoice_mixer_set_chorus_enabled(fluid_rvoice_mixer_t* mixer, int on)
{
  mixer->fx.with_chorus = on;
}

void fluid_rvoice_mixer_set_mix_fx(fluid_rvoice_mixer_t* mixer, int on)
{
  mixer->fx.mix_fx_to_out = on;
}

void fluid_rvoice_mixer_set_chorus_params(fluid_rvoice_mixer_t* mixer, int set, 
				         int nr, double level, double speed, 
				         double depth_ms, int type)
{
  fluid_chorus_set(mixer->fx.chorus, set, nr, level, speed, depth_ms, type);
}
void fluid_rvoice_mixer_set_reverb_params(fluid_rvoice_mixer_t* mixer, int set, 
					 double roomsize, double damping, 
					 double width, double level)
{
  fluid_revmodel_set(mixer->fx.reverb, set, roomsize, damping, width, level); 
}

void fluid_rvoice_mixer_reset_fx(fluid_rvoice_mixer_t* mixer)
{
  fluid_revmodel_reset(mixer->fx.reverb);
  fluid_chorus_reset(mixer->fx.chorus);
}

int fluid_rvoice_mixer_get_bufs(fluid_rvoice_mixer_t* mixer, 
				  fluid_real_t*** left, fluid_real_t*** right)
{
  *left = mixer->buffers.left_buf;
  *right = mixer->buffers.right_buf;
  return mixer->buffers.buf_count;
}


#ifdef ENABLE_MIXER_THREADS
/* Core thread function (processes voices in parallel to primary synthesis thread) */
static void
fluid_mixer_thread_func (void* data)
{
  fluid_mixer_buffers_t* buffers = data;  
  
  // get a voice
  // if no voices: signal rendered buffers, sleep
  // else: if buffer is not zeroed, zero buffers
  // then render voice to buffers

}
#endif
