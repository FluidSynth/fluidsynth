LOCAL_PATH := $(call my-dir)
TARGET_PLATFORM := android-27

GLIB_LIB = ../dep/$(APP_ABI)/

include $(CLEAR_VARS)
LOCAL_MODULE := glib-2.0
LOCAL_SRC_FILES := $(GLIB_LIB)/libglib-2.0.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := iconv
LOCAL_SRC_FILES := $(GLIB_LIB)/libiconv.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := intl 
LOCAL_SRC_FILES := $(GLIB_LIB)/libintl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := oboe
LOCAL_SRC_FILES := $(GLIB_LIB)/liboboe.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := fluidsynth_static
LOCAL_SRC_FILES := ../dep/$(APP_ABI)/libfluidsynth.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE := fluidsynth

ifeq ($(NDK_DEBUG),1)
	cmd-strip :=
endif

LOCAL_STATIC_LIBRARIES := glib-2.0 iconv intl oboe

LOCAL_WHOLE_STATIC_LIBRARIES := fluidsynth_static

LOCAL_LDLIBS := -lc -lOpenSLES -ldl -llog -landroid -L$(LOCAL_PATH)/../dist/$(APP_ABI) -loboe-c

include $(BUILD_SHARED_LIBRARY)

