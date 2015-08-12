LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL2

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include

LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
	../../../main/enemy.c \
	../../../main/battle.c \
	../../../main/global.c \
	../../../main/map.c \
	../../../main/res.c \
	../../../main/ui.c \
	../../../main/ending.c \
	../../../main/input.c \
	../../../main/rngplay.c \
	../../../main/uibattle.c \
	../../../main/fight.c \
	../../../main/palcommon.c \
	../../../main/scene.c \
	../../../main/uigame.c \
	../../../main/font.c \
	../../../main/itemmenu.c \
	../../../main/palette.c \
	../../../main/script.c \
	../../../main/util.c \
	../../../main/game.c \
	../../../main/magicmenu.c \
	../../../main/play.c \
	../../../main/sound.c \
	../../../main/video.c \
	../../../main/getopt.c \
	../../../main/main.c \
	../../../main/private.c \
	../../../main/text.c \
	../../../main/yj1.c \
	../../../main/rixplay.cpp \
	../../../adplug/binfile.cpp \
	../../../adplug/binio.cpp \
	../../../adplug/dosbox_opl.cpp \
	../../../adplug/emuopl.cpp \
	../../../adplug/fmopl.c \
	../../../adplug/fprovide.cpp \
	../../../adplug/player.cpp \
	../../../adplug/rix.cpp \
	../../../adplug/surroundopl.cpp \
	../../../libmad/bit.c \
	../../../libmad/decoder.c \
	../../../libmad/fixed.c \
	../../../libmad/frame.c \
	../../../libmad/huffman.c \
	../../../libmad/layer12.c \
	../../../libmad/layer3.c \
	../../../libmad/music_mad.c \
	../../../libmad/stream.c \
	../../../libmad/synth.c \
	../../../libmad/timer.c

LOCAL_SHARED_LIBRARIES := SDL2

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)
