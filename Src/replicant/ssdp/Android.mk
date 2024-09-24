LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_MODULE := ssdp
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_STATIC_LIBRARIES :=  nu foundation
LOCAL_SHARED_LIBRARIES := libjnet nx

LOCAL_SRC_FILES :=  main.cpp SSDPAPI.cpp


LOCAL_LDFLAGS = -llog
include $(BUILD_SHARED_LIBRARY)
