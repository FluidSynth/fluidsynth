
# FluidSynth

| Build Status | glib < 2.30 | glib >= 2.30 |
|---|---|---|
| Linux/MacOSX| n.a. | [![Build Status Travis](https://travis-ci.org/FluidSynth/fluidsynth.svg?branch=master)](https://travis-ci.org/FluidSynth/fluidsynth/branches) |
| Windows | [![Build status Appveyor](https://ci.appveyor.com/api/projects/status/n24ybk0dmttjwdk2/branch/master?svg=true)](https://ci.appveyor.com/project/derselbst/fluidsynth/branch/master) | [![Build status](https://ci.appveyor.com/api/projects/status/anbmtebt5uk4q1it/branch/master?svg=true)](https://ci.appveyor.com/project/derselbst/fluidsynth-g2ouw/branch/master) |

### FluidSynth is a software real-time synthesizer based on the Soundfont 2 specifications.

[![OHLOH Project Stats](https://www.openhub.net/p/fluidsynth/widgets/project_thin_badge?format=gif)](https://www.openhub.net/p/fluidsynth)

FluidSynth reads and handles MIDI events from the MIDI input
device. It is the software analogue of a MIDI synthesizer. FluidSynth
can also play midifiles using a Soundfont.


## Information on the web

The place to look if you are looking for the latest information on
FluidSynth is the web site at http://www.fluidsynth.org/.

For documentation, please [see the links below](#documentation).

For information on how to build FluidSynth from source, please [see our wiki page](https://github.com/FluidSynth/fluidsynth/wiki/BuildingWithCMake).


## Why did we do it

The synthesizer grew out of a project, started by Samuel Bianchini and
Peter Hanappe, and later joined by Johnathan Lee, that aimed at
developing a networked multi-user game.

Sound (and music) was considered a very important part of the game. In
addition, users had to be able to extend the game with their own
sounds and images. Johnathan Lee proposed to use the Soundfont
standard combined with an intelligent use of midifiles. The arguments
were:

- Wave table synthesis is low on CPU usage, it is intuitive and it can
  produce rich sounds

- Hardware acceleration is possible if the user owns a Soundfont
  compatible soundcard (important for games!)

- MIDI files are small and Soundfont2 files can be made small thru the
  intelligent use of loops and wavetables. Together, they are easier to
  downloaded than MP3 or audio files.

- Graphical editors are available for both file format: various
  Soundfont editors are available on PC and on Linux (Smurf!), and
  MIDI sequencers are available on all platforms.

It seemed like a good combination to use for an (online) game. 

In order to make Soundfonts available on all platforms (Linux, Mac,
and Windows) and for all sound cards, we needed a software Soundfont
synthesizer. That is why we developed FluidSynth.



## Design decisions

The synthesizer was designed to be as self-contained as possible for
several reasons:

- It had to be multi-platform (Linux, MacOS, Win32). It was therefore
  important that the code didn't rely on any platform specific
  library.

- It had to be easy to integrate the synthesizer modules in various
  environements, as a plugin or as a dynamically loadable object. I
  wanted to make the synthesizer available as a plugin (jMax, LADSPA,
  Xmms, WinAmp, Director, ...); develop language bindings (Python,
  Java, Perl, ...); and integrate it into (game) frameworks (Crystal
  Space, SDL, ...). For these reasons I've decided it would be easiest
  if the project stayed very focussed on it's goal (a Soundfont
  synthesizer), stayed small (ideally one file) and didn't dependent
  on external code.


## Links

### Home Page

- http://www.fluidsynth.org

### Documentation

- FluidSynth's wiki, https://github.com/FluidSynth/fluidsynth/wiki

- FluidSynth's API documentation, http://www.fluidsynth.org/api/

- Introduction to SoundFonts, by Josh Green,
  http://smurf.sourceforge.net/sfont_intro.php

- Soundfont2 Documentation, http://www.synthfont.com/SFSPEC21.PDF (if
  it moved, do a search on sfspec21.pdf).

- Soundfont.com FAQ, http://www.soundfont.com/faqs.html

- The MIDI Manufacturers Association has a standard called "Downloadable
  Sounds (DLS)" that closely ressembles the Soundfont Specifications,
  http://www.midi.org/about-midi/dls/abtdls.htm


### Software SoundFont Synthesizers:

- LiveSynth Pro DXi and Crescendo from LiveUpdate (Win),
http://www.livesynth.com/lspro.html

- Unity DS-1 from Bitheadz (Win & Mac), http://www.bitheadz.com/

- QuickTime 5 from Apple (Win & Mac), http://www.apple.com/quicktime/

- Logic from eMagic, http://www.emagic.de


### Soundfont Editors

- Project SWAMI by Josh Green (Linux), http://www.swamiproject.org/

- Vienna SoundFont Editor by Creative Technology Ltd. (Win)

- Alive Soundfont Editor by Soundfaction (Win), http://www.soundfaction.com/alive/index.htm

- Polyphone, http://polyphone-soundfonts.com/en/

    **Note:** We cannot recommend using Audio Compositor for creating or editing Soundfonts, as it generates files that violate the Soundfont2 spec (specifically the order of generators as defined in section 8.1.2) and are therefore unusable with fluidsynth!

### Conversion Tools

- CDxtract from CDxtract  (Win), http://www.cdxtract.com

- ReCycle from Propellerhead Software (Win & Mac),
http://www.propellerheads.se/products/recycle/

- Translator from Rubber Chicken Software (Win & Mac),
http://www.chickensys.com/translator


### Soundfont Databases

- HammerSound, http://www.hammersound.net

