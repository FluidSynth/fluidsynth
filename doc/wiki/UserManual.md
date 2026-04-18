# 📖 User Manual

This document has the same layout as the manpage, but it contains links to other pages for more information: [FluidFeatures](FluidFeatures.md) and [FluidSettings](FluidSettings.md).

## NAME

FluidSynth - a [SoundFont](SoundFont.md) synthesizer 

## USAGE

`fluidsynth [options] [soundFonts] [ midifiles ]`

## DESCRIPTION

FluidSynth is a real-time MIDI synthesizer based on the [SoundFont](SoundFont.md)&reg; 2 specifications. It can be used to render MIDI input or MIDI files to audio. The MIDI events are read from a MIDI device. The sound is rendered in real-time to the sound output device. 
See [FluidFeatures](FluidFeatures.md) for a comprehensive list of features implemented and working. 

The easiest way to start the synthesizer is to give it a [SoundFont](SoundFont.md) on the command line: `fluidsynth output.sf2`. FluidSynth will load the [SoundFont](SoundFont.md) and read MIDI events from the default MIDI device using the default MIDI driver. Once FluidSynth is running, it reads commands from stdin. There are commands to send MIDI events manually, to load or unload [SoundFonts](SoundFont.md), and so forth. Available commands are [discussed below](#shell-commands). 

FluidSynth can also be used to play a list of MIDI files. Simply run FluidSynth with the [SoundFont](SoundFont.md) and the list of MIDI files to play. In this case, you might not want to open the MIDI device to read external events. Use the `-n` option to deactivate MIDI input. You may also want to deactivate the shell causing FluidSynth to quit as soon as all MIDI files have been played. Start FluidSynth with the `-i` option to do so:

```shell
fluidsynth -ni soundfont.sf2 midifile1.mid midifile2.mid
``` 

Run FluidSynth with the `--help` option to check for changes in the list of options. 

For some scenarios, it might be useful to execute certain shell commands right upon starting FluidSynth, e.g. to do some custom default initialization of the synth, change the default audio driver in use, etc. This can be achieved with a configuration file as described below. If no such file is provided via the `-f` command-line argument, FluidSynth tries to load a user-specific configuration file (as given by [fluid_get_userconf()](https://www.fluidsynth.org/api/group__command__interface.html)). If that fails, it tries to load a system-wide configuration file (as given by [fluid_get_sysconf()](https://www.fluidsynth.org/api/group__command__interface.html)).

## OPTIONS

FluidSynth accepts the following options (call FluidSynth with the --help option to get most up-to-date information): 

-a, --audio-driver=[label]


> The audio driver to use. "-a help" to list valid options 

-b, --bank-offset=[num]

> A positional flag that specifies the bank-offset for any Soundfonts following that flag. Can be specified multiple times. Available since FluidSynth 2.5.0. Example:
>
> fluidsynth A.sf2 -b 1 someTune.mid B.sf2 C.sf2 -b 2 D.sf2
>
> Would cause A.sf2 to be loaded with a bank-offset of zero, B.sf2 and C.sf2 loaded with bank-offset=1, and D.sf2 loaded with bank-offset=2.


-C, --chorus


> Turn the chorus on or off [0|1|yes|no, default = on] 

-c, --audio-bufcount=[count]


> Number of audio buffers 

-d, --dump


> Dump incoming and outgoing MIDI events to stdout 

-E, --audio-file-endian


> Audio file endian for fast rendering or aufile driver ("-E help" for list) 

-f, --load-config


> Right upon starting, load and execute a configuration file containing fluidsynth related shell commands as described [in the Section below](https://github.com/FluidSynth/fluidsynth/wiki/UserManual#shell-commands). 

-F, --fast-render=[file]


> Render MIDI file to raw audio data and store in [file] 

-G, --audio-groups


> Defines the number of LADSPA audio nodes 

-g, --gain


> Set the master gain [0 &lt; gain &lt; 10, default = 0.2] 

-h, --help


> Print out this help summary 

-i, --no-shell


> Don't read commands from the shell [default = yes] 

-j, --connect-jack-outputs


> Attempt to connect the jack outputs to the physical ports 

-K, --midi-channels=[num]


> The number of midi channels [default = 16] 

-L, --audio-channels=[num]


> The number of stereo audio channels [default = 1] 

-l, --disable-lash


> Don't connect to LASH server 

-m, --midi-driver=[label]


> The name of the midi driver to use [oss,alsa,alsa_seq,...] 

-n, --no-midi-in


> Don't create a midi driver to read MIDI input events [default = yes] 

-O, --audio-file-format


> Audio file format for fast rendering or aufile driver ("-O help" for list) 

-o


> Define a setting, -o name=value ("-o help" to dump current values). See [FluidSettings](FluidSettings.md) for details 

-p, --portname=[label]

> Set MIDI port name (alsa_seq, coremidi drivers) 

-q, --quiet

> Do not print welcome message or other informational output. (Windows only: also suppress all log messages lower than PANIC)

-Q, --query-audio-devices

> Windows only, enumerate available WASAPI devices and their supported sample rates


-R, --reverb


> Turn the reverb on or off [0|1|yes|no, default = on] 

-r, --sample-rate


> Set the sample rate 

-s, --server


> Start FluidSynth as a server process 

-T, --audio-file-type


> Audio file type for fast rendering or aufile driver ("-T help" for list) 

-v, --verbose


> Print out verbose messages about midi events (synth.verbose=1) as well as other debug messages

-V, --version


> Show version of program 

-z, --audio-bufsize=[size]


> Size of each audio buffer 

## SETTINGS

All the settings that can be passed with the `-o` flag to FluidSynth are known as [FluidSettings](FluidSettings.md). Refer to the wiki page for more details.

## SHELL COMMANDS

When starting FluidSynth an interactive shell opens. This section describes the most common commands.

### GENERAL

- `help`  
  Shows help topics (`help TOPIC` for more info)

- `help help`  
  Shows a list of help topics (`help <topic>`)

- `help all`  
  Prints help for all topics

- `quit`  
  Quits the synthesizer

- `source filename`  
  Loads commands from a file and executes them line by line

- `echo arg`  
  Prints `arg` to output

- `sleep duration`  
  Sleeps for `duration` milliseconds

- `reset`  
  System reset (all notes off, all sound off, reset controllers, etc.)

### SOUNDFONTS & SYNTH

- `load filename [reset] [bankofs]`  
  Load a SoundFont onto the SoundFont stack. If `reset` is 1 (which is the implicit default), all currently in-use SoundFont presets will be re-evaluated with the newly loaded SoundFont taken into account. Optionally, you can specify a _non-zero_ bank offset for the new SoundFont. For example the command `load soundfont.sf2 0 10` will load the `soundfont.sf2` with a bank offset of 10 without re-evaluating the presets.

- `unload number [reset]`  
  Unload a [SoundFont](SoundFont.md) by its stack index (`reset=1` default)

- `reload number`  
  Reload the [SoundFont](SoundFont.md) by its stack index

- `fonts`  
  List all loaded [SoundFonts](SoundFont.md) on the stack

- `inst number`  
  Print available instruments for the [SoundFont](SoundFont.md) at given stack index

- `channels [-verbose]`  
  Print out the presets of all channels. Use `-verbose` for more details.

- `voice_count`  
  Prints the number of active synthesis voices

### MIDI EVENTS

- `noteon channel key velocity`  
  Send a note-on event

- `noteoff channel key`  
  Send a note-off event

- `pitch_bend channel offset`  
  Send a pitch bend event

- `pitch_bend_range channel range`  
  Set pitch bend range for the MIDI channel

- `cc channel ctrl value`  
  Send a control change MIDI event

- `prog channel num`  
  Send a program change event

- `select channel sfont bank prog`  
  Combination of bank-select and program-change

### AUDIO SYNTHESIS

- `gain value`  
  Set the master gain (0 < gain < 5)

- `interp num`  
  Choose the interpolation method for all channels

- `interpc channel num`  
  Choose the interpolation method for one channel

### POLYMONO & CHANNEL MODES

- `basicchannels`  
  Prints the list of basic channels

- `resetbasicchannels [chan1 chan2..]`  
  Resets all or some basic channels

- `setbasicchannels [chan mode val...]`  
  Sets default, adds basic channels

- `channelsmode [chan1 chan2..]`  
  Prints channel mode for specified/all channels

- `legatomode [chan1 chan2..]`  
  Prints legato mode for specified/all channels

- `setlegatomode chan mode [chan mode..]`  
  Set legato mode for channels

- `portamentomode [chan1 chan2..]`  
  Prints portamento mode for specified/all channels

- `setportamentomode chan mode [chan mode..]`  
  Set portamento mode for channels. The mode corresponds to the `enum fluid_channel_portamento_mode`, i.e.
  * Mode 0 - Portamento on every note (staccato or legato)
  * Mode 1 - Portamento only on legato notes
  * Mode 2 - Portamento only on staccato notes

- `breathmode [chan1 chan2..]`  
  Print breath options for channels

- `setbreathmode chan poly(1/0) mono(1/0) breath_sync(1/0) [..]`  
  Set breath mode for channels

### TUNING COMMANDS

- `tuning name bank prog`  
  Create a tuning with name, bank, and program number

- `tune bank prog key pitch`  
  Tune a key to a specific pitch

- `settuning chan bank prog`  
  Set a tuning for a MIDI channel

- `resettuning chan`  
  Restore default tuning for a MIDI channel

- `tunings`  
  Lists available tunings

- `dumptuning bank prog`  
  Print pitch details of the tuning


### SETTINGS

- `set name value`  
  Set the value of a setting (must be real-time for immediate effect)

- `get name`  
  Get the value of a setting

- `info name`  
  Get information about a setting

- `settings`  
  List all settings


### REVERB


!!! Note

    Before FluidSynth 2.0 custom reverb commands existed. 
    Starting with 2.0 users are encouraged to set reverb parameters via the realtime [FluidSettings](FluidSettings.md) as described below.

- `set synth.reverb.active [0|1]`  
  Turn the reverb on or off 


- `set synth.reverb.room-size num`__
  Change reverb room size (i.e. the reverb time) in the range `[0 to 1.0]` (default: 0.2) 

- `set synth.reverb.damp num`  
  Change reverb damping in the range `[0.0 to 1.0]` (default: 0.0)
    * When 0.0, no damping.
    * Between 0.0 and 1.0, higher frequencies have less reverb time than lower frequencies.
    * When 1.0, all frequencies are damped even if room size is at maximum value.

- `set synth.reverb.width num`  
  Change reverb width in the range `[0.0 to 100.0]` (default: 0.5)
  `num` value defines how much the right channel output is separated of the left channel output.
    * When 0.0, there is no separation (i.e. the output is mono).
    * When 100.0, the stereo effect is maximum.

- `set synth.reverb.level num`  
  Change reverb output level in the range `[0.0 to 1.0]` (default: 0.9)

### CHORUS

!!! Note

    Before FluidSynth 2.0 custom chorus commands existed. 
    Starting with 2.0 users are encouraged to set chorus parameters via the realtime [FluidSettings](FluidSettings.md) as described below.

- `set synth.chorus.active [0|1]`  
  Turn the chorus on or off

- `set synth.chorus.nr n`  
  Use n delay lines (default 3)

- `set synth.chorus.level num`  
  Set output level of each chorus line to `num`

- `set synth.chorus.speed num`  
  Set mod speed of chorus to `num` (Hz)

- `set synth.chorus.depth num`  
  Set chorus modulation depth to `num` (ms)

### ROUTER

- `router_default`  
  Reload default MIDI routing rules

- `router_clear`  
  Delete all MIDI routing rules

- `router_begin [note|cc|prog|pbend|cpress|kpress]`  
  Start a new routing rule for an event type

- `router_chan min max mul add`  
  Filter/map MIDI channels for the current rule

- `router_par1 min max mul add`  
  Filter/map parameter 1 (key/ctrl nr) for the current rule

- `router_par2 min max mul add`  
  Filter/map parameter 2 (vel/cc val) for the current rule

- `router_end`  
  Commit the current routing rule

### PLAYER

- `player_start`  
  Start playing from the beginning of the current song

- `player_stop`  
  Stop playing

- `player_cont`  
  Continue playing

- `player_seek num`  
  Move forward/backward in current song by +/-num ticks

- `player_next`  
  Move to next song

- `player_loop num`  
  Set loop number (-1 = loop forever)

- `player_tempo_bpm num`  
  Set tempo to num beats per minute

- `player_tempo_int [mul]`  
  Set internal tempo multiplied by mul (default 1.0)

### LADSPA (if compiled with LADSPA support and enabled via `synth.ladspa.active=1`)

- `ladspa_effect <name> <library> [plugin] [--mix [gain]]`  
  Create a new effect from a LADSPA plugin

- `ladspa_link <effect> <port> <buffer or host port>`  
  Connect an effect port to a host port or buffer

- `ladspa_buffer <name>`  
  Create a LADSPA buffer

- `ladspa_set <effect> <port> <value>`  
  Set the value of an effect control port

- `ladspa_check`  
  Check LADSPA configuration

- `ladspa_start`  
  Start LADSPA effects

- `ladspa_stop`  
  Stop LADSPA effect unit

- `ladspa_reset`  
  Stop and reset LADSPA effects

### PROFILING (if compiled with profiling support)

- `profile`  
  Prints default profiling parameters

- `prof_set_notes nbr [bank prog]`  
  Sets notes number and preset bank/program for profiling

- `prof_set_print mode`  
  Sets the profiling print mode

- `prof_start [n_prof [dur]]`  
  Starts n_prof profiling measures of duration dur (ms) each

---

## AUTHORS

Please check the [AUTHORS](https://github.com/FluidSynth/fluidsynth/blob/master/AUTHORS), and [THANKS](https://github.com/FluidSynth/fluidsynth/blob/master/THANKS) files for all credits 

## DISCLAIMER

SoundFont® is a registered trademark of Creative Technology Ltd. 
