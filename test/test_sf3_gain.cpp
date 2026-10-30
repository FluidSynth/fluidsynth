#include "test.h"
#include "fluid_sffile.h"
#include "fluid_sfont.h"
#include <sndfile.h>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <limits>

/* Regression for #1856: saturate decoded peaks without changing the gain of
 * the remaining samples. */

static void test_stereo_rejected(int format, const fluid_file_callbacks_t *callbacks)
{
    constexpr int frames = 1025;
    const char *name = "test_sf3_stereo.ogg";
    float input[frames * 2] = {};
    SF_INFO info = {};
    info.samplerate = 44100;
    info.channels = 2;
    info.format = format;
    SNDFILE *encoded = sf_open(name, SFM_WRITE, &info);
    TEST_ASSERT(encoded != nullptr);
    TEST_ASSERT(sf_writef_float(encoded, input, frames) == frames);
    TEST_ASSERT(sf_close(encoded) == 0);

    info = {};
    encoded = sf_open(name, SFM_READ, &info);
    TEST_ASSERT(encoded != nullptr && info.frames == frames && info.channels == 2);
    TEST_ASSERT(sf_close(encoded) == 0);

    SFData source = {};
    source.sffd = static_cast<FILE *>(callbacks->fopen(name));
    TEST_ASSERT(source.sffd != nullptr);
    TEST_SUCCESS(callbacks->fseek(source.sffd, 0, SEEK_END));
    source.samplesize = callbacks->ftell(source.sffd);
    source.fcbs = callbacks;
    fluid_rec_mutex_init(source.mtx);
    short *decoded = nullptr;
    char *data24 = nullptr;

    /* Reject stereo before allocating or decoding mono sample buffers. */
    TEST_ASSERT(fluid_sffile_read_sample_data(&source, 0, source.samplesize - 1,
                FLUID_SAMPLETYPE_OGG_VORBIS, &decoded, &data24) == -1);
    TEST_ASSERT(decoded == nullptr && data24 == nullptr);

    fluid_rec_mutex_destroy(source.mtx);
    TEST_SUCCESS(callbacks->fclose(source.sffd));
    TEST_ASSERT(std::remove(name) == 0);
}

int main()
{
    /* Exercise multiple decode blocks and a one-frame tail. */
    constexpr int frames = 8193;
    const char *name = "test_sf3_gain.ogg";
    fluid_settings_t *settings = new_fluid_settings();
    TEST_ASSERT(settings != nullptr);
    fluid_sfloader_t *loader = new_fluid_defsfloader(settings);
    TEST_ASSERT(loader != nullptr);
    const fluid_file_callbacks_t *callbacks = &loader->file_callbacks;

    /* Float WAV preserves exact ties and their adjacent floats, unlike Vorbis
     * or PCM16. Exercise the same conversion through the SF3 loader. */
    const float half_lsb = 0.5f / 32768.0f;
    const struct
    {
        float input;
        short expected;
    } boundaries[] =
    {
        {0.0f, 0}, {-0.0f, 0},
        {half_lsb, 0}, {-half_lsb, 0},
        {3 * half_lsb, 2}, {-3 * half_lsb, -2},
        {std::nextafter(half_lsb, 0.0f), 0},
        {std::nextafter(half_lsb, 1.0f), 1},
        {std::nextafter(-half_lsb, 0.0f), 0},
        {std::nextafter(-half_lsb, -1.0f), -1},
        {std::nextafter(3 * half_lsb, 0.0f), 1},
        {std::nextafter(3 * half_lsb, 1.0f), 2},
        {std::nextafter(-3 * half_lsb, 0.0f), -1},
        {std::nextafter(-3 * half_lsb, -1.0f), -2},
        {-1.0f, -32768}, {32767.0f / 32768.0f, 32767},
        {1.0f, 32767}, {-1.25f, -32768}, {1.25f, 32767},
        {std::nextafter(-1.0f, 0.0f), -32768},
        {std::nextafter(-1.0f, -2.0f), -32768},
        {std::nextafter(32767.0f / 32768.0f, 0.0f), 32767},
        {std::nextafter(32767.0f / 32768.0f, 1.0f), 32767},
        {std::numeric_limits<float>::quiet_NaN(), 0},
        {std::numeric_limits<float>::infinity(), 32767},
        {-std::numeric_limits<float>::infinity(), -32768}
    };
    constexpr int boundary_count = FLUID_N_ELEMENTS(boundaries);

    /* Also cover the non-OGG compatibility path accepted by the SF3 loader. */
    for(int format : {SF_FORMAT_OGG | SF_FORMAT_VORBIS, SF_FORMAT_WAV | SF_FORMAT_PCM_16,
                     SF_FORMAT_WAV | SF_FORMAT_FLOAT})
    {
        test_stereo_rejected(format, callbacks);

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
                input[i] = format == (SF_FORMAT_WAV | SF_FORMAT_FLOAT) &&
                           i < boundary_count ? boundaries[i].input
                           : amplitude * std::sin(i * 0.14);
            }

            TEST_ASSERT(sf_writef_float(encoded, input, frames) == frames);
            TEST_ASSERT(sf_close(encoded) == 0);

            /* Vorbis is lossy, so use decoded floats rather than the original
             * waveform as the reference for PCM conversion. */
            info = {};
            encoded = sf_open(name, SFM_READ, &info);
            TEST_ASSERT(encoded != nullptr && info.frames == frames && info.channels == 1);
            TEST_ASSERT(sf_readf_float(encoded, reference, frames) == frames);
            TEST_ASSERT(sf_close(encoded) == 0);

            SFData source = {};
            source.sffd = static_cast<FILE *>(callbacks->fopen(name));
            TEST_ASSERT(source.sffd != nullptr);
            TEST_SUCCESS(callbacks->fseek(source.sffd, 0, SEEK_END));
            source.samplesize = callbacks->ftell(source.sffd);
            source.fcbs = callbacks;
            fluid_rec_mutex_init(source.mtx);
            short *decoded = nullptr;
            char *data24 = nullptr;
            TEST_ASSERT(fluid_sffile_read_sample_data(&source, 0, source.samplesize - 1,
                        FLUID_SAMPLETYPE_OGG_VORBIS, &decoded, &data24) == frames);

            int saturated = 0;
            for(int i = 0; i < frames; i++)
            {
                const float value = reference[i];
                int expected;
                if(format == (SF_FORMAT_WAV | SF_FORMAT_FLOAT) && i < boundary_count)
                {
                    expected = boundaries[i].expected;
                }
                else
                {
                    expected = value >= 32767.0f / 32768.0f ? 32767
                               : value <= -1 ? -32768 : std::lrint(value * 32768.0f);
                }

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
            TEST_SUCCESS(callbacks->fclose(source.sffd));
        }
    }

    delete_fluid_sfloader(loader);
    delete_fluid_settings(settings);
    TEST_ASSERT(std::remove(name) == 0);
    return EXIT_SUCCESS;
}
