LOCAL_PATH := $(call my-dir)/../..
include $(CLEAR_VARS)

LOCAL_MODULE     := fluidsynth

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/android/jni \
	$(LOCAL_PATH)/android/jni/include \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/src/synth \
	$(LOCAL_PATH)/src/midi \
	$(LOCAL_PATH)/src/rvoice \
	$(LOCAL_PATH)/src/sfloader \
	$(LOCAL_PATH)/src/utils \
	$(LOCAL_PATH)/src/bindings \
	$(LOCAL_PATH)/src/drivers

LOCAL_ARM_MODE   := arm
LOCAL_CFLAGS     += -DHAVE_CONFIG_H -Wall -Werror -std=gnu11 -Wno-unused-variable

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
	LOCAL_CFLAGS += -DWITH_FLOAT
endif
ifeq ($(TARGET_ARCH_ABI), x86)
	LOCAL_CFLAGS += -DWITH_FLOAT
endif

LOCAL_SRC_FILES := \
	src/midi/fluid_midi.c \
	src/midi/fluid_midi_router.c \
	src/midi/fluid_seq.c \
	src/midi/fluid_seqbind.c \
	src/rvoice/fluid_adsr_env.c \
	src/rvoice/fluid_chorus.c \
	src/rvoice/fluid_iir_filter.c \
	src/rvoice/fluid_lfo.c \
	src/rvoice/fluid_rev.c \
	src/rvoice/fluid_rvoice.c \
	src/rvoice/fluid_rvoice_dsp.c \
	src/rvoice/fluid_rvoice_event.c \
	src/rvoice/fluid_rvoice_mixer.c \
	src/sfloader/fluid_defsfont.c \
	src/sfloader/fluid_ramsfont.c \
	src/synth/fluid_chan.c \
	src/synth/fluid_event.c \
	src/synth/fluid_gen.c \
	src/synth/fluid_mod.c \
	src/synth/fluid_synth.c \
	src/synth/fluid_tuning.c \
	src/synth/fluid_voice.c \
	src/utils/fluid_conv.c \
	src/utils/fluid_hash.c \
	src/utils/fluid_list.c \
	src/utils/fluid_ringbuffer.c \
	src/utils/fluid_settings.c \
	src/bindings/fluid_filerenderer.c \
	src/drivers/fluid_adriver.c \
	src/drivers/fluid_mdriver.c \
	src/utils/fluid_sys.c \
	android/jni/src/bindings/fluid_cmd.c

include $(BUILD_SHARED_LIBRARY)

