LOCAL_PATH := $(call my-dir)

OGG_DIR := dependencies/android/libogg
VORBIS_DIR := dependencies/android/libvorbis
THEORA_DIR := dependencies/android/libtheora
TXML_DIR := dependencies/all/tinyxml2
ASIO_DIR := dependencies/all/asio/asio/include
STBIMG_DIR := dependencies/all/stb-image

OGG_INCLUDES    := $(LOCAL_PATH)/$(OGG_DIR)/include
VORBIS_INCLUDES := $(LOCAL_PATH)/$(VORBIS_DIR)/include \
	                 $(LOCAL_PATH)/$(VORBIS_DIR)/lib
THEORA_INCLUDES := $(LOCAL_PATH)/$(THEORA_DIR)/include \
	                 $(LOCAL_PATH)/$(THEORA_DIR)/lib

######################################################################
# OGG
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE   := libogg
LOCAL_CFLAGS   := -ffast-math -fsigned-char -O2 -fPIC -DPIC \
                  -DBYTE_ORDER=LITTLE_ENDIAN -D_ARM_ASSEM_

LOCAL_C_INCLUDES := $(OGG_INCLUDES)

WILDCARD_SETUP := \
  $(wildcard $(LOCAL_PATH)/$(OGG_DIR)/src/*.c)


LOCAL_SRC_FILES := \
	$(subst jni/src/, , $(WILDCARD_SETUP))

include $(BUILD_STATIC_LIBRARY)

######################################################################
# VORBIS
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE   := libvorbis
LOCAL_CFLAGS   := -ffast-math -fsigned-char -O2 -fPIC -DPIC \
                  -DBYTE_ORDER=LITTLE_ENDIAN -D_ARM_ASSEM_

LOCAL_C_INCLUDES := $(OGG_INCLUDES) $(VORBIS_INCLUDES)

WILDCARD_SETUP := \
  $(wildcard $(LOCAL_PATH)/$(VORBIS_DIR)/lib/*.c)
FILTERED := $(filter-out $(LOCAL_PATH)/$(VORBIS_DIR)/lib/psytune.c, $(WILDCARD_SETUP))

LOCAL_SRC_FILES := \
	$(subst jni/src/, , $(FILTERED))

include $(BUILD_STATIC_LIBRARY)

######################################################################
# THEORA
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE   := libtheora
LOCAL_CFLAGS   := -ffast-math -fsigned-char -O2 -fPIC -DPIC \
                  -DBYTE_ORDER=LITTLE_ENDIAN -D_ARM_ASSEM_

LOCAL_C_INCLUDES := $(OGG_INCLUDES) $(THEORA_INCLUDES)

WILDCARD_SETUP := \
  $(wildcard $(LOCAL_PATH)/$(THEORA_DIR)/lib/*.c)

LOCAL_SRC_FILES := \
	$(subst jni/src/, , $(WILDCARD_SETUP))

include $(BUILD_STATIC_LIBRARY)

######################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL

LOCAL_CFLAGS   := -fexceptions

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/RSDKv4 \
    $(LOCAL_PATH)/RSDKv4/NativeObjects \
    $(LOCAL_PATH)/$(SDL_PATH)/include \
    $(LOCAL_PATH)/dependencies/all \
    $(LOCAL_PATH)/$(VORBIS_DIR)/include \
    $(LOCAL_PATH)/$(OGG_DIR)/include \
    $(LOCAL_PATH)/$(THEORA_DIR)/include \
    $(LOCAL_PATH)/dependencies/all/theoraplay \
    $(LOCAL_PATH)/$(TXML_DIR) \
    $(LOCAL_PATH)/$(STBIMG_DIR) \
    $(LOCAL_PATH)/$(ASIO_DIR)


WILDCARD_SETUP := \
  $(wildcard $(LOCAL_PATH)/RSDKv4/*.cpp) \
  $(LOCAL_PATH)/RSDKv4/NativeObjects/All.cpp \
  $(wildcard $(LOCAL_PATH)/dependencies/all/theoraplay/*.c) \
  $(wildcard $(LOCAL_PATH)/dependencies/all/stb-image/*.cpp) \
  $(wildcard $(LOCAL_PATH)/dependencies/all/tinyxml2/*.cpp)

LOCAL_SRC_FILES := $(subst jni/src/, , $(WILDCARD_SETUP))

LOCAL_SHARED_LIBRARIES := SDL2 libogg libvorbis libtheora


LOCAL_LDLIBS := -lGLESv1_CM -llog

include $(BUILD_SHARED_LIBRARY)
