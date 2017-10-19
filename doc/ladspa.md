# FluidSynth LADSPA Interface

The [LADSPA](http://ladspa.org/) (Linux Audio Developer's Simple Plugin API)
binding can be used to route the FluidSynth audio output through any number
of LADSPA plugins. As the name implies, it is only available on Linux.

## Configuration

To configure and compile FluidSynth with LADSPA support, make sure you have
the LADSPA SDK (basically the ladspa.h header file) installed. Then enable
LADSPA when calling cmake:

    cmake path/to/source/ -Denable-ladspa=1

You should see `LADSPA support: yes` in the cmake output.

To enable the LADSPA engine, use the `synth.ladspa.active` setting when
starting FluidSynth:

    fluidsynth -o synth.ladspa.active=1 ...


# Signal Flow

The LADSPA effects unit runs immediately after the internal reverb and chorus
effects have been processed. When no plugins have been configured, the
effects unit is dormant and uses no additional system resources.

When at least one plugin is configured and the engine is activated, the
rendered audio is passed into the LADSPA effects unit, each plugin is
run in the order that they were created and the resulting audio is
passed back into FluidSynth (and from there to the sound card or other
output).


# Loading and Connecting Plugins

Currently the only way to configure the effects unit is via the FluidSynth
shell or via a config file.

## Example Setups

All examples assume that your `LADSPA_PATH` environment variable points
to the directory containing the plugin libraries (e.g. /usr/lib/ladspa).

### Single Plugin

The following loads the delay.so plugin library from the LADSPA SDK and
instantiates the `delay_5s` plugin from that library. It connects the
main left channel output from FluidSynth with the plugin input, the
main left channel input to FluidSynth with the plugin output. It also
sets the two control ports of the plugin to example values and starts
the engine.

    ladspa_plugin delay.so delay_5s
    ladspa_port Input < in1_L
    ladspa_port Output > out1_L
    ladspa_port Delay = 1.0
    ladspa_port Dry/Wet = 0.5

    ladspa_start

The audible effect should be an untouch right channel and a slightly
lower volume on the left with a delay effect of 1 second on top.
