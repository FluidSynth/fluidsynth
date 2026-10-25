#include "test.h"
#include "fluid_sffile.h"
#include "fluid_sfont.h"
#include <sndfile.h>
#include <cmath>
#include <cstdio>
#include <initializer_list>

/* Regression for #1856: saturate decoded peaks without changing the gain of
 * the remaining samples. */

static int read_bytes(void *ptr, fluid_long_long_t count, void *file)
{
    return std::fread(ptr, 1, count, static_cast<FILE *>(file)) == static_cast<size_t>(count) ? FLUID_OK : FLUID_FAILED;
}

static int seek_bytes(void *file, fluid_long_long_t offset, int origin)
{
    return std::fseek(static_cast<FILE *>(file), offset, origin) == 0 ? FLUID_OK : FLUID_FAILED;
}

static fluid_long_long_t tell_bytes(void *file)
{
    return std::ftell(static_cast<FILE *>(file));
}

int main()
{
    /* Exercise multiple decode blocks and a one-frame tail. */
    constexpr int frames = 8193;
    const char *name = "test_sf3_gain.ogg";
    const fluid_file_callbacks_t callbacks = {nullptr, read_bytes, seek_bytes, nullptr, tell_bytes};

    /* Also cover the non-OGG compatibility path accepted by the SF3 loader. */
    for(int format : {SF_FORMAT_OGG | SF_FORMAT_VORBIS, SF_FORMAT_WAV | SF_FORMAT_PCM_16})
    {
        /* Quiet and normal samples must keep their gain; loud samples must clip. */
        for(float amplitude : {0.01f, 0.5f, 1.25f})
        {
            float input[frames], reference[frames];
            SF_INFO info = {};
            info.samplerate = 44100;
            info.channels = 1;
            info.format = format;
            SNDFILE *encoded = sf_open(name, SFM_WRITE, &info);
            TEST_ASSERT(encoded != nullptr);

            for(int i = 0; i < frames; i++)
            {
                input[i] = amplitude * std::sin(i * 0.14);
            }

            TEST_ASSERT(sf_writef_float(encoded, input, frames) == frames);
            TEST_ASSERT(sf_close(encoded) == 0);

            /* Vorbis is lossy, so use decoded floats rather than the original
             * waveform as the reference for PCM conversion. */
            info = {};
            encoded = sf_open(name, SFM_READ, &info);
            TEST_ASSERT(encoded != nullptr && info.frames == frames);
            TEST_ASSERT(sf_readf_float(encoded, reference, frames) == frames);
            sf_close(encoded);

            SFData source = {};
            source.sffd = std::fopen(name, "rb");
            TEST_ASSERT(source.sffd != nullptr);
            std::fseek(source.sffd, 0, SEEK_END);
            source.samplesize = std::ftell(source.sffd);
            source.fcbs = &callbacks;
            fluid_rec_mutex_init(source.mtx);
            short *decoded = nullptr;
            char *data24 = nullptr;
            TEST_ASSERT(fluid_sffile_read_sample_data(&source, 0, source.samplesize - 1,
                        FLUID_SAMPLETYPE_OGG_VORBIS, &decoded, &data24) == frames);

            int saturated = 0;
            for(int i = 0; i < frames; i++)
            {
                const float value = reference[i];
                const int expected = value >= 32767.0f / 32768.0f ? 32767
                                     : value <= -1 ? -32768 : std::lrint(value * 32768.0f);
                TEST_ASSERT(decoded[i] == expected);
                saturated += value > 1 || value < -1;
            }

            /* Ensure the loud Vorbis case triggers the full-scale overflow
             * that used to rescale the entire waveform. */
            if(amplitude > 1 && (format & SF_FORMAT_OGG))
            {
                TEST_ASSERT(saturated > 0);
            }

            FLUID_FREE(decoded);
            fluid_rec_mutex_destroy(source.mtx);
            std::fclose(source.sffd);
        }
    }

    std::remove(name);
    return EXIT_SUCCESS;
}
