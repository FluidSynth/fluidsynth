/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#include "fluid_adriver.h"
#include "fluid_settings.h"

/*
 * fluid_adriver_definition_t
 */

typedef struct _fluid_audriver_definition_t
{
  const char* name;
  fluid_audio_driver_t* (*new)(fluid_settings_t* settings, fluid_synth_t* synth);
  fluid_audio_driver_t* (*new2)(fluid_settings_t* settings,
				fluid_audio_func_t func,
				void* data);
  int (*free)(fluid_audio_driver_t* driver);
  void (*settings)(fluid_settings_t* settings);
} fluid_audriver_definition_t;


#if PULSE_SUPPORT
fluid_audio_driver_t* new_fluid_pulse_audio_driver(fluid_settings_t* settings,
						   fluid_synth_t* synth);
fluid_audio_driver_t* new_fluid_pulse_audio_driver2(fluid_settings_t* settings,
						    fluid_audio_func_t func, void* data);
int delete_fluid_pulse_audio_driver(fluid_audio_driver_t* p);
void fluid_pulse_audio_driver_settings(fluid_settings_t* settings);
#endif

#if ALSA_SUPPORT
fluid_audio_driver_t* new_fluid_alsa_audio_driver(fluid_settings_t* settings,
						  fluid_synth_t* synth);
fluid_audio_driver_t* new_fluid_alsa_audio_driver2(fluid_settings_t* settings,
						 fluid_audio_func_t func, void* data);
int delete_fluid_alsa_audio_driver(fluid_audio_driver_t* p);
void fluid_alsa_audio_driver_settings(fluid_settings_t* settings);
#endif

#if OSS_SUPPORT
fluid_audio_driver_t* new_fluid_oss_audio_driver(fluid_settings_t* settings,
						 fluid_synth_t* synth);
fluid_audio_driver_t* new_fluid_oss_audio_driver2(fluid_settings_t* settings,
						fluid_audio_func_t func, void* data);
int delete_fluid_oss_audio_driver(fluid_audio_driver_t* p);
void fluid_oss_audio_driver_settings(fluid_settings_t* settings);
#endif

#if COREAUDIO_SUPPORT
fluid_audio_driver_t* new_fluid_core_audio_driver(fluid_settings_t* settings,
						  fluid_synth_t* synth);
fluid_audio_driver_t* new_fluid_core_audio_driver2(fluid_settings_t* settings,
						      fluid_audio_func_t func,
						      void* data);
int delete_fluid_core_audio_driver(fluid_audio_driver_t* p);
void fluid_core_audio_driver_settings(fluid_settings_t* settings);
#endif

#if DSOUND_SUPPORT
fluid_audio_driver_t* new_fluid_dsound_audio_driver(fluid_settings_t* settings,
						  fluid_synth_t* synth);
int delete_fluid_dsound_audio_driver(fluid_audio_driver_t* p);
void fluid_dsound_audio_driver_settings(fluid_settings_t* settings);
#endif

#if PORTAUDIO_SUPPORT
void fluid_portaudio_driver_settings (fluid_settings_t *settings);
fluid_audio_driver_t* new_fluid_portaudio_driver(fluid_settings_t* settings,
						 fluid_synth_t* synth);
int delete_fluid_portaudio_driver(fluid_audio_driver_t* p);
#endif

#if JACK_SUPPORT
fluid_audio_driver_t* new_fluid_jack_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth);
fluid_audio_driver_t* new_fluid_jack_audio_driver2(fluid_settings_t* settings,
						 fluid_audio_func_t func, void* data);
int delete_fluid_jack_audio_driver(fluid_audio_driver_t* p);
void fluid_jack_audio_driver_settings(fluid_settings_t* settings);
#endif

#if SNDMAN_SUPPORT
fluid_audio_driver_t* new_fluid_sndmgr_audio_driver(fluid_settings_t* settings,
						  fluid_synth_t* synth);
fluid_audio_driver_t* new_fluid_sndmgr_audio_driver2(fluid_settings_t* settings,
						   fluid_audio_func_t func,
						   void* data);
int delete_fluid_sndmgr_audio_driver(fluid_audio_driver_t* p);
#endif

#if DART_SUPPORT
fluid_audio_driver_t* new_fluid_dart_audio_driver(fluid_settings_t* settings,
                          fluid_synth_t* synth);
int delete_fluid_dart_audio_driver(fluid_audio_driver_t* p);
void fluid_dart_audio_driver_settings(fluid_settings_t* settings);
#endif

#if AUFILE_SUPPORT
fluid_audio_driver_t* new_fluid_file_audio_driver(fluid_settings_t* settings,
						  fluid_synth_t* synth);
int delete_fluid_file_audio_driver(fluid_audio_driver_t* p);
#endif


/* Available audio drivers, listed in order of preference */
#if JACK_SUPPORT
    #define JACK_DEF_INIT \
    { "jack", \
        new_fluid_jack_audio_driver, \
        new_fluid_jack_audio_driver2, \
        delete_fluid_jack_audio_driver, \
        fluid_jack_audio_driver_settings },
#else
    #define JACK_DEF_INIT
#endif

#if ALSA_SUPPORT
    #define ALSA_DEF_INIT \
    { "alsa", \
        new_fluid_alsa_audio_driver, \
        new_fluid_alsa_audio_driver2, \
        delete_fluid_alsa_audio_driver, \
        fluid_alsa_audio_driver_settings },
#else
    #define ALSA_DEF_INIT
#endif

#if OSS_SUPPORT
    #define OSS_DEF_INIT \
    { "oss", \
        new_fluid_oss_audio_driver, \
        new_fluid_oss_audio_driver2, \
        delete_fluid_oss_audio_driver, \
        fluid_oss_audio_driver_settings },
#else
    #define OSS_DEF_INIT
#endif

#if PULSE_SUPPORT
    #define PULSE_DEF_INIT \
    { "pulseaudio", \
        new_fluid_pulse_audio_driver, \
        new_fluid_pulse_audio_driver2, \
        delete_fluid_pulse_audio_driver, \
        fluid_pulse_audio_driver_settings },
#else
    #define PULSE_DEF_INIT
#endif

#if COREAUDIO_SUPPORT
    #define COREAUDIO_DEF_INIT \
    { "coreaudio", \
        new_fluid_core_audio_driver, \
        new_fluid_core_audio_driver2, \
        delete_fluid_core_audio_driver, \
        fluid_core_audio_driver_settings },
#else
    #define COREAUDIO_DEF_INIT
#endif

#if DSOUND_SUPPORT
    #define DSOUND_DEF_INIT \
    { "dsound", \
        new_fluid_dsound_audio_driver, \
        NULL, \
        delete_fluid_dsound_audio_driver, \
        fluid_dsound_audio_driver_settings },
#else
    #define DSOUND_DEF_INIT
#endif

#if PORTAUDIO_SUPPORT
    #define PORTAUDIO_DEF_INIT \
    { "portaudio", \
        new_fluid_portaudio_driver, \
        NULL, \
        delete_fluid_portaudio_driver, \
        fluid_portaudio_driver_settings },
#else
    #define PORTAUDIO_DEF_INIT
#endif

#if SNDMAN_SUPPORT
    #define SNDMAN_DEF_INIT \
    { "sndman", \
        new_fluid_sndmgr_audio_driver, \
        new_fluid_sndmgr_audio_driver2, \
        delete_fluid_sndmgr_audio_driver, \
        NULL },
#else
    #define SNDMAN_DEF_INIT
#endif

#if DART_SUPPORT
    #define DART_DEF_INIT \
    { "dart", \
        new_fluid_dart_audio_driver, \
        NULL, \
        delete_fluid_dart_audio_driver, \
        fluid_dart_audio_driver_settings },
#else
    #define DART_DEF_INIT
#endif

#if AUFILE_SUPPORT
    #define AUFILE_DEF_INIT \
    { "file", \
        new_fluid_file_audio_driver, \
        NULL, \
        delete_fluid_file_audio_driver, \
        NULL },
#else
    #define AUFILE_DEF_INIT
#endif

#define AVAILABLE_AUDRIVERS \
    JACK_DEF_INIT \
    ALSA_DEF_INIT \
    OSS_DEF_INIT \
    PULSE_DEF_INIT \
    COREAUDIO_DEF_INIT \
    DSOUND_DEF_INIT \
    PORTAUDIO_DEF_INIT \
    SNDMAN_DEF_INIT \
    DART_DEF_INIT \
    AUFILE_DEF_INIT

/* fluid_audio_drivers_template is a compile-constant template containing all audio drivers
 * fluidsynth has been built with
 *
 * fluid_audio_drivers contains all the drivers registered for usage with
 * fluid_audio_driver_register()
 * 
 * To maintain backwards compatibility, all available drivers are initially registered, so
 * this must be the same as fluid_audio_drivers_template. But arrays are unassignable in C
 * and copying them at runtime with memcpy in fluid_audio_driver_settings() wouldnt be
 * threadsafe. So use this ugly macro hack to initialize both equally at compiletime.
 */
static const fluid_audriver_definition_t fluid_audio_drivers_template[] =
{
    AVAILABLE_AUDRIVERS
};

static fluid_audriver_definition_t fluid_audio_drivers[] =
{
    AVAILABLE_AUDRIVERS
};


void fluid_audio_driver_settings(fluid_settings_t* settings)
{
  unsigned int i;

  fluid_settings_register_str(settings, "audio.sample-format", "16bits", 0, NULL, NULL);
  fluid_settings_add_option(settings, "audio.sample-format", "16bits");
  fluid_settings_add_option(settings, "audio.sample-format", "float");

#if defined(WIN32)
  fluid_settings_register_int(settings, "audio.period-size", 512, 64, 8192, 0, NULL, NULL);
  fluid_settings_register_int(settings, "audio.periods", 8, 2, 64, 0, NULL, NULL);
#elif defined(MACOS9)
  fluid_settings_register_int(settings, "audio.period-size", 64, 64, 8192, 0, NULL, NULL);
  fluid_settings_register_int(settings, "audio.periods", 8, 2, 64, 0, NULL, NULL);
#else
  fluid_settings_register_int(settings, "audio.period-size", 64, 64, 8192, 0, NULL, NULL);
  fluid_settings_register_int(settings, "audio.periods", 16, 2, 64, 0, NULL, NULL);
#endif

  fluid_settings_register_int (settings, "audio.realtime-prio",
                               FLUID_DEFAULT_AUDIO_RT_PRIO, 0, 99, 0, NULL, NULL);

  /* Set the default driver */
#if JACK_SUPPORT
  fluid_settings_register_str(settings, "audio.driver", "jack", 0, NULL, NULL);
#elif ALSA_SUPPORT
  fluid_settings_register_str(settings, "audio.driver", "alsa", 0, NULL, NULL);
#elif PULSE_SUPPORT
  fluid_settings_register_str(settings, "audio.driver", "pulseaudio", 0, NULL, NULL);
#elif OSS_SUPPORT
  fluid_settings_register_str(settings, "audio.driver", "oss", 0, NULL, NULL);
#elif COREAUDIO_SUPPORT
  fluid_settings_register_str(settings, "audio.driver", "coreaudio", 0, NULL, NULL);
#elif DSOUND_SUPPORT
  fluid_settings_register_str(settings, "audio.driver", "dsound", 0, NULL, NULL);
#elif SNDMAN_SUPPORT
  fluid_settings_register_str(settings, "audio.driver", "sndman", 0, NULL, NULL);
#elif PORTAUDIO_SUPPORT
  fluid_settings_register_str(settings, "audio.driver", "portaudio", 0, NULL, NULL);
#elif DART_SUPPORT
  fluid_settings_register_str(settings, "audio.driver", "dart", 0, NULL, NULL);
#elif AUFILE_SUPPORT
  fluid_settings_register_str(settings, "audio.driver", "file", 0, NULL, NULL);
#else
  fluid_settings_register_str(settings, "audio.driver", "", 0, NULL, NULL);
#endif

  /* Add all drivers to the list of options */
#if PULSE_SUPPORT
  fluid_settings_add_option(settings, "audio.driver", "pulseaudio");
#endif
#if ALSA_SUPPORT
  fluid_settings_add_option(settings, "audio.driver", "alsa");
#endif
#if OSS_SUPPORT
  fluid_settings_add_option(settings, "audio.driver", "oss");
#endif
#if COREAUDIO_SUPPORT
  fluid_settings_add_option(settings, "audio.driver", "coreaudio");
#endif
#if DSOUND_SUPPORT
  fluid_settings_add_option(settings, "audio.driver", "dsound");
#endif
#if SNDMAN_SUPPORT
  fluid_settings_add_option(settings, "audio.driver", "sndman");
#endif
#if PORTAUDIO_SUPPORT
  fluid_settings_add_option(settings, "audio.driver", "portaudio");
#endif
#if JACK_SUPPORT
  fluid_settings_add_option(settings, "audio.driver", "jack");
#endif
#if DART_SUPPORT
  fluid_settings_add_option(settings, "audio.driver", "dart");
#endif
#if AUFILE_SUPPORT
  fluid_settings_add_option(settings, "audio.driver", "file");
#endif

  for (i = 0; i < FLUID_N_ELEMENTS(fluid_audio_drivers) && fluid_audio_drivers[i].name != NULL; i++) {
    if (fluid_audio_drivers[i].settings != NULL) {
      fluid_audio_drivers[i].settings(settings);
    }
  }
}

/**
 * Create a new audio driver.
 * @param settings Configuration settings used to select and create the audio
 *   driver.
 * @param synth Synthesizer instance for which the audio driver is created for.
 * @return The new audio driver instance.
 *
 * Creates a new audio driver for a given 'synth' instance with a defined set
 * of configuration 'settings'.
 */
fluid_audio_driver_t*
new_fluid_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth)
{
  unsigned int i;
  fluid_audio_driver_t* driver = NULL;
  char* name;
  char *allnames;

  for (i = 0; i < FLUID_N_ELEMENTS(fluid_audio_drivers) && fluid_audio_drivers[i].name != NULL; i++) {
    if (fluid_settings_str_equal(settings, "audio.driver", fluid_audio_drivers[i].name)) {
      FLUID_LOG(FLUID_DBG, "Using '%s' audio driver", fluid_audio_drivers[i].name);
      driver = (*fluid_audio_drivers[i].new)(settings, synth);
      if (driver) {
	driver->name = fluid_audio_drivers[i].name;
      }
      return driver;
    }
  }

  allnames = fluid_settings_option_concat (settings, "audio.driver", NULL);
  fluid_settings_dupstr (settings, "audio.driver", &name);       /* ++ alloc name */
  FLUID_LOG(FLUID_ERR, "Couldn't find the requested audio driver %s. Valid drivers are: %s.",
            name ? name : "NULL", allnames ? allnames : "ERROR");
  if (name) FLUID_FREE (name);
  if (allnames) FLUID_FREE (allnames);
  return NULL;
}

/**
 * Create a new audio driver.
 * @param settings Configuration settings used to select and create the audio
 *   driver.
 * @param func Function called to fill audio buffers for audio playback
 * @param data User defined data pointer to pass to 'func'
 * @return The new audio driver instance.
 *
 * Like new_fluid_audio_driver() but allows for custom audio processing before
 * audio is sent to audio driver.  It is the responsibility of the callback
 * 'func' to render the audio into the buffers.
 *
 * NOTE: Not as efficient as new_fluid_audio_driver().
 */
fluid_audio_driver_t*
new_fluid_audio_driver2(fluid_settings_t* settings, fluid_audio_func_t func, void* data)
{
  unsigned int i;
  fluid_audio_driver_t* driver = NULL;
  char* name;

  for (i = 0; i < FLUID_N_ELEMENTS(fluid_audio_drivers) && fluid_audio_drivers[i].name != NULL; i++) {
    if (fluid_settings_str_equal(settings, "audio.driver", fluid_audio_drivers[i].name) &&
	(fluid_audio_drivers[i].new2 != NULL)) {
      FLUID_LOG(FLUID_DBG, "Using '%s' audio driver", fluid_audio_drivers[i].name);
      driver = (*fluid_audio_drivers[i].new2)(settings, func, data);
      if (driver) {
	driver->name = fluid_audio_drivers[i].name;
      }
      return driver;
    }
  }

  fluid_settings_dupstr(settings, "audio.driver", &name);       /* ++ alloc name */
  FLUID_LOG(FLUID_ERR, "Couldn't find the requested audio driver: %s",
            name ? name : "NULL");
  if (name) FLUID_FREE (name);
  return NULL;
}

/**
 * Deletes an audio driver instance.
 * @param driver Audio driver instance to delete
 *
 * Shuts down an audio driver and deletes its instance.
 */
void
delete_fluid_audio_driver(fluid_audio_driver_t* driver)
{
  unsigned int i;

  /* iterate over fluid_audio_drivers_template to ensure deleting even drivers currently not registered */
  for (i = 0; i < FLUID_N_ELEMENTS(fluid_audio_drivers_template); i++) {
    if (fluid_audio_drivers_template[i].name == driver->name) {
      fluid_audio_drivers_template[i].free(driver);
      return;
    }
  }
}


/**
 * @brief Registers audio drivers to use
 * 
 * When creating a settings instance with new_fluid_settings(), all audio drivers are initialized once.
 * In the past this has caused segfaults and application crashes due to buggy soundcard drivers.
 * 
 * This function enables the user to only initialize specific audio drivers when settings instances are created.
 * Therefore pass a NULL-terminated array of C-strings containing the \c names of audio drivers to register
 * for the usage with fluidsynth.
 * The \c names are the same as being used for the \c audio.driver setting.
 * 
 * By default all audio drivers fluidsynth has been compiled with are registered.
 * 
 * Any attempt of using audio drivers that have not been registered is undefined behaviour!
 * 
 * @param adrivers NULL-terminated array of audio drivers to register. Pass NULL to register all available drivers.
 * @return #FLUID_OK if all the audio drivers requested by the user are supported by fluidsynth and have been
 * successfully registered. Otherwise #FLUID_FAILED is returned and ALL available audio drivers are registered instead.
 * 
 * @warning This function may only be called if no thread is residing in fluidsynth's API and no instances
 * of any kind are alive (e.g. as it would be the case right after fluidsynth's inital creation).
 * Else the behaviour is undefined.
 * 
 * @note This function is not thread safe and will never be!
 */
int fluid_audio_driver_register(const char** adrivers)
{
    unsigned int i=0, add=0;
    int res = FLUID_FAILED;
    
    if(adrivers == NULL)
    {
        res = FLUID_OK;
        goto cleanup;
    }
    else
    {
        FLUID_MEMSET(fluid_audio_drivers, 0, sizeof(fluid_audio_drivers));
        for(i=0; adrivers[i] != NULL; i++)
        {
            unsigned int j;
            /* search the requested audio driver in the template and copy it over if found */
            for (j = 0; j < FLUID_N_ELEMENTS(fluid_audio_drivers_template); j++)
            {
                if (FLUID_STRCMP(adrivers[i], fluid_audio_drivers_template[j].name) == 0)
                {
                    FLUID_MEMCPY(&fluid_audio_drivers[add], &fluid_audio_drivers_template[j], sizeof(fluid_audio_drivers[add]));
                    add++;
                    break;
                }
            }
            
            if(j >= FLUID_N_ELEMENTS(fluid_audio_drivers_template))
            {
                /* requested driver not found, failure */
                goto cleanup;
            }
        }
    }
    
    if(i >= FLUID_N_ELEMENTS(fluid_audio_drivers_template))
    {
        /* user requested more drivers than this build of fluidsynth supports, failure */
        goto cleanup;
    }
    
    res = FLUID_OK;
    return res;
    
cleanup:
    FLUID_MEMCPY(fluid_audio_drivers, fluid_audio_drivers_template, sizeof(fluid_audio_drivers));
    return res;
}
