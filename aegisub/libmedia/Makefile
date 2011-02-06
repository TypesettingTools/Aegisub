# $Id$
include ../Makefile.inc


LIB_SHARED = libmedia_aegisub-3.0.so
LIB_SHARED_INSTALL = yes
LIB_VERSION = 3

CXXFLAGS = -Iinclude -I../libaegisub/include -I../src -I.. -DMAGI -fPIC -Wno-variadic-macros

PRECOMPILED_HEADER_NAME = magi_pre.h
magi_pre.h.gch: CXXFLAGS := $(CXXFLAGS)

#######################
# AUDIO / VIDEO SUPPORT
#######################
ifeq (yes, $(HAVE_PROVIDER_FFMPEGSOURCE))
SRC_OPT += audio/ffms_audio.cpp common/ffms_common.cpp video/ffms_video.cpp
common/ffms_common.o: CXXFLAGS += $(CFLAGS_FFMPEGSOURCE) $(CFLAGS_LIBAVFORMAT) $(CFLAGS_LIBAVCODEC) $(CFLAGS_LIBSWSCALE) $(CFLAGS_LIBAVUTIL) $(CFLAGS_LIBPOSTPROC)
audio/ffms_audio.o: CXXFLAGS += $(CFLAGS_FFMPEGSOURCE) $(CFLAGS_LIBAVFORMAT) $(CFLAGS_LIBAVCODEC) $(CFLAGS_LIBSWSCALE) $(CFLAGS_LIBAVUTIL) $(CFLAGS_LIBPOSTPROC)
video/ffms_video.o: CXXFLAGS += $(CFLAGS_FFMPEGSOURCE) $(CFLAGS_LIBAVFORMAT) $(CFLAGS_LIBAVCODEC) $(CFLAGS_LIBSWSCALE) $(CFLAGS_LIBAVUTIL) $(CFLAGS_LIBPOSTPROC)
LDFLAGS_POST += $(LDFLAGS_FFMPEGSOURCE)
endif


SRC = \
	common/video_frame.cpp \
	audio/downmix.cpp \
	audio/convert.cpp \
	audio/dummy_audio.cpp \
	audio/pcm.cpp \
	cache/audio_ram.cpp \
	cache/video_cache.cpp \
	$(SRC_OPT)

HEADERS = \
	*/*.h \
	include/libmedia/*.h \


include ../Makefile.target
-include */*.d
