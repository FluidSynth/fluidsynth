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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fluidsynth_priv.h"

#if !defined(WIN32) && !defined(MACINTOSH)
#define _GNU_SOURCE
#include <getopt.h>
#endif

#if defined(WIN32)
#include <windows.h>
#endif

#include "fluidsynth.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(WIN32) && !defined(MINGW32)
#include "config_win32.h"
#endif

#ifdef HAVE_SIGNAL_H
#include "signal.h"
#endif

#ifdef HAVE_LADCCA
#include <pthread.h>
#include <ladcca/ladcca.h>
static void * cca_run (void * data);
extern cca_client_t * fluid_cca_client;
#endif /* HAVE_LADCCA */

#ifndef WITH_MIDI
#define WITH_MIDI 1
#endif

/* default audio fragment count (if none specified) */
#ifdef WIN32
#define DEFAULT_FRAG_COUNT 32
#else
#define DEFAULT_FRAG_COUNT 16
#endif

void print_usage(void);
void print_help(void);
void print_welcome(void);
void print_version(void);

static fluid_cmd_handler_t* newclient(void* data, char* addr);

/*
 * the globals
 */
char* appname = NULL;

/*
 * macros to wrap readline functions
 */

/*
 * support for the getopt function
 */
#if !defined(WIN32) && !defined(MACINTOSH)
int getopt(int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int optind, opterr, optopt;
#endif


/* process_o_cmd_line_option
 *
 * Purpose:
 * Process a command line option -o setting=value, 
 * for example: -o synth.polyhony=16
 */
void process_o_cmd_line_option(fluid_settings_t* settings, char* optarg){
  char* val;
  for (val = optarg; *val != '\0'; val++) {
    if (*val == '=') {
      *val++ = 0;
      break;
    }
  }
  
  /* At this point:
   * optarg => "synth.polyphony"
   * val => "16"
   */
  switch(fluid_settings_get_type(settings, optarg)){
  case FLUID_NUM_TYPE:
    if (fluid_settings_setnum(settings, optarg, atof(val))){
      break;
    };
  case FLUID_INT_TYPE:
    if (fluid_settings_setint(settings, optarg, atoi(val))){
      break;
    };
  case FLUID_STR_TYPE:
    if (fluid_settings_setstr(settings, optarg, val)){
      break;
    };
  default:
    fprintf (stderr, "Settings argument on command line: Failed to set \"%s\" to \"%s\".\n"
	      "Most likely the parameter \"%s\" does not exist.\n", optarg, val, optarg);
  }  
}


#ifdef HAVE_SIGNAL_H
/* 
 * handle_signal
 */
void handle_signal(int sig_num)
{
}
#endif


/*
 * main
 */
int main(int argc, char** argv) 
{
  fluid_settings_t* settings;
  int arg1 = 1;
  char buf[512];
  int c, i, fragcount = DEFAULT_FRAG_COUNT;
  int interactive = 1;
  int midi_in = 1;
  fluid_player_t* player = NULL; 
  fluid_midi_router_t* router = NULL;
  fluid_midi_driver_t* mdriver = NULL;
  fluid_audio_driver_t* adriver = NULL;
  fluid_synth_t* synth = NULL;
  fluid_server_t* server = NULL;
  fluid_cmd_handler_t* cmd_handler = NULL;
  char* midi_id = NULL;
  char* midi_driver = NULL;
  char* midi_device = NULL;
  int audio_groups = 0;
  int audio_channels = 0;
  int with_server = 0;
  int dump = 0;
  int ladcca_connect = 1;
#ifdef HAVE_LADCCA
  cca_args_t * cca_args;
  pthread_t cca_thread;

  cca_args = cca_extract_args (&argc, &argv);
#endif

  appname = argv[0];
  settings = new_fluid_settings();

#if !defined(WIN32) && !defined(MACINTOSH)

  /* handle command options. on posix machines only */
  opterr = 0;

  while (1) {
    int option_index = 0;

    static struct option long_options[] = {
      {"no-midi-in", 0, 0, 'n'},
      {"midi-driver", 1, 0, 'm'},
      {"midi-channels", 1, 0, 'K'},
      {"audio-driver", 1, 0, 'a'},
      {"connect-jack-outputs", 0, 0, 'j'},
      {"audio-channels", 1, 0, 'L'},
      {"audio-groups", 1, 0, 'G'},
      {"audio-bufsize", 1, 0, 'z'},
      {"audio-bufcount", 1, 0, 'c'},
      {"sample-rate", 1, 0, 'r'},
      {"disable-ladcca", 0, 0, 'l'},
      {"verbose", 0, 0, 'v'},
      {"reverb", 1, 0, 'R'},
      {"chorus", 1, 0, 'C'},
      {"gain", 1, 0, 'g'},
      {"server", 0, 0, 's'},
      {"help", 0, 0, 'h'},
      {"dump", 0, 0, 'd'},
      {"no-shell", 0, 0, 'i'},
      {"version", 0, 0, 'V'},
      {"option", 1, 0, 'o'},
      {0, 0, 0, 0}
    };

    c = getopt_long(argc, argv, "vnixdhVsplm:K:L:M:a:A:s:z:c:R:C:r:G:o:g:j", 
		    long_options, &option_index);
    if (c == -1) {
      break;
    }

    switch (c) {
    case 0:
      printf ("option %s", long_options[option_index].name);
      if (optarg) {
	printf (" with arg %s", optarg);
      }
      printf ("\n");
      break;
    case 'm':
      fluid_settings_setstr(settings, "midi.driver", optarg);
      break;
    case 'a':
      fluid_settings_setstr(settings, "audio.driver", optarg);
      break;
    case 'j':
      fluid_settings_setint(settings, "audio.jack.autoconnect", 1);
      break;
    case 'z':
      fluid_settings_setint(settings, "audio.period-size", atoi(optarg));
      break;
    case 'c':
      fluid_settings_setint(settings, "audio.periods", atoi(optarg));
      break;
    case 'g':
      fluid_settings_setnum(settings, "synth.gain", atof(optarg));
      break;
    case 'r':
      fluid_settings_setnum(settings, "synth.sample-rate", atof(optarg));
      break;
    case 'l':			/* disable LADCCA */
      ladcca_connect = 0;
      break;
    case 'L':
      audio_channels = atoi(optarg);
      fluid_settings_setint(settings, "synth.audio-channels", audio_channels);
      break;
    case 'G':
      audio_groups = atoi(optarg);
      break;
    case 'K':
      fluid_settings_setint(settings, "synth.midi-channels", atoi(optarg));
      break;
    case 'v':
      fluid_settings_setstr(settings, "synth.verbose", "yes");
      break;
    case 'd':
      fluid_settings_setstr(settings, "synth.dump", "yes");
      dump = 1;
      break;
    case 'R':
      if ((optarg != NULL) && ((strcmp(optarg, "0") == 0) || (strcmp(optarg, "no") == 0))) {
	fluid_settings_setstr(settings, "synth.reverb.active", "no");
      } else {
	fluid_settings_setstr(settings, "synth.reverb.active", "yes");
      }
      break;
    case 'C':
      if ((optarg != NULL) && ((strcmp(optarg, "0") == 0) || (strcmp(optarg, "no") == 0))) {
	fluid_settings_setstr(settings, "synth.chorus.active", "no");
      } else {
	fluid_settings_setstr(settings, "synth.chorus.active", "yes");
      }
      break;
    case 'o': 
      process_o_cmd_line_option(settings, optarg);
      break;
    case 'n':
      midi_in = 0;
      break;
    case 'i':
      interactive = 0;
      break;
    case 's':
      with_server = 1;
      break;
    case 'V':
      print_version(); /* and don't come back */
      break;
    case 'h':
      print_help();
      break;
    case '?':
      printf ("Unknown option %c\n", optopt);
      print_usage();
      exit(0);
      break;
    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }

  arg1 = optind;
#else
  for (i = 1; i < argc; i++) {
    char *optarg;

    if ((argv[i][0] == '-') && (argv[i][1] != '\0')) {
      switch (argv[i][1]) {
      case 'm':
	if (++i < argc) {
	  fluid_settings_setstr(settings, "midi.driver", argv[i]);
	} else {
	  printf ("Option -m requires an argument\n");	  
	}
	break;
      case 'a':
	if (++i < argc) {
	  fluid_settings_setstr(settings, "audio.driver", argv[i]);
	} else {
	  printf ("Option -a requires an argument\n");	  
	}
	break;
      case 'j':
        fluid_settings_setint(settings, "audio.jack.autoconnect", 1);
        break;
      case 'z':
	if (++i < argc) {
	  fluid_settings_setnum(settings, "audio.period-size", atof(argv[i]));
	} else {
	  printf ("Option -z requires an argument\n");	  
	}
	break;
      case 'c':
	if (++i < argc) {
	  fluid_settings_setnum(settings, "audio.periods", atof(argv[i]));
	} else {
	  printf ("Option -c requires an argument\n");	  
	}
	break;
      case 'g':
	if (++i < argc) {
	  fluid_settings_setnum(settings, "synth.gain", atof(argv[i]));
	} else {
	  printf ("Option -g requires an argument\n");	  
	}
	break;
      case 'r':
	if (++i < argc) {
	  fluid_settings_setnum(settings, "synth.sample-rate", atof(argv[i]));
	} else {
	  printf ("Option -r requires an argument\n");	  
	}
	break;
      case 'l':			/* disable LADCCA */
	ladcca_connect = 0;
	break;
      case 'L':
	if (++i < argc) {
	  audio_channels = atoi(argv[i]);
	  fluid_settings_setint(settings, "synth.audio-channels", audio_channels);
	} else {
	  printf ("Option -L requires an argument\n");	  
	}
	break;
      case 'G':
	audio_groups = atoi(optarg);
	break;
      case 'K':
	if (++i < argc) {
	  fluid_settings_setint(settings, "synth.midi-channels", atoi(argv[i]));
	} else {
	  printf ("Option -K requires an argument\n");	  
	}
	break;
      case 'v':
	fluid_settings_setstr(settings, "synth.verbose", "yes");
	break;
      case 'R':
	optarg = (++i < argc)? argv[i] : NULL;
	if ((optarg != NULL) && ((strcmp(optarg, "0") == 0) || (strcmp(optarg, "no") == 0))) {
	  fluid_settings_setstr(settings, "synth.reverb.active", "no");
	} else {
	  fluid_settings_setstr(settings, "synth.reverb.active", "yes");
	}
	break;
      case 'C':
	optarg = (++i < argc)? argv[i] : NULL;
	if ((optarg != NULL) && ((strcmp(optarg, "0") == 0) || (strcmp(optarg, "no") == 0))) {
	  fluid_settings_setstr(settings, "synth.chorus.active", "no");
	} else {
	  fluid_settings_setstr(settings, "synth.chorus.active", "yes");
	}
	break;
      case 'o': 
	process_o_cmd_line_option(settings, optarg);
	break;
      case 'n':
	midi_in = 0;
	break;
      case 'i':
	interactive = 0;
	break;
      case 'V':
	print_version(); 
	break;
      case 'h':
	print_help();
	break;
      }
    }
  }

    arg1 = 1;
#endif

#ifdef WIN32
  SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif


  /* connect to the ladcca server */
#ifdef HAVE_LADCCA
  if (ladcca_connect)
    {
      int flags;
      char * str;

      flags = CCA_Config_Data_Set | CCA_Terminal;

      /*  Removed from LADCCA? It is sufficient to just set the ALSA id or
	  Jack client name - JG

      if (fluid_settings_str_equal(settings, "audio.driver", "jack")) {
	flags |= CCA_Use_Jack;
      }
      if (fluid_settings_str_equal(settings, "midi.driver", "alsa_seq")) {
	flags |= CCA_Use_Alsa;
      }
      */

      fluid_cca_client = cca_init (cca_args, "FluidSynth", flags, CCA_PROTOCOL (1,1));

      if (fluid_cca_client)
	fluid_settings_setint (settings, "ladcca.enable", cca_enabled (fluid_cca_client) ? 1 : 0);
    }
#endif /* HAVE_LADCCA */



  /* The 'groups' setting is only relevant for LADSPA operation
   * If not given, set number groups to number of audio channels, because
   * they are the same (there is nothing between synth output and 'sound card')
   */
  if ((audio_groups == 0) && (audio_channels != 0)) {
      audio_groups = audio_channels;
  } 
  fluid_settings_setint(settings, "synth.audio-groups", audio_groups);
  
  /* create the synthesizer */
  synth = new_fluid_synth(settings);
  if (synth == NULL) {
    printf("Failed to create the synthesizer\n");
    exit(-1);
  }
  
  cmd_handler = new_fluid_cmd_handler(synth);
  if (cmd_handler == NULL) {
    printf("Failed to create the command handler\n");
    goto cleanup;
  }

  /* try to load the user or system configuration */
  if (fluid_get_userconf(buf, 512) != NULL) {
    fluid_source(cmd_handler, buf);
  } else if (fluid_get_sysconf(buf, 512) != NULL) {
    fluid_source(cmd_handler, buf);
  }

  /* load the soundfonts */
  for (i = arg1; i < argc; i++) {
    if ((argv[i][0] != '-') && fluid_is_soundfont(argv[i])) {
      if (fluid_synth_sfload(synth, argv[i], 1) == -1) {
	printf("Failed to load the SoundFont %s\n", argv[i]);
      }
    }
  }

#ifdef HAVE_SIGNAL_H
/*   signal(SIGINT, handle_signal); */
#endif

  /* start the synthesis thread */
  adriver = new_fluid_audio_driver(settings, synth);
  if (adriver == NULL) {
    printf("Failed to create the audio driver\n");
    goto cleanup;
  }


  /* start the midi router and link it to the synth */
#if WITH_MIDI
  if (midi_in) {
    /* In dump mode, text output is generated for events going into and out of the router.
     * The example dump functions are put into the chain before and after the router..
     */

    router = new_fluid_midi_router(
      settings, 
      dump ? fluid_midi_dump_postrouter : fluid_synth_handle_midi_event, 
      (void*)synth);
    
    if (router == NULL) {
      printf("Failed to create the MIDI input router; no MIDI input\n"
	     "will be available. You can access the synthesizer \n"
	     "through the console.\n");
    } else {
      fluid_synth_set_midi_router(synth, router); /* Fixme, needed for command handler */
      mdriver = new_fluid_midi_driver(
	settings, 
	dump ? fluid_midi_dump_prerouter : fluid_midi_router_handle_midi_event,
	(void*) router);
      if (mdriver == NULL) {
	printf("Failed to create the MIDI thread; no MIDI input\n"
	       "will be available. You can access the synthesizer \n"
	       "through the console.\n");
      }
    }
  }
#endif

  /* play the midi files, if any */
  for (i = arg1; i < argc; i++) {
    if ((argv[i][0] != '-') && fluid_is_midifile(argv[i])) {

      if (player == NULL) {
	player = new_fluid_player(synth);
	if (player == NULL) {
	  printf("Failed to create the midifile player.\n"
		 "Continuing without a player.\n");
	  break;
	}
      }

      fluid_player_add(player, argv[i]);
    }
  }

  if (player != NULL) {
    fluid_player_play(player);
  }

  /* run the server, if requested */
#if !defined(MACINTOSH) && !defined(WIN32)
  if (with_server) {
    server = new_fluid_server(settings, newclient, synth);
    if (server == NULL) {
      printf("Failed to create the server.\n"
	     "Continuing without it.\n");
    }
  }
#endif


#ifdef HAVE_LADCCA
  if (ladcca_connect && cca_enabled (fluid_cca_client))
    pthread_create (&cca_thread, NULL, cca_run, synth);
#endif /* HAVE_LADCCA */

  /* run the shell */
  if (interactive) {
    print_welcome();

    /* In dump mode we set the prompt to "". The UI cannot easily
     * handle lines, which don't end with CR.  Changing the prompt
     * cannot be done through a command, because the current shell
     * does not handle empty arguments.  The ordinary case is dump ==
     * 0.
     */
    fluid_settings_setstr(settings, "shell.prompt", dump ? "" : "> ");
    fluid_usershell(settings, cmd_handler);    
  }

 cleanup:

#if !defined(MACINTOSH) && !defined(WIN32)
  if (server != NULL) {
    /* if the user typed 'quit' in the shell, kill the server */
    if (!interactive) {
      fluid_server_join(server);
    }
    delete_fluid_server(server);
  }
#endif

  if (cmd_handler != NULL) {
    delete_fluid_cmd_handler(cmd_handler);
  }

  if (player != NULL) {
    /* if the user typed 'quit' in the shell, stop the player */
    if (interactive) {
      fluid_player_stop(player);
    } 
    fluid_player_join(player);
    delete_fluid_player(player);
  }

  if (router) {
#if WITH_MIDI
    if (mdriver) {
      delete_fluid_midi_driver(mdriver);
    }
    delete_fluid_midi_router(router);
#endif
  }

  if (adriver) {
    delete_fluid_audio_driver(adriver);
  }

  if (synth) {
    delete_fluid_synth(synth);
  }

  if (settings) {
    delete_fluid_settings(settings);
  }

  return 0;
}

static fluid_cmd_handler_t* newclient(void* data, char* addr)
{
  fluid_synth_t* synth = (fluid_synth_t*) data;
  return new_fluid_cmd_handler(synth);
}


/*
 * print_usage
 */
void 
print_usage() 
{
  printf("Usage: %s [options] [soundfonts]\n", appname);
  printf("Try -h for help.\n");
  exit(0);
}

/*
 * print_welcome
 */
void 
print_welcome() 
{
  printf("fluidsynth version %s\n"
	 "Copyright (C) 2000-2002 Peter Hanappe and others.\n"
	 "FLUID Synth comes with ABSOLUTELY NO WARRANTY.\n"
	 "This is free software, and you are welcome to redistribute it\n"
	 "under certain conditions; see the COPYING file for details.\n"
	 "SoundFont(R) is a registered trademark of E-mu Systems, Inc.\n\n"
	 "Type 'help' to get information on the shell commands.\n\n", FLUIDSYNTH_VERSION);
}

/*
 * print_version
 */
void 
print_version()
{
  printf("fluidsynth %s\n", VERSION);
  exit(0);
}

/*
 * print_help
 */
void 
print_help() 
{
  printf("Usage: \n");
  printf("  fluidsynth [options] [soundfonts] [midifiles]\n");
  printf("Possible options:\n");
  printf(" -n, --no-midi-in\n"
	 "    Don't create a midi driver to read MIDI input events [default = yes]\n\n");
  printf(" -m, --midi-driver=[label]\n"
	 "    The name of the midi driver to use [oss,alsa,alsa_seq,...]\n\n");
  printf(" -K, --midi-channels=[num]\n"
	 "    The number of midi channels [default = 16]\n\n");
  printf(" -a, --audio-driver=[label]\n"
	 "    The audio driver [alsa,jack,oss,dsound,...]\n\n");
  printf(" -j, --connect-jack-outputs\n"
	 "    Attempt to connect the jack outputs to the physical ports\n\n");
  printf(" -L, --audio-channels=[num]\n"
	 "    The number of stereo audio channels [default = 1]\n\n");
  printf(" -z, --audio-bufsize=[size]\n"
	 "    Size of each audio buffer\n\n");
  printf(" -c, --audio-bufcount=[count]\n"
	 "    Number of audio buffers\n\n");
  printf(" -r, --sample-rate\n"
	 "    Set the sample rate\n\n");
  printf(" -R, --reverb\n"
	 "    Turn the reverb on or off [0|1|yes|no, default = on]\n\n");
  printf(" -C, --chorus\n"
	 "    Turn the chorus on or off [0|1|yes|no, default = on]\n\n");
  printf(" -g, --gain\n"
	 "    Set the master gain [0 < gain < 10, default = 0.2]\n\n");
#ifdef HAVE_LADCCA
  printf(" -l, --disable-ladcca\n"
	 "    Don't connect to LADCCA server");
#endif
  printf(" -o\n"
	 "    Define a setting, -o name=value\n\n");
  printf(" -i, --no-shell\n"
	 "    Don't read commands from the shell [default = yes]\n\n");
  printf(" -v, --verbose\n"
	 "    Print out verbose messages about midi events\n\n");
  printf(" -h, --help\n"
	 "    Print out this help summary\n\n");
  printf(" -V, --version\n"
	 "    Show version of program\n\n");
  exit(0);
}

#ifdef HAVE_LADCCA
#include <unistd.h>		/* for usleep() */
#include "fluid_synth.h"	/* JG - until fluid_sfont_get_name is public */

static void
cca_save (fluid_synth_t * synth)
{
  int i;
  int sfcount;
  fluid_sfont_t * sfont;
  cca_config_t * config;
  char num[32];
  
  sfcount = fluid_synth_sfcount (synth);
  for (i = sfcount - 1; i >= 0; i--)
    {
      sfont = fluid_synth_get_sfont (synth, i);
      config = cca_config_new ();
      
      sprintf (num, "%d", i);
      
      cca_config_set_key (config, num);
      cca_config_set_value_string (config, sfont->get_name (sfont));
      
      cca_send_config (fluid_cca_client, config);
    }
}

static void
cca_load (fluid_synth_t * synth, const char * filename)
{
  fluid_synth_sfload (synth, filename, 1);
}

static void *
cca_run (void * data)
{
  cca_event_t * event;
  cca_config_t * config;
  fluid_synth_t * synth;
  
  synth = (fluid_synth_t *) data;
  
  while (cca_enabled (fluid_cca_client))
    {
  
      while ( (event = cca_get_event (fluid_cca_client)) )
        {
          switch (cca_event_get_type (event))
            {
            case CCA_Save_Data_Set:
              cca_save (synth);
              cca_send_event (fluid_cca_client, event);
              break;
            case CCA_Restore_Data_Set:
              cca_send_event (fluid_cca_client, event);
              break;
            case CCA_Quit:
              exit (0);
            default:
              fprintf (stderr, "Recieved unknown LADCCA event of type %d\n", cca_event_get_type (event));
               cca_event_destroy (event);
               break;
            }
        }
  
      while ( (config = cca_get_config (fluid_cca_client)) )
        {
          cca_load (synth, cca_config_get_value_string (config));
          cca_config_destroy (config);
        }
      
      usleep (10000);
    }
  
  return NULL;
}

#endif /* HAVE_LADCCA */


