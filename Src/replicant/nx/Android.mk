NX_LOCAL_PATH := $(call my-dir)
include $(NX_LOCAL_PATH)/cpufeatures/Android.mk

LOCAL_PATH:= $(NX_LOCAL_PATH)


include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_MODULE := nx
LOCAL_MODULE_FILENAME := libnx.$(PLATFORM_NAME)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/..
LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_SRC_FILES := android/nxcondition.c android/nxstring.c android/nxuri.c android/nxmutablestring.c android/nxdata.c android/nxsemaphore.c android/nxthread.c android/nxfile.c android/nxlog.c android/NXFileObject.cpp android/NXFileProgressiveDownloader.cpp android/nxonce.c
LOCAL_SRC_FILES += linux/nxsleep.c linux/nxpath.c
LOCAL_SRC_FILES += cpufeatures/cpu-features.c

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_SRC_FILES += android-armv7/nxonce-armv6.S 
else
LOCAL_SRC_FILES += android-armv5/nxonce-armv5.S
endif

LOCAL_STATIC_LIBRARIES := nu foundation
LOCAL_SHARED_LIBRARIES := nu foundation jnet
LOCAL_LDFLAGS := -llog
include $(BUILD_SHARED_LIBRARY)
