# ‍🔬 About SoundFonts

**SoundFont** is a file format for sample-based instrument sounds. You will need a SoundFont to use FluidSynth. If you are not familiar with them, check out Josh Green's [**Introduction to SoundFonts**](https://freepats.zenvoid.org/sf2/SF2_Intro.txt) and [**Soundfont 2.1 application note**](https://freepats.zenvoid.org/sf2/sfapp21.pdf).

If you just need to play General MIDI files, these SoundFonts are known to work well with FluidSynth: 

  * [S. Christian Collins GeneralUser GS](http://www.schristiancollins.com/generaluser.php) - 30 MB 
  * [Fluid (R3) General MIDI SoundFont (GM)](https://packages.debian.org/fluid-soundfont-gm) - 140 MB 

## Soundfont Resources

  * [Polyphone Soundfont Collection](https://www.polyphone-soundfonts.com/download-soundfonts) - A collection of Soundfonts on the Polyphone website
  * [Hammersound](http://www.hammersound.net/) - A nice resource for downloading free SoundFont instrument files.
  * [Musical artifacts](https://musical-artifacts.com/) - A website for sharing SoundFonts.
  * [Magic Sound Font, version 2.0](http://www.personalcopy.com/sfarkfonts1.htm) (68 MB)
  * [Arachno SoundFont, version 1.0](http://www.arachnosoft.com/main/download.php?id=soundfont-sf2) (148MB)
  * [TimGM6mb](http://sourceforge.net/p/mscore/code/HEAD/tree/trunk/mscore/share/sound/TimGM6mb.sf2?format=raw) (6 MB)
  * [MuseScore_General.sf2](ftp://ftp.osuosl.org/pub/musescore/soundfont/MuseScore_General/MuseScore_General.sf2) (208 MB)
  * [Timbres Of Heaven GM_GS_XG_SFX V 3.4](http://midkar.com/soundfonts/) (246 MB)
  * [Sonatina Symphonic Orchestra](http://ftp.osuosl.org/pub/musescore/soundfont/Sonatina_Symphonic_Orchestra_SF2.zip) (503 MB uncompressed)
  * [Aegean Symphonic Orchestra v2.5 universal](https://sites.google.com/view/hed-sounds/aegean-symphonic-orchestra) (350 MB)
  * [Salamander C5 Light](https://sites.google.com/view/hed-sounds/salamander-c5-light) (25 MB)


## SoundFont editors

- Project **SWAMI** by Josh Green (Win, Linux, macOS), http://www.swamiproject.org/

- **Polyphone** (Win, Linux, macOS), http://polyphone-soundfonts.com/en/

- **Vienna SoundFont Studio** by Creative Technology Ltd. (Win)

- **SynthFont Viena** (Win), http://www.synthfont.com/Viena_news.html

- **Alive** Soundfont Editor by Soundfaction (Win)

- **SpessaFont** SF2/DLS editor (Web), https://spessasus.github.io/SpessaFont/

!!! Warning
 
    We cannot recommend using Audio Compositor for creating or editing Soundfonts, 
    as it generates files that violate the Soundfont2 spec (specifically the order of generators as defined in section 8.1.2) and are therefore **unusable with FluidSynth!**

## Conversion Tools

- [CDxtract](http://www.cdxtract.com) by Safta Consulting, Inc.  (Win)

- [ReCycle](http://www.propellerheads.se/products/recycle/) by Propellerhead Software (Win & Mac),

- [Translator](http://www.chickensys.com/translator) by Rubber Chicken Software (Win & Mac),

- [Awave Studio](https://www.fmjsoft.com/awavestudio.html) by FMJ-Software (Win)

- [sf1to2](http://www.ibiblio.org/thammer/HammerSound/localfiles/soundfonts/sf1to2.zip) (Dos)

- [DLS to SF2 converter](https://spessasus.github.io/dls-to-sf2) (Web)

!!! Note
 
    Most of the editors mentioned above may also perform conversions to some extent.

## Software SoundFont Synthesizers:

- LiveSynth Pro DXi and Crescendo from LiveUpdate (Win)

- Unity DS-1 from Bitheadz (Win & Mac)

- [QuickTime 5](http://www.apple.com/quicktime/) from Apple (Win & Mac)

- [Logic from eMagic](http://www.emagic.de) (Mac)

- [SynthFont](http://www.synthfont.com/) (Win)

- [BASSMIDI](https://www.un4seen.com/index.php) (Win)

- [SpessaSynth](https://spessasus.github.io/SpessaSynth) (Web)

## Developer Resources

The SoundFont format was originally created by Creative Labs and EMU Systems and used in the SoundBlaster AWE 32 and later cards. There are now many other hardware platforms and software synthesizers supporting this format. SoundFont 2.0 and later are open formats and the specification is freely available. 

  * [Wikipedia SoundFont page](http://en.wikipedia.org/wiki/SoundFont) - Good overview of SoundFont format and other resources.
  * [Soundfont 2.1 application note](http://freepats.zenvoid.org/sf2/sfapp21.pdf) - PDF document describing fundamental SoundFont concepts.
    (should be read first as an overview before diving in [SoundFont 2.4 specification](http://freepats.zenvoid.org/sf2/sfspec24.pdf)).
  * [SoundFont 2.4 specification](http://freepats.zenvoid.org/sf2/sfspec24.pdf) - PDF document describing SoundFont format technical details.
  * The MIDI Manufacturers Association has a standard called [Downloadable
  Sounds (DLS)](https://midi.org/dls) that closely resembles the Soundfont Specifications.
  * [Creative Labs Developer Documentation](https://web.archive.org/web/20100728160132/http://connect.creativelabs.com/developer/SoundFont/Forms/AllItems.aspx) - Specifications, docs, SF2 test files, etc. (on archive.org, the original is no longer accessible)
  * [Soundfont.com FAQ](https://web.archive.org/web/20070107111507/http://www.soundblaster.com/soundfont/faqs/) (on archive.org, the original is no longer accessible)
  * [MuseScore's officially unofficial SoundFont 3 extension](SoundFont3Format.md) - An extension of the SF2 format that allows sample compression.
  * [SoundFont spec compliance test](https://github.com/mrbumpy409/SoundFont-Spec-Test) - An excellent test suite created by S. Christian Collins

## FluidSynth's implementation details of the SoundFont 2 spec

The main goal of FluidSynth is to implement SoundFont 2 spec as accurately as possible. However, some minor adjustments have been made which are described here.

1. The default "note velocity to filter cut off" modulator is inconsistently defined by the spec and is therefore [actively disabled by fluidsynth](https://github.com/FluidSynth/fluidsynth/blob/b1fbace6cb29f4e54d445495ff44f527401285d7/src/synth/fluid_mod.c#L399-L431). Generally, this is no problem, as many people feel that musically it doesn't make sense anyway. Therefore, it is usually not missed by users.

2. FluidSynth applies a custom default modulator to every SoundFont in order to handle CC 8 (Balance) correctly. For details, pls. have a look at [this Pull Request](https://github.com/FluidSynth/fluidsynth/pull/317) or the [related discussion on the mailing list](http://lists.nongnu.org/archive/html/fluid-dev/2018-01/msg00006.html).

3. The SoundFont spec allows to link multiple modulators to each other. An [attempt](https://github.com/FluidSynth/fluidsynth/pull/505) was made to implement this. However, during implementation, it was discovered that this feature is essentially "under-specified". Due to a lack for real-world use-cases and because it's hard to understand for a SoundFont designer what's going on in a linked modulator chain, implementing this feature was dropped.

4. According to the spec, stereo sample pairs should both use pitch generators from the right channel. This is not implemented at the moment, as we are lacking an example SoundFont file where this issue would really make an audible difference, see [related issue](https://github.com/FluidSynth/fluidsynth/issues/61).

5. The spec makes various constraints to sample loop points and is absolutely clear, that samples should be dropped or ignored when these conditions are not met. On the other hand, those constraints do not consider the effects of loop offset modulators on instrument level, which could potentially turn invalid sample loops into valid loops on instrument level, or turn valid sample loops into invalid ones. This makes the entire sample loop topic yet another "gray-area" in the SoundFont spec. Rather than messing around with incorrect sample loops or even dropping entire samples, FluidSynth will be very permissive on sample loops. It reports any problems it experiences, but generally leaves them as-is.

6. As of version 2.4.0, fluidsynth supports the sample mode 2 introduced in Polyphone 2.4, see [#1400](https://github.com/FluidSynth/fluidsynth/pull/1400) for details.

7. Starting with version 2.4.4, fluidsynth will no longer enforce the 46 zero-sample-padding space between samples, as mentioned in section 7.10 of the spec.

8. Starting with version 2.4.0, fluidsynth understands NRPN used by the AWE32 and tries to map those into the Soundfont2 world by overriding the initial amount of the destination generators.

9. Starting with version 2.5.0, fluidsynth supports dealing with default modulators read from `DMOD` INFO-subchunk, see [#1582](https://github.com/FluidSynth/fluidsynth/issues/1582), [here](https://github.com/spessasus/soundfont-proposals/blob/ff1e5d0af91f80f4b51dedbb7b6673fa04bbcd99/default_modulators.md) or [here](https://github.com/SFe-Team-was-taken/SFe/discussions/38) for details.

## Differentiation to SBK / SF1

Before Soundfont2 existed, EMU released the AWE32 hardware synthesizer. The sound bank format used by this synth (and also by later synth's like Sound Blaster Live! and Audigy2 for backward compatibility) is known as SBK - a predecessor of the SF2 standard, whose implementation was heavily defined by its hardware. Due to technical reasons, fluidsynth isn't capable of handling SBK files. In order to use them, SBK needs to be converted to SF2.

A first starting point is to use EMU's "SoundFont Librarian" program. Pls. google for a `sflib10c.zip` on the web. Load the SBK and safe it as SF2. If you like the result, you're done at this point.

However, most SBK files converted to SF2 need further edits in order for the SoundFont to render correctly on a SoundFont 2.01 or 2.04-compliant device. This is also true for version 2.00 SoundFonts such as those created on the Sound Blaster Live!. Here is a list of the changes that must be made across all the SoundFonts to approximate SoundFont 1.0 / 2.0 behavior:

1. Disable default *velocity→filter cutoff* modulator for each instrument (both 2.01 and 2.04 spec versions).
2. Enable *velocity→filter cutoff* modulator (2.04 spec version) for any samples with volume envelope attack at 0.008 seconds or greater.
3. Re-route mod wheel (CC1) and channel aftertouch to modulate modLFO (LFO1) instead of vibLFO (LFO2). (Was CC1 switched over to using vibLFO before SoundFont version 2.01?)
4. Increase reverb (CC91) and chorus (CC93) controller sensitivity to better match AWE32 fx levels when using Audigy 2 or FluidSynth.
5. Replace ROM samples with RAM copies.
6. In the original SoundFont implementation, if a note was played on a drum kit (MIDI channel 10) on bank 1 or higher that didn't have an assigned sample in the preset, the corresponding note from bank 0 would be played instead. This is not true for SoundFont 2.01 or 2.04 devices, where the notes missing from the bank 1 preset will just play silence. I'm not sure whether this was changed in SoundFont revision 2.0 or 2.01. Anyway, some AWE32 songs rely on this behavior, so the missing percussion sounds must be copied into the SoundFont's percussion preset from a GM bank.
7. Fix various SoundFont Librarian conversion errors as I encounter them. This part is very hard to do without reference to any AWE32 recordings. For example, I was able to find and fix a very bad pitch error in *Tekkniko* by Niko Boese thanks to [this YouTube video](https://www.youtube.com/watch?v=FZKDBpK9nbA). Without that, I might never have noticed one instrument playing inaudibly 5 octaves below its correct pitch.
