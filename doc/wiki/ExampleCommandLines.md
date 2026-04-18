# 💻 Command Line Examples

## Introduction

This page gives examples of how to start fluidsynth from the command line with different configurations. Please feel free to add the configuration that works best for you. 

## fluidsynth with PulseAudio

```shell
fluidsynth -a pulseaudio -m alsa_seq -o midi.autoconnect=1 -g 1.0 /path/to/some/soundfont.sf2
```

Starts a new fluidsynth instance using ALSA MIDI interface and PulseAudio as output automatically connecting to any MIDI devices that are plugged in. This should work on all modern Linux desktops without any further configuration or prior setup needed. For a quick start use a sound font from [here](https://www.flstudiomusic.com/2010/02/25-piano-soundfonts.html). Adjust the -g parameter (gain) to control default volume.

### fluidsynth with direct connection to ALSA drivers

Normally you connect fluidsynth to ALSA Sequencer System, and Sequencer is connected to MIDI device, via ALSA MIDI Device Driver. But you can also connect synthesizer directly to Device Driver omitting the Sequencer. You use `-m alsa_raw` for this case:

```shell
$ amidi -l
Dir Device    Name
IO  hw:1,0,0  Keystation MIDI 1

$ fluidsynth -a pulseaudio -m alsa_raw -o "midi.alsa.device=hw:1,0,0" -g 1.0 /path/to/some/soundfont.sf2
```  

## fluidsynth with JACK

```shell
fluidsynth -a jack -j -m jack /path/to/some/soundfont.sf2
```

Starts a new instance of fluidsynth that uses jack as audio and midi driver. If jackd is not already running, it should be started automatically. 
The parameter `-j` attempts to automatically connect fluidsynth's output to system loudspeakers. Otherwise, this must be done manually.

## fluidsynth on Mac OS X

```shell
fluidsynth -a coreaudio -m coremidi /path/to/some/soundfont.sf2
```

Starts an instance of fluidsynth that uses coreaudio as audio driver and coremidi as midi driver, using the given soundfont. Use a MIDI Patchbay to connect MIDI I/O devices to fluidsynth. Note that using `-o midi.autoconnect=1` will connect any MIDI devices already plugged in.

## fluidsynth to STDOUT

```shell
fluidsynth -T raw -F- -q  /path/to/some/soundfont.sf2 /optional/path/to/some/MIDI.mid | aplay -f cd
```

Renders RAW audio directly to stdout, allowing to pipe the stream to something else. In the example above, `aplay` is used for playback.

!!! Note
 
    This requires at least fluidsynth 2.1.0 .

## fluidsynth on NetBooks and low performance computers

First, you need to reduce the CPU usage which helps reduce the chances of data under-run which causes the audio to cut out. This can be done by turning off the Reverb and Chorus with the flags `-C0 -R0` and also by halving the sample rate with the flag `-r22050`.
Changing the sample rate does not work with the ALSA hw layer so use the plug layer instead. Increasing the size of each audio buffer to the maximum number of frames with the `-z8192` flag may also help to drastically reduce CPU usage. For example, this command line works quite well on an eeePC 901 NetBook. And consider reducing the polyphony count (i.e. number of voices that can be played in parallel) in order to relax memory requirements of fluidsynth.

```shell
fluidsynth -C0 -R0 -r22050 -z8192 -l -a alsa -o audio.alsa.device=plughw:0 -o synth.polyphony=64
```

If you get problems with unsteady playback or the audio cutting out then try closing all other programs, 

turning off your wireless network and unplugging any network cable. 
