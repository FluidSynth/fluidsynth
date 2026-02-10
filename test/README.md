 
This directory contains small executables to verify fluidsynths correct behaviour, i.e. unit tests.

#### Do *not* blindly use the tests as template for your application!

Although some tests might serve as educational demonstration of how to use certain parts of fluidsynth,
they are **not** intended to do so! It is most likely that those tests will consist of many hacky parts
that are necessary to test fluidsynth (e.g. including fluidsynth's private headers to access internal
data types and functions). For user applications this programming style is strongly discouraged!
Keep referring to the documentation and code examples listed in the [API documentation](https://www.fluidsynth.org/api/).

#### Developers

To add a unit test just duplicate an existing one, give it a unique name and update the CMakeLists.txt by

* adding a call to `ADD_FLUID_TEST()` and
* a dependency to the custom `check` target.

Execute the tests via `make check`. Unit tests should use the `VintageDreamsWaves-v2.sf2` as test soundfont.
Use the `TEST_SOUNDFONT` macro to access it.

#### Manual audio regression tests

The manual render suite (`check_manual`) can be compared against a reference FluidSynth revision using
`test/run-manual-regression.sh`. The script builds the current checkout and a reference revision, renders
the audio, and then reports SNR, RMS, and absolute differences using SoX.
It requires `sox`, `cmake`, and `git`, plus the build dependencies for FluidSynth (including libsndfile).

Example:

```
REFERENCE_REF=HEAD~1 SNR_MIN=60 RMS_MAX=0.0001 ABS_MAX=0.01 test/run-manual-regression.sh
```

Additional CMake options can be provided via `REGRESSION_CMAKE_FLAGS`.
Use `--allow-missing` (or `ALLOW_MISSING=1`) to keep going when file pairs are missing.
