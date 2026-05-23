\page ReverbOverview Reverb Overview

The following page gives an overview of different reverbator engines. It concentrates on objective criteria rather than trying to elaborate on the "quality" of the reverb engines. Listening examples are provided so you can make up your own opinion. Here are the dry versions. The reverb settings of all examples below share those settings: <code>-o synth.reverb.room-size=0.7 -o synth.reverb.width=1 -o synth.reverb.damp=0 -o synth.reverb.level=0.7</code>


Example 1 - Piano:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/1455_dry.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1455_dry.oga">download/play</a>.
</audio>
\endhtmlonly


Example 2 - Snap:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/1496_dry.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1496_dry.oga">download/play</a>.
</audio>
\endhtmlonly

Example 3 - Water drops + Triangle:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/reverb_water_triangle_test_dry.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/reverb_water_triangle_test_dry.oga">download/play</a>.
</audio>
\endhtmlonly


# 1. Freeverb

---

Freeverb was the reverb engine fluidsynth had used up to including version 2.0.9 and was based on: https://ccrma.stanford.edu/~jos/pasp/Freeverb.html

The reverbator receives a monophonic input signal, which then passes 8 parallel comb filters and finally 4 allpass filters.

The design of the reverbator is not capable of processing a stereo input signal.

Due to its design and the usage of comb filters in particular, the reverb introduces a resonance tone on mid- to high-frequency tones. This is often referred to as "ringing" and perceived unpleasant.

Freeverb is slightly more CPU-expensive than FDN.

Example 1 - Piano:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/1455_freeverb.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1455_freeverb.oga">download/play</a>.
</audio>
\endhtmlonly


Example 2 - Snap:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/1496_freeverb.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1496_freeverb.oga">download/play</a>.
</audio>
\endhtmlonly

Example 3 - Water drops + Triangle:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_freeverb.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_freeverb.oga">download/play</a>.
</audio>
\endhtmlonly

# 2. FDN

---

Due to its rinning nature, Freeverb was replaced by "Feedback Delay Networks"-reverbator in version 2.1.0. It's based on: https://ccrma.stanford.edu/~jos/pasp/FDN_Reverberation.html

FDN receives a monophonic input signal and - as the name suggests - routes it through several delay lines. Fluidsynth's default implementation makes use of 8 delay lines, although there is a compile-time switch to allow using 12 delay lines, that slightly increases the reverb's quality by an increased frequency density. However, it also increases CPU usage beyound what the previous Freeverb would have required, so it was decided to use only 8 delay lines.

To fight ringing and high-frequency reverb "reflections", the delay lines are modulated with a sine. While this is generally inaudible, it can be perceived on certain high-frequent usually clear sounding instruments, like a triangle, causing a slight vibrato effect to be applied to them.

FDN tends to add unnaturally sounding distortions for samples that have a wide frequency range. Take a listen to the "Snap" example 2.

FDN is less CPU-expensive than Freeverb and is not capable of receiving a stereo input signal. The reverb settings (room-size, width, damp, level) were tuned and scaled to match up well with the behavior of the original Freeverb.


Example 1 - Piano:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/1455_fdn.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1455_fdn.oga">download/play</a>.
</audio>
\endhtmlonly


Example 2 - Snap:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/1496_fdn.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1496_fdn.oga">download/play</a>.
</audio>
\endhtmlonly

Example 3 - Water drops + Triangle:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_fdn.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_fdn.oga">download/play</a>.
</audio>
\endhtmlonly


# 3. Lexverb

---

This reverb engine is inspired by Lexicon reverbators. It's the first reverb engine capable of processing a stereo input signal (though due to limitations by fluidsynth, it still only receives a monophonic input signal).

There are 10 allpass filters involved: Lexverb routes the left input channel through 5 allpass filter, and the right input channel through another 5 allpass filters. Delay lines between these filter chains feed a part of the left-wet signal back to the right filter chain and vice versa.

Due to the use of only allpass filters, the gain of the individual frequencies is not changed, only their phase is messed around with. It's similar to the reverb found in many Nintendo64 games... and possibly other stuff from the 90s.

Lexverb currently suffers from the fact that it receives a monophonic input signal, causing the wet-reverb sound to be effectively panned to the center, instead of bouncing left and right.

Lexverb is CPU-cheaper than FDN.


Example 1 - Piano:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/1455_lexverb.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1455_lexverb.oga">download/play</a>.
</audio>
\endhtmlonly


Example 2 - Snap:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/1496_lexverb.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1496_lexverb.oga">download/play</a>.
</audio>
\endhtmlonly

Example 3 - Water drops + Triangle:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_lexverb.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_lexverb.oga">download/play</a>.
</audio>
\endhtmlonly


# 4. Dattorro

---

This "plate-class" reverb engine is named after Jon Dattorro and based on this paper: https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf

A monophonic input signal is fed into a sophisticated network of delay lines and allpass filters to create a rich and colorful reverb sound.

Dattorro is most CPU-expensive.


Example 1 - Piano:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/1455_dat.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1455_dat.oga">download/play</a>.
</audio>
\endhtmlonly


Example 2 - Snap:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/1496_dat.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/1496_dat.oga">download/play</a>.
</audio>
\endhtmlonly

Example 3 - Water drops + Triangle:

\htmlonly
<audio controls preload="metadata">
  <source
    src="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_dat.oga"
    type="audio/ogg"
  />
  Your browser does not support the HTML5 audio element.
  Try opening the file directly:
  <a href="/audio/reverb_water_triangle_test_l0.7_s0.7_w1.0_d0.0_dat.oga">download/play</a>.
</audio>
\endhtmlonly
