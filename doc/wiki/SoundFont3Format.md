# ➕ The SoundFont3 Format

*RFC: Description of the SoundFont 3 Format*

The SoundFont 3 format (SF3) is an unofficial extension of the SoundFont 2
format (SF2). It was originally created by Werner Schweer for
[MuseScore](https://musescore.org) with the aim of reducing the file size of
their bundled Soundfont(s) by enabling support for Ogg Vorbis compressed
samples.

Support for SF3 has since been incorporated into many other SoundFont software
packages, for example Polyphone, Swami, QSynth, FluidSynth, and other software
synthesizers. As there was no official specification available, the inner workings
and output of the `sf3convert` utility from
[musescore/sftools](https://github.com/musescore/sftools) can be seen as the
most official source of truth regarding this format.

This RFC is meant to be the start of an "official" specification of
the unofficial SF3 format and the basis for further expansion (like adding additional encoder
support, e.g. FLAC or Opus).

Since SF3 is intended as an _extension_, much effort has been taken to remain backward compatibility to SF2 whenever applicable.

Comments, suggestions and any other feedback are very welcome!

Please note: This document only explains the differences of SF3 compared to the [official SF2 specification](http://freepats.zenvoid.org/sf2/sfspec24.pdf). It assumes that the reader is familiar with the SoundFont 2 file structure.

Further note that files produced by cognitone's tool [`sf2convert`](https://github.com/cognitone/sf2convert) are incompatible with this spec and not officially supported by fluidsynth!

## File identification

SF3 files use major version 3 in the *ifil sub-chunk*:
```c
sfVersionTag.wMajor = 3;
```

## Additional flag for sfSampleType field in SHDR

Bit 4 of the `sfSampleType` field is used to indicate a compressed sample.

```c
sfSampleType |= 0x10;
```

The flag does not necessarily indicate OGG Vorbis compression! It rather indicates that the sample has received _any_ kind of encoding/compression. It is up to the application to identify the encoding when decompressing the sample. If the compressed sample is invalid the implementation is encouraged to reject all instruments and presets that make use of this sample.

A sample becomes invalid if any of the following condition applies:

* The type of compression or encoding is unknown.
* The type of compression or encoding is not supported.
* The compression or encoding is recognized to be broken
* The compressed sample is not a mono sample (i.e. has a channel count != 1)

## Breaking Change: Interpretation of sample data index fields in SHDR

If the `0x10` flag in `sfSampleType` set, the interpretation of the four sample data index fields (`dwStart`, `dwEnd`,
`dwStartloop` and `dwEndloop`) in the SHDR sub-chunk has changed:

* `dwStart` points to the first byte of the compressed byte stream, relative to the beginning of the SMPL sub-chunk.
* And `dwEnd` points to the last byte in the compressed data stream (not to the first zero-value sample data point after the sample data as in SF2).
* `dwStartloop` and `dwEndloop` specify loop points relative to the start of the individual uncompressed sample data, in sample data points (words).

If the `0x10` flag is not set, the rules of the SF2 spec apply.


## Individually compressed samples

Mixed sample compression is supported! I.e. there may be regular uncompressed SF2 samples as well any number of individually encoded samples.

If any PCM samples exist they must be placed at the beginning of SMPL chunk, i.e. before the compressed sample byte stream.

It's worth noting that each sample is compressed individually (e.g. by the Ogg Vorbis encoder). The resulting
byte streams of all encoded samples are written to the SMPL sub-chunk. The SMPL chunk however may also contain little-endian PCM samples.

For compressed byte streams, it is not necessary to add the minimum 46 zero-valued sample data points required after each sample.

The length of the SMPL sub-chunk is no longer required to be a multiple of two. Its surrounding LIST chunk is also not padded to a multiple of two as a consequence.

## 24-bit PCM samples

The basic logic of the 24-bit support brought by the 2.04 spec remains unchanged. That means the SM24 sub-chunk may still contain the least significant byte counterpart for every PCM sample. However, compressed samples do not make use of the SM24 chunk.
If a SM24 chunk is present, its respective byte counterparts to the compressed byte stream as stored in the SMPL chunk remain unused. Since all PCM samples are stored before the encoded samples in the SMPL chunk, the size of the SM24 chunk can be kept at the absolute required minimum.

The SM24 size constraint, as required by the SF2 spec, no longer applies.????

## Sample Links

While MuseScore's SF3 converted files do not contain sample link information in SHDR (`wSampleLink` left at 0), pieces of software like Polyphone writes them correctly. It is recommended to preserve them for future editing purposes.
