# Copyright (C) 2011 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := nfc_nci.st21nfc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := \
    $(call all-c-files-under, hal) \
    $(call all-c-files-under, include) \
    $(call all-c-files-under, gki) \
    $(call all-c-files-under, adaptation) \
    $(call all-cpp-files-under, adaptation) \
    nfc_nci_st21nfc.c \
    hal_wrapper.c

LOCAL_SHARED_LIBRARIES := liblog libcutils libdl libhardware

LOCAL_CFLAGS := $(D_CFLAGS)
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/gki/ulinux \
    $(LOCAL_PATH)/gki/common \
    $(LOCAL_PATH)/adaptation \
    $(LOCAL_PATH)/hal \

LOCAL_CFLAGS += -DANDROID \
        -DST21NFC -DDEBUG



#
# nfc_nci.$(TARGET_DEVICE)
#
include $(BUILD_SHARED_LIBRARY)
