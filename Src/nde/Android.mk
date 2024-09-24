LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := nde
LOCAL_CFLAGS += -D__ANDROID__
LOCAL_C_INCLUDES := $(LOCAL_PATH)/..
# make base stuff
LOCAL_SRC_FILES :=  Crc.cpp Database.cpp DBUtils.cpp Field.cpp Filter.cpp Index.cpp Int64Field.cpp Int128Field.cpp LinkedList.cpp NDEString.cpp
# make platform-specific field classes
LOCAL_SRC_FILES += android/Binary32Field.cpp android/BinaryField.cpp android/ColumnField.cpp android/FilenameField.cpp android/IndexField.cpp
LOCAL_SRC_FILES += android/IntegerField.cpp android/StringField.cpp
# make rest of platform-specific classes
LOCAL_SRC_FILES += android/IndexRecord.cpp android/nde_c.cpp android/nde_init.cpp android/Query.cpp android/Record.cpp android/Scanner.cpp android/Table.cpp android/Vfs.cpp

include $(BUILD_SHARED_LIBRARY)
