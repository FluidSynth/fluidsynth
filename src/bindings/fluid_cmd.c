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

#include "fluidsynth_priv.h"
#include "fluid_cmd.h"
#include "fluid_synth.h"
#include "fluid_settings.h"
#include "fluid_hash.h"
#include "fluid_sys.h"
#include "fluid_midi_router.h"
#include "fluid_sfont.h"
#include "fluid_chan.h"

#if WITH_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

/* FIXME: LADSPA used to need a lot of parameters on a single line. This is not
 * necessary anymore, so the limits below could probably be reduced */
#define MAX_TOKENS 100
#define MAX_COMMAND_LEN 1024	/* max command length accepted by fluid_command() */
#define FLUID_WORKLINELENGTH 1024

#define FLUID_ENTRY_COMMAND(data) fluid_cmd_handler_t* handler=(fluid_cmd_handler_t*)(data)

/* the shell cmd handler struct */
struct _fluid_cmd_handler_t {
  fluid_synth_t* synth;
  fluid_midi_router_t* router;
  fluid_cmd_hash_t* commands;

  fluid_midi_router_rule_t *cmd_rule;        /* Rule currently being processed by shell command handler */
  int cmd_rule_type;                         /* Type of the rule (#fluid_midi_router_rule_type) */
};


struct _fluid_shell_t {
  fluid_settings_t* settings;
  fluid_cmd_handler_t* handler;
  fluid_thread_t* thread;
  fluid_istream_t in;
  fluid_ostream_t out;
};


static fluid_thread_return_t fluid_shell_run(void* data);
static void fluid_shell_init(fluid_shell_t* shell,
                             fluid_settings_t* settings, fluid_cmd_handler_t* handler,
                             fluid_istream_t in, fluid_ostream_t out);
static int fluid_handle_voice_count (void* data, int ac, char **av,
                                     fluid_ostream_t out);

void fluid_shell_settings(fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "shell.prompt", "", 0);
  fluid_settings_register_int(settings, "shell.port", 9800, 1, 65535, 0);
}


/** the table of all handled commands */

static const fluid_cmd_t fluid_commands[] = {
  { "help", "general", fluid_handle_help,
    "help                       Show help topics ('help TOPIC' for more info)" },
  { "quit", "general", fluid_handle_quit,
    "quit                       Quit the synthesizer" },
  { "source", "general", fluid_handle_source,
  "source filename            Load a file and parse every line as a command" },
  { "noteon", "event", fluid_handle_noteon,
    "noteon chan key vel        Send noteon" },
  { "noteoff", "event", fluid_handle_noteoff,
    "noteoff chan key           Send noteoff"  },
  { "pitch_bend", "event", fluid_handle_pitch_bend,
    "pitch_bend chan offset           Bend pitch"  },
  { "pitch_bend_range", "event", fluid_handle_pitch_bend_range,
    "pitch_bend chan range           Set bend pitch range"  },
  { "cc", "event", fluid_handle_cc,
    "cc chan ctrl value         Send control-change message" },
  { "prog", "event", fluid_handle_prog,
    "prog chan num              Send program-change message" },
  { "select", "event", fluid_handle_select,
    "select chan sfont bank prog  Combination of bank-select and program-change" },
  { "load", "general", fluid_handle_load,
    "load file [reset] [bankofs] Load SoundFont (reset=0|1, def 1; bankofs=n, def 0)" },
  { "unload", "general", fluid_handle_unload,
    "unload id [reset]          Unload SoundFont by ID (reset=0|1, default 1)"},
  { "reload", "general", fluid_handle_reload,
    "reload id                  Reload the SoundFont with the specified ID" },
  { "fonts", "general", fluid_handle_fonts,
    "fonts                      Display the list of loaded SoundFonts" },
  { "inst", "general", fluid_handle_inst,
    "inst font                  Print out the available instruments for the font" },
  { "channels", "general", fluid_handle_channels,
    "channels [-verbose]        Print out preset of all channels" },
  { "interp", "general", fluid_handle_interp,
    "interp num                 Choose interpolation method for all channels" },
  { "interpc", "general", fluid_handle_interpc,
    "interpc chan num           Choose interpolation method for one channel" },
  { "rev_preset", "reverb", fluid_handle_reverbpreset,
    "rev_preset num             Load preset num into the reverb unit" },
  { "rev_setroomsize", "reverb", fluid_handle_reverbsetroomsize,
    "rev_setroomsize num        Change reverb room size" },
  { "rev_setdamp", "reverb", fluid_handle_reverbsetdamp,
    "rev_setdamp num            Change reverb damping" },
  { "rev_setwidth", "reverb", fluid_handle_reverbsetwidth,
    "rev_setwidth num           Change reverb width" },
  { "rev_setlevel", "reverb", fluid_handle_reverbsetlevel,
    "rev_setlevel num           Change reverb level" },
  { "reverb", "reverb", fluid_handle_reverb,
    "reverb [0|1|on|off]        Turn the reverb on or off" },
  { "cho_set_nr", "chorus", fluid_handle_chorusnr,
    "cho_set_nr n               Use n delay lines (default 3)" },
  { "cho_set_level", "chorus", fluid_handle_choruslevel,
    "cho_set_level num          Set output level of each chorus line to num" },
  { "cho_set_speed", "chorus", fluid_handle_chorusspeed,
    "cho_set_speed num          Set mod speed of chorus to num (Hz)" },
  { "cho_set_depth", "chorus", fluid_handle_chorusdepth,
    "cho_set_depth num          Set chorus modulation depth to num (ms)" },
  { "chorus", "chorus", fluid_handle_chorus,
    "chorus [0|1|on|off]        Turn the chorus on or off" },
  { "gain", "general", fluid_handle_gain,
    "gain value                 Set the master gain (0 < gain < 5)" },
  { "voice_count", "general", fluid_handle_voice_count,
    "voice_count                Get number of active synthesis voices" },
  { "tuning", "tuning", fluid_handle_tuning,
    "tuning name bank prog      Create a tuning with name, bank number, \n"
    "                           and program number (0 <= bank,prog <= 127)" },
  { "tune", "tuning", fluid_handle_tune,
    "tune bank prog key pitch   Tune a key" },
  { "settuning", "tuning", fluid_handle_settuning,
    "settuning chan bank prog   Set the tuning for a MIDI channel" },
  { "resettuning", "tuning", fluid_handle_resettuning,
    "resettuning chan           Restore the default tuning of a MIDI channel" },
  { "tunings", "tuning", fluid_handle_tunings,
    "tunings                    Print the list of available tunings" },
  { "dumptuning", "tuning", fluid_handle_dumptuning,
    "dumptuning bank prog       Print the pitch details of the tuning" },
  { "reset", "general", fluid_handle_reset,
    "reset                      System reset (all notes off, reset controllers)" },
  { "set", "settings", fluid_handle_set,
    "set name value             Set the value of a controller or settings" },
  { "get", "settings", fluid_handle_get,
    "get name                   Get the value of a controller or settings" },
  { "info", "settings", fluid_handle_info,
    "info name                  Get information about a controller or settings" },
  { "settings", "settings", fluid_handle_settings,
    "settings                   Print out all settings" },
  { "echo", "general", fluid_handle_echo,
    "echo arg                   Print arg" },
  /* LADSPA-related commands */
#ifdef LADSPA
  { "ladspa_effect", "ladspa", fluid_handle_ladspa_effect,
    "ladspa_effect              Create a new effect from a LADSPA plugin"},
  { "ladspa_link", "ladspa", fluid_handle_ladspa_link,
    "ladspa_link                Connect an effect port to a host port or buffer"},
  { "ladspa_buffer", "ladspa", fluid_handle_ladspa_buffer,
    "ladspa_buffer              Create a LADSPA buffer"},
  { "ladspa_set", "ladspa", fluid_handle_ladspa_set,
    "ladspa_set                 Set the value of an effect control port"},
  { "ladspa_check", "ladspa", fluid_handle_ladspa_check,
    "ladspa_check               Check LADSPA configuration"},
  { "ladspa_start", "ladspa", fluid_handle_ladspa_start,
    "ladspa_start               Start LADSPA effects"},
  { "ladspa_stop", "ladspa", fluid_handle_ladspa_stop,
    "ladspa_stop                Stop LADSPA effect unit"},
  { "ladspa_reset", "ladspa", fluid_handle_ladspa_reset,
    "ladspa_reset               Stop and reset LADSPA effects"},
#endif
  { "router_clear", "router", fluid_handle_router_clear,
    "router_clear               Clears all routing rules from the midi router"},
  { "router_default", "router", fluid_handle_router_default,
    "router_default             Resets the midi router to default state"},
  { "router_begin", "router", fluid_handle_router_begin,
    "router_begin [note|cc|prog|pbend|cpress|kpress]: Starts a new routing rule"},
  { "router_chan", "router", fluid_handle_router_chan,
    "router_chan min max mul add      filters and maps midi channels on current rule"},
  { "router_par1", "router", fluid_handle_router_par1,
    "router_par1 min max mul add      filters and maps parameter 1 (key/ctrl nr)"},
  { "router_par2", "router", fluid_handle_router_par2,
    "router_par2 min max mul add      filters and maps parameter 2 (vel/cc val)"},
  { "router_end", "router", fluid_handle_router_end,
    "router_end                 closes and commits the current routing rule"}
};

/**
 * Process a string command.
 * NOTE: FluidSynth 1.0.8 and above no longer modifies the 'cmd' string.
 * @param handler FluidSynth command handler
 * @param cmd Command string (NOTE: Gets modified by FluidSynth prior to 1.0.8)
 * @param out Output stream to display command response to
 * @return Integer value corresponding to: -1 on command error, 0 on success,
 *   1 if 'cmd' is a comment or is empty and -2 if quit was issued
 */
int
fluid_command(fluid_cmd_handler_t* handler, const char *cmd, fluid_ostream_t out)
{
  int result, num_tokens = 0;
  char** tokens = NULL;

  if (cmd[0] == '#' || cmd[0] == '\0') {
    return 1;
  }

  if (!g_shell_parse_argv(cmd, &num_tokens, &tokens, NULL)) {
    fluid_ostream_printf(out, "Error parsing command\n");
    return FLUID_FAILED;
  }

  result = fluid_cmd_handler_handle(handler, num_tokens, &tokens[0], out);
  g_strfreev(tokens);

  return result;
}

/**
 * Create a new FluidSynth command shell.
 * @param settings Setting parameters to use with the shell
 * @param handler Command handler
 * @param in Input stream
 * @param out Output stream
 * @param thread TRUE if shell should be run in a separate thread, FALSE to run
 *   it in the current thread (function blocks until "quit")
 * @return New shell instance or NULL on error
 */
fluid_shell_t *
new_fluid_shell(fluid_settings_t* settings, fluid_cmd_handler_t* handler,
                fluid_istream_t in, fluid_ostream_t out, int thread)
{
  fluid_shell_t* shell = FLUID_NEW(fluid_shell_t);
  if (shell == NULL) {
    FLUID_LOG (FLUID_PANIC, "Out of memory");
    return NULL;
  }


  fluid_shell_init(shell, settings, handler, in, out);

  if (thread) {
    shell->thread = new_fluid_thread("shell", fluid_shell_run, shell,
                                     0, TRUE);
    if (shell->thread == NULL) {
      delete_fluid_shell(shell);
      return NULL;
    }
  } else {
    shell->thread = NULL;
    fluid_shell_run(shell);
  }

  return shell;
}

static void
fluid_shell_init(fluid_shell_t* shell,
		 fluid_settings_t* settings, fluid_cmd_handler_t* handler,
		 fluid_istream_t in, fluid_ostream_t out)
{
  shell->settings = settings;
  shell->handler = handler;
  shell->in = in;
  shell->out = out;
}

/**
 * Delete a FluidSynth command shell.
 * @param shell Command shell instance
 */
void
delete_fluid_shell(fluid_shell_t* shell)
{
    fluid_return_if_fail(shell != NULL);
    
  if (shell->thread != NULL) {
    delete_fluid_thread(shell->thread);
  }

  FLUID_FREE(shell);
}

static fluid_thread_return_t
fluid_shell_run(void* data)
{
  fluid_shell_t* shell = (fluid_shell_t*)data;
  char workline[FLUID_WORKLINELENGTH];
  char* prompt = NULL;
  int cont = 1;
  int errors = FALSE;
  int n;

  if (shell->settings)
    fluid_settings_dupstr(shell->settings, "shell.prompt", &prompt);    /* ++ alloc prompt */

  /* handle user input */
  while (cont) {

    n = fluid_istream_readline(shell->in, shell->out, prompt ? prompt : "", workline, FLUID_WORKLINELENGTH);

    if (n < 0) {
      break;
    }

#if WITH_READLINE
    if (shell->in == fluid_get_stdin()) {
      add_history(workline);
    }
#endif

    /* handle the command */
    switch (fluid_command(shell->handler, workline, shell->out)) {

    case 1: /* empty line or comment */
      break;

    case FLUID_FAILED: /* erronous command */
      errors = TRUE;
    case FLUID_OK: /* valid command */
      break;

    case -2: /* quit */
      cont = 0;
      break;
    }

    if (n == 0) {
       break;
    }
  }

  if (prompt) FLUID_FREE (prompt);      /* -- free prompt */

  /* return FLUID_THREAD_RETURN_VALUE on success, something else on failure */
  return errors ? (fluid_thread_return_t)(-1) : FLUID_THREAD_RETURN_VALUE;
}

/**
 * A convenience function to create a shell interfacing to standard input/output
 * console streams.
 * @param settings Settings instance for the shell
 * @param handler Command handler callback
 */
void
fluid_usershell(fluid_settings_t* settings, fluid_cmd_handler_t* handler)
{
  fluid_shell_t shell;
  fluid_shell_init(&shell, settings, handler, fluid_get_stdin(), fluid_get_stdout());
  fluid_shell_run(&shell);
}

/**
 * Execute shell commands in a file.
 * @param handler Command handler callback
 * @param filename File name
 * @return 0 on success, a negative value on error
 */
int
fluid_source(fluid_cmd_handler_t* handler, const char *filename)
{
  int file;
  fluid_shell_t shell;
  int result;

#ifdef WIN32
  file = _open(filename, _O_RDONLY);
#else
  file = open(filename, O_RDONLY);
#endif
  if (file < 0) {
    return file;
  }
  fluid_shell_init(&shell, NULL, handler, file, fluid_get_stdout());
  result = (fluid_shell_run(&shell) == FLUID_THREAD_RETURN_VALUE) ? 0 : -1;

#ifdef WIN32
  _close(file);
#else
  close(file);
#endif

  return result;
}

/**
 * Get the user specific FluidSynth command file name.
 * @param buf Caller supplied string buffer to store file name to.
 * @param len Length of \a buf
 * @return Returns \a buf pointer or NULL if no user command file for this system type.
 */
char*
fluid_get_userconf(char* buf, int len)
{
#if defined(WIN32) || defined(MACOS9)
  return NULL;
#else
  char* home = getenv("HOME");
  if (home == NULL) {
    return NULL;
  } else {
    FLUID_SNPRINTF (buf, len, "%s/.fluidsynth", home);
    return buf;
  }
#endif
}

/**
 * Get the system FluidSynth command file name.
 * @param buf Caller supplied string buffer to store file name to.
 * @param len Length of \a buf
 * @return Returns \a buf pointer or NULL if no system command file for this system type.
 */
char*
fluid_get_sysconf(char* buf, int len)
{
#if defined(WIN32) || defined(MACOS9)
  return NULL;
#else
  FLUID_SNPRINTF (buf, len, "/etc/fluidsynth.conf");
  return buf;
#endif
}


/*
 *  handlers
 */
int
fluid_handle_noteon(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  if (ac < 3) {
    fluid_ostream_printf(out, "noteon: too few arguments\n");
    return FLUID_FAILED;
  }
  if (!fluid_is_number(av[0]) || !fluid_is_number(av[1]) || !fluid_is_number(av[2])) {
    fluid_ostream_printf(out, "noteon: invalid argument\n");
    return FLUID_FAILED;
  }
  return fluid_synth_noteon(handler->synth, atoi(av[0]), atoi(av[1]), atoi(av[2]));
}

int
fluid_handle_noteoff(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  if (ac < 2) {
    fluid_ostream_printf(out, "noteoff: too few arguments\n");
    return FLUID_FAILED;
  }
  if (!fluid_is_number(av[0]) || !fluid_is_number(av[1])) {
    fluid_ostream_printf(out, "noteon: invalid argument\n");
    return FLUID_FAILED;
  }
  return fluid_synth_noteoff(handler->synth, atoi(av[0]), atoi(av[1]));
}

int
fluid_handle_pitch_bend(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  if (ac < 2) {
    fluid_ostream_printf(out, "pitch_bend: too few arguments\n");
    return FLUID_FAILED;
  }
  if (!fluid_is_number(av[0]) || !fluid_is_number(av[1])) {
    fluid_ostream_printf(out, "pitch_bend: invalid argument\n");
    return FLUID_FAILED;
  }
  return fluid_synth_pitch_bend(handler->synth, atoi(av[0]), atoi(av[1]));
}

int
fluid_handle_pitch_bend_range(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int channum;
  int value;
  if (ac < 2) {
    fluid_ostream_printf(out, "pitch_bend_range: too few arguments\n");
    return FLUID_FAILED;
  }
  if (!fluid_is_number(av[0]) || !fluid_is_number(av[1])) {
    fluid_ostream_printf(out, "pitch_bend_range: invalid argument\n");
    return FLUID_FAILED;
  }
  channum = atoi(av[0]);
  value = atoi(av[1]);
  fluid_channel_set_pitch_wheel_sensitivity(handler->synth->channel[channum], value);
  return FLUID_OK;
}

int
fluid_handle_cc(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  if (ac < 3) {
    fluid_ostream_printf(out, "cc: too few arguments\n");
    return FLUID_FAILED;
  }
  if (!fluid_is_number(av[0]) || !fluid_is_number(av[1]) || !fluid_is_number(av[2])) {
    fluid_ostream_printf(out, "cc: invalid argument\n");
    return FLUID_FAILED;
  }
  return fluid_synth_cc(handler->synth, atoi(av[0]), atoi(av[1]), atoi(av[2]));
}

int
fluid_handle_prog(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  if (ac < 2) {
    fluid_ostream_printf(out, "prog: too few arguments\n");
    return FLUID_FAILED;
  }
  if (!fluid_is_number(av[0]) || !fluid_is_number(av[1])) {
    fluid_ostream_printf(out, "prog: invalid argument\n");
    return FLUID_FAILED;
  }
  return fluid_synth_program_change(handler->synth, atoi(av[0]), atoi(av[1]));
}

int
fluid_handle_select(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int sfont_id;
  int chan;
  int bank;
  int prog;

  if (ac < 4) {
    fluid_ostream_printf(out, "preset: too few arguments\n");
    return FLUID_FAILED;
  }
  if (!fluid_is_number(av[0]) || !fluid_is_number(av[1])
      || !fluid_is_number(av[2]) || !fluid_is_number(av[3])) {
    fluid_ostream_printf(out, "preset: invalid argument\n");
    return FLUID_FAILED;
  }

  chan = atoi(av[0]);
  sfont_id = atoi(av[1]);
  bank = atoi(av[2]);
  prog = atoi(av[3]);

  if (sfont_id != 0) {
    return fluid_synth_program_select(handler->synth, chan, sfont_id, bank, prog);
  } else {
    if (fluid_synth_bank_select(handler->synth, chan, bank) == FLUID_OK) {
      return fluid_synth_program_change(handler->synth, chan, prog);
    }
    return FLUID_FAILED;
  }
}

int
fluid_handle_inst(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int font;
  fluid_sfont_t* sfont;
  fluid_preset_t preset;
  int offset;

  if (ac < 1) {
    fluid_ostream_printf(out, "inst: too few arguments\n");
    return FLUID_FAILED;
  }

  if (!fluid_is_number(av[0])) {
    fluid_ostream_printf(out, "inst: invalid argument\n");
    return FLUID_FAILED;
  }

  font = atoi(av[0]);

  sfont = fluid_synth_get_sfont_by_id(handler->synth, font);
  offset = fluid_synth_get_bank_offset(handler->synth, font);

  if (sfont == NULL) {
    fluid_ostream_printf(out, "inst: invalid font number\n");
    return FLUID_FAILED;
  }

  fluid_sfont_iteration_start(sfont);

  while (fluid_sfont_iteration_next(sfont, &preset)) {
    fluid_ostream_printf(out, "%03d-%03d %s\n",
			fluid_preset_get_banknum(&preset) + offset,
			fluid_preset_get_num(&preset),
			fluid_preset_get_name(&preset));
  }

  return FLUID_OK;
}


int
fluid_handle_channels(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_preset_t* preset;
  int verbose = 0;
  int i;

  if (ac > 0 && strcmp( av[0], "-verbose") == 0) verbose = 1;

  for (i = 0; i < fluid_synth_count_midi_channels(handler->synth); i++) {
    preset = fluid_synth_get_channel_preset(handler->synth, i);
    if (preset == NULL) fluid_ostream_printf(out, "chan %d, no preset\n", i);
    else if (!verbose) fluid_ostream_printf(out, "chan %d, %s\n", i, fluid_preset_get_name(preset));
    else fluid_ostream_printf(out, "chan %d, sfont %d, bank %d, preset %d, %s\n", i,
                              fluid_sfont_get_id( preset->sfont),
                              fluid_preset_get_banknum(preset),
                              fluid_preset_get_num(preset),
                              fluid_preset_get_name(preset));
  }

  return FLUID_OK;
}

int
fluid_handle_load(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  char buf[1024];
  int id;
  int reset = 1;
  int offset = 0;

  if (ac < 1) {
    fluid_ostream_printf(out, "load: too few arguments\n");
    return FLUID_FAILED;
  }
  if (ac == 2) {
    reset = atoi(av[1]);
  }
  if (ac == 3) {
    offset = atoi(av[2]);
  }

  /* Load the SoundFont without resetting the programs. The reset will
   * be done later (if requested). */
  id = fluid_synth_sfload(handler->synth, fluid_expand_path(av[0], buf, 1024), 0);

  if (id == -1) {
    fluid_ostream_printf(out, "failed to load the SoundFont\n");
    return FLUID_FAILED;
  } else {
    fluid_ostream_printf(out, "loaded SoundFont has ID %d\n", id);
  }

  if (offset) {
    fluid_synth_set_bank_offset(handler->synth, id, offset);
  }

  /* The reset should be done after the offset is set. */
  if (reset) {
    fluid_synth_program_reset(handler->synth);
  }

  return FLUID_OK;
}

int
fluid_handle_unload(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int reset = 1;
  if (ac < 1) {
    fluid_ostream_printf(out, "unload: too few arguments\n");
    return FLUID_FAILED;
  }
  if (!fluid_is_number(av[0])) {
    fluid_ostream_printf(out, "unload: expected a number as argument\n");
    return FLUID_FAILED;
  }
  if (ac == 2) {
    reset = atoi(av[1]);
  }
  if (fluid_synth_sfunload(handler->synth, atoi(av[0]), reset) != 0) {
    fluid_ostream_printf(out, "failed to unload the SoundFont\n");
    return FLUID_FAILED;
  }
  return FLUID_OK;
}

int
fluid_handle_reload(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  if (ac < 1) {
    fluid_ostream_printf(out, "reload: too few arguments\n");
    return FLUID_FAILED;
  }
  if (!fluid_is_number(av[0])) {
    fluid_ostream_printf(out, "reload: expected a number as argument\n");
    return FLUID_FAILED;
  }
  if (fluid_synth_sfreload(handler->synth, atoi(av[0])) == -1) {
    fluid_ostream_printf(out, "failed to reload the SoundFont\n");
    return FLUID_FAILED;
  }
  return FLUID_OK;
}


int
fluid_handle_fonts(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int i;
  fluid_sfont_t* sfont;
  int num;

  num = fluid_synth_sfcount(handler->synth);

  if (num == 0) {
    fluid_ostream_printf(out, "no SoundFont loaded (try load)\n");
    return FLUID_OK;
  }

  fluid_ostream_printf(out, "ID  Name\n");

  for (i = 0; i < num; i++) {
    sfont = fluid_synth_get_sfont(handler->synth, i);
    if (sfont) {
      fluid_ostream_printf(out, "%2d  %s\n",
                       fluid_sfont_get_id(sfont),
                       fluid_sfont_get_name(sfont));
    }
    else {
      fluid_ostream_printf(out, "sfont is \"NULL\" for index %d\n", i);
    }
  }

  return FLUID_OK;
}

/* Purpose:
 * Response to 'rev_preset' command.
 * Load the values from a reverb preset into the reverb unit. */
int
fluid_handle_reverbpreset(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int reverb_preset_number;
  if (ac < 1) {
    fluid_ostream_printf(out, "rev_preset: too few arguments\n");
    return FLUID_FAILED;
  }
  reverb_preset_number = atoi(av[0]);
  if (fluid_synth_set_reverb_preset(handler->synth, reverb_preset_number)!=FLUID_OK){
    fluid_ostream_printf(out, "rev_preset: Failed. Parameter out of range?\n");
    return FLUID_FAILED;
  };
  return FLUID_OK;
}

/* Purpose:
 * Response to 'rev_setroomsize' command.
 * Load the new room size into the reverb unit. */
int
fluid_handle_reverbsetroomsize(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_real_t room_size;
  if (ac < 1) {
    fluid_ostream_printf(out, "rev_setroomsize: too few arguments.\n");
    return FLUID_FAILED;
  }
  room_size = atof(av[0]);
  if (room_size < 0){
    fluid_ostream_printf(out, "rev_setroomsize: Room size must be positive!\n");
    return FLUID_FAILED;
  }
  if (room_size > 1.0){
    fluid_ostream_printf(out, "rev_setroomsize: Room size too big!\n");
    return FLUID_FAILED;
  }
  fluid_synth_set_reverb_roomsize(handler->synth, room_size);
  return FLUID_OK;
}

/* Purpose:
 * Response to 'rev_setdamp' command.
 * Load the new damp factor into the reverb unit. */
int
fluid_handle_reverbsetdamp(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_real_t damp;
  if (ac < 1) {
    fluid_ostream_printf(out, "rev_setdamp: too few arguments.\n");
    return FLUID_FAILED;
  }
  damp = atof(av[0]);
  if ((damp < 0.0f) || (damp > 1)){
    fluid_ostream_printf(out, "rev_setdamp: damp must be between 0 and 1!\n");
    return FLUID_FAILED;
  }
  fluid_synth_set_reverb_damp(handler->synth, damp);
  return FLUID_OK;
}

/* Purpose:
 * Response to 'rev_setwidth' command.
 * Load the new width into the reverb unit. */
int
fluid_handle_reverbsetwidth(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_real_t width;
  if (ac < 1) {
    fluid_ostream_printf(out, "rev_setwidth: too few arguments.\n");
    return FLUID_FAILED;
  }
  width = atof(av[0]);
  if ((width < 0) || (width > 100)){
    fluid_ostream_printf(out, "rev_setroomsize: Too wide! (0..100)\n");
    return FLUID_FAILED;
  }
  fluid_synth_set_reverb_width(handler->synth, width);
  return FLUID_OK;
}

/* Purpose:
 * Response to 'rev_setlevel' command.
 * Load the new level into the reverb unit. */
int
fluid_handle_reverbsetlevel(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_real_t level;
  if (ac < 1) {
    fluid_ostream_printf(out, "rev_setlevel: too few arguments.\n");
    return FLUID_FAILED;
  }
  level = atof(av[0]);
  if (fabs(level) > 30){
    fluid_ostream_printf(out, "rev_setlevel: Value too high! (Value of 10 =+20 dB)\n");
    return FLUID_FAILED;
  }
  fluid_synth_set_reverb_level(handler->synth, level);
  return FLUID_OK;
}

/* Purpose:
 * Response to 'reverb' command.
 * Change the FLUID_REVERB flag in the synth */
int
fluid_handle_reverb(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  if (ac < 1) {
    fluid_ostream_printf(out, "reverb: too few arguments.\n");
    return FLUID_FAILED;
  }

  if ((strcmp(av[0], "0") == 0) || (strcmp(av[0], "off") == 0)) {
    fluid_synth_set_reverb_on(handler->synth,0);
  } else if ((strcmp(av[0], "1") == 0) || (strcmp(av[0], "on") == 0)) {
    fluid_synth_set_reverb_on(handler->synth,1);
  } else {
    fluid_ostream_printf(out, "reverb: invalid arguments %s [0|1|on|off]", av[0]);
    return FLUID_FAILED;
  }

  return FLUID_OK;
}


/* Purpose:
 * Response to 'chorus_setnr' command */
int
fluid_handle_chorusnr(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int nr;
  if (ac < 1) {
    fluid_ostream_printf(out, "cho_set_nr: too few arguments.\n");
    return FLUID_FAILED;
  }
  nr = atoi(av[0]);
  fluid_synth_set_chorus_nr(handler->synth, nr);
  return FLUID_OK;
}

/* Purpose:
 * Response to 'chorus_setlevel' command */
int
fluid_handle_choruslevel(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_real_t level;
  if (ac < 1) {
    fluid_ostream_printf(out, "cho_set_level: too few arguments.\n");
    return FLUID_FAILED;
  }
  level = atof(av[0]);
  fluid_synth_set_chorus_level(handler->synth, level);
  return FLUID_OK;

}

/* Purpose:
 * Response to 'chorus_setspeed' command */
int
fluid_handle_chorusspeed(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_real_t speed;
  if (ac < 1) {
    fluid_ostream_printf(out, "cho_set_speed: too few arguments.\n");
    return FLUID_FAILED;
  }
  speed = atof(av[0]);
  fluid_synth_set_chorus_speed(handler->synth, speed);
  return FLUID_OK;
}

/* Purpose:
 * Response to 'chorus_setdepth' command */
int
fluid_handle_chorusdepth(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_real_t depth;
  if (ac < 1) {
    fluid_ostream_printf(out, "cho_set_depth: too few arguments.\n");
    return FLUID_FAILED;
  }
  depth = atof(av[0]);
  fluid_synth_set_chorus_depth(handler->synth, depth);
  return FLUID_OK;
}

int
fluid_handle_chorus(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  if (ac < 1) {
    fluid_ostream_printf(out, "chorus: too few arguments\n");
    return FLUID_FAILED;
  }

  if ((strcmp(av[0], "0") == 0) || (strcmp(av[0], "off") == 0)) {
    fluid_synth_set_chorus_on(handler->synth,0);
  } else if ((strcmp(av[0], "1") == 0) || (strcmp(av[0], "on") == 0)) {
    fluid_synth_set_chorus_on(handler->synth,1);
  } else {
    fluid_ostream_printf(out, "chorus: invalid arguments %s [0|1|on|off]", av[0]);
    return FLUID_FAILED;
  }

  return FLUID_OK;
}

/* Purpose:
 * Response to the 'echo' command.
 * The command itself is useful, when the synth is used via TCP/IP.
 * It can signal for example, that a list of commands has been processed.
 */
int
fluid_handle_echo(void* data, int ac, char** av, fluid_ostream_t out)
{
  if (ac < 1) {
    fluid_ostream_printf(out, "echo: too few arguments.\n");
    return FLUID_FAILED;
  }

  fluid_ostream_printf(out, "%s\n",av[0]);

  return FLUID_OK;
}

int
fluid_handle_source(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  if (ac < 1) {
    fluid_ostream_printf(out, "source: too few arguments.\n");
    return FLUID_FAILED;
  }

  fluid_source(handler, av[0]);

  return FLUID_OK;
}

/* Purpose:
 * Response to 'gain' command. */
int
fluid_handle_gain(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  float gain;

  if (ac < 1) {
    fluid_ostream_printf(out, "gain: too few arguments.\n");
    return FLUID_FAILED;
  }

  gain = atof(av[0]);

  if ((gain < 0.0f) || (gain > 5.0f)) {
    fluid_ostream_printf(out, "gain: value should be between '0' and '5'.\n");
    return FLUID_FAILED;
  };

  fluid_synth_set_gain(handler->synth, gain);

  return FLUID_OK;
}

/* Response to voice_count command */
static int
fluid_handle_voice_count (void* data, int ac, char **av,
                          fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_ostream_printf (out, "voice_count: %d\n",
                        fluid_synth_get_active_voice_count (handler->synth));
  return FLUID_OK;
}

/* Purpose:
 * Response to 'interp' command. */
int
fluid_handle_interp(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int interp;
  int chan=-1; /* -1: Set all channels */

  if (ac < 1) {
    fluid_ostream_printf(out, "interp: too few arguments.\n");
    return FLUID_FAILED;
  }

  interp = atoi(av[0]);

  if ((interp < 0) || (interp > FLUID_INTERP_HIGHEST)) {
    fluid_ostream_printf(out, "interp: Bad value\n");
    return FLUID_FAILED;
  };

  fluid_synth_set_interp_method(handler->synth, chan, interp);

  return FLUID_OK;
}

/* Purpose:
 * Response to 'interp' command. */
int
fluid_handle_interpc(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int interp;
  int chan;

  if (ac < 2) {
    fluid_ostream_printf(out, "interpc: too few arguments.\n");
    return FLUID_FAILED;
  }

  chan = atoi(av[0]);
  interp = atoi(av[1]);

  if ((chan < 0) || (chan >= fluid_synth_count_midi_channels(handler->synth))){
    fluid_ostream_printf(out, "interp: Bad value for channel number.\n");
    return FLUID_FAILED;
  };
  if ((interp < 0) || (interp > FLUID_INTERP_HIGHEST)) {
    fluid_ostream_printf(out, "interp: Bad value for interpolation method.\n");
    return FLUID_FAILED;
  };

  fluid_synth_set_interp_method(handler->synth, chan, interp);

  return FLUID_OK;
}

int
fluid_handle_tuning(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  char *name;
  int bank, prog;

  if (ac < 3) {
    fluid_ostream_printf(out, "tuning: too few arguments.\n");
    return FLUID_FAILED;
  }

  name = av[0];

  if (!fluid_is_number(av[1])) {
    fluid_ostream_printf(out, "tuning: 2nd argument should be a number.\n");
    return FLUID_FAILED;
  }
  bank = atoi(av[1]);
  if ((bank < 0) || (bank >= 128)){
    fluid_ostream_printf(out, "tuning: invalid bank number.\n");
    return FLUID_FAILED;
  };

  if (!fluid_is_number(av[2])) {
    fluid_ostream_printf(out, "tuning: 3rd argument should be a number.\n");
    return FLUID_FAILED;
  }
  prog = atoi(av[2]);
  if ((prog < 0) || (prog >= 128)){
    fluid_ostream_printf(out, "tuning: invalid program number.\n");
    return FLUID_FAILED;
  };

  fluid_synth_activate_key_tuning(handler->synth, bank, prog, name, NULL, FALSE);

  return FLUID_OK;
}

int
fluid_handle_tune(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int bank, prog, key;
  double pitch;

  if (ac < 4) {
    fluid_ostream_printf(out, "tune: too few arguments.\n");
    return FLUID_FAILED;
  }

  if (!fluid_is_number(av[0])) {
    fluid_ostream_printf(out, "tune: 1st argument should be a number.\n");
    return FLUID_FAILED;
  }
  bank = atoi(av[0]);
  if ((bank < 0) || (bank >= 128)){
    fluid_ostream_printf(out, "tune: invalid bank number.\n");
    return FLUID_FAILED;
  };

  if (!fluid_is_number(av[1])) {
    fluid_ostream_printf(out, "tune: 2nd argument should be a number.\n");
    return FLUID_FAILED;
  }
  prog = atoi(av[1]);
  if ((prog < 0) || (prog >= 128)){
    fluid_ostream_printf(out, "tune: invalid program number.\n");
    return FLUID_FAILED;
  };

  if (!fluid_is_number(av[2])) {
    fluid_ostream_printf(out, "tune: 3rd argument should be a number.\n");
    return FLUID_FAILED;
  }
  key = atoi(av[2]);
  if ((key < 0) || (key >= 128)){
    fluid_ostream_printf(out, "tune: invalid key number.\n");
    return FLUID_FAILED;
  };

  pitch = atof(av[3]);
  if (pitch < 0.0f) {
    fluid_ostream_printf(out, "tune: invalid pitch.\n");
    return FLUID_FAILED;
  };

  fluid_synth_tune_notes(handler->synth, bank, prog, 1, &key, &pitch, 0);

  return FLUID_OK;
}

int
fluid_handle_settuning(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int chan, bank, prog;

  if (ac < 3) {
    fluid_ostream_printf(out, "settuning: too few arguments.\n");
    return FLUID_FAILED;
  }

  if (!fluid_is_number(av[0])) {
    fluid_ostream_printf(out, "tune: 1st argument should be a number.\n");
    return FLUID_FAILED;
  }
  chan = atoi(av[0]);
  if ((chan < 0) || (chan >= fluid_synth_count_midi_channels(handler->synth))){
    fluid_ostream_printf(out, "tune: invalid channel number.\n");
    return FLUID_FAILED;
  };

  if (!fluid_is_number(av[1])) {
    fluid_ostream_printf(out, "tuning: 2nd argument should be a number.\n");
    return FLUID_FAILED;
  }
  bank = atoi(av[1]);
  if ((bank < 0) || (bank >= 128)){
    fluid_ostream_printf(out, "tuning: invalid bank number.\n");
    return FLUID_FAILED;
  };

  if (!fluid_is_number(av[2])) {
    fluid_ostream_printf(out, "tuning: 3rd argument should be a number.\n");
    return FLUID_FAILED;
  }
  prog = atoi(av[2]);
  if ((prog < 0) || (prog >= 128)){
    fluid_ostream_printf(out, "tuning: invalid program number.\n");
    return FLUID_FAILED;
  };

  fluid_synth_activate_tuning(handler->synth, chan, bank, prog, FALSE);

  return FLUID_OK;
}

int
fluid_handle_resettuning(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int chan;

  if (ac < 1) {
    fluid_ostream_printf(out, "resettuning: too few arguments.\n");
    return FLUID_FAILED;
  }

  if (!fluid_is_number(av[0])) {
    fluid_ostream_printf(out, "tune: 1st argument should be a number.\n");
    return FLUID_FAILED;
  }
  chan = atoi(av[0]);
  if ((chan < 0) || (chan >= fluid_synth_count_midi_channels(handler->synth))){
    fluid_ostream_printf(out, "tune: invalid channel number.\n");
    return FLUID_FAILED;
  };

  fluid_synth_deactivate_tuning(handler->synth, chan, FALSE);

  return FLUID_OK;
}

int
fluid_handle_tunings(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int bank, prog;
  char name[256];
  int count = 0;

  fluid_synth_tuning_iteration_start(handler->synth);

  while (fluid_synth_tuning_iteration_next(handler->synth, &bank, &prog)) {
    fluid_synth_tuning_dump(handler->synth, bank, prog, name, 256, NULL);
    fluid_ostream_printf(out, "%03d-%03d %s\n", bank, prog, name);
    count++;
  }

  if (count == 0) {
    fluid_ostream_printf(out, "No tunings available\n");
  }

  return FLUID_OK;
}

int
fluid_handle_dumptuning(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int bank, prog, i, res;
  double pitch[128];
  char name[256];

  if (ac < 2) {
    fluid_ostream_printf(out, "dumptuning: too few arguments.\n");
    return FLUID_FAILED;
  }

  if (!fluid_is_number(av[0])) {
    fluid_ostream_printf(out, "dumptuning: 1st argument should be a number.\n");
    return FLUID_FAILED;
  }
  bank = atoi(av[0]);
  if ((bank < 0) || (bank >= 128)){
    fluid_ostream_printf(out, "dumptuning: invalid bank number.\n");
    return FLUID_FAILED;
  };

  if (!fluid_is_number(av[1])) {
    fluid_ostream_printf(out, "dumptuning: 2nd argument should be a number.\n");
    return FLUID_FAILED;
  }
  prog = atoi(av[1]);
  if ((prog < 0) || (prog >= 128)){
    fluid_ostream_printf(out, "dumptuning: invalid program number.\n");
    return FLUID_FAILED;
  };

  res = fluid_synth_tuning_dump(handler->synth, bank, prog, name, 256, pitch);
  if (FLUID_OK != res) {
    fluid_ostream_printf(out, "Tuning %03d-%03d does not exist.\n", bank, prog);
    return FLUID_FAILED;
  }

  fluid_ostream_printf(out, "%03d-%03d %s:\n", bank, prog, name);

  for (i = 0; i < 128; i++) {
    fluid_ostream_printf(out, "key %03d, pitch %5.2f\n", i, pitch[i]);
  }

  return FLUID_OK;
}

int
fluid_handle_set(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  int hints;
  int ival;

  if (ac < 2) {
    fluid_ostream_printf(out, "set: Too few arguments.\n");
    return FLUID_FAILED;
  }

  switch (fluid_settings_get_type (handler->synth->settings, av[0]))
  {
    case FLUID_NO_TYPE:
      fluid_ostream_printf (out, "set: Parameter '%s' not found.\n", av[0]);
      break;
    case FLUID_INT_TYPE:
      if (fluid_settings_get_hints (handler->synth->settings, av[0], &hints) == FLUID_OK
          && hints & FLUID_HINT_TOGGLED)
      {
          if (FLUID_STRCASECMP (av[1], "yes") == 0
              || FLUID_STRCASECMP (av[1], "true") == 0
              || FLUID_STRCASECMP (av[1], "t") == 0)
          ival = 1;
          else ival = atoi (av[1]);
      }
      else ival = atoi (av[1]);

      fluid_settings_setint (handler->synth->settings, av[0], ival);
      break;
    case FLUID_NUM_TYPE:
      fluid_settings_setnum (handler->synth->settings, av[0], atof (av[1]));
      break;
    case FLUID_STR_TYPE:
      fluid_settings_setstr(handler->synth->settings, av[0], av[1]);
      break;
    case FLUID_SET_TYPE:
      fluid_ostream_printf (out, "set: Parameter '%s' is a node.\n", av[0]);
      break;
  }

  return FLUID_OK;
}

int
fluid_handle_get(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  if (ac < 1) {
    fluid_ostream_printf(out, "get: too few arguments.\n");
    return FLUID_FAILED;
  }

  switch (fluid_settings_get_type(fluid_synth_get_settings(handler->synth), av[0])) {
  case FLUID_NO_TYPE:
    fluid_ostream_printf(out, "get: no such setting '%s'.\n", av[0]);
    return FLUID_FAILED;

  case FLUID_NUM_TYPE: {
    double value;
    fluid_settings_getnum(handler->synth->settings, av[0], &value);
    fluid_ostream_printf(out, "%.3f", value);
    break;
  }

  case FLUID_INT_TYPE: {
    int value;
    fluid_settings_getint(handler->synth->settings, av[0], &value);
    fluid_ostream_printf(out, "%d", value);
    break;
  }

  case FLUID_STR_TYPE: {
    char* s;
    fluid_settings_dupstr(handler->synth->settings, av[0], &s);       /* ++ alloc string */
    fluid_ostream_printf(out, "%s", s ? s : "NULL");
    if (s) FLUID_FREE (s);      /* -- free string */
    break;
  }

  case FLUID_SET_TYPE:
    fluid_ostream_printf(out, "%s is a node", av[0]);
    break;
  }

  return FLUID_OK;
}

struct _fluid_handle_settings_data_t {
  int len;
  fluid_synth_t* synth;
  fluid_ostream_t out;
};

static void fluid_handle_settings_iter1(void* data, const char* name, int type)
{
  struct _fluid_handle_settings_data_t* d = (struct _fluid_handle_settings_data_t*) data;

  int len = FLUID_STRLEN(name);
  if (len > d->len) {
    d->len = len;
  }
}

static void fluid_handle_settings_iter2(void* data, const char* name, int type)
{
  struct _fluid_handle_settings_data_t* d = (struct _fluid_handle_settings_data_t*) data;

  int len = FLUID_STRLEN(name);
  fluid_ostream_printf(d->out, "%s", name);
  while (len++ < d->len) {
    fluid_ostream_printf(d->out, " ");
  }
  fluid_ostream_printf(d->out, "   ");

  switch (fluid_settings_get_type(fluid_synth_get_settings(d->synth), name)) {
  case FLUID_NUM_TYPE: {
    double value;
    fluid_settings_getnum(d->synth->settings, name, &value);
    fluid_ostream_printf(d->out, "%.3f\n", value);
    break;
  }

  case FLUID_INT_TYPE: {
    int value, hints;
    fluid_settings_getint(d->synth->settings, name, &value);

    if(fluid_settings_get_hints (d->synth->settings, name, &hints) == FLUID_OK)
    {
        if (!(hints & FLUID_HINT_TOGGLED))
        fluid_ostream_printf(d->out, "%d\n", value);
        else fluid_ostream_printf(d->out, "%s\n", value ? "True" : "False");
    }
    break;
  }

  case FLUID_STR_TYPE: {
    char* s;
    fluid_settings_dupstr(d->synth->settings, name, &s);     /* ++ alloc string */
    fluid_ostream_printf(d->out, "%s\n", s ? s : "NULL");
    if (s) FLUID_FREE (s);      /* -- free string */
    break;
  }
  }
}

int
fluid_handle_settings(void* d, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(d);
  struct _fluid_handle_settings_data_t data;

  data.len = 0;
  data.synth = handler->synth;
  data.out = out;

  fluid_settings_foreach(fluid_synth_get_settings(handler->synth), &data, fluid_handle_settings_iter1);
  fluid_settings_foreach(fluid_synth_get_settings(handler->synth), &data, fluid_handle_settings_iter2);
  return FLUID_OK;
}


struct _fluid_handle_option_data_t {
  int first;
  fluid_ostream_t out;
};

void fluid_handle_print_option(void* data, const char* name, const char* option)
{
  struct _fluid_handle_option_data_t* d = (struct _fluid_handle_option_data_t*) data;

  if (d->first) {
    fluid_ostream_printf(d->out, "%s", option);
    d->first = 0;
  } else {
    fluid_ostream_printf(d->out, ", %s", option);
  }
}

int
fluid_handle_info(void* d, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(d);
  fluid_settings_t* settings = fluid_synth_get_settings(handler->synth);
  struct _fluid_handle_option_data_t data;

  if (ac < 1) {
    fluid_ostream_printf(out, "info: too few arguments.\n");
    return FLUID_FAILED;
  }

  switch (fluid_settings_get_type(settings, av[0])) {
  case FLUID_NO_TYPE:
    fluid_ostream_printf(out, "info: no such setting '%s'.\n", av[0]);
    return FLUID_FAILED;

  case FLUID_NUM_TYPE: {
    double value, min, max, def;
    if(fluid_settings_getnum_range(settings, av[0], &min, &max) == FLUID_OK
       && fluid_settings_getnum(settings, av[0], &value) == FLUID_OK
       && fluid_settings_getnum_default(settings, av[0], &def) == FLUID_OK)
    {
        fluid_ostream_printf(out, "%s:\n", av[0]);
        fluid_ostream_printf(out, "Type:          number\n");
        fluid_ostream_printf(out, "Value:         %.3f\n", value);
        fluid_ostream_printf(out, "Minimum value: %.3f\n", min);
        fluid_ostream_printf(out, "Maximum value: %.3f\n", max);
        fluid_ostream_printf(out, "Default value: %.3f\n", def);
        fluid_ostream_printf(out, "Real-time:     %s\n",
                fluid_settings_is_realtime(settings, av[0])? "yes" : "no");
    }
    else
    {
        fluid_ostream_printf(out, "An error occurred when processing %s\n", av[0]);
    }
    break;
  }

  case FLUID_INT_TYPE: {
    int value, min, max, def, hints;

    if(fluid_settings_getint_range(settings, av[0], &min, &max) == FLUID_OK
       && fluid_settings_getint(settings, av[0], &value) == FLUID_OK
       && fluid_settings_get_hints(settings, av[0], &hints) == FLUID_OK
       && fluid_settings_getint_default (settings, av[0], &def) == FLUID_OK)
    {
        fluid_ostream_printf(out, "%s:\n", av[0]);

        if (!(hints & FLUID_HINT_TOGGLED))
        {
        fluid_ostream_printf(out, "Type:          integer\n");
        fluid_ostream_printf(out, "Value:         %d\n", value);
        fluid_ostream_printf(out, "Minimum value: %d\n", min);
        fluid_ostream_printf(out, "Maximum value: %d\n", max);
        fluid_ostream_printf(out, "Default value: %d\n", def);
        }
        else
        {
        fluid_ostream_printf(out, "Type:          boolean\n");
        fluid_ostream_printf(out, "Value:         %s\n", value ? "True" : "False");
        fluid_ostream_printf(out, "Default value: %s\n", def ? "True" : "False");
        }

        fluid_ostream_printf(out, "Real-time:     %s\n",
                fluid_settings_is_realtime(settings, av[0])? "yes" : "no");
    }
    else
    {
        fluid_ostream_printf(out, "An error occurred when processing %s\n", av[0]);
    }
    break;
  }

  case FLUID_STR_TYPE: {
    char *s;
    fluid_settings_dupstr(settings, av[0], &s);         /* ++ alloc string */
    fluid_ostream_printf(out, "%s:\n", av[0]);
    fluid_ostream_printf(out, "Type:          string\n");
    fluid_ostream_printf(out, "Value:         %s\n", s ? s : "NULL");
    fluid_settings_getstr_default(settings, av[0], &s);
    fluid_ostream_printf(out, "Default value: %s\n", s);

    if (s) FLUID_FREE (s);

    data.out = out;
    data.first = 1;
    fluid_ostream_printf(out, "Options:       ");
    fluid_settings_foreach_option (settings, av[0], &data, fluid_handle_print_option);
    fluid_ostream_printf(out, "\n");

    fluid_ostream_printf(out, "Real-time:     %s\n",
			fluid_settings_is_realtime(settings, av[0])? "yes" : "no");
    break;
  }

  case FLUID_SET_TYPE:
    fluid_ostream_printf(out, "%s:\n", av[0]);
    fluid_ostream_printf(out, "Type:          node\n");
    break;
  }

  return FLUID_OK;
}

int
fluid_handle_reset(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_synth_system_reset(handler->synth);
  return FLUID_OK;
}

int
fluid_handle_quit(void* data, int ac, char** av, fluid_ostream_t out)
{
  fluid_ostream_printf(out, "cheers!\n");
  return -2;
}

int
fluid_handle_help(void* data, int ac, char** av, fluid_ostream_t out)
{
  /* Purpose:
   * Prints the help text for the command line commands.
   * Can be used as follows:
   * - help
   * - help (topic), where (topic) is 'general', 'chorus', etc.
   * - help all
   */
  char* topic = "help"; /* default, if no topic is given */
  int count = 0;
  unsigned int i;

  fluid_ostream_printf(out, "\n");
  /* 1st argument (optional): help topic */
  if (ac >= 1) {
    topic = av[0];
  }
  if (strcmp(topic,"help") == 0){
    /* "help help": Print a list of all topics */
    fluid_ostream_printf(out,
			"*** Help topics:***\n"
			"help all (prints all topics)\n");
    for (i = 0; i < FLUID_N_ELEMENTS(fluid_commands); i++) {
      int listed_first_time = 1;
      unsigned int ii;
      for (ii = 0; ii < i; ii++){
	if (strcmp(fluid_commands[i].topic, fluid_commands[ii].topic) == 0){
	  listed_first_time = 0;
	}; /* if topic has already been listed */
      }; /* for all topics (inner loop) */
      if (listed_first_time){
	fluid_ostream_printf(out, "help %s\n",fluid_commands[i].topic);
      };
    }; /* for all topics (outer loop) */
  } else {
    /* help (arbitrary topic or "all") */
    for (i = 0; i < FLUID_N_ELEMENTS(fluid_commands); i++) {
      if (fluid_commands[i].help != NULL) {
	if (strcmp(topic,"all") == 0 || strcmp(topic,fluid_commands[i].topic) == 0){
	  fluid_ostream_printf(out, "%s\n", fluid_commands[i].help);
	  count++;
	}; /* if it matches the topic */
      }; /* if help text exists */
    }; /* foreach command */
    if (count == 0){
      fluid_ostream_printf(out, "Unknown help topic. Try 'help help'.\n");
    };
  };
  return FLUID_OK;
}

#define CHECK_VALID_ROUTER(_router, _out)                                                \
  if (router == NULL) {                                                                  \
    fluid_ostream_printf(out, "cannot execute router command without a midi router.\n"); \
    return FLUID_FAILED;                                                                 \
  }

/* Command handler for "router_clear" command */
int fluid_handle_router_clear(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_midi_router_t *router = handler->router;

  if (ac != 0) {
    fluid_ostream_printf (out, "router_clear needs no arguments.\n");
    return FLUID_FAILED;
  }

  CHECK_VALID_ROUTER (router, out);

  fluid_midi_router_clear_rules (router);

  return FLUID_OK;
}

/* Command handler for "router_default" command */
int fluid_handle_router_default(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_midi_router_t *router = handler->router;

  if (ac != 0) {
    fluid_ostream_printf(out, "router_default needs no arguments.\n");
    return FLUID_FAILED;
  }

  CHECK_VALID_ROUTER (router, out);

  fluid_midi_router_set_default_rules (router);

  return FLUID_OK;
}

/* Command handler for "router_begin" command */
int fluid_handle_router_begin(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_midi_router_t* router = handler->router;

  if (ac != 1) {
    fluid_ostream_printf (out, "router_begin requires [note|cc|prog|pbend|cpress|kpress]\n");
    return FLUID_FAILED;
  }

  CHECK_VALID_ROUTER (router, out);

  if (FLUID_STRCMP (av[0], "note") == 0)
    handler->cmd_rule_type = FLUID_MIDI_ROUTER_RULE_NOTE;
  else if (FLUID_STRCMP (av[0], "cc") == 0)
    handler->cmd_rule_type = FLUID_MIDI_ROUTER_RULE_CC;
  else if (FLUID_STRCMP (av[0], "prog") == 0)
    handler->cmd_rule_type = FLUID_MIDI_ROUTER_RULE_PROG_CHANGE;
  else if (FLUID_STRCMP (av[0], "pbend") == 0)
    handler->cmd_rule_type = FLUID_MIDI_ROUTER_RULE_PITCH_BEND;
  else if (FLUID_STRCMP (av[0], "cpress") == 0)
    handler->cmd_rule_type = FLUID_MIDI_ROUTER_RULE_CHANNEL_PRESSURE;
  else if (FLUID_STRCMP (av[0], "kpress") == 0)
    handler->cmd_rule_type = FLUID_MIDI_ROUTER_RULE_KEY_PRESSURE;
  else
  {
    fluid_ostream_printf (out, "router_begin requires [note|cc|prog|pbend|cpress|kpress]\n");
    return FLUID_FAILED;
  }

  if (handler->cmd_rule)
    delete_fluid_midi_router_rule (handler->cmd_rule);

  handler->cmd_rule = new_fluid_midi_router_rule ();

  if (!handler->cmd_rule)
    return FLUID_FAILED;

  return FLUID_OK;
}

/* Command handler for "router_end" command */
int fluid_handle_router_end(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_midi_router_t* router = handler->router;

  if (ac != 0) {
    fluid_ostream_printf (out, "router_end needs no arguments.\n");
    return FLUID_FAILED;
  }

  CHECK_VALID_ROUTER (router, out);

  if (!handler->cmd_rule)
  {
    fluid_ostream_printf (out, "No active router_begin command.\n");
    return FLUID_FAILED;
  }

  /* Add the rule */
  if (fluid_midi_router_add_rule (router, handler->cmd_rule, handler->cmd_rule_type) != FLUID_OK)
    delete_fluid_midi_router_rule (handler->cmd_rule);   /* Free on failure */

  handler->cmd_rule = NULL;

  return FLUID_OK;
}

/* Command handler for "router_chan" command */
int fluid_handle_router_chan(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_midi_router_t* router = handler->router;

  if (ac != 4) {
    fluid_ostream_printf(out, "router_chan needs four args: min, max, mul, add.");
    return FLUID_FAILED;
  }

  CHECK_VALID_ROUTER (router, out);

  if (!handler->cmd_rule)
  {
    fluid_ostream_printf (out, "No active router_begin command.\n");
    return FLUID_FAILED;
  }

  fluid_midi_router_rule_set_chan (handler->cmd_rule, atoi (av[0]), atoi (av[1]),
                                   atof (av[2]), atoi (av[3]));
  return FLUID_OK;
}

/* Command handler for "router_par1" command */
int fluid_handle_router_par1(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_midi_router_t* router = handler->router;

  if (ac != 4) {
    fluid_ostream_printf(out, "router_par1 needs four args: min, max, mul, add.");
    return FLUID_FAILED;
  }

  CHECK_VALID_ROUTER (router, out);

  if (!handler->cmd_rule)
  {
    fluid_ostream_printf (out, "No active router_begin command.\n");
    return FLUID_FAILED;
  }

  fluid_midi_router_rule_set_param1 (handler->cmd_rule, atoi (av[0]), atoi (av[1]),
                                     atof (av[2]), atoi (av[3]));
  return FLUID_OK;
}

/* Command handler for "router_par2" command */
int fluid_handle_router_par2(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_midi_router_t* router = handler->router;

  if (ac != 4) {
    fluid_ostream_printf(out, "router_par2 needs four args: min, max, mul, add.");
    return FLUID_FAILED;
  }

  CHECK_VALID_ROUTER (router, out);

  if (!handler->cmd_rule)
  {
    fluid_ostream_printf (out, "No active router_begin command.\n");
    return FLUID_FAILED;
  }

  fluid_midi_router_rule_set_param2 (handler->cmd_rule, atoi (av[0]), atoi (av[1]),
                                     atof (av[2]), atoi (av[3]));
  return FLUID_OK;
}

#ifdef LADSPA

#define CHECK_LADSPA_ENABLED(_fx, _out) \
  if (_fx == NULL) \
  { \
    fluid_ostream_printf(_out, "LADSPA is not enabled.\n"); \
    return FLUID_FAILED; \
  }

#define CHECK_LADSPA_INACTIVE(_fx, _out) \
  if (fluid_ladspa_is_active(_fx)) \
  { \
    fluid_ostream_printf(_out, "LADSPA already started.\n"); \
    return FLUID_FAILED; \
  }

#define LADSPA_ERR_LEN (1024)

/**
 * ladspa_start
 */
int fluid_handle_ladspa_start(void* data, int ac, char **av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;
    char error[LADSPA_ERR_LEN];

    if (ac != 0)
    {
        fluid_ostream_printf(out, "ladspa_start does not accept any arguments\n");
        return FLUID_FAILED;
    }

    CHECK_LADSPA_ENABLED(fx, out);
    CHECK_LADSPA_INACTIVE(fx, out);

    if (fluid_ladspa_check(fx, error, LADSPA_ERR_LEN) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Unable to start LADSPA: %s", error);
        return FLUID_FAILED;
    }

    if (fluid_ladspa_activate(fx) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Unable to start LADSPA.\n");
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

/**
 * ladspa_stop
 */
int fluid_handle_ladspa_stop(void* data, int ac, char **av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;

    if (ac != 0)
    {
        fluid_ostream_printf(out, "ladspa_stop does not accept any arguments\n");
        return FLUID_FAILED;
    }

    CHECK_LADSPA_ENABLED(fx, out);

    if (!fluid_ladspa_is_active(fx))
    {
        fluid_ostream_printf(out, "LADSPA has not been started.\n");
    }

    if (fluid_ladspa_deactivate(fx) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Unable to stop LADSPA.\n");
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

/**
 * ladspa_reset
 */
int fluid_handle_ladspa_reset(void* data, int ac, char **av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;

    if (ac != 0)
    {
        fluid_ostream_printf(out, "ladspa_reset does not accept any arguments\n");
        return FLUID_FAILED;
    }

    CHECK_LADSPA_ENABLED(fx, out);

    fluid_ladspa_reset(fx);

    return FLUID_OK;
}

/**
 * ladspa_check
 */
int fluid_handle_ladspa_check(void* data, int ac, char **av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;
    char error[LADSPA_ERR_LEN];

    if (ac != 0)
    {
        fluid_ostream_printf(out, "ladspa_reset does not accept any arguments\n");
        return FLUID_FAILED;
    }

    CHECK_LADSPA_ENABLED(fx, out);

    if (fluid_ladspa_check(fx, error, LADSPA_ERR_LEN) != FLUID_OK)
    {
        fluid_ostream_printf(out, "LADSPA check failed: %s", error);
        return FLUID_FAILED;
    }

    fluid_ostream_printf(out, "LADSPA check ok\n");

    return FLUID_OK;
}

/**
 * ladspa_set <effect> <port> <value>
 */
int fluid_handle_ladspa_set(void *data, int ac, char **av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;

    if (ac != 3)
    {
        fluid_ostream_printf(out, "ladspa_set needs three arguments: <effect> <port> <value>\n");
        return FLUID_FAILED;
    };

    CHECK_LADSPA_ENABLED(fx, out);

    /* Redundant check, just here to give a more detailed error message */
    if (!fluid_ladspa_effect_port_exists(fx, av[0], av[1]))
    {
        fluid_ostream_printf(out, "Port '%s' not found on effect '%s'\n", av[1], av[0]);
        return FLUID_FAILED;
    }

    if (fluid_ladspa_effect_set_control(fx, av[0], av[1], atof(av[2])) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Failed to set port '%s' on effect '%s', "
                "maybe it is not a control port?\n", av[1], av[0]);
        return FLUID_FAILED;
    }

    return FLUID_OK;
};

/**
 * ladspa_buffer <name>
 */
int fluid_handle_ladspa_buffer(void *data, int ac, char **av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;

    if (ac != 1)
    {
        fluid_ostream_printf(out, "ladspa_buffer needs one argument: <name>\n");
        return FLUID_FAILED;
    };

    CHECK_LADSPA_ENABLED(fx, out);
    CHECK_LADSPA_INACTIVE(fx, out);

    if (fluid_ladspa_add_buffer(fx, av[0]) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Failed to add buffer\n");
        return FLUID_FAILED;
    }

    return FLUID_OK;
};

/**
 * ladspa_effect <name> <library> [plugin] [--mix [gain]]
 */
int fluid_handle_ladspa_effect(void* data, int ac, char **av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;
    char *plugin_name = NULL;
    int pos;
    int mix = FALSE;
    float gain = 1.0f;

    if (ac < 2 || ac > 5)
    {
        fluid_ostream_printf(out, "ladspa_effect invalid arguments: "
                "<name> <library> [plugin] [--mix [gain]]\n");
        return FLUID_FAILED;
    }

    pos = 2;
    /* If the first optional arg is not --mix, then it must be the plugin label */
    if ((pos < ac) && (FLUID_STRCMP(av[pos], "--mix") != 0))
    {
        plugin_name = av[pos];
        pos++;
    }

    /* If this optional arg is --mix and there's an argument after it, that that
     * must be the gain */
    if ((pos < ac) && (FLUID_STRCMP(av[pos], "--mix") == 0))
    {
        mix = TRUE;
        if (pos + 1 < ac)
        {
            gain = atof(av[pos + 1]);
        }
    }

    CHECK_LADSPA_ENABLED(fx, out);
    CHECK_LADSPA_INACTIVE(fx, out);

    if (fluid_ladspa_add_effect(fx, av[0], av[1], plugin_name) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Failed to create effect\n");
        return FLUID_FAILED;
    }

    if (mix)
    {
        if (!fluid_ladspa_effect_can_mix(fx, av[0]))
        {
            fluid_ostream_printf(out, "Effect '%s' does not support --mix mode\n", av[0]);
            return FLUID_FAILED;
        }

        if (fluid_ladspa_effect_set_mix(fx, av[0], mix, gain) != FLUID_OK)
        {
            fluid_ostream_printf(out, "Failed to set --mix mode\n");
            return FLUID_FAILED;
        }
    }

    return FLUID_OK;
}

/*
 * ladspa_link <effect> <port> <buffer or host port>
 */
int fluid_handle_ladspa_link(void* data, int ac, char **av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;

    if (ac != 3)
    {
        fluid_ostream_printf(out, "ladspa_link needs 3 arguments: "
                "<effect> <port> <buffer or host name>\n");
        return FLUID_FAILED;
    }

    CHECK_LADSPA_ENABLED(fx, out);
    CHECK_LADSPA_INACTIVE(fx, out);

    if (!fluid_ladspa_effect_port_exists(fx, av[0], av[1]))
    {
        fluid_ostream_printf(out, "Port '%s' not found on effect '%s'\n", av[1], av[0]);
        return FLUID_FAILED;
    }

    if (!fluid_ladspa_host_port_exists(fx, av[2]) && !fluid_ladspa_buffer_exists(fx, av[2]))
    {
        fluid_ostream_printf(out, "Host port or buffer '%s' not found.\n", av[2]);
        return FLUID_FAILED;
    }

    if (fluid_ladspa_effect_link(fx, av[0], av[1], av[2]) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Failed to link port\n");
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

#endif /* LADSPA */

int
fluid_is_number(char* a)
{
  while (*a != 0) {
    if (((*a < '0') || (*a > '9')) && (*a != '-') && (*a != '+') && (*a != '.')) {
      return FALSE;
    }
    a++;
  }
  return TRUE;
}

char*
fluid_expand_path(char* path, char* new_path, int len)
{
#if defined(WIN32) || defined(MACOS9)
  FLUID_SNPRINTF (new_path, len - 1, "%s", path);
#else
  if ((path[0] == '~') && (path[1] == '/')) {
    char* home = getenv("HOME");
    if (home == NULL) {
      FLUID_SNPRINTF (new_path, len - 1, "%s", path);
    } else {
      FLUID_SNPRINTF (new_path, len - 1, "%s%s", home, &path[1]);
    }
  } else {
    FLUID_SNPRINTF (new_path, len - 1, "%s", path);
  }
#endif

  new_path[len - 1] = 0;
  return new_path;
}



/*
 * Command
 */

fluid_cmd_t* fluid_cmd_copy(const fluid_cmd_t* cmd)
{
  fluid_cmd_t* copy = FLUID_NEW(fluid_cmd_t);
  if (copy == NULL) {
    FLUID_LOG (FLUID_PANIC, "Out of memory");
    return NULL;
  }

  copy->name = FLUID_STRDUP(cmd->name);
  copy->topic = FLUID_STRDUP(cmd->topic);
  copy->help = FLUID_STRDUP(cmd->help);
  copy->handler = cmd->handler;
  return copy;
}

void delete_fluid_cmd(fluid_cmd_t* cmd)
{
    fluid_return_if_fail(cmd != NULL);
    FLUID_FREE(cmd->name);
    FLUID_FREE(cmd->topic);
    FLUID_FREE(cmd->help);
    FLUID_FREE(cmd);
}

/*
 * Command handler
 */

static void
fluid_cmd_handler_destroy_hash_value (void *value)
{
  delete_fluid_cmd ((fluid_cmd_t *)value);
}

/**
 * Create a new command handler.
 * @param synth If not NULL, all the default synthesizer commands will be added to the new handler.
 * @param router If not NULL, all the default midi_router commands will be added to the new handler.
 * @return New command handler, or NULL if alloc failed
 */
fluid_cmd_handler_t* new_fluid_cmd_handler(fluid_synth_t* synth, fluid_midi_router_t* router)
{
  unsigned int i;
  fluid_cmd_handler_t* handler;

  handler = FLUID_NEW(fluid_cmd_handler_t);
  if (handler == NULL) {
    return NULL;
  }
  handler->commands = new_fluid_hashtable_full (fluid_str_hash, fluid_str_equal,
                                        NULL, fluid_cmd_handler_destroy_hash_value);
  if (handler->commands == NULL) {
    FLUID_FREE(handler);
    return NULL;
  }

  handler->synth = synth;
  handler->router = router;

  if (synth != NULL) {
    for (i = 0; i < FLUID_N_ELEMENTS(fluid_commands); i++)
    {
        fluid_cmd_handler_register(handler, &fluid_commands[i]);
    }
  }

  return handler;
}

/**
 * Delete a command handler.
 * @param handler Command handler to delete
 */
void
delete_fluid_cmd_handler(fluid_cmd_handler_t* handler)
{
    fluid_return_if_fail(handler != NULL);
    
  delete_fluid_hashtable(handler->commands);
  FLUID_FREE(handler);
}

/**
 * Register a new command to the handler.
 * @param handler Command handler instance
 * @param cmd Command info (gets copied)
 * @return #FLUID_OK if command was inserted, #FLUID_FAILED otherwise
 */
int
fluid_cmd_handler_register(fluid_cmd_handler_t* handler, const fluid_cmd_t* cmd)
{
  fluid_cmd_t* copy = fluid_cmd_copy(cmd);
  fluid_hashtable_insert(handler->commands, copy->name, copy);
  return FLUID_OK;
}

/**
 * Unregister a command from a command handler.
 * @param handler Command handler instance
 * @param cmd Name of the command
 * @return TRUE if command was found and unregistered, FALSE otherwise
 */
int
fluid_cmd_handler_unregister(fluid_cmd_handler_t* handler, const char *cmd)
{
  return fluid_hashtable_remove(handler->commands, cmd);
}

int
fluid_cmd_handler_handle(void* data, int ac, char** av, fluid_ostream_t out)
{
  FLUID_ENTRY_COMMAND(data);
  fluid_cmd_t* cmd;

  cmd = fluid_hashtable_lookup(handler->commands, av[0]);

  if (cmd && cmd->handler)
    return (*cmd->handler)(handler, ac - 1, av + 1, out);

  fluid_ostream_printf(out, "unknown command: %s (try help)\n", av[0]);
  return FLUID_FAILED;
}


#ifdef NETWORK_SUPPORT

struct _fluid_server_t {
  fluid_server_socket_t* socket;
  fluid_settings_t* settings;
  fluid_synth_t* synth;
  fluid_midi_router_t* router;
  fluid_list_t* clients;
  fluid_mutex_t mutex;
};

static void fluid_server_close(fluid_server_t* server)
{
  fluid_list_t* list;
  fluid_list_t* clients;
  fluid_client_t* client;

  fluid_return_if_fail(server != NULL);

  fluid_mutex_lock(server->mutex);
  clients = server->clients;
  server->clients = NULL;
  fluid_mutex_unlock(server->mutex);

  list = clients;

  while (list) {
    client = fluid_list_get(list);
    fluid_client_quit(client);
    list = fluid_list_next(list);
  }

  delete_fluid_list(clients);

  if (server->socket) {
    delete_fluid_server_socket(server->socket);
    server->socket = NULL;
  }
}

static int
fluid_server_handle_connection(fluid_server_t* server, fluid_socket_t client_socket, char* addr)
{
  fluid_client_t* client;

  client = new_fluid_client(server, server->settings, client_socket);
  if (client == NULL) {
    return -1;
  }
  fluid_server_add_client(server, client);

  return 0;
}

void fluid_server_add_client(fluid_server_t* server, fluid_client_t* client)
{
  fluid_mutex_lock(server->mutex);
  server->clients = fluid_list_append(server->clients, client);
  fluid_mutex_unlock(server->mutex);
}

void fluid_server_remove_client(fluid_server_t* server, fluid_client_t* client)
{
  fluid_mutex_lock(server->mutex);
  server->clients = fluid_list_remove(server->clients, client);
  fluid_mutex_unlock(server->mutex);
}

struct _fluid_client_t {
  fluid_server_t* server;
  fluid_settings_t* settings;
  fluid_cmd_handler_t* handler;
  fluid_socket_t socket;
  fluid_thread_t* thread;
};


static fluid_thread_return_t fluid_client_run(void* data)
{
  fluid_shell_t shell;
  fluid_client_t* client = (fluid_client_t*)data;
  
  fluid_shell_init(&shell,
		  client->settings,
		  client->handler,
		  fluid_socket_get_istream(client->socket),
		  fluid_socket_get_ostream(client->socket));
  fluid_shell_run(&shell);
  fluid_server_remove_client(client->server, client);
  delete_fluid_client(client);
  
  return FLUID_THREAD_RETURN_VALUE;
}


fluid_client_t*
new_fluid_client(fluid_server_t* server, fluid_settings_t* settings, fluid_socket_t sock)
{
  fluid_client_t* client;

  client = FLUID_NEW(fluid_client_t);
  if (client == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  client->server = server;
  client->socket = sock;
  client->settings = settings;
  client->handler = new_fluid_cmd_handler(server->synth, server->router);
  client->thread = new_fluid_thread("client", fluid_client_run, client,
                                    0, FALSE);

  if (client->handler == NULL || client->thread == NULL) {
    goto error_recovery;
  }

  return client;

error_recovery:
  FLUID_LOG(FLUID_ERR, "Out of memory");
  delete_fluid_client(client);
  return NULL;

}

void fluid_client_quit(fluid_client_t* client)
{
    fluid_socket_close(client->socket);
    
  FLUID_LOG(FLUID_DBG, "fluid_client_quit: joining");
  fluid_thread_join(client->thread);
  FLUID_LOG(FLUID_DBG, "fluid_client_quit: done");
}

void delete_fluid_client(fluid_client_t* client)
{
    fluid_return_if_fail(client != NULL);
    
    delete_fluid_cmd_handler(client->handler);
    fluid_socket_close(client->socket);
    delete_fluid_thread(client->thread);
    
  FLUID_FREE(client);
}

#endif /* NETWORK_SUPPORT */

/**
 * Create a new TCP/IP command shell server.
 * @param settings Settings instance to use for the shell
 * @param synth If not NULL, the synth instance for the command handler to be used by the client
 * @param router If not NULL, the midi_router instance for the command handler to be used by the client
 * @return New shell server instance or NULL on error
 */
fluid_server_t*
new_fluid_server(fluid_settings_t* settings,
		fluid_synth_t* synth, fluid_midi_router_t* router)
{
#ifdef NETWORK_SUPPORT
  fluid_server_t* server;
  int port;

  server = FLUID_NEW(fluid_server_t);
  if (server == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  server->settings = settings;
  server->clients = NULL;
  server->synth = synth;
  server->router = router;

  fluid_mutex_init(server->mutex);

  fluid_settings_getint(settings, "shell.port", &port);

  server->socket = new_fluid_server_socket(port,
					  (fluid_server_func_t) fluid_server_handle_connection,
					  server);
  if (server->socket == NULL) {
    FLUID_FREE(server);
    return NULL;
  }

  return server;
#else
  FLUID_LOG(FLUID_WARN, "Network support disabled on this platform.");
  return NULL;
#endif
}

/**
 * Delete a TCP/IP shell server.
 * @param server Shell server instance
 */
void
delete_fluid_server(fluid_server_t* server)
{
#ifdef NETWORK_SUPPORT
  fluid_return_if_fail(server != NULL);

  fluid_server_close(server);

  FLUID_FREE(server);
#endif
}

/**
 * Join a shell server thread (wait until it quits).
 * @param server Shell server instance
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int fluid_server_join(fluid_server_t* server)
{
#ifdef NETWORK_SUPPORT
  return fluid_server_socket_join(server->socket);
#else
  return FLUID_FAILED;
#endif
}
