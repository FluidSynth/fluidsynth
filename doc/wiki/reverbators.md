# 🌀 Reverb Overview

The following page gives an overview of different reverbator engines provided by fluidsynth. It concentrates on objective criteria, although a quality assessment is attempted below. Sound examples are provided so you can make up your own opinion. Listening with headphones is strongly recommended! The reverb settings of all examples below share those settings:

`-o synth.reverb.room-size=0.7 -o synth.reverb.width=1 -o synth.reverb.damp=0 -o synth.reverb.level=0.7`

We start with the dry versions of the provided sound examples (the MIDIs may be found [here](https://github.com/FluidSynth/testdata/tree/master/reverb)):

Example 1 - Piano:

<audio controls preload="metadata">
  <source
    src="/audio/1455_dry.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1455_dry.oga">download/play</a>.
</audio>

Example 2 - Snap:

<audio controls preload="metadata">
  <source
    src="/audio/1496_dry.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1496_dry.oga">download/play</a>.
</audio>

Example 3 - Water drops + Triangle:

<audio controls preload="metadata">
  <source
    src="/audio/reverb_water_triangle_test_dry.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/reverb_water_triangle_test_dry.oga">download/play</a>.
</audio>


---

## 1. Freeverb

Freeverb was the reverb engine fluidsynth had used up to including version 2.0.9 and was based on: https://ccrma.stanford.edu/~jos/pasp/Freeverb.html

It was added back to fluidsynth in version 2.6.0 as comparison baseline for the other reverbators.

The reverbator receives a monophonic input signal, which then passes 8 parallel comb filters and finally 4 allpass filters.

The design of the reverbator is not capable of processing a stereo input signal.

Due to its design and the usage of comb filters in particular, the reverb introduces a resonance tone on mid- to high-frequency tones. This is often referred to as "ringing" and perceived unpleasant.

Freeverb is slightly more CPU-expensive than [FDN](#2-fdn).

Example 1 - Piano:

<audio controls preload="metadata">
  <source
    src="/audio/1455_freeverb.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1455_freeverb.oga">download/play</a>.
</audio>

Example 2 - Snap:

<audio controls preload="metadata">
  <source
    src="/audio/1496_freeverb.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1496_freeverb.oga">download/play</a>.
</audio>

Example 3 - Water drops + Triangle:

<audio controls preload="metadata">
  <source
    src="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_freeverb.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_freeverb.oga">download/play</a>.
</audio>

---

## 2. FDN

Due to its ringing nature, [Freeverb](#1-freeverb) was replaced by "Feedback Delay Networks"-reverbator in version 2.1.0. It's based on: https://ccrma.stanford.edu/~jos/pasp/FDN_Reverberation.html

FDN receives a monophonic input signal and - as the name suggests - routes it through several delay lines. Fluidsynth's default implementation makes use of 8 delay lines, although there is a compile-time switch to allow using 12 delay lines, that slightly increases the reverb's quality by an increased frequency density. However, it also increases CPU usage beyond what the previous Freeverb would have required, so it was decided to use only 8 delay lines.

To fight ringing and high-frequency reverb "reflections", the delay lines are modulated with a sine. While this is generally inaudible, it can be perceived on certain high-frequent usually clear sounding instruments, like a triangle, causing a slight vibrato effect to be applied to them.

FDN tends to add unnaturally sounding distortions for samples that have a wide frequency range. Take a listen to the "Snap" example 2.

FDN is less CPU-expensive than [Freeverb](#1-freeverb) and is not capable of receiving a stereo input signal. The reverb settings (room-size, width, damp, level) were tuned and scaled to match up well with the behavior of the original Freeverb.

Example 1 - Piano:

<audio controls preload="metadata">
  <source
    src="/audio/1455_fdn.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1455_fdn.oga">download/play</a>.
</audio>

Example 2 - Snap:

<audio controls preload="metadata">
  <source
    src="/audio/1496_fdn.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1496_fdn.oga">download/play</a>.
</audio>

Example 3 - Water drops + Triangle:

<audio controls preload="metadata">
  <source
    src="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_fdn.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_fdn.oga">download/play</a>.
</audio>

---

## 3. Lexverb

This reverb engine is inspired by Lexicon reverbators and is available since fluidsynth 2.6.0. It's the first reverb engine capable of processing a stereo input signal (though due to limitations by fluidsynth, it still only receives a monophonic input signal).

There are 10 allpass filters involved: Lexverb routes the left input channel through 5 allpass filter, and the right input channel through another 5 allpass filters. Delay lines between these filter chains feed a part of the left-wet signal back to the right filter chain and vice versa.

Due to the use of only allpass filters, the gain of the individual frequencies is not changed, only their phase is messed around with. It's similar to the reverb found in many Nintendo64 games... and possibly other stuff from the 90s.

Lexverb currently suffers from the fact that it receives a monophonic input signal, causing the wet-reverb sound to be effectively panned to the center, instead of bouncing left and right.

Lexverb is CPU-cheaper than [FDN](#2-fdn).

Example 1 - Piano:

<audio controls preload="metadata">
  <source
    src="/audio/1455_lexverb.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1455_lexverb.oga">download/play</a>.
</audio>

Example 2 - Snap:

<audio controls preload="metadata">
  <source
    src="/audio/1496_lexverb.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1496_lexverb.oga">download/play</a>.
</audio>

Example 3 - Water drops + Triangle:

<audio controls preload="metadata">
  <source
    src="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_lexverb.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_lexverb.oga">download/play</a>.
</audio>

---

## 4. Dattorro

This "plate-class" reverb engine is named after Jon Dattorro and based on this paper: https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf

It is available since fluidsynth 2.6.0. A monophonic input signal is fed into a sophisticated network of delay lines and allpass filters to create a rich and colorful reverb sound.

Dattorro is a bit more CPU-expensive than [Freeverb](#1-freeverb).

Example 1 - Piano:

<audio controls preload="metadata">
  <source
    src="/audio/1455_dat.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1455_dat.oga">download/play</a>.
</audio>

Example 2 - Snap:

<audio controls preload="metadata">
  <source
    src="/audio/1496_dat.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1496_dat.oga">download/play</a>.
</audio>

Example 3 - Water drops + Triangle:

<audio controls preload="metadata">
  <source
    src="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_dat.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_dat.oga">download/play</a>.
</audio>


## 5. Signalsmith

This is a FDN based reverb engine, similar to [Fluidsynth's original FDN reverb](#2-fdn), but more sophisticated and better engineered.

It is available since fluidsynth 2.6.0, but only available when fluidsynth was compiled with Signalsmith Audio Basics support - the same library that also provides the limiter implementation. Signalsmith reverb receives a stereo input signal (though due to limitations by fluidsynth, it still only receives a monophonic input signal) and creates a rich and colorful reverb sound similar to [Dattorro](#4-dattorro).

It performs better on transient and high-frequent sounds.

Signalsmith is significantly more CPU-expensive than [Dattorro](#4-dattorro). Furthermore, its runtime varies by a factor of 10 and might therefore not be suited for real-time performances.

Example 1 - Piano:

<audio controls preload="metadata">
  <source
    src="/audio/1455_smith.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1455_smith.oga">download/play</a>.
</audio>

Example 2 - Snap:

<audio controls preload="metadata">
  <source
    src="/audio/1496_smith.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1496_smith.oga">download/play</a>.
</audio>

Example 3 - Water drops + Triangle:

<audio controls preload="metadata">
  <source
    src="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_smith.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_smith.oga">download/play</a>.
</audio>


## Comparison

### Architecture & Design

|  | Freeverb | FDN | Lexverb | Dattorro | Signalsmith |
|---|---|---|---|---|---|
| **Algorithm family** | Schroeder-Moorer | Jot FDN | Lexicon-inspired allpass network | Plate reverb | Diffused FDN |
| **Core structure** | 8 parallel combs + 4 series allpass, separate L/R banks, [Flowchart](https://ccrma.stanford.edu/~jos/pasp/Freeverb.html) | 8 modulated delay lines + feedback matrix , [Flowchart](https://github.com/FluidSynth/fluidsynth/blob/a481306665d7d789ef7eb5a63ba4f88f3a6f568b/src/rvoice/fluid_rev_fdn.cpp#L32-L49) | 10 allpass filters (5/side) + 2 L↔R cross-delay lines, [Flowchart](https://github.com/FluidSynth/fluidsynth/blob/a481306665d7d789ef7eb5a63ba4f88f3a6f568b/src/rvoice/fluid_rev_lexverb.h#L16-L37) | Predelay + 4 input allpass diffusers + 2 tank loops + 14 output taps, [Flowchart](https://github.com/FluidSynth/fluidsynth/blob/a481306665d7d789ef7eb5a63ba4f88f3a6f568b/src/rvoice/fluid_rev_dattorro.h#L22-L58) | 8-ch FDN + 4 Hadamard diffusion stages + early reflections stage |
| **Feedback/mixing matrix** | None (parallel combs) | `A = P − (2/N)·uuᵀ` (Jot/Householder) | Implicit via cross-feedback delays | Fixed plate tank topology | Householder (feedback) + Hadamard (diffusion) |
| **Diffusion** | 4 allpass per side (Freeverb approximation) | Input tone corrector only; matrix provides diffusion | 5-stage allpass cascade per L/R side (Schroeder) | 4-stage input diffuser + 2 decay diffusions per tank | Explicit pre-diffusion and post-diffusion |
| **Modulation** | None | All 8 lines, sinusoidal LFO, 1st-order allpass interpolation | None | None | 3 of 8 channels, sinusoidal LFO, 7th-order Lagrange interpolation |
| **Damping filter** | 1-pole LPF inside each comb | 1-pole LPF per line | 1-pole LPF on AP4, AP9 and both cross-delays | 1-pole LPF on input (bandwidth) + per-tank LPF | Biquad low-shelf + high-shelf per FDN channel; separate LP/HP output cut |
| **Early reflections** | No | No | No | No | Yes |
| **Stereo input cabable** | 🔴 Mono-in by design | 🔴 Mono-in by design | 🟢 Stereo capable (⚠️ fluidsynth feeds mono currently) | 🔴 Mono-in by design | 🟢 True stereo in/out (⚠️ fluidsynth feeds mono currently) |
| **Denormalisation** | DC offset injection | DC offset injection | Not handled | Not handled | Not handled |
| **CPU cost** | More than FDN | Less than Freeverb | Cheapest | Slightly more expensive than Freeverb | Most expensive |

### Quality Assessment

|  | Freeverb | FDN | Lexverb | Dattorro | Signalsmith |
|---|---|---|---|---|---|
| **Tail smoothness** | 🔴 Poor | 🟡 Fair, but modulated | 🟢 Clean | 🟢 Clean | 🟢 Excellent |
| **Ringing** | 🔴 High (comb resonances) | 🟡 Low | 🟢 Minimal | 🟡 Low | 🟢 Minimal |
| **Transient broadband handling** | 🟡 Fair | 🔴 Distorted | 🟢 Good | 🟢 Good | 🟢 Excellent |
| **Echo density** | 🔴 Poor | 🟡 Fair, no pre-diffusion | 🟡 Sparse, no pre-diffusion | 🟢 Good (4-stage input diffuser) | 🟢 Excellent (multi-stage) |
| **Stereo output** | 🟡 Separate L/R comb banks | 🟡 ±1 gain vector providing functional stereo, but not deeply decorrelated | 🟢 Cross-coupled stereo by design | 🟡 14-tap decorrelated L/R output | 🟢 Full stereo by design |
| **Spectral character** | Colored (comb resonances) | Colored, sometimes overloaded or synthetic | Flat & simple | Rich, colorful (plate character) | Rich, colorful |
| **Best for** | Legacy / compatibility | ? | CPU-constrained use, nostalgic video game sound | Smooth long reverbs | Rich creative "real-room" reverb, acoustic realism |
| **Worst for** | High-frequent solo instruments, long decay | Broadband transients (snaps, clicks) | ? | ? | Real-time and live-performances |
| **Overall** | ⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |


*[Tail smoothness]: Exponential decay of the "wet" reverb sound, free of any artefacts (no flutter, no pumping)
*[Ringing]: Audible resonant peaks ("metallic" sound), especially on transients and silence decay
*[Transient broadband handling]: Handling of high-amplitude and short-lived burst sound (=transient), whose energy is spread across a wide range of frequencies (broadband), e.g. like the "snap" percussion sound sample.
*[Echo density]: Density of reverb reflections
*[Spectral character]: Whether the reverb tail sounds tonally neutral or noticeably colored
*[tap]: A point in a filter or delay line processing chain, where audio samples are picked-off from.
*[FDN]: Feedback Delay Network, i.e. several delay lines interconnected with each other
