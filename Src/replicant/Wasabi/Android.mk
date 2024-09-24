LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := Wasabi
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)

LOCAL_SRC_FILES :=  api.cpp ServiceManager.cpp SysCallbacks.cpp


LOCAL_STATIC_LIBRARIES := nu foundation
LOCAL_SHARED_LIBRARIES := nx

include $(BUILD_STATIC_LIBRARY)


