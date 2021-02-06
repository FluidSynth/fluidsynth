/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

/* fluid_oboe.c
 *
 * Audio driver for Android Oboe.
 *
 * This file may make use of C++14, because it's required by oboe anyway.
 */

extern "C" {

#include "fluid_adriver.h"
#include "fluid_settings.h"

} // extern "C"

#if OBOE_SUPPORT

#include <oboe/Oboe.h>
#include <sstream>
#include <stdexcept>

using namespace oboe;

constexpr int NUM_CHANNELS = 2;

class OboeAudioStreamCallback;

/** fluid_oboe_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct
{
    fluid_audio_driver_t driver;
    fluid_synth_t *synth = nullptr;
    bool cont = false;
    std::unique_ptr<OboeAudioStreamCallback> oboe_callback;
    std::shared_ptr<AudioStream> stream;
} fluid_oboe_audio_driver_t;


class OboeAudioStreamCallback : public AudioStreamCallback
{
public:

    OboeAudioStreamCallback(void *userData)
        : user_data(userData)
    {
    }

    DataCallbackResult onAudioReady(AudioStream *stream, void *audioData, int32_t numFrames)
    {
        fluid_oboe_audio_driver_t *dev = static_cast<fluid_oboe_audio_driver_t *>(this->user_data);

        if(!dev->cont)
        {
            return DataCallbackResult::Stop;
        }

        if(stream->getFormat() == AudioFormat::Float)
        {
            fluid_synth_write_float(dev->synth, numFrames, static_cast<float *>(audioData), 0, 2, static_cast<float *>(audioData), 1, 2);
        }
        else
        {
            fluid_synth_write_s16(dev->synth, numFrames, static_cast<short *>(audioData), 0, 2, static_cast<short *>(audioData), 1, 2);
        }

        return DataCallbackResult::Continue;
    }

private:
    void *user_data;
};

constexpr char SRCQ_SET[] = "audio.oboe.sample-rate-conversion-quality";

void fluid_oboe_audio_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_int(settings, "audio.oboe.id", 0, 0, 0x7FFFFFFF, 0);

    fluid_settings_register_str(settings, "audio.oboe.sharing-mode", "Shared", 0);
    fluid_settings_add_option(settings,   "audio.oboe.sharing-mode", "Shared");
    fluid_settings_add_option(settings,   "audio.oboe.sharing-mode", "Exclusive");

    fluid_settings_register_str(settings, "audio.oboe.performance-mode", "None", 0);
    fluid_settings_add_option(settings,   "audio.oboe.performance-mode", "None");
    fluid_settings_add_option(settings,   "audio.oboe.performance-mode", "PowerSaving");
    fluid_settings_add_option(settings,   "audio.oboe.performance-mode", "LowLatency");

    fluid_settings_register_str(settings, SRCQ_SET, "Medium", 0);
    fluid_settings_add_option(settings,   SRCQ_SET, "None");
    fluid_settings_add_option(settings,   SRCQ_SET, "Fastest");
    fluid_settings_add_option(settings,   SRCQ_SET, "Low");
    fluid_settings_add_option(settings,   SRCQ_SET, "Medium");
    fluid_settings_add_option(settings,   SRCQ_SET, "High");
    fluid_settings_add_option(settings,   SRCQ_SET, "Best");
}

static oboe::SampleRateConversionQuality get_srate_conversion_quality(fluid_settings_t *settings)
{
    oboe::SampleRateConversionQuality q;

    if(fluid_settings_str_equal(settings, SRCQ_SET, "None"))
    {
        q = oboe::SampleRateConversionQuality::None;
    }
    else if(fluid_settings_str_equal(settings, SRCQ_SET, "Fastest"))
    {
        q = oboe::SampleRateConversionQuality::Fastest;
    }
    else if(fluid_settings_str_equal(settings, SRCQ_SET, "Low"))
    {
        q = oboe::SampleRateConversionQuality::Low;
    }
    else if(fluid_settings_str_equal(settings, SRCQ_SET, "Medium"))
    {
        q = oboe::SampleRateConversionQuality::Medium;
    }
    else if(fluid_settings_str_equal(settings, SRCQ_SET, "High"))
    {
        q = oboe::SampleRateConversionQuality::High;
    }
    else if(fluid_settings_str_equal(settings, SRCQ_SET, "Best"))
    {
        q = oboe::SampleRateConversionQuality::Best;
    }
    else
    {
        char buf[256];
        fluid_settings_copystr(settings, SRCQ_SET, buf, sizeof(buf));
        std::stringstream ss;
        ss << "'" << SRCQ_SET << "' has unexpected value '" << buf << "'";
        throw std::runtime_error(ss.str());
    }

    return q;
}

/*
 * new_fluid_oboe_audio_driver
 */
fluid_audio_driver_t *
new_fluid_oboe_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    fluid_oboe_audio_driver_t *dev = nullptr;

    try
    {
        Result result;
        AudioStreamBuilder builder_obj;
        AudioStreamBuilder *builder = &builder_obj;

        double sample_rate;
        int is_sample_format_float;
        int device_id;
        int sharing_mode; // 0: Shared, 1: Exclusive
        int performance_mode; // 0: None, 1: PowerSaving, 2: LowLatency

        dev = new fluid_oboe_audio_driver_t();

        dev->synth = synth;
        dev->oboe_callback = std::make_unique<OboeAudioStreamCallback>(dev);

        fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
        is_sample_format_float = fluid_settings_str_equal(settings, "audio.sample-format", "float");
        fluid_settings_getint(settings, "audio.oboe.id", &device_id);
        sharing_mode =
            fluid_settings_str_equal(settings, "audio.oboe.sharing-mode", "Exclusive") ? 1 : 0;
        performance_mode =
            fluid_settings_str_equal(settings, "audio.oboe.performance-mode", "PowerSaving") ? 1 :
            fluid_settings_str_equal(settings, "audio.oboe.performance-mode", "LowLatency") ? 2 : 0;

        builder->setDeviceId(device_id)
        ->setDirection(Direction::Output)
        ->setChannelCount(NUM_CHANNELS)
        ->setSampleRate(sample_rate)
        ->setFormat(is_sample_format_float ? AudioFormat::Float : AudioFormat::I16)
        ->setSharingMode(sharing_mode == 1 ? SharingMode::Exclusive : SharingMode::Shared)
        ->setPerformanceMode(
            performance_mode == 1 ? PerformanceMode::PowerSaving :
            performance_mode == 2 ? PerformanceMode::LowLatency : PerformanceMode::None)
        ->setUsage(Usage::Media)
        ->setContentType(ContentType::Music)
        ->setCallback(dev->oboe_callback.get())
        ->setSampleRateConversionQuality(get_srate_conversion_quality(settings));

        result = builder->openStream(dev->stream);

        if(result != Result::OK)
        {
            FLUID_LOG(FLUID_ERR, "Unable to open Oboe audio stream");
            goto error_recovery;
        }

        dev->cont = true;

        FLUID_LOG(FLUID_INFO, "Using Oboe driver");

        result = dev->stream->start();

        if(result != Result::OK)
        {
            FLUID_LOG(FLUID_ERR, "Unable to start Oboe audio stream");
            goto error_recovery;
        }

        return &dev->driver;
    }
    catch(const std::bad_alloc &)
    {
        FLUID_LOG(FLUID_ERR, "oboe: std::bad_alloc caught: Out of memory");
    }
    catch(const std::exception &e)
    {
        FLUID_LOG(FLUID_ERR, "oboe: std::exception caught: %s", e.what());
    }
    catch(...)
    {
        FLUID_LOG(FLUID_ERR, "Unexpected Oboe driver initialization error");
    }

error_recovery:
    delete_fluid_oboe_audio_driver(reinterpret_cast<fluid_audio_driver_t *>(dev));
    return nullptr;
}

void delete_fluid_oboe_audio_driver(fluid_audio_driver_t *p)
{
    fluid_oboe_audio_driver_t *dev = reinterpret_cast<fluid_oboe_audio_driver_t *>(p);

    fluid_return_if_fail(dev != nullptr);

    try
    {
        dev->cont = false;

        if(dev->stream != nullptr)
        {
            dev->stream->stop();
            dev->stream->close();
        }
    }
    catch(...)
    {
        FLUID_LOG(FLUID_ERR, "Exception caught while stopping and closing Oboe stream.");
    }

    delete dev;
}

#endif // OBOE_SUPPORT

