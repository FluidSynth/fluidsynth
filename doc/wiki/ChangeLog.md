# 📝 Changelog

* [2.5.3](#fluidsynth-253)
* [2.5.2](#fluidsynth-252)
* [2.5.1](#fluidsynth-251)
* [2.5.0](#fluidsynth-250)
* [2.4.8](#fluidsynth-248)
* [2.4.7](#fluidsynth-247)
* [2.4.6](#fluidsynth-246)
* [2.4.5](#fluidsynth-245)
* [2.4.4](#fluidsynth-244)
* [2.4.3](#fluidsynth-243)
* [2.4.2](#fluidsynth-242)
* [2.4.1](#fluidsynth-241)
* [2.4.0](#fluidsynth-240)
* [2.3.7](#fluidsynth-237)
* [2.3.6](#fluidsynth-236)
* [2.3.5](#fluidsynth-235)
* [2.3.4](#fluidsynth-234)
* [2.3.3](#fluidsynth-233)
* [2.3.2](#fluidsynth-232)
* [2.3.1](#fluidsynth-231)
* [2.3.0](#fluidsynth-230)
* [2.2.9](#fluidsynth-229)
* [2.2.8](#fluidsynth-228)
* [2.2.7](#fluidsynth-227)
* [2.2.6](#fluidsynth-226)
* [2.2.5](#fluidsynth-225)
* [2.2.4](#fluidsynth-224)
* [2.2.3](#fluidsynth-223)
* [2.2.2](#fluidsynth-222)
* [2.2.1](#fluidsynth-221)
* [2.2.0](#fluidsynth-220)
* [2.1.9](#fluidsynth-219)
* [2.1.8](#fluidsynth-218)
* [2.1.7](#fluidsynth-217)
* [2.1.6](#fluidsynth-216)
* [2.1.5](#fluidsynth-215)
* [2.1.4](#fluidsynth-214)
* [2.1.3](#fluidsynth-213)
* [2.1.2](#fluidsynth-212)
* [2.1.1](#fluidsynth-211)
* [2.1.0](#fluidsynth-210)
* [2.0.9](#fluidsynth-209)
* [2.0.8](#fluidsynth-208)
* [2.0.7](#fluidsynth-207)
* [2.0.6](#fluidsynth-206)
* [2.0.5](#fluidsynth-205)
* [2.0.4](#fluidsynth-204)
* [2.0.3](#fluidsynth-203)
* [2.0.2](#fluidsynth-202)
* [2.0.1](#fluidsynth-201)
* [2.0.0](#fluidsynth-200)
* [1.1.11 and older](#fluidsynth-1111)


# FluidSynth 2.5.3

* Support building for tvOS (#1743, thanks to @Ghabry)
* Several minor fixes for the CMake buildsystem (#1742)
* Several documentation updates (#1745, #1736, thanks to @jimhen3ry)
* Fix a build issue when fluidsynth is consumed as part of a bigger CMake project (#1752, thanks to @thal)
* Fix fluidsynth being unable to load SoundFonts when being compiled with an old version of MinGW (#1755)

# FluidSynth 2.5.2

* The CoreAudio driver has gained iOS support (#1720, thanks to @neoharp-dev)
* Improve error handling and reporting when loading default soundfont (#1712)
* Improve logging and side effect mitigation when basic channel configuration changes (#1695)
* Fix `CVE-2025-68617` - a heap-based use-after-free involving DLS files (GHSA-ffw2-xvvp-39ch)

# FluidSynth 2.5.1

* Previous versions have incorrectly exposed private C++ and inline symbols; they are now hidden from the library (#1676)
* FluidSynth now resets DataEntry values when it receives (N)RPN MSB/LSB to prevent off-tuned channels (#1672, thanks to @rsp4jack)
* It is now possible to install the library, header and executable independently of each other (#1693, refer to [wiki](https://github.com/FluidSynth/fluidsynth/wiki/BuildingWithCMake#installation) for details, thanks to @pedrolcl)
* Add a CI pipeline for iOS and publish precompiled iOS binaries for each release (#1703, #1677, thanks to @withSang)
* Fix the precompiled Windows binaries lacking dependency libraries (#1668)
* Fix a regression introduced in 2.5.0, that could have caused a NULL pointer deref when playing AWE32 NRPN MIDIs (#1691)
* Fix a regression introduced in 2.5.0, that caused convex unipolar modulators to use a concave mapping (#1697)
* Fix a regression introduced in 2.4.6, which in rare cases could have caused the cutoff frequency of the IIR filter to get stuck or change abruptly (#1679)
* Fix a build issue where fluidsynth was trying to find glib even when it shouldn't (#1673, thanks to @dg0yt)
* Fix a build issue on macOS (#1684, thanks to @pedrolcl)
* Fix typos in ALSA log messages (#1704, thanks to @toadster172)

# FluidSynth 2.5.0

Starting with 2.5.0, a C++11-compliant compiler and standard library will be required to compile fluidsynth! Additionally, users now have to link FluidSynth's library `libfluidsynth` against the C++ standard library that was used when compiling fluidsynth.

**It is now possible to compile fluidsynth without GLib!** Pass `-Dosal=cpp11 -Denable-libinstpatch=0` to CMake when compiling fluidsynth. In addition, if you use a C++17-compliant compiler, fluidsynth will be compiled with _native_ DLS support! Precompiled Windows binaries will be provided for both flavors: glib and C++11 as OSAL.

**Support for GLib and libinstpatch is hereby deprecated.** They will be removed in 2.6.0.

### New Features

* Add a C++11-based OS abstraction layer as a replacement for glib, bringing [these minor limitations](https://github.com/FluidSynth/fluidsynth/discussions/847#discussioncomment-13183154) when C++17 is unavailable (#1570, thanks to @mmlr)
* Add native support for **D**ownloadable **S**oundfont **F**ormat (#1626, thanks to @rsp4jack)
* FluidSynth now recognizes the [`DMOD`](https://github.com/spessasus/soundfont-proposals/blob/main/default_modulators.md) INFO-subchunk (#1582, thanks to @spessasus)
* A new positional flag has been introduced: `-b, --bank-offset`, (#1538, refer to [UserManual](https://github.com/FluidSynth/fluidsynth/wiki/UserManual#options) for details)
* FluidSynth now natively supports the long CLI option flags on Windows (#1544)
* FluidSynth now supports native Android logging (#1621, thanks to @neoharp-dev)

### Breaking Changes

* Support for SDL2 has been removed - pls. use SDL3 instead (#1594)
* The processing order of fluidsynth's configuration files has been changed (#1573, refer to [wiki](https://github.com/FluidSynth/fluidsynth/wiki/FluidSynth-configuration-files) for details)
* The legacy shell commands for reverb and chorus have been removed (#1659, refer to [UserManual](https://github.com/FluidSynth/fluidsynth/wiki/UserManual#reverb) for details)

### Musically Breaking Changes

* It was discovered that fluidsynth was mapping some modulators slightly inaccurately into their normalized range (#1651, thanks to @baskanov)
* Some Roland GS NPRN Params are now mapped to CC numbers (#1519, refer to [wiki](https://github.com/FluidSynth/fluidsynth/wiki/FluidFeatures#roland-nrpns) for details)
* FluidSynth now auto-detects whether Portamento Time is 7bit or 14bit wide - this fixes the infamous Descent Game08 tune (#705, #1232, #1311, #1456, #1495, #1517, see [`synth.portamento-time`](https://www.fluidsynth.org/api/fluidsettings.xml#synth.portamento-time) for details)
* Previously, fluidsynth's default behavior was to use portamento only for those notes, that were played in a successive / legato manner; to further improve the portamento experience, this was changed and fluidsynth now plays portamento for all notes by default (#1656)

### Other Changes

* Fix drum kits not always being selected after SysEx GS Rhythm Part (#1579)
* The LGPL license now uses the remote-only FSF address (#1568, thanks to @musicinmybrain)
* The logic for parsing `INFO` chunks has been revised to improve handling of unknown `INFO`-subchunks (#1644)
* Fix a race condition in `fluid_synth_alloc_voice()` public API function (#1664)
* Fix installing static and shared libs cmake targets on the same prefix (#1648, thanks to @pedrolcl)
* The auto-generated lookup tables - that vcpkg and cross-compiling users have learned to love so much - are gone! (#1620)

For changes related to the public API, pls. [consult the API docs](https://www.fluidsynth.org/api/RecentChanges.html).


# FluidSynth 2.4.8

* Fix systemd daemon failing with spaces in Soundfont filenames (#1608)
* Fix a build issue on OpenIndiana (#1615)
* FluidSynth now explains why it discards invalid generators when run in verbose mode (#829)

# FluidSynth 2.4.7

* All previous versions of fluidsynth have incorrectly rejected Soundfont files with unknown `INFO` subchunks (#1580, thanks to @spessasus)
* Fix build for Windows on ARM64 (#1586, thanks to @carlo-bramini)
* Fix incorrect GS DT1 SysEx checksum validation (#1578)
* Suppress a SIGFPE on OS/2 (#1592, thanks to @komh)
* Android binaries are now compiled with page sizes aligned to 16kB boundary to allow for Android 15+ support (#1600)
* Due to continuing problems with openMP, precompiled Android binaries are now built without openMP support (#1603)
* Fix a NULL pointer dereference during legato mono playing (#1602)

# FluidSynth 2.4.6

* Fix inconsistent library naming for MSVC vs. MinGW builds (#1543)
* Fix MIDI player skipping some events when seeking (#1532)
* A regression introduced in 2.4.5 could have caused a heap-based buffer overrun (#1560)
* Fix several issues when generating API docs with recent versions of Doxygen (#1566, thanks to @mmlr)


# FluidSynth 2.4.5

* Prebuilt Windows Binaries were missing `SDL3.dll` (#1510)
* Fix SDL3 intercepting signals, causing `CTRL+C` to not quit fluidsynth (#1509)
* Fix a few flaws in the AWE32 NRPN implementation (#1452, #1473)
* A regression introduced in 2.4.4 broke drum preset selection for XG MIDIs (#1508)
* Fix for OpenMP thread affinity crashes on Android devices (#1521, thanks to @looechao)
* Fix fluidsynth's systemd user daemon being unable to create lock file on some distros (#1527, thanks to @andrew-sayers)
* Fix fluidsynth ignoring `initialFilterFc` generator limits (#1502)
* A regression introduced in 2.3.6 prevented SF2 NRPN messages from being processed correctly (#1536)

_Hint to package maintainers: Pls. make sure to install the newly added `fluidsynth.tmpfile`, as [shown here](https://github.com/FluidSynth/fluidsynth/blob/f8cdcb846b5c7c32a1de99db0ff69c4841142a99/contrib/fluidsynth.spec#L101C1-L101C83)._


# FluidSynth 2.4.4

* Support for SDL3 has been added, support for SDL2 has been deprecated (#1485, #1478, thanks to @andyvand)
* Soundfonts that are not respecting the 46 zero-sample padding-space previously sounded incorrect when
  `synth.dynamic-sample-loading` was active (#1484)
* Allow drum channels to profit from Soundfont Bank Offsets by no longer ignoring MSB Bank changes (#1475)
* Revise the preset fallback logic for drum channels (#1486)
* A regression introduced in 2.4.1 may have caused interrupted real-time playback when voices were using the lowpass filter (#1481)
* Improve multi-user experience when running fluidsynth as systemd service (#1491, thanks to @andrew-sayers)
* Fix ordering and dependencies of fluidsynth's systemd service (#1500, thanks to @fabiangreffrath)
* Revise fluidsynth's man page (#1499, thanks to @fabiangreffrath)

Note about cross-compilation: The CMake variable `FLUID_HOST_COMPILER` has been superseded by the Autools convention of using environment variables like `CC_FOR_BUILD` and friends (#1501, thanks to @fabiangreffrath)


# FluidSynth 2.4.3

* It was discovered, that exclusive class note terminations were too slow (#1467, thanks to @mrbumpy409)
* Fix a regression introduced in 2.4.0 that allowed the amplitude of a voice playing in delay phase to rise infinitely (#1451)
* MSGS drum-style note-cut has been converted to an opt-in setting [`synth.note-cut`](https://www.fluidsynth.org/api/fluidsettings.xml#synth.note-cut) (#1466)
* Support for SDL2 has been disabled by default* (#1472)
* Fix a regression introduced in 2.4.1 that could have caused infinite audio gain output for some MIDI files under certain configurations (#1464)
* Silence a warning issued by Systemd v254+ (#1474, thanks to @andrew-sayers)

*It was found that SDL2 hijacks fluidsynth's `main()` function which causes build problems on Windows, see (#1472). A PR for migrating to SDL3 is highly welcome! If you still want to compile with SDL2 support, you need to explicitly enable it using CMake flag `-Denable-sdl2=1`.


# FluidSynth 2.4.2

* Fix audible clicks when turning off voices while using a high filter resonance (#1427)
* Fix a build failure with MSYS2 and MinGW when processing `VersionResource.rc` (#1448, thanks to @pedrolcl)
* Fix a crash on startup when there are no MIDI devices available on Windows (#1446, thanks to @pedrolcl)
* Restore discovery of libsndfile (#1445)
* Fix a race condition when loading SF3 files containing multiple uncompressed samples (#1457)


# FluidSynth 2.4.1

* Enable libsndfile to use filename with non-ASCII characters on Windows (#1416, thanks to @pedrolcl and @stardusteyes)
* Fix a few commandline encoding related issues on Windows (#1388, #1421, thanks to @pedrolcl)
* Fix build errors on Windows (#1419, #1422, thanks to @carlo-bramini)
* Fix clicks and pops caused when changing parameters of the lowpass filter (#1415, #1417, #1424)
* Minor adjustment to AWE32 NRPN behavior (#1430)


# FluidSynth 2.4.0

### New Features

* Implement MSGS-style Drum Note Cut (#1199)
* Support KAI audio driver on OS/2 (#1332, thanks to @komh)
* Add support for AWE32 NRPNs (#1346)
* Add support for Polyphone's sample loop mode release (#1398, thanks to @spessasus)
* Add support for SF2.04 absolute value modulators (#1391, thanks to @spessasus)

### Breaking Changes

* LASH support has been removed (#1285)

### Musically Breaking changes

* The default settings for reverb and chorus have been tuned to provide a more natural perception (#1282, thanks to @es20490446e)
* The behavior of the volume envelope's delay phase was previously incorrect (#1312)
* The `ModLfoToVolume` generator was not able to increase the volume (#1374)
* FluidSynth's default device-id was changed, allowing it to process (most) SysEX events without further configuration (#1382)
* Previously, some modulators were treated in a non-standard way (#1392, #1389, #1068)

### Bug Fixes

* Bogus interpolation of IIR filter coefficients could have caused audible clicks and cracks (#1345)  - yet there is still a potential for clicks and cracks, see #1415


# FluidSynth 2.3.7

* Fix SF3 decoder producing crackling sound for loud samples (#1380)
* MIDI Format 2 is now rejected correctly since it was never supported (#1366)
* Fix UTF8 encoding issues on Windows (#1388, thanks to @stardusteyes)
* Fix a regression introduced in 2.3.2 causing fluidsynth to fail discovering Oboe (#1393)
* Other minor bugs (#1291, #1403)

# FluidSynth 2.3.6

* Fix a build issue on OS/2 (#1320, thanks to @komh)
* Fix various encoding issues on Windows related to device names (#1322, thanks to @pedrolcl)
* Fix discovery of gobject by CMake
* Fix a numerical instability that caused the chorus effect to stop working correctly when fluidsynth was compiled in single precision mode (affected all precompiled binaries, #1331, thanks to @jjceresa)
* Fix timing issues in MIDI player related to incorrect handling of running status (#1351)
* Fix ignoring LSB for RPN Pitch bend range events (#1357, thanks to @spessasus)

# FluidSynth 2.3.5

* Fix setting `synth.chorus.speed` to its minimum value being reported as out of range (#1284)
* Fix a regression causing libinstpatch and libsndfile not to be discovered on Windows (#1299)
* Fix pipewire audio driver not cleanly being destroyed (#1305, thanks to @mawe42)
* Fix selection logic for XG drum banks (#1307)
* Add cmake flag `FLUID_HOST_COMPILER` to allow overriding default host compiler required during compilation (#1301)
* LASH support has been deprecated (#1285)

# FluidSynth 2.3.4

* Fix a build failure when specifying `CMAKE_INSTALL_LIBDIR` as an absolute path (#1261, thanks to @OPNA2608)
* Fix some MIDI files never finish playing (#1257, thanks to @joanbm)
* Implement IPv6 to IPv4 fallback (#1208, thanks to @ivan-zaera)
* Fix a build failure when using CMake's Xcode generator (#1266, thanks to @bradhowes)
* Fix pipewire's Jack implementation not found by CMake (#1268, thanks to @pedrolcl)
* Fix a regression causing the MIDI Player to terminate prematurely (#1272, thanks to @albedozero)

# FluidSynth 2.3.3

* Fix choppy sound when selecting pipewire output (#1230, thanks to @nilninull)
* Fix build issues on musl-based distros (#1229)
* Terminate MIDI player once internal synth-ticks overflow to prevent filling up the filesystem (#1233)
* Fix a bug that allowed playing notes outside of key and velocity ranges (#1250)

# FluidSynth 2.3.2

* Add support for processing all SysEx messages with `synth.device-id=127` (#1206, thanks to @trolley813)
* Various fixes to linking against static libfluidsynth when consumed through CMake or pkg-config (#1211, #1224, thanks to @FtZPetruska)
* Added `fluid_player_get_division` (#1220, thanks to @Fruchtzwerg94)
* Fix a regression introduced in 2.3.1 which broke MIDI player's seamless looping (#1227, thanks to @fabiangreffrath)

# FluidSynth 2.3.1

* Prevent MIDI player from finishing prematurely (#1159, thanks to @topaz)
* Fix a crash when enumerating rawmidi ALSA devices (#1174, thanks to @Bob131)
* Restore systemd sandboxing options and make it work with user units (#1181, thanks to @bluca)
* Handle conflict with pipewire systemd daemon (#1177)

# FluidSynth 2.3.0

### New Features

* Add Pipewire audio driver (#982, thanks to @sykhro)
* Major modernization of the CMake build system, **CMake >= 3.13 will now be required** (#969, thanks to @pedrolcl)
* Add multichannel output for the CoreAudio driver (#1081, thanks to @mattrtaylor)
* LADSPA effects are mixed before processing internal effects (#1117, thanks to @albedozero)
* MIDI auto-connect functionality has been extended (#1023, thanks to @pedrolcl)
* Add linear interpolation to convex and concave transfer functions (#1156, thanks to @md1872b)
* Mixing of effects has been parallelized (#1158)

### Bug Fixes
* Issue  #1073 has been resolved, which may change the sound articulation a bit (#1152)
* Systemd Hardening has been partly reverted (#1147, thanks to @dvzrv)
*  `audio.jack.autoconnect` erroneously connected to MIDI ports (#1149, thanks to @ReinholdH)
* Fix build when compiling against OSS 4.0 API (#1150)

# FluidSynth 2.2.9

* Fix regression in WinMIDI driver introduced in 2.2.8 (#1131, #1141 thanks to @albedozero) 
* Tracks cannot be restarted in MIDI player after reaching EOT (#1138, thanks to @albedozero)
* Add a system-wide configuration file for Windows (#1143, thanks to @daniel-1964)
* Harden systemd service file
* Dependency libs for precompiled Android binaries have been updated

# FluidSynth 2.2.8

* ALSA and WinMIDI drivers now pass system real-time messages on to user callback (#1115, thanks to @albedozero)
* Fix FPU division by zero in `fluid_player_set_tempo()` (#1111)
* Fix system-wide config file not loaded (#1118)
* PulseAudio driver now honors `audio.periods` setting (#1127, thanks to @pedrolcl)

# FluidSynth 2.2.7

* Fix file driver not working correctly on Windows (#1076)
* Add a function to create a sequencer event from a midi event (#1078, thanks to @jimhen3ry)
* Precompiled x86 binaries are now x87-FPU compatible (#1079)
* Fix fluidsynth not responding to SIGINT and SIGTERM when using recent SDL2 (#1071, thanks to @mawe42)

# FluidSynth 2.2.6

* Undeprecation and minor revisal of the OSS driver (#1038)
* Minor improvements to CoreAudio and CoreMidi drivers (#1047, thanks to @bradhowes)
* Fix sustained voices being held after `ALL_CTRL_OFF` (#1049)
* Fix clobbering of `PORTAMENTO_CTRL` after `ALL_CRTL_OFF` (#1050)
* Prevent Modulation Envelope from being stuck in decay phase, causing detuned voices and potentially other audible glitches for some soundfonts (#1059)
* Fix a compilation issue with recent glib (#1063, thanks to @devingryu)

# FluidSynth 2.2.5

* Fix a build failure with CMake < 3.12 (#1003, thanks to @komh)
* OSS and MidiShare drivers are now deprecated (#1010)
* Prevent samples accidentally having their loops disabled (#1017)
* Fix framework installation on macOS (#1029, thanks to @pedrolcl)

_Pls. note that fluidsynth 2.3.0 will require CMake >= 3.13_

# FluidSynth 2.2.4

* Per-channel `ALL_SOUND_OFF` when seeking/stopping player  (#980, thanks to @albedozero)
* Fix windows related encoding problems (#984, thanks to @tsingakbar)
* Rewind playlist when calling `fluid_player_play` after all loops are complete (#994, thanks to @albedozero)
* Fix MinGW related static linking issues (#990, thanks to @realnc) 

# FluidSynth 2.2.3

* FluidSynth did not build on macOS 10.5 and earlier (#946, thanks to @evanmiller)
* Fix build with CMake <3.7 (#944, thanks to @komh)
* Fix a NULL dereference in `delete_fluid_ladspa_effect()` (#963)

# FluidSynth 2.2.2

* The MIDI router now handles out-of-range parameters in a smarter manner (#891, thanks to @jjceresa)
* Keep pedaling effective when the same note is played more than once (#905, thanks to @jjceresa)
* Select soundfont samples by frequency instead of midi note numbers (#926, thanks to @Naturseptime)
* Fix the sequencer's event ordering for NoteOn vel=0 events (#907)
* libfluidsynth's import library was broken for MinGW builds (#874)
* fluidsynth.exe short option `-Q` not working (#915, thanks to @pedrolcl)
* Precompiled Android binaries didn't work (#894, #897)
* Fix openMP detection for XCode 12.5 (#917)
* Make `audio.jack.autoconnect` connect all available ports (#920)
* Prevent MIDI Player from continuously suppressing notes (#935, thanks to @albedozero)
* `fluidsynth.pc` now includes private libraries for static linking (#904)
* Fix typos in code and documentation (#939, thanks to @luzpaz)

# FluidSynth 2.2.1

* Make ALSA the default driver on Linux (#878)
* Coreaudio driver failed to initialize on macOS 11 (#803, thanks to @ringoz)
* WaveOut driver failed to initialize (#873, thanks to @carlo-bramini)
* COM initialization in WASAPI driver is no longer performed in the caller's context (#839, thanks to @pedrolcl)
* WASPI driver now supports reverb and chorus (#836, thanks to @chirs241097)
* Handle SysEx GM/GM2 mode on, GS reset, and XG reset (#877, thanks to @kode54)
* Old behavior of `fluid_player_join` has been restored to prevent infinite loop in client code (#872)
* The Soundfont loader code has been refactored, illegal generators will now be skipped more consequently (#823, thanks to @mawe42)

# FluidSynth 2.2.0
**This release breaks ABI compatibility!** Refer to the [API docs](https://www.fluidsynth.org/api/RecentChanges.html#NewIn2_2_0) for details.

A C++98 compliant compiler is now required to build fluidsynth ([mailing list thread](https://lists.nongnu.org/archive/html/fluid-dev/2020-01/msg00010.html))

### New features

* Support loading SoundFonts >2GiB on Windows (#629) 
* Major overhaul of the sequencer and its event queue (#604)
  * Overlapping notes can be handled (#637)
  * Performance improvement, since the event queue no longer blocks the rendering thread
  * Timescale is not limited to `1000` anymore and can therefore be used for tempo changes
* The following audio drivers have gained multichannel support
  * DSound (#667, thanks to @jjceresa)
  * WaveOut (#667, thanks to @jjceresa)
* The WinMIDI driver supports multiple devices (#677, thanks to @jjceresa)
* Handle GS DT1 SysEx messages for setting whether a channel is used for rhythm part (#708, thanks to @chirs241097)
* Support use of UTF-8 filenames under Windows (#718, thanks to @getraid-gg)
* Improved support for overriding tempo of the MIDI player (#711, #713, thanks to @jjceresa)
* Handle settings-related commands in user command file before initializing other objects (#739)
* SoundFont loading has been parallelized (#746, #812, requires openMP)
* The Oboe driver has gained a lower latency and other updates (#740, #741, #747)
* WASAPI driver has been added (#754, thanks to @chirs241097)

### General

* Fix race condition in `fluid_player_callback` (#783, thanks to @arcln)
* Improvements to LADSPA subsystem (#795, thanks to @mawe42)

# FluidSynth 2.1.9

Coreaudio driver failed to initialize on macOS 11 (#803, backport from 2.2.1)

# FluidSynth 2.1.8

* Rapidly changing channel panning could have caused audible artifacts (#768). Affects all versions back to at least `1.1.2`.
* Fix a use-after-free when loading malformed soundfonts (#808, [CVE-2021-21417](https://github.com/FluidSynth/fluidsynth/security/advisories/GHSA-6fcq-pxhc-jxc9)). Affects all versions back to at least `1.1.2`.
* The number of allowed LADSPA effect units has been increased.

# FluidSynth 2.1.7

* a regression introduced in `2.1.0` prevented chorus from being audible when `fluid_synth_process()` was used (#751, thanks to @chirs241097)
* a regression introduced in `2.0.6` prevented the MIDI player from restarting playback after all files have been played (#755, thanks to @rncbc)
* fix a double-free violation introduced in `2.0.0` after executing the `info` shell command (#756)

# FluidSynth 2.1.6

SoundFonts may never be unloaded correctly, if
  * polyphony is ever exceeded (#727), or
  * voices are still playing while their SoundFont is being unloaded.

Calling `delete_fluid_synth()` does not free those SoundFonts either. Affected are versions from `1.1.4` to `2.1.5` .

#### Other bug-fixes:

* fix a heap-based use-after-free (#733)

# FluidSynth 2.1.5

* loading DLS may have failed in certain setup environments (#666)
* fix a build failure with GCC 4.8 (#661, thanks to @ffontaine)

# FluidSynth 2.1.4

* fix an uninitialized memory access possibly triggering an FPE trap
* fix several regressions introduced in `2.1.3`:
  * `fluid_synth_start()` failed for DLS-presets
  * fix a NULL dereference in jack driver
  * fix a stack-based overflow when creating the synth

# FluidSynth 2.1.3

* fix a cross-compilation failure from Win32 to WinARM (#630)
* fix issues while `fluid_player` is seeking (#634, #646)
* fix a NULL pointer dereference if `synth.dynamic-sample-loading` is enabled (#635)
* fix a NULL pointer dereference in `delete_rvoice_mixer_threads()` (#640)
* fix a NULL pointer dereference in the soundfont loader (#647, thanks to @jjceresa)
* fix dsound driver playing garbage when terminating fluidsynth (#642, thanks to @jjceresa)
* avoid memory leaks when using libinstpatch (#643)

# FluidSynth 2.1.2

* fluidsynth now exits with error when user-provided command-line arguments are out-of-range (#623)
* add verbose error logging to opensles and oboe drivers (#627)
* fix a memory leak in oboe driver (#626)
* fix a NULL dereference in the fluidsynth commandline program

# FluidSynth 2.1.1

* a regression introduced in 2.1.0 caused the jack audio driver to not correct a sample-rate mismatch (#607)
* pkg-config is now being to used to find readline (#606, thanks to @ffontaine)
* fix various typos in the documentation (#600, thanks to @luzpaz)
* fix a memory leak in the file renderer
* fix leaking memory when sequencer clients were not explicitly unregistered (#610)
* fix a heap-based use-after-free in jack driver (#613)
* fix the linker possibly not finding libinstpatch (#617, thanks to @realnc)

# FluidSynth 2.1.0

### New features

* new, less "ringing" reverb engine (#380, thanks to @jjceresa)
* new, stereophonic chorus engine (#548, thanks to @jjceresa)
* support for Downloadable Sounds (DLS) files was added (#320, requires [libinstpatch](https://github.com/swami/libinstpatch))
* improved integrity checking of SoundFont modulators (#467, thanks to @jjceresa)
* [rendering to stdout is now possible](https://github.com/FluidSynth/fluidsynth/wiki/ExampleCommandLines#fluidsynth-to-stdout) (#553, thanks to @mawe42)
* the following Audio Drivers have been added:
  * Oboe (#464, tested on Android, thanks to @atsushieno)
  * OpenSLES (#464, tested on Android, thanks to @atsushieno)
  * SDL2 (#478, thanks to @carlo-bramini)
  * WaveOut (#466, tested on Win98, WinNT4.0, WinXP, thanks to @carlo-bramini)
* various performance improvements (#543, #545,  #547,  #569,  #573)

### Bug fixes

* generator `modEnvAttack` now has a convex shape according to SoundFont spec (#153)
* the default `MIDI Pitch Wheel to Initial Pitch` modulator now uses `Fine Tune` as destination generator, allowing it to be overridden by the soundfont designer (#154, thanks to @jjceresa)


# FluidSynth 2.0.9

* an implicitly declared function (regression of 2.0.8) caused a pointer to int truncation in CoreAudio driver (#591)
* fix a stack-based overflow in CoreAudio driver (#594, thanks to @fkmclane)

_Note that this version mistakenly reports as `2.0.8`._

# FluidSynth 2.0.8

* fix incorrect behavior of `fluid_sample_set_sound_data()` (#576, thanks to @swesterfeld)
* fix voices being mixed incorrectly, causing audible crackle esp. at sample-rates >48kHz (#580)
* make sure that defining `NDEBUG` disables assertions

# FluidSynth 2.0.7

* fix broken audio output when reverb was active after synth creation (#563)
* fix debug console messages not being visible for debug builds on Windows

# FluidSynth 2.0.6

* fix an uninitialized memory access, which could have led to NULL dereference or heap corruption in an out-of-memory situation
* fix a use-after-free when calling `fluid_player_stop()`
* fix the MIDI player not outputting any sound after stopping and restarting the playback (#550)

# FluidSynth 2.0.5

* the MIDI player erroneously assumed a default tempo of 125 BPM rather than 120 BPM (#519)
* improve integration of systemd (#516, thanks to @fleger)
* fix a buffering bug in `fluid_synth_process()` (#527, thanks to @swesterfeld)
* fix a major memory leak when unloading SF3 files (#528, thanks to @mawe42)
* fix several NULL dereferences and memory leaks in jack driver
* fix a memory leak when creating threads

# FluidSynth 2.0.4

* introduce verbose error reporting for `fluid_settings_*` functions 
* avoid undefined behavior when `fopen()` directories
* improve compatibility with FreeBSD and DragonFlyBSD (#508, thanks to @t6)
* fix build when cross compiling (#501)
* fix build on Mac OS X 10.4 (#513)
* fix build when compiling with MinGW
* enable network support on Mac by default (#513)

# FluidSynth 2.0.3

* fix handle leak in winmidi driver (#469, thanks to @carlo-bramini)
* fix build failures when cross compiling (#484)
* fix a bug when calculating the lower boundary of attenuation (#487, thanks to @jjceresa)
* fix a double free in `fluid_sample_set_sound_data()`
* silence a warning when loading soundfonts from memory (#485, thanks to @mawe42)
* minor performance improvements (#461, #471, #482, #486, thanks to @carlo-bramini and @jjceresa)

# FluidSynth 2.0.2

* fix building fluidsynth without any audio drivers  (#447)
* fix a possibly misaligned memory access in the soundfont loader (#457)
* fix a memory leak in the pulse audio driver (#458)
* fix a NULL deref in the coreaudio driver (#458)
* use cmake to query for DSound and WinMidi support (#449, thanks to @carlo-bramini)
* remove an unintended MFC dependency header (#449, thanks to @carlo-bramini)
* include Windows DLL version info for MinGW builds (#449, thanks to @carlo-bramini)
* implement `midi.autoconnect` for jack (#450, thanks to @ColinKinloch)
* add a cmake option to disable multi-threading (#463, thanks to @carlo-bramini)

# FluidSynth 2.0.1

* implement auto-conntect for CoreMidi  (#427, thanks to @ColinKinloch)
* fix a build issue with cmake < 3.3
* fix a crash when creating multiple jack drivers (#434, thanks to @rncbc)
* various fixes to dsound driver (#435, thanks to @carlo-bramini)
* fix multiple potential NULL dereferences (#437)
* fix two memory leaks in the soundfont loader (#437)
* correct upper threshold of `synth.chorus.depth`

# FluidSynth 2.0.0

### New Features
* implement polyphonic key pressure (#185, thanks to @mawe42)
* add API for manipulating default modulators (#265, #164, #71, thanks to @mawe42)
* add `midi.autoconnect` setting for automatically connecting fluidsynth with available MIDI Input ports (currently only for `alsa_seq` thanks to @tomcucinotta)
* add seek support to midi-player (#261, thanks to @loki666)
* add support for text and lyrics midi events (#111)
* add support for 24 bit sample soundfonts (#301, #329)
* consider "important midi channels" during overflow calculation `synth.overflow.important-channels` (#294, thanks to @mawe42)
* add a custom default modulator for MIDI CC8 to support proper stereo balance (#317, thanks to @mawe42)
* add support for an additional custom high-pass filter (#331, thanks to @mawe42)
* incorporate JJC's polymono patch (#306, #236, #158)
  * add basic channel support
  * implement MIDI modes Omni On, Omni Off, Poly, Mono
  * implement portamento control
  * implement legato control
  * implement breath control
* add support soundfont loading from memory (#241)
* add a profiling command interface (#345, thanks to @jjceresa)
* add support on demand sample loading (#366, thanks to @mawe42)
* add reverb and chorus settings (#49)
* allow using the midi router to manipulate midi files when playing from command line
* `fluid_synth_process()` received a new proper implementation
* `synth.effects-groups` allows to render effects of all MIDI channels to separate audio channels

### General
* CMake 3.1.0 or later is required for building
* consider channel pressure, key pressure and pitch wheel for lower attenuation boundary calculation (#163, thanks to @mawe42)
* complete rewrite of the LADSPA subsystem (#227,  #235, thanks to @mawe42)
* complete rewrite of the Soundfont Loader API (#334, #309)
* avoid reverb amplitude growing exponentially (#279, thanks to @jjceresa)
* removed deprecated autotools build system
* a minimal build of fluidsynth without requiring pkg-config is supported
* remove deprecated LADCCA support
* use unique device names for the `audio.portaudio.device` setting (#284, thanks to @jjceresa)
* documentation of the settings moved to http://www.fluidsynth.org/api/fluidsettings.xml
* adjust MIDI Pan and Balance calculations as outlined by MIDI Recommended Practice (RP-036) (#317, thanks to @mawe42)
* make network support compile-time optional (#307, thanks to @carlo-bramini)
* speed up calculation of chorus modulation waveforms for devices without FPU (#321, thanks to @carlo-bramini)
* cleanup internal audio rendering and mixing engine (#197)
* reduce memory consumption of loaded soundfonts (#370, thanks to @mawe42)

# FluidSynth 1.1.11

* fix pkgconfig file for absolute paths (#347, thanks to @krop)
* add a cmake option for OSS support  (#350, thanks to @Ne01eX)
* fix broken `enable-midishare` cmake option
* fix double free in `fluid_midi_router` (#352, thanks to @fulinux) 
* prevent malicious soundfonts from causing buffer overflows (#354, thanks to @mawe42)
* avoid SF3 files to be loaded incorrectly (#354, thanks to @mawe42)
* fix an endless loop in fast file renderer (#367)

# FluidSynth 1.1.10

### Bug Fixes

* avoid a buffer overrun when loading malformed soundfonts (#327)
* fix `synth.default-soundfont` returning incomplete path (#332)
* improve reliability of `fluid_is_soundfont()` (thanks to Orcan Ogetbil)
* minor cmake adjustments for MSVC builds to enable building fluidsynth as vcpkg package (#333, #339, thanks to @stekyne)
* fix build issues against statically built fluidsynth library on all OSs (#341)
* cmake: prevent double usage of `LIB_SUFFIX` and `LIB_INSTALL_DIR`

### New Features

* include Windows version info in libfluidsynth.dll (#216, thanks to @harborsiem)
* enable jack midi driver to support more than 16 midi channels (#326)
* enable fluidsynth to be set up as systemd user service (#66, #342, thanks to @dvzrv)

# FluidSynth 1.1.9

### Bug Fixes

* fix building the PortAudio driver on Windows (thanks to @ReinholdH)
* fix build if no MIDI drivers are available (thanks to @carlo-bramini)
* fix return value of `fluid_file_set_encoding_quality()` (thanks to @ReinholdH)
* fix use-after-free in fluid_timer
* fix memory leak in PulseAudio driver
* fix memory leak in rvoice_mixer (thanks to @mawe42)
* fix `dumptuning` shell command displaying uninitialized values (thanks to @rmattes)
* fix a resource leak in `source` shell command (thanks to @carlo-bramini)
* harmonize fluidsynth's output library naming with autotools on Windows (#271, thanks to @fabiangreffrath)
* don't set `LIB_SUFFIX` when building with MinGW (#281, thanks to @fabiangreffrath)
* avoid a possible deadlock when initializing fluidsynth's DLL on Windows (#269, #286, thanks to @carlo-bramini and @jjceresa)
* avoid a buffer overrun when mixing effects channels in `fluid_synth_nwrite_float()` (#287)
* correctly clean up `fluid_server` on Windows (#304, thanks to @carlo-bramini)

### New Features

* implement handling of `FLUID_SEQ_ALLSOUNDSOFF` events in `fluid_seq_fluidsynth_callback()`
* support for registering audio drivers based on actual needs (#218)

# FluidSynth 1.1.8
Minor maintenance release to address some issues introduced with 1.1.7:

* fix build against glib < 2.30 (#202)
* fix dsound audio driver on windows (#215)
* fix a bug around `synth.audio-groups` setting, which caused improper multichannel rendering (#225)
* cmake >= 3.0.2 is now required
* compilation with clang is now possible
* build fixes on OS/2 (thanks to @komh)

# FluidSynth 1.1.7
This is mainly considered to be a maintenance release, although it brings a few new features, see below. **Note** that this is expected to be the last release to ship the deprecated and unmaintained autotools build system! Make sure you check out [how to build fluidsynth using CMake](https://github.com/FluidSynth/fluidsynth/wiki/BuildingWithCMake).

### Bug fixes
* **consistently relicense libfluidsynth under LGPL-2.1+** (also addressing fluid_chorus.c, #165)
* `fluid_synth_set_channel_type()` was not exported properly
* introduce visibility control of exported functions (ee54995fabd260f54862cc15c6118fabf8b8b216)
* Avoid memory allocation on program change
* fix calculations for modulators (#194)
* fix SysEx parsing issues (#127, #148, thanks to Erik Ronström and Stas Sergeev)
* fix mangling with illegal sample loops, causing audible glitches (#171,  #149, thanks to @mawe42)
* fix inverse logic of audio.jack.multi option (#135)
* fix channel fine tune RPN to use correct range (#187, thanks to @mawe42)
* fix timing problems when changing the sequencers scale from a callback event (#195, thanks to @imhcyx)
* workaround incorrectly rendered audio when requesting more than 64 frames from `fluid_synth_write_*()` (#192)
* adjust ALSA MIDI port type (#139)
* avoid voice_count becoming negative (#151, thanks to Jean-Jacques Ceresa)
* avoid notes with a fixed key generator playing forever (#159)
* avoid TCP/IP connections from closing in an undefined manner (b75c8fdb17f842592b7e5af1fb0ddefde7f351d2)
* a lot of memory leaks, NULL dereferences and SegFaults (thanks to Surabhi Mishra)
* fix build
  * if EPIPE == ESTRPIPE (#133)
  * for mingw32 by checking for inet_ntop() (#132)
  * having lash support enabled
  * for > VS2015 (#189, thanks to @stekyne)

### New Features
* add support of vorbis-compressed sf3 sound fonts (#140, thanks to @fabiangreffrath)
* add sostenuto pedal to the synth (#47, #134, thanks to Jean-Jacques Ceresa)
* add vbr quality when encoding with libsndfile
* re-implement routing reverb and chorus to distinct buffers in `fluid_synth_nwrite_float()` (#135)
* add IPv6 support to socket API (#124)
* add default soundfont setting: `synth.default-soundfont`
* add `synth.lock-memory` setting
* allow sampledata sharing between different FluidSynth instances (thanks to @kmatheussen)

### New API calls
* channel, key, velocity and state getter for `fluid_voice_t`
* tempo, bpm, total length and currentBeat getter for `fluid_player` (#190, thanks to @quiasmo)

# FluidSynth 1.1.6
### Fixes and changes

* Handle MIDI End of track events, ticket #100 (Matt Giuca)
* Prevent broken rendering after a voice overflow, ticket #99  (diwic)
* Enable long arguments where available (plcl)
* Windows: Install fluidsynth.pc (pkg-config spec), ticket #101 (plcl)
* Mac OS X Lion: Fix build failure, ticket #104  (twobits)
* Linux: Prevent libdbus crash (diwic)

### Test results from the FluidSynth tester program

**Tester**
**Test Case**
**Result**

    S Christian Collins 
    SoundFont compatibility: Proper rendering of modulators, etc 
    OK 

    S Christian Collins 
    Voice stealing logic 
    OK 

    S Christian Collins 
    Reverb and chorus 
    OK 

    Sven Meier 
    Windows build (using mingw-w64) 
    OK 

    Sven Meier 
    OS X build 
    Untested 

    Bernd Casper 
    jOrgan setup 
    Untested 

    Aere Greenway 
    QSynth setup 
    OK 

    Aere Greenway 
    Low memory and slow CPU environment 
    OK 

    Matt Giuca 
    Linux build (using Ubuntu) 
    OK 

    Matt Giuca 
    Fast rendering 
    OK 

    Matt Giuca 
    FluidSynth as backend to DOSBox 
    OK 

# FluidSynth 1.1.5
This is a pure bug fix release compared to 1.1.4. 

The following bugs are fixed: 

  * Autotools build resulted in the wrong version number for libfluidsynth.so (reported by plcl, Takashi Iwai) 
  * One symbol was dropped from libfluidsynth.so - this symbol was not part of the public API though. (reported by Alessio Treglia) 
  * Windows 64 bit: Wrong prototype declaration for MIDI (reported by Graham Goode) 
  * JACK: Using jack_free instead of free when needed. Note that this might require a later version of JACK than previously. (reported by Graham Goode) 
  * Update Free Software Foundation address (reported by ogetbilo) 

# FluidSynth 1.1.4
### In short

Version 1.1.4 brings: 

  * Several improvements and fixes to the CMake build system, especially for Mac OS X 
  * Several bug fixes to the engine, notably quite a few which could cause [FluidSynth] to sound bad in some cases. 
  * API additions, that make it possible to 
    * load MIDI files from memory 
    * to inspect/modify MIDI events as they are being played from a MIDI file 
    * to change channels between melodic and drum mode 
    * and to silence all notes in one command. 
  * Improvements to the CoreAudio driver 

### In detail

(This is a summary of the commit log) 

#### Enhancements and API additions

  * Add playback callback from the MIDI file player for intercepting MIDI data on playback. [Jason Vasquez, plcl, diwic, etc] 
  * Use glib utility function for shell parsing (ticket [#44](https://github.com/FluidSynth/fluidsynth/issues/44)) [diwic] 
  * Allow in-memory midi file to be loaded by the midi engine [Matt Giuca] 
  * fluid_synth_all_notes_off and fluid_synth_all_sounds_off can now silence all channels at once, and are now public API functions. [jimmy, diwic] 
  * Allow channels to change state between melodic and drum channels [jimmy] 
  * support for "audio.coreaudio.device" option in Mac OS X CoreAudio driver [plcl] 
  * Mac CoreAudio driver adapted to AuHAL [plcl] 

#### Build system enhancements and fixes

  * Preliminary support for creating in Mac OS X a "[FluidSynth].framework" [plcl] 
  * Platform dependent options scoped to only the relevant platform [plcl] 
  * CMake build system fix: unset the variables created by check_pkg_modules() allowing to disable optional features that were formerly enabled [plcl] 
  * CMake build system fix for ticket [#90](https://github.com/FluidSynth/fluidsynth/issues/90): install dirs configurable [plcl] 
  * New macro: unset_pkg_config() [plcl] 
  * The unset() command requires CMake 2.6.3 or newer [plcl] 
  * fixed creation of the file "fluidsynth.pc", using the new *_INSTALL_DIR variables [plcl] 
  * removed the deprecated macro "CreateLibtoolFile" (unused) [plcl] 
  * renamed LT_VERSION_* variables as LIB_VERSION_* [plcl] 
  * build system fixes for OS/2 [KO Myung-Hun] 
  * Don't enable PortAudio support by default [plcl] 
  * Fix CoreAudio build problem [plcl, reported by Sven Meier] 
  * fix for ticket [#97](https://github.com/FluidSynth/fluidsynth/issues/97) : Latest fluid_midi.c fails to build under MSVC 2010 [plcl] 
  * gcc flags again: -Wno-vla removed because it is not supported by an Apple compiler [plcl] 
  * avoid to output a wrong error message [plcl] 
  * new GCC warning flag, trying to avoid a common MSVC unsupported C language construct [plcl] 
  * Build documentation instructions [plcl] 
  * Mac OS X fixes: frameworks build, midi.coremidi.id option. [plcl] 

#### Bug fixes (misc)

  * Do not use loop noise floor if sample continues in release phase (ticket [#93](https://github.com/FluidSynth/fluidsynth/issues/93)) [diwic, reported by Graham Goode] 
  * Prevent the IIR filter from loud pops on quick frequency changes (ticket [#82](https://github.com/FluidSynth/fluidsynth/issues/82)) [diwic] 
  * Fix memory leak causing soundfonts not to be deleted in delete_fluid_synth [diwic, reported by R\u0102\u0160mi Denis-Courmont] 
  * Fix incorrect samplerate for reverb and chorus (ticket [#89](https://github.com/FluidSynth/fluidsynth/issues/89)) [jaz001] 
  * Empty event queue from non-realtime context on startup, prevents timeout [diwic, reported by Krysztof Foltman] 
  * Allow sample rates down to 8 kHz [diwic] 
  * Fix for bug with duplicate sample names in [SoundFont] files [jgreen] 
  * Ignore extra size in [SoundFont] smpl chunk so that [FluidSynth] can load 24 bit [SoundFont] files as 16 bit, rather than rejecting the file. [jgreen] 
  * Better message when substituting presets, and store XG LSB changes even in drum mode. [diwic] 
  * Fix notes slightly off pitch (with floats and short loops) [diwic] 
  * fix for bug "Undefined behavior parsing a MIDI file which unexpectedly ends" (ticket [#92](https://github.com/FluidSynth/fluidsynth/issues/92)) [plcl, reported by Matt Giuca] 

#### Code cleanup / refactoring

  * Code cleanup, and remove unnecessary atomicy and shadow variables, now that the new architecture is in place [diwic] 
  * Rewrite overflow so that an extra rvoice is used [diwic] 
  * Fixed warning in fluid_synth.c [plcl] 
  * Add out-of-memory checks to fluid_player_add and fluid_player_add_mem [diwic] 
  * reformat fluid_midi.c source according to the coding style. [plcl] 

# FluidSynth 1.1.3
FluidSynth 1.1.3 is a pure bug-fix release and contains no new functionality. 

  * Compilation with LADSPA enabled was broken in 1.1.2 - fixed (plcl, diwic) 
  * Multichannel output broken when double precision was used - fixed (plcl, diwic) 
  * Doxygen settings (plcl) 
  * Mac OS X build system fixes (plcl, Benjamin Reed, Jean-Fran\u00e7ois Mertens) 
  * Fix build problem with scummvm (Alexander Hansen, Ebrahim Mayat) 
  * Optimize by not starting unused threads with multicore rendering (diwic) 
  * Window handle creation/destruction in Windows dll version (Andy Fillebrown) 
  * Race condition in alsa_seq / alsa_raw drivers caused them not to quit (diwic) 
  * Only free [example papers online](http://goodexamplepapers.com/) for students 

# FluidSynth 1.1.2
### Big changes: 

  * New CMake build system [plcl] 
    * Winbuild and Macbuild directories dropped 
    * Autotools build system is deprecated, but is still working 
  * Rewriting of thread safety [diwic] 
    * Two new settings control the thread safety mode. The default is to be backwards compatible. 

### Smaller changes: 

  * Voice overflow settings [diwic] 
  * Possible to update polyphony, up to 65536 (and beyond initial setting) [diwic] 
  * Possible to update sample rate (jack driver updates sample rate correctly) [diwic] 
  * MIDI Bank Select handling fixed [plcl] 
  * Source files moved into different subdirectories [diwic] 
  * Can use RealTimeKit (on Linux) to get real-time priority [diwic] 
  * Shell commands for pitch bend and pitch bend range [monk] 
  * PulseAudio driver: specify media role, and allow PulseAudio to adjust latency [diwic] 
  * Bug fixes [diwic, plcl, KO Myung hun, Felix Krause, laurent, nshepperd] 

For a complete list of changes, please see the svn commit log.

# FluidSynth 1.1.1 - "Clarity"

Changes from previous version 1.1.0

### Bug fixes

  * Recommit fix for voice stealing algorithm (David Henningsson) 
  * Update deltatime on midi file load, ticket [#59](https://github.com/FluidSynth/fluidsynth/issues/59) (David Henningsson and Josh Green, reported by Hans Petter Selasky) 
  * Build fix on OS X 10.4 (David Fang and Ebrahim Mayat) 
  * Fixed most asynchronous assignment/query regressions affecting QSynth (chorus, reverb, polyphony, MIDI CCs and presets) (Josh Green, reports and testing by Rui Nuno Capela) 
  * Reverted queuing of chorus and reverb assignments which fixes real-time performance issues when changing values (Josh Green) 
  * Fixed issue with audio thread changes affecting CoreAudio on OS X (Josh Green, reported by Ebrahim Mayat) 
  * Improved SMP safety with CC MIDI controls, polyphony, modulators and synth gain (Josh Green) 
  * Fixed crash bugs in fluid_timer functions (Josh Green) 
  * Reverted char * -&gt; const char * changes to function prototypes (Josh Green, reported by Rui Nuno Capela) 
  * Fixed TCP server build issue where WITHOUT_SERVER was still being set on win32 (Josh Green) 
  * Fixed crash when Jack driver was re-created (Josh Green) 
  * Fixed unknown macro warning in FluidSynth man page (David Henningsson) 

### Misc

  * Return queue process is now a thread instead of a timer and more responsive (Josh Green) 
  * Added missing dist files in doc/ (Josh Green) 
  * Updated README-OSX (Ebrahim Mayat) 

### Changes affecting developers

  * New fluid_synth_get_channel_info() function for a thread safe way of getting channel preset info (Josh Green) 
  * New fluid_synth_unset_program() function to unset a channel preset assignment (Josh Green) 
  * Marked fluid_synth_get_channel_preset() as deprecated (Josh Green) 
  * Developer API docs updated 

See "What's new in 1.1.1?" section in developer API documentation for more details: 

<http://fluidsynth.sourceforge.net/api/index.html#NewIn1_1_1>

### Contributors

  * Josh Green 
  * David Henningsson 
  * Rui Nuno Capela 
  * Ebrahim Mayat 
  * David Fang 
  * Hans Petter Selasky 

# FluidSynth 1.1.0 - "A More Solid Fluid"

Changes from previous version 1.0.9 

### Features and improvements

  * Extensive work on making [FluidSynth](index.md) more thread safe, resulting in better stability. 
  * Audio file rendering for MIDI to audio file conversion, faster than realtime (#15). 
  * Optional libsndfile support for file rendering in different audio file formats (wav, flac, Ogg Vorbis, etc.) (#30). 
  * Audio and MIDI are synchronized for MIDI file playback. 
  * Server can be specified for Jack audio and MIDI drivers. 
  * Jack audio and MIDI is now synchronized (when same Jack server used). 
  * MIDI file playback is now timed correctly, even with large audio buffer sizes. 
  * Fixed issue with missing percussion in MIDI files caused by very short notes, using synth.min-note-length setting (#1). 
  * Improved support for MIDI tuning standard, including SYSEX support and realtime tuning activation. 
  * Multicore support for utilizing multiple CPUs for synthesis or increasing speed of file rendering. 
  * Voices for a note-on event are started synchronously (#46). 
  * TCP/IP shell server support for windows (#20). 
  * Improved settings output (-o help) (alphabetically sorted and string options listed). 
  * Audio driver (-a) and MIDI driver (-m) options now accept "help" and list available options (#41). 
  * Added audio.realtime-prio and midi.realtime-prio for controlling realtime scheduling of some audio and MIDI drivers. 
  * Default priority levels of audio and MIDI threads set to 60 and 50 respectively. 
  * All yes/no string boolean settings converted to integer toggle settings (with backwards compatibility). 
  * glib is now a required dependency. 
  * Updated README-OSX 
  * Added "voice_count" shell command for getting current number of active voices. 

### New command line options

  * -F, --fast-render=[file] 
  * -T, --audio-file-type 
  * -O, --audio-file-format 
  * -E, --audio-file-endian 

### New or changed parameters

  * Audio file output settings: audio.file.endian, audio.file.format, audio.file.name, audio.file.type, audio.sample-format 
  * Realtime scheduling settings: audio.realtime-prio, midi.realtime-prio 
  * Jack settings: audio.jack.server and midi.jack.server 
  * Other settings: player.reset-synth, player.timing-source, synth.cpu-cores (experimental), synth.min-note-length 

### Bug fixes and minor changes

  * Fixed enabling of high priority scheduling in many audio drivers. 
  * Fixed bank selection logic 
  * Build fixes for mingw and VC++ builds on Windows 
  * Fix default values (after MIDI reset) to be more consistent with MIDI spec (#29). 
  * Removed VintageDreamsWaves?-v2.sf2 since it wasn't under a free license. 
  * Solaris build fix (#52) 
  * Implemented fluid_player_set_loop (#33) 
  * Fixed scaletune error (#26) 
  * Fixed synth reset between songs (#31) 
  * Fixed pitch bend error in Windows MIDI driver (#54) 
  * Skip remaining track data in MIDI file after EOT (#53) 
  * Bug fix in MIDI router where mutex was left locked with MIDI system reset message. 
  * Help command now shows list of topics instead of General help. 
  * Fixed non-blocking in alsa_raw, alsa_seq and OSS MIDI drivers. 
  * Fixed segfault on config file load (#45). 

### Changes affecting developers

  * Developer API docs overhauled and now very complete. 
  * Lots of code cleanup in fluid_synth.c, fluid_chorus.c, fluid_reverb.c and fluid_chan.c. 
  * Improved use of settings hints for all settings. 
  * char * arguments changed to const char * for many function prototypes. 
  * Many new API functions. 

See "What's new in 1.1.0?" section in developer API documentation for more details: 

<http://fluidsynth.sourceforge.net/api/index.html#NewIn1_1_0>

### Contributors

  * Josh Green 
  * David Henningsson 
  * Pedro Lopez-Cabanillas 
  * Ebrahim Mayat 

And many others! 

# FluidSynth 1.0.9 - "A Sound Future"

Changes from previous version 1.0.8 

### Features and improvements

  * New PulseAudio? driver (Josh Green) 
  * New Jack MIDI driver (Josh Green) 
  * New CoreMIDI driver (Pedro Lopez-Cabanillas) 
  * PortAudio driver re-written for PortAudio 19 (Josh Green) 
  * Support for OS/2 including Dart audio driver (KO Myung-Hun) 
  * RPN GM MIDI messages now handled for Bend Range, Fine Tune and Coarse Tune (Josh Green) 
  * MIDI channel pressure now handled (Bernat Arlandis i Ma\u00f1\u00f3) 
  * MIDI Program/Bank instrument fallback selection logic (Josh Green and thanks also to Jimmy) 
  * Added midi.portname setting to ALSA sequencer, -p command line switch (Nicolas Boulicault) 
  * Added midi.winmidi.device setting to winmidi driver (Pedro Lopez-Cabanillas) 
  * Updated Max/MSP [FluidSynth] binding (Norbert Schnell) 

### Synthesis Changes

  * Volume attenuation [SoundFont] generator now behaves more like EMU10K1 (S. Christian Collins) 
  * Stop forcing velocity based filtering (S. Christian Collins) 
  * Fixes to linear/bipolar/positive and convex/bipolar/positive modulator functions (S. Christian Collins) 
  * Added fix to properly search for percussion instrument (Josh Green) 
  * Force velocity envelope value to be that of the previous stage when switching from decay to sustain (S. Christian Collins) 
  * Filter calculation now uses synthesizer sample rate rather than fixed at 44100 (S. Christian Collins) 

### Bug fixes

  * Fixed Jack driver in "audio.jack.multi=yes" mode and Jack audio driver cleanup (Bernat Arlandis i Ma\u00f1\u00f3 and Pedro Lopez-Cabanillas) 
  * Wrong tempo changes (ticket #22 \\- Pedro Lopez-Cabanillas) 
  * Crash bug fix related to using certain modulators in a preset (S. Christian Collins) 
  * Fix to non-option command line argument processing when not using readline (Pedro Lopez-Cabanillas) 
  * dsound device can't be selected (Ticket #16 \\- Pedro Lopez-Cabanillas) 
  * Minor build fixes (Josh Green) 
  * Fixed compilation under MSVC 2008 and older (Pedro Lopez-Cabanillas) 

### Misc. stuff

  * Updated README-OSX build instructions (Ebrahim Mayat) 
  * [FluidSynth] fink package accepted for Mac OS X (Ebrahim Mayat) 
  * Minor fixes to [FluidSynth] man page (Sven Hoexter) 

# FluidSynth 1.0.8 - "Its about funky time!"

Changes from previous version 1.0.7a 

### Features and improvements

  * Improved synthesis interpolation (around loops for smoother looping, optimizations and improved flexibility) 
  * Dithering on 16 bit converted output (thanks to **Mihail Zenkov** for noting the problem and providing patches) 
  * Updated fluidmax plugin with polyphony parameter. 
  * Improved ALSA audio and sequencer drivers. 
  * Fixed some Floating Point Exceptions which were causing excessive CPU usage 
  * Some fixes to Chorus and Reverb parameters (thanks to **David Hilvert** for noting problems and providing patches) 

### Bug fixes

  * Looping/tuning problem when loop end close to end of sample (Ticket #4 \\- Thanks to **Tomas Nykung** for reporting and providing example SoundFonts). 
  * FluidSynth crashes with QSynth and audio meters turned on (Ticket #5 \\- Thanks to **David Hilvert** for reporting) 
  * ALSA sequencer driver no longer prints out false error messages 
  * Many memory leaks plugged (thanks to **Paul Millar** for pointing out issues and providing some patches) 
  * Warning message printed if a non option is not a valid [SoundFont] or MIDI file (thanks to **Nick Daly** for the patch). 

### Code cleanup other misc. stuff

  * Fixed Visual Studio Windows project files and other fixes to build on this platform 
  * Removed broken SSE support. 
  * Removed memory alignment hacks (no longer needed). 
  * Function comments sprinkled throughout the code (not finished yet) 
  * Updated fluidsynth man page. 
  * Updated README-OSX (from **Ebrahim Mayat**) 
  * Added --enable-fpe-check and --enable-trap-on-fpe configure options to aid in Floating Point Exception debugging 
