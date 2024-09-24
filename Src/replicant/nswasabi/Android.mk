LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := nswasabi
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_SRC_FILES := PlaybackBase.cpp XMLString.cpp ID3v2Metadata.cpp ID3v1Metadata.cpp APEv2Metadata.cpp ApplicationBase.cpp
LOCAL_STATIC_LIBRARIES := nu
LOCAL_EXPORT_LDLIBS := -llog
include $(BUILD_STATIC_LIBRARY)

