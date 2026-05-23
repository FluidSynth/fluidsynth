# 🔈 Built-in Audio Drivers

FluidSynth supports a large range of audio drivers across many platforms. Below is a table of all
implemented audio drivers and some additional information for each driver.

Please note that the audio drivers available in your version of FluidSynth not only depend on your
platform and FluidSynth version, but also on which driver libraries where available during compilation.

## Audio Drivers

| id         | Name                     | Platforms                                | Multi-Channel Output |
|------------|--------------------------|------------------------------------------|----------------------|
| alsa       | ALSA                     | Linux                                    | No                   |
| coreaudio  | Core Audio               | macOS, iOS                               | No                   |
| dart       | DART                     | OS/2                                     | No                   |
| dsound     | DirectSound              | Windows                                  | Yes                  |
| file       | Direct file output       | All Platforms                            | No                   |
| jack       | Jack                     | Linux, Windows                           | Yes                  |
| oboe       | Oboe                     | Android                                  | No                   |
| opensles   | OpenSL ES                | Android                                  | No                   |
| oss        | Open Sound System        | Linux, Unix                              | No                   |
| portaudio  | PortAudio                | Linux, Unix, Windows, macOS, ...         | No                   |
| pulseaudio | PulseAudio               | Linux                                    | No                   |
| sdl2       | Simple DirectMedia Layer | Linux, Windows, macOS, iOS, Android, ... | No                   |
| sndman     | Sound Manager            | Classic Mac OS                           | No                   |
| wasapi     | WASAPI                   | Windows                                  | Yes                  |
| waveout    | WaveOut                  | Windows                                  | Yes                  |


*Multi-Channel Output*: Drivers with multichannel output support can output more than one stereo pair of audio channels.
