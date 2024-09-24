LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := replicant-metadata
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden -fno-rtti

LOCAL_SRC_FILES :=  metadata.cpp MetadataManager.cpp ArtworkManager.cpp

LOCAL_STATIC_LIBRARIES := nu foundation
LOCAL_SHARED_LIBRARIES := nx

include $(BUILD_STATIC_LIBRARY)


