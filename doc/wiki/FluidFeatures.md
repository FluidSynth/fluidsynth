# ✨ Fluid Features

This is a list of MIDI events that fluidsynth handles by default. That "handling" is either given by

* the [SoundFont](SoundFont.md) spec, or
* the General MIDI standard.

While the explanations in the [SoundFont](SoundFont.md) spec are very precise in terms of the intended audio-synthetic articulation, the descriptions in the MIDI standard are very vague when it comes to sound articulation. Thus, you cannot expect that fluidsynth behaves in a certain way (which you may know from other synths) just because you send some MIDI CCs.

In order to control those _"underspecified"_ aspects of the sound synthesis (like ADSR, low pass filter cutoff & resonance, tremolo & vibrato depth, etc.) custom **SoundFont modulators** must be used! The SoundFont spec wants the SoundFont designer to define those modulators in the SoundFont file itself. Doing so will give you great portability between any SF2 compliant synth, i.e. you would get the same sound articulation when sending your custom CCs to any SF2 compliant synth to trigger your custom effects. Unfortunately, defining those modulators must be done for every single instrument or preset in the SoundFont, which can be tiresome and error-prone. Alternatively, you can use fluidsynth's API for manipulating default modulators (see `fluid_synth_add_default_mod()` and `fluid_synth_remove_default_mod()` resp.). This will allow you to insert or remove your own custom modulators, which will then affect all loaded SoundFonts equally. However, this technique is not portable and limited to fluidsynth.

#### Legend

- ✔️ Implemented according to MIDI or SoundFont spec and usable by default
- ✅ Partially or customly implemented and usable by default
- ⚠️ Attention, this may require special setup of fluidsynth to be usable by default
- ❌ Not handled by default, but usable via custom SoundFont modulators

## MIDI Message Implementation Chart

| MIDI Message      | Implementation Status                                                   |
|-------------------|-------------------------------------------------------------------------|
| NOTE_OFF          | ✔️                                                                      |
| NOTE_ON           | ✔️                                                                      |
| CONTROL_CHANGE    | ✅ [See related table below.](#midi-control-change-implementation-chart) |
| MIDI_SET_TEMPO    | ✔️                                                                      |
| PROGRAM_CHANGE    | ✔️                                                                      |
| CHANNEL_PRESSURE  | ✔️ SF2 default modulator                                                |
| KEY_PRESSURE      | ❌                                                                       |
| PITCH_BEND        | ✔️ SF2 default modulator                                                |
| MIDI_SYSTEM_RESET | ✔️                                                                      |

## MIDI Control Change Implementation Chart

Note that unless otherwise documented, CCs are interpreted individually, i.e. as 7-bit values.

| MIDI CC                                          | Implementation Status                                                                                                                                      |
|--------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------|
| (000) Bank Select                                | ✔️ Interpretation of MSB and LSB depends on [`synth.midi-bank-select`](http://www.fluidsynth.org/api/fluidsettings.xml#synth.midi-bank-select)             |
| (001) Modulation Wheel                           | ✔️ SF2 default modulator                                                                                                                                   |
| (002) Breath Controller                          | ⚠️ Usable in _breathmode_, see [PolyMono documentation](https://github.com/FluidSynth/fluidsynth/raw/master/doc/polymono/FluidPolyMono-0004.pdf)           |
| (004) Foot Controller                            | ❌                                                                                                                                                          |
| (005) Portamento Time                            | ✔️ MSB and LSB (i.e. 14-bit value!), see [PolyMono documentation](https://github.com/FluidSynth/fluidsynth/raw/master/doc/polymono/FluidPolyMono-0004.pdf) |
| (006) Data Entry                                 | ✔️ MSB and LSB (i.e. 14-bit value!).                                                                                                                       |
| (007) Channel Volume                             | ✔️ SF2 default modulator                                                                                                                                   |
| (008) Balance                                    | ✅ *non-standard* default SF2 modulator                                                                                                                     |
| (010) Pan                                        | ✔️ SF2 default modulator                                                                                                                                   |
| (011) Expression                                 | ✔️ SF2 default modulator                                                                                                                                   |
| (064) Sustain Pedal                              | ✔️ See [Sostenuto documentation](https://github.com/FluidSynth/fluidsynth/raw/master/doc/FluidSostenuto-005.pdf)                                           |
| (065) Portamento Switch                          | ✔️ See [PolyMono documentation](https://github.com/FluidSynth/fluidsynth/raw/master/doc/polymono/FluidPolyMono-0004.pdf)                                   |
| (066) Sostenuto Pedal                            | ✔️ See [Sostenuto documentation](https://github.com/FluidSynth/fluidsynth/raw/master/doc/FluidSostenuto-005.pdf)                                           |
| (068) Legato Switch                              | ✔️ See [PolyMono documentation](https://github.com/FluidSynth/fluidsynth/raw/master/doc/polymono/FluidPolyMono-0004.pdf)                                   |
| (072) Sound Controller 3 (default: Release Time) | ❌                                                                                                                                                          |
| (073) Sound Controller 4 (default: Attack Time)  | ❌                                                                                                                                                          |
| (074) Sound Controller 5 (default: Brightness)   | ❌                                                                                                                                                          |
| (084) Portamento Control (PTC)                   | ✔️ See [PolyMono documentation](https://github.com/FluidSynth/fluidsynth/raw/master/doc/polymono/FluidPolyMono-0004.pdf)                                   |
| (091) Effects 1 Depth (Reverb Send Level)        | ✔️ SF2 default modulator                                                                                                                                   |
| (092) Effects 2 Depth (Tremolo Depth)            | ❌                                                                                                                                                          |
| (093) Effects 3 Depth (Chorus Send Level)        | ✔️ SF2 default modulator                                                                                                                                   |
| (094) Effects 4 Depth (Celeste (Detune) Depth)   | ❌                                                                                                                                                          |
| (095) Effects 5 Depth (Phaser Depth)             | ❌                                                                                                                                                          |
| (098) NRPN LSB                                   | ✔️ [See related table below](#nrpn-control-change-implementation-chart)                                                                                    |
| (099) NRPN MSB                                   | ✔️ [See related table below](#nrpn-control-change-implementation-chart)                                                                                    |
| (100) RPN LSB                                    | ✅ [See related table below](#rpn-control-change-implementation-chart)                                                                                      |
| (101) RPN MSB                                    | ✅ [See related table below](#rpn-control-change-implementation-chart)                                                                                      |
| (120) All Sound Off                              | ✔️                                                                                                                                                         |
| (121) Reset All Controllers                      | ✔️                                                                                                                                                         |
| (121) Local Control                              | ✔️ Ignored, because not applicable                                                                                                                         |
| (123) All Notes Off                              | ✔️                                                                                                                                                         |
| (124) Omni Mode Off                              | ✔️ See [PolyMono documentation](https://github.com/FluidSynth/fluidsynth/raw/master/doc/polymono/FluidPolyMono-0004.pdf)                                   |
| (125) Omni Mode On                               | ✔️ See [PolyMono documentation](https://github.com/FluidSynth/fluidsynth/raw/master/doc/polymono/FluidPolyMono-0004.pdf)                                   |
| (126) Mono Mode                                  | ✔️ See [PolyMono documentation](https://github.com/FluidSynth/fluidsynth/raw/master/doc/polymono/FluidPolyMono-0004.pdf)                                   |
| (127) Poly Mode                                  | ✔️ See [PolyMono documentation](https://github.com/FluidSynth/fluidsynth/raw/master/doc/polymono/FluidPolyMono-0004.pdf)                                   |

## RPN Control Change Implementation Chart

| RPN CC                          | Implementation Status                |
|---------------------------------|--------------------------------------|
| (000) RPN_PITCH_BEND_RANGE      | ✔️ SF2 default modulator             |
| (001) RPN_CHANNEL_FINE_TUNE     | ✔️                                   |
| (002) RPN_CHANNEL_COARSE_TUNE   | ✔️                                   |
| (003) Tuning Program Select     | ✔️                                   |
| (004) Tuning Bank Select        | ✔️                                   |
| (005) Modulation Depth Range    | ✔️ Support added in fluidsynth 2.6.0, see [#1775](https://github.com/FluidSynth/fluidsynth/pull/1775) |
| (006) MPE Configuration Message | ❌ Not yet implemented                |

## NRPN Control Change Implementation Chart

### SF2 NRPNs

✔️ **All SF2 generators can be altered with NRPN Control Change messages.** See section 8.1.2 in the spec.

### AWE32 NRPNs

As of version 2.4.5, fluidsynth also supports NRPN control found in the AWE32. Most of it is implemented according to the "AWE32 Developer's Information Pack", which however was found to be quite buggy, so fluidsynth's behavior may differ from that of real SoundBlaster card. Unless otherwise noted, the data value received by any of these NRPNs overrides the initial generator value of the corresponding SF2 generator.

| NRPN MSB | NRPN LSB | Implementation Status                                                                                              |
|----------|----------|--------------------------------------------------------------------------------------------------------------------|
| 127      | 0        | ✅ overrides `Modulation LFO delay` generator                                                                       |
| 127      | 1        | ✅ overrides `Modulation LFO frequency` generator                                                                   |
| 127      | 2        | ✅ overrides `Vibrato LFO delay` generator                                                                          |
| 127      | 3        | ✅ overrides `Vibrato LFO frequency` generator                                                                      |
| 127      | 4        | ✅ overrides `Modulation envelope delay` generator                                                                  |
| 127      | 5        | ✅ overrides `Modulation envelope attack` generator                                                                 |
| 127      | 6        | ✅ overrides `Modulation envelope hold` generator                                                                   |
| 127      | 7        | ✅ overrides `Modulation envelope decay` generator                                                                  |
| 127      | 8        | ✅ overrides `Modulation envelope sustain` generator                                                                |
| 127      | 9        | ✅ overrides `Modulation envelope release` generator                                                                |
| 127      | 10       | ✅ overrides `Volume envelope delay` generator                                                                      |
| 127      | 11       | ✅ overrides `Volume envelope attack` generator                                                                     |
| 127      | 12       | ✅ overrides `Volume envelope hold` generator                                                                       |
| 127      | 13       | ✅ overrides `Volume envelope decay` generator                                                                      |
| 127      | 14       | ✅ overrides `Volume envelope sustain` generator                                                                    |
| 127      | 15       | ✅ overrides `Volume envelope release` generator                                                                    |
| 127      | 16       | ⚠️ treated as if a regular MIDI Pitch Wheel event is received                                                      |
| 127      | 17       | ✅ overrides `Modulation LFO to pitch` generator                                                                    |
| 127      | 18       | ✅ overrides `Vibrato LFO to pitch` generator                                                                       |
| 127      | 19       | ✅ overrides `Modulation envelope to pitch` generator                                                               |
| 127      | 20       | ✅ overrides `Modulation LFO to volume` generator                                                                   |
| 127      | 21       | ✔️ overrides `Filter cutoff` generator, ranges verified through listening tests (AWE32 documentation is incorrect) |
| 127      | 22       | ✔️ overrides `Filter Q` generator, ranges verified through listening tests (AWE32 documentation is incorrect)      |
| 127      | 23       | ✅ overrides `Modulation LFO to filter cutoff` generator                                                            |
| 127      | 24       | ✅ overrides `Modulation envelope to filter cutoff` generator                                                       |
| 127      | 25       | ⚠️ received data value is transformed by default SF2 chorus modulator before applied to chorus generator           |
| 127      | 26       | ⚠️ received data value is transformed by default SF2 reverb modulator before applied to reverb generator           |


### Roland NRPNs

Additionally, as of version 2.5.0, fluidsynth maps the following Roland NRPNs to regular CCs, allowing them to be overridden in a Soundfont by using modulators. The mapping was implemented according to the [SC8850 owner's manual](https://cdn.roland.com/assets/media/pdf/SC-8850_OM.pdf#page=227&zoom=133,-64,835) pages 227 and 228. Note that this mapping will only occur if the synth is operated in GS mode (i.e. `synth.midi-bank-select` is set to `gs`).

| NRPN MSB | NRPN LSB | Implementation Status             |
|----------|----------|-----------------------------------|
| 0x01     | 0x08     | ✅ mapped to CC 76 (vibrato rate)  |
| 0x01     | 0x09     | ✅ mapped to CC 77 (vibrato depth) |
| 0x01     | 0x0A     | ✅ mapped to CC 78 (vibrato delay) |

## SysEx Messages

!!! Note

    FluidSynth only processes those SysEx messages,
    if the "device-id" in the SysEx message matches the `synth.device-id` setting the synth has been initialized with! Broadcast SysEx messages are always processed.

| SysEx                | Implementation Status                                                                                                                                                                   |
|----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| MIDI Tuning Standard | ✔️ See [`enum midi_sysex_tuning_msg_id`](https://github.com/FluidSynth/fluidsynth/blob/b8fb6c81e1ca27c0bba2f6a0168832214f91d497/src/midi/fluid_midi.h#L195-L207) for supported messages |
| GS DT1               | ✅ Only rhythm / melodic part selection messages are supported (since fluidsynth 2.2.0, see the "Patch Part parameters section in SC-88Pro/8850 owner's manual")                         |
| GM/GM2 mode on       | ✔️                                                                                                                                                                                      |
| GS reset             | ✔️                                                                                                                                                                                      |
| XG reset             | ✔️                                                                                                                                                                                      |