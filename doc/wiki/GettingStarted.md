# 🚀 Getting started with FluidSynth

## Introduction

**fluidsynth** is a software music synthesizer that reads midi input events either from a MIDI piano keyboard or from a software application (e.g. MIDI sequencer) and then generates in realtime a musical audio output that corresponds to all the midi notes being played. 

To work fluidsynth requires a Sound Font 2 file (`.sf2` file) or Sound Font 3 file (`.sf3` file) which contains all the audio waveforms for all the different musical instruments sounds that can be produced by fluidsynth. The Sound Font files `FluidR3_GM.sf2`and `FluidR3_GS.sf2` work with fluidsynth, and also have the advantage of having the Creative Commons License. These files are available in many Linux distributions in the packages `fluid-soundfont-gm` and `fluid-soundfont-gs` and they can also be downloaded from the net (try searching for `FluidR3_GM.sf2`). The GM stands for <dfn>General Midi</dfn>, which defines a standard mapping of MIDI patch numbers to musical instrument sounds. 

## Running fluidsynth

The easiest way test that fluidsynth is working correctly and to hear some MIDI music playing is to use the command line and to pass a Sound Font file and the MIDI file as the parameter. For example the following command line tests that fluidsynth is working on Ubuntu Linux. 
    
    
    fluidsynth /usr/share/sounds/sf2/FluidR3_GM.sf2 mymusicfile.mid
    
    

Normally, you would not pass a MIDI file to fluidsynth, but instead use another application to pass MIDI events to fluidsynth. In this case, you would start fluidsynth the following parameters. 
    
    
    fluidsynth /usr/share/sounds/sf2/FluidR3_GM.sf2
    
    

For further examples on how to start fluidsynth, see [example command lines](ExampleCommandLines.md). 

fluidsynth can also be started using [QSynth](https://qsynth.sourceforge.io/), which provides a graphical user interface to the synth.

## Getting Help

For help with starting fluidsynth, see the [user manual](UserManual.md) or type `man fluidsynth` on the command line. Typing `fluidsynth --help` lists all the command line options. 

Once fluidsynth has started up, type `help` to access the built-in help. Type `settings` to show all the current settings. Type `info audio.driver` to list the available hardware devices. 
