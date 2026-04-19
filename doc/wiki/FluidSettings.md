# ⚙️ Fluid Settings

FluidSynth provides numerous options that allow tweaking various aspects of the synthesizing process, midi player and audio drivers. These are referred to as **FluidSettings**. Each setting is handled as a string, while the value this setting can be set to may either be an integer, number (float), bool or string type. They can be either used via [fluidsynth's API](https://www.fluidsynth.org/api/fluidsettings.html) or with the fluidsynth executable like:

```
fluidsynth -o audio.driver=alsa -o audio.alsa.device=plughw:0
```

Starting with **FluidSynth 2.0**, the [**FluidSettings are documented in this XML file**](http://www.fluidsynth.org/api/fluidsettings.xml) ([and developed here](https://github.com/FluidSynth/fluidsynth/blob/master/doc/fluidsettings.xml)).

For FluidSynth 1.1.x pls. refer to FluidSynths man page at that time.
