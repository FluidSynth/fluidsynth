# FluidSynth LADSPA Interface

The [LADSPA](http://ladspa.org/) (Linux Audio Developer's Simple Plugin API)
binding can be used to route the FluidSynth audio output through any number of
LADSPA plugins. Please note that even though the "L" in LADSPA stands for
"Linux", it can also be used on different platforms, for example Windows or
MacOS. Check the "LADSPA on other Platforms" section at the end of this guide
for more information.

## Configuration

To configure and compile FluidSynth with LADSPA support, make sure you have the
LADSPA SDK installed (or at least the ladspa.h header file available in an
include path). Then compile FluidSynth in the usual way. You should see
`LADSPA support: yes` in the cmake output.

To enable the LADSPA engine, use the `synth.ladspa.active` setting when
starting FluidSynth:

    fluidsynth -o synth.ladspa.active=1 ...


# Quickstart Tutorial

The following walks you through the process of adding a LADSPA plugin into your
FluidSynth configuration. It assumes that you are running FluidSynth on Linux,
that you have some experience with running Linux shell commands and that you
know how to start FluidSynth from the command line and use it to play a MIDI
file.

## Introduction to LADSPA

You don't need to to have detailed knowledge of LADSPA to use effects with
FluidSynth, but knowing some of it's concepts will help if you want to make the
best use of it.

If you have the LADSPA SDK installed you should be able to use the `listplugins`
Linux command to list all plugins installed in your LADSPA path. And to show
more details about a particular plugin library, you can use the `analyseplugin`
Linux command. Here is an example showing the details of the `delay.so` plugin
from the LADSPA SDK:

```
user@host:$ analyseplugin /usr/lib/ladspa/delay.so

Plugin Name: "Simple Delay Line"
Plugin Label: "delay_5s"
Plugin Unique ID: 1043
Maker: "Richard Furse (LADSPA example plugins)"
Copyright: "None"
Must Run Real-Time: No
Has activate() Function: Yes
Has deactivate() Function: No
Has run_adding() Function: No
Environment: Normal or Hard Real-Time
Ports:	"Delay (Seconds)" input, control, 0 to 5, default 1
	"Dry/Wet Balance" input, control, 0 to 1, default 0.5
	"Input" input, audio
	"Output" output, audio
```

This output tells you that the `delay.so` library contains only a single plugin
called "Simple Delay Line". Most importantly it lists the input and output
ports, which can be used to set plugin parameters and connect the audio input
and output to FluidSynth.

"Delay (Seconds)" and "Dry/Wet Balance" are input controls. They are the
parameters that a user can set to affect the way the plugin works. They control
how long the delay should be and how the dry and wet signals should be mixed
before writing them to the output.

"Input" and "Output" are audio ports which carry samples into the plugin and out
again after it has run. Mono plugins usually provide one set of input and output
audio ports, stereo plugins usually provide two sets. But there are even plugins
that only have a single output port and no input at all (think of noise
generators...)

Also note the line `Has run_adding() Function: No`. This specifies that this
plugin can not mix it's audio output into an output buffer, but will always
replace anything that is already there. This will become important again later
on.

## FluidSynth Host Ports

Just as LADSPA plugins have input and output ports, FluidSynth provides it's
own audio ports that can be connected to plugins. On a standard stereo setup,
the following four ports are automatically created:

- Main:L
- Main:R
- Reverb:Send
- Chorus:Send

The "Main:L" and "Main:R" ports can be connected to effect input and output
ports. They carry the main audio signals into the LADSPA effects and the
modified signals back into FluidSynth.

"Reverb:Send" and "Chorus:Send" can be used as effect inputs. They carry the
mono effect send signals (as determined by the reverb and chorus send
generators for each voice) into the LADSPA effects.

Please note that if you run FluidSynth with the internal reverb and chorus
effects active (which is the default), then those effects are already mixed
into the Main:L and Main:R channels. Fore more details, please see the "Signal
Flow" section below.

For host port setups in multi-channel configurations, please see the
"Multi-Channel Output" section below.

## Creating a Configuration File

You can configure LADSPA effects using the FluidSynth shell, but writing the
commands into a file and loading it at startup is much more comfortable. So
let's create a file `effects.txt` with the following contents:

effects.txt
```
ladspa_effect e1 /usr/lib/ladspa/delay.so
ladspa_link e1 Input Main:L
ladspa_link e1 Output Main:L

ladspa_effect e2 /usr/lib/ladspa/delay.so delay_5s
ladspa_link e2 Input Main:R
ladspa_link e2 Output Main:R

ladspa_start
```

As the "Simple Delay Line" plugin only works on a mono signal, the configuration
above creates two effects: the one we named "e1" reads from and writes to the
left FluidSynth audio channel "Main:L", the "e2" effect reads from and
writes to the right channel "Main:R".

Please note that we only specified the path to the library
`/usr/lib/ladspa/delay.so` when creating the "e1" effect, but not which plugin
from the library to use. This is possible because the delay.so library contains
only a single plugin. If you want to use a library that contains more than one
plugin, you would need to give the plugin name as well, as we've done when
creating the "e2" effect. The string to use here is what is called "Plugin
Label" in the `analyseplugin` output.

## Using the Configuration File

Lets start FluidSynth with ALSA output and the standard SoundFont, enable LADSPA
effects, load the effects.txt config file and give it a test MIDI file to play:
(You will need to replace the `test.mid` with your own MIDI file and maybe
change the paths to the effects.txt file and the SoundFont)

```
user@host:$ fluidsynth -a alsa -o synth.ladspa.active=1 -f effects.txt FluidR3_GM.sf2 test.mid
```

You should now hear the MIDI file played at a slightly lower volume with a one
second delay effect added on both left and right channel. If not, please check
the FluidSynth output for any error messages.

## Changing Parameters

You probably noticed that we did not set any values for the "Delay (Seconds)"
and "Dry/Wet Balance" control ports. The delay plugin specifies default
values for these parameters: 1 second delay and a dry/wet balance of 0.5 (check
the `analyseplugin` output above). So when you don't override them, the defaults
are automatically used for rendering.

Let's set different values now and set the delay time on the left channel to
half a second and to 1.5 seconds on the right channel:

```
ladspa_effect e1 /usr/lib/ladspa/delay.so
ladspa_link e1 Input Main:L
ladspa_link e1 Output Main:L
ladspa_set e1 Delay 0.5

ladspa_effect e2 /usr/lib/ladspa/delay.so
ladspa_link e2 Input Main:R
ladspa_link e2 Output Main:R
ladspa_set e2 Delay 1.5

ladspa_start
```

Start FluidSynth again and you should hear that the delay is shorter on the
left channel, longer on the right. You can even change control parameters while
FluidSynth is running. Just type the `ladspa_set ...` commands into the
FluidSynth shell.

And to check the difference that the LADSPA effects have on the sound output,
you can turn them off and on again during run-time. Just type in `ladspa_stop`
and `ladspa_start` into the FluidSynth shell.

### Port Name Matching

Plugin port names are sometimes very long, because the plugin writers want them
to be self-documenting. But note that we didn't need to give the complete port
name "Delay (Seconds)" in the `ladspa_set` commands, but chose to use a much
shorter version: "Delay".

When specifying a port name for the `ladspa_link` and `ladspa_set` commands,
the system will look for any port that *starts with* the name you gave it. If
there is only one match, then that port is chosen. If there are multiple
matches (meaning your port name is ambiguous), you will see an error asking
you to be more specific. So the configuration for the "e1" effect could also
have been written with much shorter port names:
```
ladspa_effect e1 /usr/lib/ladspa/delay.so
ladspa_link e1 In Main:L
ladspa_link e1 Out Main:L
ladspa_set e1 Del 0.5
```

# Signal Flow

The LADSPA effects unit runs immediately after the internal reverb and chorus
effects have been processed. When no effects have been configured, the LADSPA
engine is dormant and uses no additional system resources.

When at least one effect is configured and the engine is activated, the rendered
audio is passed into the LADSPA effects engine, the effects are run in the order
that they were created and the resulting audio is passed back into FluidSynth
(and from there to the sound card or other output).

## Effect Sends

Please note that SoundFont designers can specify how much signal each
instrument should add to the reverb and chorus effect sends. When FluidSynth
renders a block of audio, all currently sounding instruments are mixed into the
`Main` output channels. In addition, all instruments add their signal to the
effect send ports (`Reverb:Send` and `Chorus:Send`) according to the effect
send amount specified in the SoundFont.

If you want to replace the internal reverb or chorus effects with a LADSPA
plugin and you want to honour the decisions made by the SoundFont designer, you
should use the `Reverb:Send` or `Chorus:Send` ports as effect input and
`Main:L` and `Main:R` ports as effect outputs. (See the "Example Setups" section
below for an example on how to replace the internal reverb with a LADSPA plugin.)

Please note that FluidSynth uses a mono signal for both effects, that is why
there is only a single send port for reverb and chorus.


# LADSPA Command Reference

The following is a description of all LADSPA-related commands that are
available in the FluidSynth shell if it has been compiled with LADSPA
support.

- `ladspa_effect`: Create a new effect from a plugin library
- `ladspa_buffer`: Create a new buffer
- `ladspa_link`: Link an effect port to a host port or a buffer
- `ladspa_set`: Set the value of an effect control
- `ladspa_check`: Check the effect setup for any problems
- `ladspa_start`: Start the effects unit
- `ladspa_stop`: Stop the effects unit
- `ladspa_reset`: Reset the effects unit

## ladspa_effect

```
ladspa_effect <effect-name> <library-path> [plugin-name] [--mix [gain]]
```

Load the LADSPA plugin library given by `<library-path>` and create a new effect
(i.e. an instance of a plugin). `<effect-name>` can be chosen by the user and must
unique. `<plugin-name>` is optional if the library contains only one plugin.

If the optional `--mix` parameter is given, then the LADSPA engine will call the
`run_adding` interface of the plugin. This will make the effect add it's output
to the output buffers instead of replacing them. The `--mix` parameter takes an
optional float value `gain`, which will be multiplied with each sample before
adding to the output buffers.

Please note that there is no command to delete a single effect once created. To
remove effects, please use `ladspa_reset` to clear everything start from
scratch.

Can only be called when the effect unit is not active.

## ladspa_buffer

```
ladspa_buffer <buffer-name>
```

Create a new audio buffer called `<buffer-name>`. The buffer is able to be used as
mono output or mono input to an effect. Buffers can be used to connect plugins
between each other without overwriting the host ports with temporary data.

Please note that there is no command to delete a buffer. To remove buffers,
please use `ladspa_reset` to clear everything and start from scratch.

Can only be used when the effect unit is not active.

## ladspa_link

```
ladspa_link <effect-name> <audio-port-name> <buffer-or-host-port-name>
```

Connects an effect input or output port with a buffer or a host port. This
command can be called multiple times and will overwrite the previous connection
made on that effect port.

Please note that there is no command to unlink an effect port. Use
`ladspa_reset` to clear everything and start from scratch.

Can only be used when the effect unit is not active.

## ladspa_set

```
ladspa_set <effect-name> <control-port-name> <float-value>
```

Sets a control port of an effect to a float value. Can be used at any time,
even when the effect unit is active.

## ladspa_check

```
ladspa_check
```

Checks the LADSPA effect configuration for errors. This command is also
implicitly called when executing `ladspa_start`.

## ladspa_start

```
ladspa_start
```

Activates the effects unit and inserts the configured effects into FluidSynth's
audio rendering pipeline.

## ladspa_stop

```
ladspa_stop
```

Deactivates the effects unit and removes the configured effects from
FluidSynth's audio rendering pipeline. The configuration is left untouched, so
it can be started again with `ladspa_start`.

## ladspa_reset

```
ladspa_reset
```

Deactivates the effects unit if active and clears all configuration and loaded
plugins.


# Example Setups

All examples assume that your `LADSPA_PATH` environment variable points to the
directory containing the plugin libraries (e.g. /usr/lib/ladspa).

## Single Plugin

The following loads the delay.so plugin library from the LADSPA SDK and
instantiates the delay effect under the name "e1". It connects the main left
audio channel from FluidSynth with the plugin input and output and starts the
effects engine.

```
ladspa_effect e1 delay.so
ladspa_link e1 Input Main:L
ladspa_link e1 Output Main:L
ladspa_start
```

The audible effect should be an untouched right channel and a slightly
lower volume on the left with a delay effect of 1 second on top.

## Replacing the FluidSynth Reverb Effect

If you would like a different reverb implementation than the one built-in to
FluidSynth, you can use a LADSPA reverb plugin like the "TAP Reverb" from
[Tom's Audio Processing plugins](http://tap-plugins.sourceforge.net/ladspa.html).

Here is the analyseplugin output for the `tap_reverb.so` plugin:
```
user@host:$ analyseplugin /usr/lib/ladspa/tap_reverb.so

Plugin Name: "TAP Reverberator"
Plugin Label: "tap_reverb"
Plugin Unique ID: 2142
Maker: "Tom Szilagyi"
Copyright: "GPL"
Must Run Real-Time: No
Has activate() Function: Yes
Has deactivate() Function: No
Has run_adding() Function: Yes
Environment: Normal
Ports:	"Decay [ms]" input, control, 0 to 10000, default 2500
	"Dry Level [dB]" input, control, -70 to 10, default 0
	"Wet Level [dB]" input, control, -70 to 10, default 0
	"Comb Filters" input, control, toggled, default 1
	"Allpass Filters" input, control, toggled, default 1
	"Bandpass Filter" input, control, toggled, default 1
	"Enhanced Stereo" input, control, toggled, default 1
	"Reverb Type" input, control, 0 to 42.1, default 0, integer
	"Input Left" input, audio
	"Output Left" output, audio
	"Input Right" input, audio
	"Output Right" output, audio
```

Using this information we can create a LADSPA configuration:

effects.txt
```
ladspa_effect e1 /usr/lib/ladspa/tap_reverb.so
ladspa_link e1 "Input Left" Reverb:Send
ladspa_link e1 "Input Right" Reverb:Send
ladspa_link e1 "Output Left" Main:L
ladspa_link e1 "Output Right" Main:R
ladspa_start
```

Start FluidSynth with the internal reverb disabled. (You will need to replace
the `test.mid` with your own MIDI file and maybe change the paths to the
effects.txt file and the SoundFont)

```
user@host:$ fluidsynth -a alsa -R0 -o synth.ladspa.active=1 -f effects.txt FluidR3_GM.sf2 test.mid
```

You will hear the output with a reverb effect from the plugin. And you can
change the reverb control ports with the `ladspa_set` command while the MIDI
file is playing.

# Multi-Channel Output

FluidSynth is capable of generating multi-channel output by specifying the
`synth.audio-groups` and `synth.audio-channels` configuration settings.
Explaining multi-channel output in detail is out of scope for this guide. But
using multiple output channels has an effect on the host ports that are
available to LADSPA plugins.

As soon as you configure more than one audio-channel, the main audio ports will
not be called "Main:L" and "Main:R" anymore, but will have indices added to
their name. So if you start FluidSynth with `-o synth.audio-groups=2`, then the
following ports will be created:

- Main:L1
- Main:R1
- Main:L2
- Main:R2
- Reverb:Send
- Chorus:Send

If you want all main ports to act as outputs as well as inputs to the effects,
then you also need to increase the `synth.audio-channels` setting.


# LADSPA on other Platforms

LADSPA is a very simple plugin architecture and only requires the ladspa.h
header file as compile-time dependency. To build FluidSynth on non-Linux
platform with LADSPA support, download the ladspa.h file from
http://www.ladspa.org and place it somewhere in your compiler include path. Then
configure and build LADSPA as you normally would.

All information in the above documentation is valid for all other platforms as
well. Just make sure you use the file path format specific to your platform in
the `ladspa_effect` calls. For example, on Windows you should use
```
ladspa_effect c:\path\to\ladspa\plugin.dll
```
instead of
```
ladspa_effect /path/to/ladspa/plugin.so
```

Audacity provides a large number of precompiled LADSPA plugins for Windows and
MacOS: http://www.audacityteam.org/download/plug-ins/

To get the `analyseplugin` and `listplugins` commands on Windows, you can either
compile them yourself using the LADSPA-SDK source code from ladspa.org or install
ladspa-sdk via Cygwin.
