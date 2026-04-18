# 🗂️ FluidSynth configuration files

These are the defined names and locations of the configuration file which is automatically loaded at startup if the client application implements the mechanism. Currently only fluidsynth's CLI program (`fluidsynth.exe` in Windows, `fluidsynth` otherwise) honors this feature.

For application developers willing to implement this feature in their own clients, please see:

* `char * fluid_get_userconf (char *buf, int len)`  
  https://www.fluidsynth.org/api/group__command__interface.html#ga331346c938f00a657325f3a0b66f793f
* `char * fluid_get_sysconf (char *buf, int len)`  
  https://www.fluidsynth.org/api/group__command__interface.html#ga6e8489153a1c2041e526a3b27644c36d

As those two functions suggest, fluidsynth knows about a system-wide and a user-specific config file, see below.

Before fluidsynth 2.5.0, the user configuration file took precedence over the system configuration. If both were found, only the user configuration was processed.

Starting with fluidsynth 2.5.0, this behavior was changed. Now if the system-wide config exists, it will always be loaded. If the user config exists, it will be loaded as well, potentially overriding settings of the system-wide config. If additionally, a config file has been passed to fluidsynth via the `-f` flag, it will be loaded as well overriding settings of the user config. This allows users to stack the configuration. E.g., the system config may contain "always use the jack audio-driver" on this system. The user config may always load the user's favorite GeneralMidi soundfont with an optional bank-offset, while the config passed via the `-f` flag may set reverb and chorus parameters specific for a particular song or setup.

## User configuration

### Unix

The user configuration file is located at the `$HOME` directory with the name: `~/.fluidsynth`

### Windows

The user configuration file is located at `%USERPROFILE%\fluidsynth.cfg`

## System-wide configuration

### Unix

The system configuration file is located at `/etc/fluidsynth.conf`

### Windows

The system configuration file is located at `%ProgramData%\fluidsynth\fluidsynth.cfg`

## Example configuration file

This would be a valid configuration file for Linux:

```
set audio.driver pulseaudio
set audio.period-size 512
set audio.periods 8
set midi.driver alsa_seq
set midi.alsa_seq.id fluidsynth
set midi.autoconnect True
set synth.gain 1.0
set synth.chorus.active False
set synth.default-soundfont /home/user/.local/share/soundfonts/default.sf2
```
