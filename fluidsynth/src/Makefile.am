## Process this file with automake to produce Makefile.in

# Tests for optional drivers
if ALSA_SUPPORT
fluid_alsa = fluid_alsa.c
endif

if COREAUDIO_SUPPORT
fluid_coreaudio = fluid_coreaudio.c
endif

if JACK_SUPPORT
fluid_jack = fluid_jack.c
endif

if MINGW32_SUPPORT
fluid_windows = fluid_dll.c fluid_dsound.c fluid_winmidi.c
endif

if OSS_SUPPORT
fluid_oss = fluid_oss.c
endif

# if LASH_SUPPORT || LADCCA_SUPPORT (Makefile supports OR?)
if LASH_SUPPORT
fluid_lash = fluid_lash.c
else
if LADCCA_SUPPORT
fluid_lash = fluid_lash.c
endif
endif


# Extra files and optional drivers
EXTRA_DIST = fluid_dll.c fluid_dsound.c fluid_winmidi.c fluid_portaudio.c \
	fluid_coreaudio.c fluid_alsa.c fluid_oss.c \
	fluid_dsp_simple.c \
	fluid_sndmgr.c config_macos.h config_macosx.h config_macosx_pb.h \
	config_win32.h fluid_jack.c

lib_LTLIBRARIES = libfluidsynth.la
bin_PROGRAMS = fluidsynth 

libfluidsynth_la_SOURCES = \
	$(fluid_alsa) \
	$(fluid_coreaudio) \
	$(fluid_jack) \
	$(fluid_lash) \
	$(fluid_oss) \
	$(fluid_windows) \
	fluid_adriver.c \
	fluid_adriver.h \
	fluid_chan.c \
	fluid_chan.h \
	fluid_chorus.c \
	fluid_chorus.h \
	fluid_cmd.c \
	fluid_cmd.h \
	fluid_conv.c \
	fluid_conv.h \
	fluid_defsfont.c \
	fluid_defsfont.h \
	fluid_dsp_float.c \
	fluid_event.c \
	fluid_event_priv.h \
	fluid_gen.c \
	fluid_gen.h \
	fluid_hash.c \
	fluid_hash.h \
	fluid_io.c \
	fluid_io.h \
	fluid_ladspa.c \
	fluid_ladspa.h \
	fluid_lash.h \
	fluid_list.c \
	fluid_list.h \
	fluid_mdriver.c \
	fluid_mdriver.h \
	fluid_midi.c \
	fluid_midi.h \
	fluid_midi_router.c \
	fluid_midi_router.h \
	fluid_midishare.c \
	fluid_mod.c \
	fluid_mod.h \
	fluid_phase.h \
	fluid_ramsfont.c \
	fluid_ramsfont.h \
	fluid_rev.c \
	fluid_rev.h \
	fluid_seqbind.c \
	fluid_seq.c \
	fluid_settings.c \
	fluid_settings.h \
	fluid_sfont.h \
	fluid_synth.c \
	fluid_synth.h \
	fluidsynth_priv.h \
	fluid_sys.c \
	fluid_sys.h \
	fluid_tuning.c \
	fluid_tuning.h \
	fluid_voice.c \
	fluid_voice.h \
	fluid_aufile.c 

INCLUDES = -I$(top_srcdir)/include $(LASH_CFLAGS) $(LADCCA_CFLAGS) \
  $(READLINE_CFLAGS) $(JACK_CFLAGS) $(ALSA_CFLAGS)

libfluidsynth_la_LIBADD = $(LIBFLUID_LIBS) $(LASH_LIBS) $(LADCCA_LIBS) \
  $(READLINE_LIBS) $(COREAUDIO_LIBS) $(JACK_LIBS) $(ALSA_LIBS)
libfluidsynth_la_LDFLAGS = \
	-version-info @LT_VERSION_INFO@ \
	-export-dynamic $(LIBFLUID_LDFLAGS)
libfluidsynth_la_CPPFLAGS = $(LIBFLUID_CPPFLAGS)

fluidsynth_SOURCES = fluidsynth.c
fluidsynth_LDADD = libfluidsynth.la
fluidsynth_CPPFLAGS = $(FLUIDSYNTH_CPPFLAGS)
