## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

# Tests for optional drivers

if PULSE_SUPPORT
fluid_pulse = drivers/fluid_pulse.c
endif

if ALSA_SUPPORT
fluid_alsa = drivers/fluid_alsa.c
endif

if COREAUDIO_SUPPORT
fluid_coreaudio = drivers/fluid_coreaudio.c
endif

if COREMIDI_SUPPORT
fluid_coremidi = drivers/fluid_coremidi.c
endif

if DBUS_SUPPORT
fluid_rtkit = bindings/fluid_rtkit.c bindings/fluid_rtkit.h
endif

if JACK_SUPPORT
fluid_jack = drivers/fluid_jack.c
endif

if PORTAUDIO_SUPPORT
fluid_portaudio = drivers/fluid_portaudio.c
endif

if MINGW32_SUPPORT
fluid_windows = fluid_dll.c drivers/fluid_dsound.c drivers/fluid_winmidi.c
endif

if OSS_SUPPORT
fluid_oss = drivers/fluid_oss.c
endif


# if LASH_SUPPORT || LADCCA_SUPPORT (Makefile supports OR?)
if LASH_SUPPORT
fluid_lash = bindings/fluid_lash.c bindings/fluid_lash.h
else
if LADCCA_SUPPORT
fluid_lash = bindings/fluid_lash.c bindings/fluid_lash.h
endif
endif

if DART_SUPPORT
fluid_dart = drivers/fluid_dart.c
endif

# Extra files and optional drivers
EXTRA_DIST = fluid_dll.c \
    bindings/fluid_ladspa.c \
    bindings/fluid_ladspa.h \
    drivers/fluid_alsa.c \
    drivers/fluid_coreaudio.c \
    drivers/fluid_coremidi.c \
    drivers/fluid_dart.c \
    drivers/fluid_dsound.c \
    drivers/fluid_jack.c \
    drivers/fluid_midishare.c \
    drivers/fluid_oss.c \
    drivers/fluid_portaudio.c \
    drivers/fluid_pulse.c \
    drivers/fluid_sndmgr.c \
    drivers/fluid_winmidi.c \
    config_macos.h \
    config_macosx.h \
    config_macosx_pb.h \
    CMakeLists.txt \
    config_win32.cmake \
    config_win32.h.in \
    config.cmake

DISTCLEANFILES = config_win32.h

lib_LTLIBRARIES = libfluidsynth.la
bin_PROGRAMS = fluidsynth 

libfluidsynth_la_SOURCES = \
	$(fluid_alsa) \
	$(fluid_coreaudio) \
	$(fluid_coremidi) \
	$(fluid_jack) \
	$(fluid_ladspa) \
	$(fluid_lash) \
	$(fluid_oss) \
	$(fluid_portaudio) \
	$(fluid_pulse) \
	$(fluid_windows) \
	$(fluid_dart) \
	$(fluid_rtkit) \
    utils/fluid_conv.c \
    utils/fluid_conv.h \
    utils/fluid_hash.c \
    utils/fluid_hash.h \
    utils/fluid_list.c \
    utils/fluid_list.h \
    utils/fluid_ringbuffer.c \
    utils/fluid_ringbuffer.h \
    utils/fluid_settings.c \
    utils/fluid_settings.h \
    utils/fluidsynth_priv.h \
    utils/fluid_sys.c \
    utils/fluid_sys.h \
    sfloader/fluid_defsfont.c \
    sfloader/fluid_defsfont.h \
    sfloader/fluid_ramsfont.c \
    sfloader/fluid_ramsfont.h \
    sfloader/fluid_sfont.h \
    rvoice/fluid_adsr_env.c \
    rvoice/fluid_adsr_env.h \
    rvoice/fluid_chorus.c \
    rvoice/fluid_chorus.h \
    rvoice/fluid_iir_filter.c \
    rvoice/fluid_iir_filter.h \
    rvoice/fluid_lfo.c \
    rvoice/fluid_lfo.h \
    rvoice/fluid_rvoice.h \
    rvoice/fluid_rvoice.c \
    rvoice/fluid_rvoice_dsp.c \
    rvoice/fluid_rvoice_event.h \
    rvoice/fluid_rvoice_event.c \
    rvoice/fluid_rvoice_mixer.h \
    rvoice/fluid_rvoice_mixer.c \
    rvoice/fluid_phase.h \
    rvoice/fluid_rev.c \
    rvoice/fluid_rev.h \
    synth/fluid_chan.c \
    synth/fluid_chan.h \
    synth/fluid_event.c \
    synth/fluid_event_priv.h \
    synth/fluid_gen.c \
    synth/fluid_gen.h \
    synth/fluid_mod.c \
    synth/fluid_mod.h \
    synth/fluid_synth.c \
    synth/fluid_synth.h \
    synth/fluid_tuning.c \
    synth/fluid_tuning.h \
    synth/fluid_voice.c \
    synth/fluid_voice.h \
    midi/fluid_midi.c \
    midi/fluid_midi.h \
    midi/fluid_midi_router.c \
    midi/fluid_midi_router.h \
    midi/fluid_seqbind.c \
    midi/fluid_seq.c \
    drivers/fluid_adriver.c \
    drivers/fluid_adriver.h \
    drivers/fluid_mdriver.c \
    drivers/fluid_mdriver.h \
    drivers/fluid_aufile.c \
    bindings/fluid_cmd.c \
    bindings/fluid_cmd.h \
    bindings/fluid_ladspa.c \
    bindings/fluid_ladspa.h \
    bindings/fluid_filerenderer.c 

INCLUDES = -I$(top_srcdir)/include \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/drivers \
  -I$(top_srcdir)/src/synth \
  -I$(top_srcdir)/src/rvoice \
  -I$(top_srcdir)/src/midi \
  -I$(top_srcdir)/src/utils \
  -I$(top_srcdir)/src/sfloader \
  -I$(top_srcdir)/src/bindings \
  $(LASH_CFLAGS) $(LADCCA_CFLAGS) \
  $(READLINE_CFLAGS) $(JACK_CFLAGS) $(ALSA_CFLAGS) $(PULSE_CFLAGS) \
  $(PORTAUDIO_CFLAGS) $(DART_CFLAGS) $(GLIB_CFLAGS) $(LIBSNDFILE_CFLAGS) \
  $(DBUS_CFLAGS)

libfluidsynth_la_LIBADD = $(LIBFLUID_LIBS) $(LASH_LIBS) $(LADCCA_LIBS) \
  $(READLINE_LIBS) $(COREAUDIO_LIBS) $(COREMIDI_LIBS) $(JACK_LIBS) \
  $(ALSA_LIBS) $(PULSE_LIBS) $(PORTAUDIO_LIBS) $(DART_LIBS)  \
  $(GLIB_LIBS) $(LIBSNDFILE_LIBS) $(DBUS_LIBS)

libfluidsynth_la_LDFLAGS = \
	-version-info @LT_VERSION_INFO@ \
	-export-dynamic $(LIBFLUID_LDFLAGS)

libfluidsynth_la_CPPFLAGS = $(LIBFLUID_CPPFLAGS)

fluidsynth_SOURCES = fluidsynth.c
fluidsynth_LDADD = libfluidsynth.la
fluidsynth_CPPFLAGS = $(FLUIDSYNTH_CPPFLAGS)

if GNU_LD_SUPPORT
fluidsynth_LDFLAGS = -Wl,--as-needed
endif

