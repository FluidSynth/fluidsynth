# FluidSynth Developer Documentation

[FluidSynth](https://www.fluidsynth.org) is a software synthesizer based on the
[SoundFont 2](https://en.wikipedia.org/wiki/SoundFont) specifications. The
synthesizer is available as a shared library that can easily be reused in any
application that wants to use wave-table synthesis.

## What is FluidSynth?

- FluidSynth is a software synthesizer based on the SoundFont 2 specifications.
  The synthesizer is available as a shared object (a concept also named Dynamic
  Linking Library, or DLL) that can be easily reused in any application for
  wave-table synthesis.

- FluidSynth provides a Command Line Interface program ready to be used from the
  console terminal, offering most of the library functionalities to end users,
  including the ability to render and play Standard MIDI Files, receive real-time
  MIDI events from external hardware ports and other applications, perform
  advanced routing of such events, and enable a local shell as well as a remote
  server commands interface.

- FluidSynth is an API (Application Programming Interface) relieving programmers
  from a lot of details of reading SoundFont and MIDI events and files, and
  sending the digital audio output to a sound card. These tasks can be
  accomplished using a small set of functions. This document explains most of the
  API functions and gives short examples about them.

- FluidSynth uses instrument samples contained in standard SF2 (SoundFont 2)
  files. FluidSynth runs on Linux, Mac OS X, and Windows.

- FluidSynth is open source and in active development. For more details, visit
  <https://www.fluidsynth.org>.

## License

All source code examples in this document are in the public domain; you can use
them as you please. This document is licensed under the
[Creative Commons Attribution-Share Alike 3.0 Unported License](https://creativecommons.org/licenses/by-sa/3.0/).
The FluidSynth library is distributed under the
[GNU Lesser General Public License](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html).

## Disclaimer

This documentation may be partly incomplete. As always, the source code is the
final reference.

SoundFont® is a registered trademark of E-mu Systems, Inc.

## Getting started

Use the navigation on the left to browse the **Usage Guide**, the
**API Reference**, and the **Settings Reference**.

The [Usage Guide](usage/index.md) contains step-by-step tutorials explaining how
to set up the synthesizer, load SoundFonts, send MIDI events, and more.

The [API Reference](api/index.md) documents every public function, type, and
constant exposed by `libfluidsynth`.

The [Settings Reference](settings/index.md) describes every configuration
parameter understood by FluidSynth.
