# $Id$
include ../Makefile.inc


LIB_SHARED = libaegisub-3.0.so
LIB_SHARED_INSTALL = yes
LIB_VERSION = 3

CXXFLAGS = -Iinclude -I../src -I.. -DLAGI -fPIC -Wno-variadic-macros

PRECOMPILED_HEADER_NAME = lagi_pre.h
lagi_pre.h.gch: CXXFLAGS := $(CXXFLAGS)

ifeq (yes, $(BUILD_DARWIN))
CXXFLAGS += -I/Developer/SDKs/MacOSX10.5.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/LaunchServices.framework/Versions/A/Headers/
LDFLAGS += -L../universalchardet -luniversalchardet
endif

common/charset_conv.o: CXXFLAGS += $(CFLAGS_ICONV)

#######################
# AUDIO / VIDEO SUPPORT
#######################
ifeq (yes, $(HAVE_PROVIDER_FFMPEGSOURCE))
SRC_OPT += media/ffms_audio.cpp media/ffms_common.cpp media/ffms_video.cpp
common/ffms.o: CXXFLAGS += $(CFLAGS_FFMPEGSOURCE) $(CFLAGS_LIBAVFORMAT) $(CFLAGS_LIBAVCODEC) $(CFLAGS_LIBSWSCALE) $(CFLAGS_LIBAVUTIL) $(CFLAGS_LIBPOSTPROC)
common/ffms_audio.o: CXXFLAGS += $(CFLAGS_FFMPEGSOURCE) $(CFLAGS_LIBAVFORMAT) $(CFLAGS_LIBAVCODEC) $(CFLAGS_LIBSWSCALE) $(CFLAGS_LIBAVUTIL) $(CFLAGS_LIBPOSTPROC)
common/ffms_video.o: CXXFLAGS += $(CFLAGS_FFMPEGSOURCE) $(CFLAGS_LIBAVFORMAT) $(CFLAGS_LIBAVCODEC) $(CFLAGS_LIBSWSCALE) $(CFLAGS_LIBAVUTIL) $(CFLAGS_LIBPOSTPROC)
#LDFLAGS += $(LDFLAGS_FFMPEGSOURCE) $(LDFLAGS_LIBAVFORMAT) $(LDFLAGS_LIBAVCODEC) $(LDFLAGS_LIBSWSCALE) $(LDFLAGS_LIBAVUTIL) $(LDFLAGS_LIBPOSTPROC)
LDFLAGS_POST += $(LDFLAGS_FFMPEGSOURCE)
endif


SRC = \
	common/charset.cpp \
	common/charset_conv.cpp \
	common/charset_ucd.cpp \
	common/hotkey.cpp \
	common/json.cpp \
	common/mru.cpp \
	common/option.cpp \
	common/option_visit.cpp \
	common/keyframe.cpp \
	common/log.cpp \
	common/validator.cpp \
	common/vfr.cpp \
	media/video_frame.cpp \
	unix/util.cpp \
	unix/io.cpp \
	unix/access.cpp \
	unix/log.cpp \
	$(SRC_OPT)

ifeq (yes, $(BUILD_DARWIN))
SRC += \
	osx/util.cpp
endif

HEADERS = \
	*/*.h \
	include/aegisub/*.h \
	include/aegisub/cajun/*.h


include ../Makefile.target
-include */*.d
