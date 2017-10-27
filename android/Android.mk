LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := fluidsynth_shared
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libfluidsynth.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../include $(LOCAL_PATH)/jni/include
TARGET_PLATFORM = android-14

include $(PREBUILT_SHARED_LIBRARY)
