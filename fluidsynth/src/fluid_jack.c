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


/* fluid_jack.c
 *
 * Driver for the JACK
 *
 * This code is derived from the simple_client example in the JACK
 * source distribution. Many thanks to Paul Davis.
 *
 */

#include "fluidsynth_priv.h"
#include "fluid_sys.h"
#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#include <jack/jack.h>

#include "config.h"
#include "fluid_lash.h"


/**************************************************************
 *
 *        JACK audio driver
 *
 */

/*
 * fluid_jack_audio_driver_t
 *
 */
typedef struct {
  fluid_audio_driver_t driver;
  fluid_synth_t* synth;
  jack_client_t *client;
  int audio_channels;
  int effects_channels;
  jack_port_t **output_ports;
  int num_output_ports;
  float **output_bufs;
  jack_port_t **fx_ports;
  int num_fx_ports;
  float **fx_bufs;
  jack_port_t **input_ports;
  int num_input_ports;
  float **input_bufs;
  fluid_audio_func_t callback;
  void* data;
} fluid_jack_audio_driver_t;


int delete_fluid_jack_audio_driver(fluid_audio_driver_t* p);
void fluid_jack_audio_driver_shutdown(void *arg);
int fluid_jack_audio_driver_srate(jack_nframes_t nframes, void *arg);
int fluid_jack_audio_driver_bufsize(jack_nframes_t nframes, void *arg);
int fluid_jack_audio_driver_process(jack_nframes_t nframes, void *arg);
int fluid_jack_audio_driver_process2(jack_nframes_t nframes, void *arg);

void
fluid_jack_audio_driver_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "audio.jack.id", "fluidsynth", 0, NULL, NULL);
  fluid_settings_register_str(settings, "audio.jack.multi", "no", 0, NULL, NULL);
  fluid_settings_register_int(settings, "audio.jack.autoconnect", 0, 0, 1, FLUID_HINT_TOGGLED, NULL, NULL);
}


/*
 * new_fluid_alsa_audio_driver
 */
fluid_audio_driver_t* 
new_fluid_jack_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth)
{
  fluid_jack_audio_driver_t* dev = NULL;
  char name[64];
  int i, audio_count, fx_count;
  /* for looking up ports */
  const char ** jack_ports;
  char* client_name;
  int autoconnect = 0;
  int jack_srate;
  double sample_rate;
  
  dev = FLUID_NEW(fluid_jack_audio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;    
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_jack_audio_driver_t));

  dev->synth = synth;

  /* try to become a client of the JACK server */

  if (fluid_settings_getstr(settings, "audio.jack.id", &client_name)
      && (client_name != NULL) 
      && (strlen(client_name) > 0)) {
    snprintf(name, 64, "%s", client_name);
  } else {
    snprintf(name, 64, "fluidsynth");
  }

  if ((dev->client = jack_client_new(name)) == 0) {
    FLUID_LOG(FLUID_ERR, "Jack server not running?");
    goto error_recovery;
  }

  /* set callbacks */
  jack_set_process_callback(dev->client, fluid_jack_audio_driver_process, (void*) dev);
  jack_set_buffer_size_callback(dev->client, fluid_jack_audio_driver_bufsize, (void*) dev);
  jack_set_sample_rate_callback(dev->client, fluid_jack_audio_driver_srate, (void*) dev);
  jack_on_shutdown(dev->client, fluid_jack_audio_driver_shutdown, (void*) dev);

  /* display the current sample rate. once the client is activated 
     (see below), you should rely on your own sample rate
     callback (see above) for this value.
  */
  jack_srate = jack_get_sample_rate(dev->client);
  FLUID_LOG(FLUID_DBG, "Jack engine sample rate: %lu", jack_srate);

  fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);

  if ((int)sample_rate != jack_srate) {
    /* There's currently no way to change the sampling rate of the
       synthesizer after it's been created. */
    FLUID_LOG(FLUID_WARN, "Jack sample rate mismatch, expect tuning issues"
	      " (synth.sample-rate=%lu, jackd=%lu)", (int)sample_rate, jack_srate);
  }

  if (!fluid_settings_str_equal(settings, "audio.jack.multi", "yes")) {

    /* create the two audio output ports */
    dev->num_output_ports = 2;

    dev->output_ports = FLUID_ARRAY(jack_port_t*, dev->num_output_ports);
    if (dev->output_ports == NULL) {
      FLUID_LOG(FLUID_PANIC, "Jack server not running?");
      goto error_recovery;
    }

    FLUID_MEMSET(dev->output_ports, 0, dev->num_output_ports * sizeof(jack_port_t*));

    dev->output_ports[0] = jack_port_register(dev->client, "left", 
					     JACK_DEFAULT_AUDIO_TYPE, 
					     JackPortIsOutput, 0);
    
    dev->output_ports[1] = jack_port_register(dev->client, "right", 
					     JACK_DEFAULT_AUDIO_TYPE, 
					     JackPortIsOutput, 0);
  } else {

    dev->num_output_ports = fluid_synth_count_audio_channels(synth);

    dev->output_ports = FLUID_ARRAY(jack_port_t*, 2 * dev->num_output_ports);
    if (dev->output_ports == NULL) {
      FLUID_LOG(FLUID_PANIC, "Out of memory");
      goto error_recovery;
    }

    dev->output_bufs = FLUID_ARRAY(float*, 2 * dev->num_output_ports);
    if (dev->output_bufs == NULL) {
      FLUID_LOG(FLUID_PANIC, "Out of memory");
      goto error_recovery;
    }

    FLUID_MEMSET(dev->output_ports, 0, 2 * dev->num_output_ports * sizeof(jack_port_t*));

    for (i = 0; i < dev->num_output_ports; i++) {
      sprintf(name, "l_%02d", i);
      dev->output_ports[2 * i] = jack_port_register(dev->client, name, 
						   JACK_DEFAULT_AUDIO_TYPE, 
						   JackPortIsOutput, 0);

      sprintf(name, "r_%02d", i);
      dev->output_ports[2 * i + 1] = jack_port_register(dev->client, name, 
						       JACK_DEFAULT_AUDIO_TYPE, 
						       JackPortIsOutput, 0);
    }

    dev->num_fx_ports = fluid_synth_count_effects_channels(synth);

    dev->fx_ports = FLUID_ARRAY(jack_port_t*, 2 * dev->num_fx_ports);
    if (dev->fx_ports == NULL) {
      FLUID_LOG(FLUID_PANIC, "Out of memory");
      goto error_recovery;
    }

    dev->fx_bufs = FLUID_ARRAY(float*, 2 * dev->num_fx_ports);
    if (dev->fx_bufs == NULL) {
      FLUID_LOG(FLUID_PANIC, "Out of memory");
      goto error_recovery;
    }

    FLUID_MEMSET(dev->fx_ports, 0, 2 * dev->num_fx_ports * sizeof(jack_port_t*));

    for (i = 0; i < dev->num_fx_ports; i++) {
      sprintf(name, "fxl_%02d", i);
      dev->fx_ports[2 * i] = jack_port_register(dev->client, name, 
						JACK_DEFAULT_AUDIO_TYPE, 
						JackPortIsOutput, 0);

      sprintf(name, "fxr_%02d", i);
      dev->fx_ports[2 * i + 1] = jack_port_register(dev->client, name, 
						    JACK_DEFAULT_AUDIO_TYPE, 
						    JackPortIsOutput, 0);
    }
  }

  /* tell the JACK server that we are ready to roll */
  if (jack_activate(dev->client)) {
    FLUID_LOG(FLUID_ERR, "Cannot activate the fluidsynth as a JACK client");
    goto error_recovery;
  }

  /* tell the lash server our client name */
#ifdef LASH_ENABLED
  {
    int enable_lash = 0;
    fluid_settings_getint (settings, "lash.enable", &enable_lash);
    if (enable_lash)
      fluid_lash_jack_client_name (fluid_lash_client, name);
  }
#endif /* LASH_ENABLED */


  /* connect the ports. */


  /* FIXME: should be done by a patchbay application */
  
  /* find some physical ports and connect to them */
  fluid_settings_getint (settings, "audio.jack.autoconnect", &autoconnect);
  if (autoconnect) {
    jack_ports = jack_get_ports (dev->client, NULL, NULL, JackPortIsInput|JackPortIsPhysical);
    if (jack_ports) {
      int err;
      int connected = 0;
      
      for (i = 0; jack_ports[i] && (connected < 2); ++i) {
        err = jack_connect (dev->client, jack_port_name
			    (connected == 0 ? dev->output_ports[0] 
			     : dev->output_ports[1]), jack_ports[i]);
        if (err) {
          FLUID_LOG(FLUID_ERR, "Error connecting jack port");
        } else {
          connected++;
        }
      }

      free (jack_ports);  /* free jack ports array (not the port values!) */
    } else {
      FLUID_LOG(FLUID_WARN, "Could not connect to any physical jack ports; fluidsynth is unconnected");
    }
  }

  return (fluid_audio_driver_t*) dev;

 error_recovery:
  
  delete_fluid_jack_audio_driver((fluid_audio_driver_t*) dev);
  return NULL;
}

fluid_audio_driver_t* 
new_fluid_jack_audio_driver2(fluid_settings_t* settings, fluid_audio_func_t func, void* data)
{
  fluid_jack_audio_driver_t* dev = NULL;
  char name[64];
  int i, audio_count, fx_count;
  /* for looking up ports */
  const char ** jack_ports;
  char* client_name;
  int nin, nout;
  int autoconnect = 0;


  dev = FLUID_NEW(fluid_jack_audio_driver_t);
  if (dev == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;    
  }
  FLUID_MEMSET(dev, 0, sizeof(fluid_jack_audio_driver_t));


  dev->callback = func;
  dev->data = data;

  /* try to become a client of the JACK server */

  if (fluid_settings_getstr(settings, "audio.jack.id", &client_name)
      && (client_name != NULL) 
      && (strlen(client_name) > 0)) {
    snprintf(name, 64, "%s", client_name);
  } else {
    snprintf(name, 64, "fluidsynth");
  }

  if ((dev->client = jack_client_new(name)) == 0) {
    FLUID_LOG(FLUID_ERR, "Jack server not running?");
    goto error_recovery;
  }

  /* set callbacks */
  jack_set_process_callback(dev->client, fluid_jack_audio_driver_process2, (void*) dev);
  jack_set_buffer_size_callback(dev->client, fluid_jack_audio_driver_bufsize, (void*) dev);
  jack_set_sample_rate_callback(dev->client, fluid_jack_audio_driver_srate, (void*) dev);
  jack_on_shutdown(dev->client, fluid_jack_audio_driver_shutdown, (void*) dev);

  /* display the current sample rate. once the client is activated 
     (see below), you should rely on your own sample rate
     callback (see above) for this value.
  */
  FLUID_LOG(FLUID_DBG, "Jack engine sample rate: %lu", jack_get_sample_rate(dev->client));

  fluid_settings_getint(settings, "audio.output-channels", &dev->num_output_ports);
  fluid_settings_getint(settings, "audio.input-channels", &dev->num_input_ports);

  /* create output ports */
  if (dev->num_output_ports > 0) {
	  dev->output_ports = FLUID_ARRAY(jack_port_t*, dev->num_output_ports);
	  if (dev->output_ports == NULL) {
		  FLUID_LOG(FLUID_PANIC, "Out of memory");
		  goto error_recovery;
	  }
	  
	  dev->output_bufs = FLUID_ARRAY(float*, dev->num_output_ports);
	  if (dev->output_bufs == NULL) {
		  FLUID_LOG(FLUID_PANIC, "Out of memory");
		  goto error_recovery;
	  }
	  
	  for (i = 0; i < dev->num_output_ports; i++) {
		  sprintf(name, "out_%02d", i);
		  dev->output_ports[i] = jack_port_register(dev->client, name, 
							    JACK_DEFAULT_AUDIO_TYPE, 
							    JackPortIsOutput, 0);
	  }
  }

  /* create input ports */
  if (dev->num_input_ports > 0) {
	  dev->input_ports = FLUID_ARRAY(jack_port_t*, dev->num_input_ports);
	  if (dev->input_ports == NULL) {
		  FLUID_LOG(FLUID_PANIC, "Out of memory");
		  goto error_recovery;
	  }
	  
	  dev->input_bufs = FLUID_ARRAY(float*, dev->num_input_ports);
	  if (dev->input_bufs == NULL) {
		  FLUID_LOG(FLUID_PANIC, "Out of memory");
		  goto error_recovery;
	  }
	  
	  for (i = 0; i < dev->num_input_ports; i++) {
		  sprintf(name, "in_%02d", i);
		  dev->input_ports[i] = jack_port_register(dev->client, name, 
							   JACK_DEFAULT_AUDIO_TYPE, 
							   JackPortIsInput, 0);
	  }
  }

  /* effects are not used */
  dev->num_fx_ports = 0;
  dev->fx_ports = NULL;
  dev->fx_bufs = NULL;

  /* tell the JACK server that we are ready to roll */
  if (jack_activate(dev->client)) {
    FLUID_LOG(FLUID_ERR, "Cannot activate the fluidsynth as a JACK client");
    goto error_recovery;
  }

  /* connect the ports. */


  /* FIXME: should be done by a patchbay application */
  
  /* find some physical ports and connect to them */
  fluid_settings_getint (settings, "audio.jack.autoconnect", &autoconnect);
  if (autoconnect) {
    if (dev->num_output_ports > 0) {
      jack_ports = jack_get_ports(dev->client, NULL, NULL, JackPortIsInput|JackPortIsPhysical);
      if (jack_ports) {
        int err;
        int connected = 0;
      
        for (i = 0; jack_ports[i] && (connected < dev->num_output_ports); ++i) {
  	  err = jack_connect (dev->client,
	  		      jack_port_name(dev->output_ports[i]),
			      jack_ports[i]);
	  if (err) {
	    FLUID_LOG(FLUID_ERR, "Error connecting jack port");
	  } else {
	    connected++;
	  }
        }
    
        free (jack_ports);	/* free the jack port array */
      } else {
        FLUID_LOG(FLUID_WARN, "Could not connect to any physical jack ports; "
	         "fluidsynth is unconnected");
      }
    }

    if (dev->num_input_ports > 0) {
      jack_ports = jack_get_ports(dev->client, NULL, NULL, JackPortIsOutput|JackPortIsPhysical);
      if (jack_ports) {
        int err;
        int connected = 0;
      
        for (i = 0; jack_ports[i] && (connected < dev->num_input_ports); ++i) {
	  err = jack_connect (dev->client,
	 		      jack_ports[i],
			      jack_port_name(dev->input_ports[i]));
	  if (err) {
	    FLUID_LOG(FLUID_ERR, "Error connecting jack port");
	  } else {
	    connected++;
	  }
        }

        free (jack_ports); /* free the jack port array */

      }
    } else {
      FLUID_LOG(FLUID_WARN, "Could not connect to any physical jack ports; "
	       "fluidsynth is unconnected");
    }
  }

  return (fluid_audio_driver_t*) dev;

 error_recovery:
  
  delete_fluid_jack_audio_driver((fluid_audio_driver_t*) dev);
  return NULL;
}

/*
 * delete_fluid_jack_audio_driver
 */
int 
delete_fluid_jack_audio_driver(fluid_audio_driver_t* p) 
{
  fluid_jack_audio_driver_t* dev = (fluid_jack_audio_driver_t*) p;  
  int i;

  if (dev == NULL) {
    return 0;
  }

  if (dev->client != 0) {
    jack_client_close(dev->client);
  }

  if (dev->output_bufs) {
    FLUID_FREE(dev->output_bufs);
  }

  if (dev->output_ports) {
    FLUID_FREE(dev->output_ports);
  }

  if (dev->fx_bufs) {
    FLUID_FREE(dev->fx_bufs);
  }

  if (dev->fx_ports) {
    FLUID_FREE(dev->fx_ports);
  }

  FLUID_FREE(dev);
  return 0;
}


int
fluid_jack_audio_driver_process(jack_nframes_t nframes, void *arg)
{
  fluid_jack_audio_driver_t* dev = (fluid_jack_audio_driver_t*) arg;  

  if (dev->fx_ports == NULL) {
    float *left;
    float *right;

    left = (float*) jack_port_get_buffer(dev->output_ports[0], nframes);
    right = (float*) jack_port_get_buffer(dev->output_ports[1], nframes);

    fluid_synth_write_float(dev->synth, nframes, left, 0, 1, right, 0, 1);

  } else {

    int i, k;

    for (i = 0, k = dev->num_output_ports; i < dev->num_output_ports; i++, k++) {
      dev->output_bufs[i] = (float*) jack_port_get_buffer(dev->output_ports[2*i], nframes);
      dev->output_bufs[k] = (float*) jack_port_get_buffer(dev->output_ports[2*i+1], nframes);
    }
    for (i = 0, k = dev->num_fx_ports; i < dev->num_fx_ports; i++, k++) {
      dev->fx_bufs[i] = (float*) jack_port_get_buffer(dev->fx_ports[2*i], nframes);
      dev->fx_bufs[k] = (float*) jack_port_get_buffer(dev->fx_ports[2*i+1], nframes);
    }

    fluid_synth_nwrite_float(dev->synth, nframes, 
			    dev->output_bufs,
			    dev->output_bufs + dev->num_output_ports,
			    dev->fx_bufs,
			    dev->fx_bufs + dev->num_fx_ports);
  }

  return 0;      
}

int
fluid_jack_audio_driver_process2(jack_nframes_t nframes, void *arg)
{
  fluid_jack_audio_driver_t* dev = (fluid_jack_audio_driver_t*) arg;  
  int i;
  
  for (i = 0; i < dev->num_output_ports; i++) {
    dev->output_bufs[i] = (float*) jack_port_get_buffer(dev->output_ports[i], nframes);
  }
  for (i = 0; i < dev->num_input_ports; i++) {
    dev->input_bufs[i] = (float*) jack_port_get_buffer(dev->input_ports[i], nframes);
  }

  return (*dev->callback)(dev->data, nframes,
			  dev->num_input_ports, dev->input_bufs,
			  dev->num_output_ports, dev->output_bufs);
}

int
fluid_jack_audio_driver_bufsize(jack_nframes_t nframes, void *arg)
{
/*   printf("the maximum buffer size is now %lu\n", nframes); */
  return 0;
}

int
fluid_jack_audio_driver_srate(jack_nframes_t nframes, void *arg)
{
/*   printf("the sample rate is now %lu/sec\n", nframes); */
  /* FIXME: change the sample rate of the synthesizer! */
  return 0;
}

void
fluid_jack_audio_driver_shutdown(void *arg)
{
  fluid_jack_audio_driver_t* dev = (fluid_jack_audio_driver_t*) arg;  
  FLUID_LOG(FLUID_ERR, "Help! Lost the connection to the JACK server");
/*   exit (1); */
}
