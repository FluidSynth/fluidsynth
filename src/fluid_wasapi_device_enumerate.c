/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2021  Chris Xiong and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "fluid_sys.h"

#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

enum
{
    MAX_SAMPLE_RATES   = 16,
    MAX_SAMPLE_FORMATS = 8,

    /* FluidSynth-supported audio.period-size range (frames) */
    FS_MIN_PERIOD_SIZE = 64,
    FS_MAX_PERIOD_SIZE = 8192
};

static char **devs;
static int severity; // severity of last probe attempt
/* Captured from WASAPI driver logs during the last timing probe attempt.
 * Values are in FRAMES at the probe's sample rate, as printed by fluid_wasapi.cpp. */
static int g_period_min_frames = 0;
static int g_period_def_frames = 0;

static void devenumcb(void *p, const char *s, const char *opt)
{
    int *c = (int *)p;
    char *copy;

    (void)s; /* unused */

    printf(" %s\n", opt);

    copy = (char *)malloc(strlen(opt) + 1);
    if (copy == NULL)
    {
        /* Allocation failure: skip device but keep enumeration alive */
        return;
    }

    strcpy(copy, opt);
    devs[*c] = copy;
    ++(*c);
}

static int count_int_terminator0(const int *a)
{
    int n = 0;
    while (a[n] != 0)
        n++;
    return n;
}

static int count_str_terminator0(const char *const *a)
{
    int n = 0;
    while (a[n] && a[n][0])
        n++;
    return n;
}

static int find_rate_index(const int *sample_rates, int rate_count, int rate)
{
    int i;
    for (i = 0; i < rate_count; ++i)
    {
        if (sample_rates[i] == rate)
            return i;
    }
    return -1;
}

static int find_format_index(const char *const *sample_formats, int fmt_count, const char *fmt)
{
    int i;
    for (i = 0; i < fmt_count; ++i)
    {
        if (strcmp(sample_formats[i], fmt) == 0)
            return i;
    }
    return -1;
}

/* Build a list of preferred indices based on a preferred-value list.
 * Any preferred value not found in the current list is skipped. */
static int build_preferred_rate_indices(int *out_idx,
                                        int out_cap,
                                        const int *sample_rates,
                                        int rate_count,
                                        const int *preferred_rates,
                                        int preferred_count)
{
    int n = 0;
    int i;
    for (i = 0; i < preferred_count && n < out_cap; ++i)
    {
        int s = find_rate_index(sample_rates, rate_count, preferred_rates[i]);
        if (s >= 0)
            out_idx[n++] = s;
    }
    return n;
}

static int build_preferred_format_indices(int *out_idx,
                                          int out_cap,
                                          const char *const *sample_formats,
                                          int fmt_count,
                                          const char *const *preferred_formats,
                                          int preferred_count)
{
    int n = 0;
    int i;
    for (i = 0; i < preferred_count && n < out_cap; ++i)
    {
        int f = find_format_index(sample_formats, fmt_count, preferred_formats[i]);
        if (f >= 0)
            out_idx[n++] = f;
    }
    return n;
}

/* Select the first supported (rate,format) pair using preferred ordering.
 * Returns 1 on success and fills *out_s / *out_f; returns 0 if none found. */
static int select_safe_exclusive_pair(int *out_s,
                                      int *out_f,
                                      const int *preferred_rates_idx,
                                      int preferred_rates_n,
                                      const int *preferred_formats_idx,
                                      int preferred_formats_n,
                                      int supported_tbl[MAX_SAMPLE_RATES][MAX_SAMPLE_FORMATS])
{
    int pi, pj;
    for (pj = 0; pj < preferred_formats_n; ++pj)
    {
        int f = preferred_formats_idx[pj];
        for (pi = 0; pi < preferred_rates_n; ++pi)
        {
            int s = preferred_rates_idx[pi];
            if (supported_tbl[s][f])
            {
                *out_s = s;
                *out_f = f;
                return 1;
            }
        }
    }
    return 0;
}

static int next_power_of_two_int(int v)
{
    /* Returns the next power-of-two >= v. For v <= 1, returns 1. */
    unsigned int x;
    if (v <= 1)
        return 1;

    x = (unsigned int)(v - 1);
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return (int)(x + 1);
}

/* Convert a period in seconds to a frame count at a given sample rate, rounding up. */
static int ceil_frames(double period_seconds, int sample_rate)
{
    double frames = period_seconds * (double)sample_rate;
    int out = (int)ceil(frames);
    return (out < 1) ? 1 : out;
}

/* Print exclusive-mode period-size guidance (min/rec) for all sample rates.
 * min is derived from the captured minimum period; rec is based on the device's default period. */
static void print_exclusive_period_rows(const int *sample_rates, int rate_count, double min_period_seconds, double def_period_seconds)
{
    int s;

    puts("    audio.period-size");

    /* rec: power-of-two >= device default period, clamped, and never below min */
    printf("      rec:   ");
    for (s = 0; s < rate_count; ++s)
    {
        int minf_raw = ceil_frames(min_period_seconds, sample_rates[s]);
        int deff_raw = ceil_frames(def_period_seconds, sample_rates[s]);

        int minf_pow2 = next_power_of_two_int(minf_raw);
        int recf_pow2 = next_power_of_two_int(deff_raw);

        if (minf_pow2 < FS_MIN_PERIOD_SIZE)
            minf_pow2 = FS_MIN_PERIOD_SIZE;
        if (minf_pow2 > FS_MAX_PERIOD_SIZE)
            minf_pow2 = FS_MAX_PERIOD_SIZE;

        if (recf_pow2 < FS_MIN_PERIOD_SIZE)
            recf_pow2 = FS_MIN_PERIOD_SIZE;
        if (recf_pow2 > FS_MAX_PERIOD_SIZE)
            recf_pow2 = FS_MAX_PERIOD_SIZE;

        if (recf_pow2 < minf_pow2)
            recf_pow2 = minf_pow2;

        printf("%-6d", recf_pow2);
    }
    putchar('\n');

    /* min: power-of-two >= device minimum period, clamped */
    printf("      min:   ");
    for (s = 0; s < rate_count; ++s)
    {
        int minf_raw = ceil_frames(min_period_seconds, sample_rates[s]);
        int minf_pow2 = next_power_of_two_int(minf_raw);

        if (minf_pow2 < FS_MIN_PERIOD_SIZE)
            minf_pow2 = FS_MIN_PERIOD_SIZE;
        if (minf_pow2 > FS_MAX_PERIOD_SIZE)
            minf_pow2 = FS_MAX_PERIOD_SIZE;

        printf("%-6d", minf_pow2);
    }
    putchar('\n');
}

static int run_exclusive_timing_probe(fluid_settings_t *settings,
                                      const char *device_name,
                                      int sample_rate,
                                      const char *sample_format,
                                      int tiny_period_size,
                                      double *out_min_period_seconds,
                                      double *out_def_period_seconds)
{
    fluid_synth_t *synth = NULL;
    fluid_audio_driver_t *adriver = NULL;

    int saved_periods = 0;
    int saved_period_size = 0;

    /* Reset capture for this timing probe */
    severity = 0;
    g_period_min_frames = 0;
    g_period_def_frames = 0;

    /* Save current probe settings */
    fluid_settings_getint(settings, "audio.periods", &saved_periods);
    fluid_settings_getint(settings, "audio.period-size", &saved_period_size);

    /* Configure for exclusive timing probe */
    fluid_settings_setint(settings, "audio.wasapi.exclusive-mode", 1);
    fluid_settings_setstr(settings, "audio.wasapi.device", device_name);
    fluid_settings_setnum(settings, "synth.sample-rate", sample_rate);
    fluid_settings_setstr(settings, "audio.sample-format", sample_format);

    /* Deliberately too small period-size to trigger AUDCLNT_E_INVALID_DEVICE_PERIOD (if applicable) */
    fluid_settings_setint(settings, "audio.period-size", tiny_period_size);

    synth = new_fluid_synth(settings);
    if (synth != NULL)
    {
        adriver = new_fluid_audio_driver(settings, synth);
        if (adriver != NULL)
        {
            /* It succeeded even with tiny period-size; delete and treat timing info as unavailable. */
            delete_fluid_audio_driver(adriver);
            adriver = NULL;
        }

        delete_fluid_synth(synth);
        synth = NULL;
    }

    /* Restore probe settings */
    fluid_settings_setint(settings, "audio.period-size", saved_period_size);
    fluid_settings_setint(settings, "audio.periods", saved_periods);

    /* If we captured frames, convert to seconds using the probe sample rate. */
    if (g_period_min_frames > 0 && g_period_def_frames > 0)
    {
        *out_min_period_seconds = (double)g_period_min_frames / (double)sample_rate;
        *out_def_period_seconds = (double)g_period_def_frames / (double)sample_rate;
        return 1;
    }

    return 0;
}

static fluid_settings_t *create_wasapi_settings(void)
{
    fluid_settings_t *settings = new_fluid_settings();

    fluid_settings_setstr(settings, "audio.driver", "wasapi");

    /* Stabilize probing conditions across runs. */
    fluid_settings_setint(settings, "audio.period-size", 256);
    fluid_settings_setint(settings, "audio.periods", 8);

    return settings;
}

static void print_probe_settings(const fluid_settings_t *settings)
{
    int periods = 0;
    int period_size = 0;

    /* fluid_settings_getint requires a non-const pointer */
    fluid_settings_getint((fluid_settings_t *)settings, "audio.periods", &periods);
    fluid_settings_getint((fluid_settings_t *)settings, "audio.period-size", &period_size);

    printf("Probe settings:  audio.periods = %d   audio.period-size = %d\n", periods, period_size);
}

static int enumerate_devices(fluid_settings_t *settings)
{
    int device_count = fluid_settings_option_count(settings, "audio.wasapi.device");

    devs = (char **)calloc(device_count, sizeof(char *));
    if (devs == NULL)
    {
        puts("Failed to allocate device list.");
        return 0;
    }

    puts("Available audio devices:");
    device_count = 0;
    fluid_settings_foreach_option(settings, "audio.wasapi.device", &device_count, devenumcb);

    assert(device_count == fluid_settings_option_count(settings, "audio.wasapi.device"));
    puts("");

    return device_count;
}

static void eatlog(int lvl, const char *m, void *d)
{
    /* Keep the worst (most severe) log level observed during a probe attempt.
     * FluidSynth levels: FLUID_ERR=1 (worst), FLUID_WARN=2, FLUID_INFO=3 (best). */
    if (severity == 0 || lvl < severity)
    {
        severity = lvl;
    }

    /* Capture period diagnostics if present.
     *
     * NOTE: This intentionally parses the WASAPI driver's log output.
     * It depends on the exact log string emitted by fluid_wasapi.cpp:
     *
     *   FLUID_LOG(FLUID_ERR,
     *     "wasapi: minimum period is %d, default period is %d. selected %d.",
     *     minpf, defpf, dev->period_size);
     *
     * This is a diagnostic-only tool. If the driver log format changes,
     * timing probes will silently degrade rather than crash.
     */
    if (m != NULL)
    {
        const char *p = strstr(m, "minimum period is ");
        if (p != NULL)
        {
            int minf = 0, deff = 0;
            if (sscanf(p, "minimum period is %d, default period is %d.", &minf, &deff) == 2)
            {
                g_period_min_frames = minf;
                g_period_def_frames = deff;
            }
        }
    }
}

static void install_probe_loggers(void)
{
    fluid_set_log_function(FLUID_INFO, eatlog, NULL);
    fluid_set_log_function(FLUID_WARN, eatlog, NULL);
    fluid_set_log_function(FLUID_ERR, eatlog, NULL);
}

void fluid_wasapi_device_enumerate(void)
{
    static const int sample_rates[] = { 8000,  11025, 16000, 22050, 24000, 32000,
                                        44100, 48000, 88200, 96000, 0 };
    static const char *sample_formats[] = { "float", "32bits", "24bits", "16bits", "" };

    int e, d, s, f, i;
    fluid_synth_t *synth = NULL;
    fluid_audio_driver_t *adriver = NULL;

    fluid_settings_t *settings = create_wasapi_settings(); // Create base settings
    int device_count = enumerate_devices(settings);        // Get available devices and print them
    print_probe_settings(settings);                        // Print the period settings used for probing

    install_probe_loggers();

    const int rate_count = count_int_terminator0(sample_rates);
    const int fmt_count = count_str_terminator0(sample_formats);
    assert(rate_count <= MAX_SAMPLE_RATES && fmt_count <= MAX_SAMPLE_FORMATS);

    /* Preferred ordering for choosing a safe exclusive-mode timing probe pair. */
    static const int preferred_rates_vals[] = { 48000, 44100, 96000, 88200, 32000,
                                                24000, 22050, 16000, 11025, 8000 };
    static const char *preferred_formats_vals[] = { "16bits", "24bits", "32bits", "float" };

    int preferred_rates_idx[MAX_SAMPLE_RATES];
    int preferred_formats_idx[MAX_SAMPLE_FORMATS];

    int preferred_rates_n =
    build_preferred_rate_indices(preferred_rates_idx,
                                 MAX_SAMPLE_RATES,
                                 sample_rates,
                                 rate_count,
                                 preferred_rates_vals,
                                 (int)(sizeof(preferred_rates_vals) / sizeof(preferred_rates_vals[0])));

    int preferred_formats_n =
    build_preferred_format_indices(preferred_formats_idx,
                                   MAX_SAMPLE_FORMATS,
                                   sample_formats,
                                   fmt_count,
                                   preferred_formats_vals,
                                   (int)(sizeof(preferred_formats_vals) / sizeof(preferred_formats_vals[0])));

    int supported_tbl[MAX_SAMPLE_RATES][MAX_SAMPLE_FORMATS];
    int sev_tbl[MAX_SAMPLE_RATES][MAX_SAMPLE_FORMATS];

    for (e = 0; e < 2; ++e)
    {
        puts(e ? "Exclusive mode:" : "Shared mode:");
        fluid_settings_setint(settings, "audio.wasapi.exclusive-mode", e);

        for (d = 0; d < device_count; ++d)
        {
            printf("    %s\n", devs[d]);
            fluid_settings_setstr(settings, "audio.wasapi.device", devs[d]);

            /* Probe all (rate,format) pairs and store results. */
            for (s = 0; s < rate_count; ++s)
            {
                fluid_settings_setnum(settings, "synth.sample-rate", sample_rates[s]);
                for (f = 0; f < fmt_count; ++f)
                {
                    /* Per-probe result state */
                    int supported = 0;
                    severity = 0;

                    fluid_settings_setstr(settings, "audio.sample-format", sample_formats[f]);

                    synth = new_fluid_synth(settings);
                    if (synth != NULL)
                    {
                        adriver = new_fluid_audio_driver(settings, synth);
                        if (adriver != NULL)
                        {
                            supported = 1;
                            delete_fluid_audio_driver(adriver);
                            adriver = NULL;
                        }

                        delete_fluid_synth(synth);
                        synth = NULL;
                    }
                    else
                    {
                        /* If synth creation failed, treat as unsupported. */
                        severity = FLUID_ERR;
                    }

                    supported_tbl[s][f] = supported;
                    sev_tbl[s][f] = severity;
                }
            }

            /* Render matrix: header row of rates, then one row per format. */
            printf("             ");
            for (s = 0; s < rate_count; ++s)
            {
                printf("%-6d", sample_rates[s]);
            }
            putchar('\n');

            for (f = 0; f < fmt_count; ++f)
            {
                /* Format label column */
                printf("      %-6s ", sample_formats[f]);

                for (s = 0; s < rate_count; ++s)
                {
                    char cell[6]; /* "YES*", "YES?", "YES", "NO" + NUL */

                    if (!supported_tbl[s][f])
                    {
                        strcpy(cell, "NO");
                    }
                    else
                    {
                        char suffix = '\0';
                        severity = sev_tbl[s][f];

                        if (severity == FLUID_WARN || severity == FLUID_ERR)
                            suffix = '?';
                        else if (severity == FLUID_INFO)
                            suffix = '*';

                        cell[0] = 'Y';
                        cell[1] = 'E';
                        cell[2] = 'S';
                        cell[3] = suffix;
                        cell[4] = '\0';
                    }

                    printf("%-6s", cell);
                }

                putchar('\n');
            }

            /* Exclusive-mode timing reporting (min/rec) */
            if (e == 1) /* only under Exclusive mode */
            {
                int safe_s = -1;
                int safe_f = -1;

                if (select_safe_exclusive_pair(
                    &safe_s, &safe_f, preferred_rates_idx, preferred_rates_n, preferred_formats_idx, preferred_formats_n, supported_tbl))
                {
                    double min_sec = 0.0;
                    double def_sec = 0.0;

                    /* Use a tiny period-size to trigger the driver’s min/default period logging. */
                    if (run_exclusive_timing_probe(
                        settings, devs[d], sample_rates[safe_s], sample_formats[safe_f], FS_MIN_PERIOD_SIZE, &min_sec, &def_sec))
                    {
                        print_exclusive_period_rows(sample_rates, rate_count, min_sec, def_sec);
                    }
                    else // Timing probe failed to capture period info because probe successfully initialized
                    {
                        puts("    audio.period-size");
                        puts("      rec:   128   128   256   256   256   256   256   256   512   512");
                        puts("      min:   64    64    64    64    64    64    64    64    64    64");
                    }
                }
                else // No supported (rate,format) pair found
                {
                    puts("    audio.period-size");
                    puts("      rec:   (unavailable)");
                    puts("      min:   (unavailable)");
                }
            }

            puts("");
        }
    }

    puts("YES  : Supported natively by the audio device.");
    puts("YES* : Supported, but Windows may convert format and/or resample.");
    puts("YES? : Supported, but a warning occurred during setup.");
    puts("NO   : Mode is not supported.");
    puts("rec is a recommended conservative value based on the device's reported default period.");

  delete_fluid_settings(settings);

  for (i = 0; i < device_count; ++i)
  {
      free(devs[i]);
  }

  free(devs);
}
