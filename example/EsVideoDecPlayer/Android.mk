ifeq (,$(wildcard media_hal))
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := EsVideoDecPlayer
LOCAL_MODULE_TAGS := optional
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES := \
        EsVideoDecPlayer.cpp \
        crc32.cpp

LOCAL_C_INCLUDES := \
        $(LOCAL_PATH)/../../videodec/include \
        $(LOCAL_PATH)/../../include

LOCAL_SHARED_LIBRARIES := libdl

LOCAL_VENDOR_MODULE := true

LOCAL_CFLAGS += -DANDROID_PLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION)

LOCAL_LICENSE_KINDS := SPDX-license-identifier-GPL SPDX-license-identifier-GPL-2.0 legacy_unencumbered
LOCAL_LICENSE_CONDITIONS := restricted unencumbered
LOCAL_NOTICE_FILE := $(LOCAL_PATH)/../../LICENSE
include $(BUILD_EXECUTABLE)
endif
