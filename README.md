# FluidSynth

| Build Status | glib < 2.30 | glib >= 2.30 |
|---|---|---|
| ![Linux](https://raw.githubusercontent.com/microsoft/azure-pipelines-tasks/master/docs/res/linux_med.png) **Linux** | n.a. | [![Build Status Travis](https://travis-ci.org/FluidSynth/fluidsynth.svg?branch=master)](https://travis-ci.org/FluidSynth/fluidsynth/branches) |
| <img src="https://www.theinquirer.net/w-images/866eae81-b13b-47b5-8180-929943e9dc21/0/daemonhammerfreebsd-580x358.jpg" height="25"> **FreeBSD** | n.a. | [![Build Status](https://api.cirrus-ci.com/github/FluidSynth/fluidsynth.svg?branch=master)](https://cirrus-ci.com/github/FluidSynth/fluidsynth) |
| ![Win](https://raw.githubusercontent.com/microsoft/azure-pipelines-tasks/master/docs/res/win_med.png) **Windows** && ![macOS](https://raw.githubusercontent.com/microsoft/azure-pipelines-tasks/master/docs/res/apple_med.png) **MacOSX** | [![Build Status](https://dev.azure.com/tommbrt/tommbrt/_apis/build/status/FluidSynth.fluidsynth?branchName=master)](https://dev.azure.com/tommbrt/tommbrt/_build/latest?definitionId=3&branchName=master) | [![Build status](https://ci.appveyor.com/api/projects/status/anbmtebt5uk4q1it/branch/master?svg=true)](https://ci.appveyor.com/project/derselbst/fluidsynth-g2ouw/branch/master) |


#### FluidSynth is a cross-platform, real-time software synthesizer based on the Soundfont 2 specification.

FluidSynth generates audio by reading and handling MIDI events from MIDI input devices by using a [SoundFont](https://github.com/FluidSynth/fluidsynth/wiki/SoundFont). It is the software analogue of a MIDI synthesizer. FluidSynth can also play MIDI files.

[![OHLOH Project Stats](https://www.openhub.net/p/fluidsynth/widgets/project_thin_badge?format=gif)](https://www.openhub.net/p/fluidsynth)

## Documentation

The central place for documentation and further links is our **wiki** here at GitHub:

**https://github.com/FluidSynth/fluidsynth/wiki**

If you are missing parts of the documentation, let us know by writing to our mailing list.
Of course, you are welcome to edit and improve the wiki yourself. All you need is an account at GitHub. Alternatively, you may send an EMail to our mailing list along with your suggested changes. Further information about the mailing list is available in the wiki as well.

Latest information about FluidSynth is also available on the web site at http://www.fluidsynth.org/.

## License

The source code for FluidSynth is distributed under the terms of the [GNU Lesser General Public License](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html), see the [LICENSE](https://github.com/FluidSynth/fluidsynth/blob/master/LICENSE) file. To better understand the conditions how FluidSynth can be used in e.g. commercial or closed-source projects, please refer to the [LicensingFAQ in our wiki](https://github.com/FluidSynth/fluidsynth/wiki/LicensingFAQ).

## Building from source

For information on how to build FluidSynth from source, please [refer to our wiki](https://github.com/FluidSynth/fluidsynth/wiki/BuildingWithCMake).

## Links

- FluidSynth's Home Page, http://www.fluidsynth.org

- FluidSynth's wiki, https://github.com/FluidSynth/fluidsynth/wiki

- FluidSynth's API documentation, http://www.fluidsynth.org/api/

---

## Historical background

### Why did we do it

The synthesizer grew out of a project, started by Samuel Bianchini and
Peter Hanappe, and later joined by Johnathan Lee, that aimed at
developing a networked multi-user game.

Sound (and music) was considered a very important part of the game. In
addition, users had to be able to extend the game with their own
sounds and images. Johnathan Lee proposed to use the Soundfont
standard combined with intelligent use of midifiles. The arguments
were:

- Wavetable synthesis is low on CPU usage, it is intuitive and it can
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

### Design decisions

The synthesizer was designed to be as self-contained as possible for
several reasons:

- It had to be multi-platform (Linux, macOS, Win32). It was therefore
  important that the code didn't rely on any platform-specific
  library.

- It had to be easy to integrate the synthesizer modules in various
  environments, as a plugin or as a dynamically loadable object. I
  wanted to make the synthesizer available as a plugin (jMax, LADSPA,
  Xmms, WinAmp, Director, ...); develop language bindings (Python,
  Java, Perl, ...); and integrate it into (game) frameworks (Crystal
  Space, SDL, ...). For these reasons I've decided it would be easiest
  if the project stayed very focussed on its goal (a Soundfont
  synthesizer), stayed small (ideally one file) and didn't dependent
  on external code.
