include ../Makefile.inc

LIB = libaegisub.a

CXXFLAGS += -Iinclude -I../src -I.. -DLAGI -fPIC -Wno-variadic-macros

PRECOMPILED_HEADER_NAME = lagi_pre.h
lagi_pre.h.gch: CXXFLAGS := $(CXXFLAGS)

ifeq (yes, $(BUILD_DARWIN))
CXXFLAGS += -I/Developer/SDKs/MacOSX10.5.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/LaunchServices.framework/Versions/A/Headers/
LIBS += -L../universalchardet -luniversalchardet
endif

common/charset_conv.o: CXXFLAGS += $(CFLAGS_ICONV)
unix/path.o: CXXFLAGS += -DP_DATA=\"$(P_DATA)\" -DP_DOC=\"$(P_DOC)\" -DP_LOCALE=\"$(P_LOCALE)\"

SRC += \
	common/cajun/elements.cpp \
	common/cajun/reader.cpp \
	common/cajun/writer.cpp \
	common/charset.cpp \
	common/charset_6937.cpp \
	common/charset_conv.cpp \
	common/charset_ucd.cpp \
	common/color.cpp \
	common/hotkey.cpp \
	common/io.cpp \
	common/json.cpp \
	common/keyframe.cpp \
	common/mru.cpp \
	common/option.cpp \
	common/option_visit.cpp \
	common/parser.cpp \
	common/util.cpp \
	common/log.cpp \
	common/thesaurus.cpp \
	common/vfr.cpp \
	unix/util.cpp \
	unix/access.cpp \
	unix/log.cpp

ifeq (yes, $(BUILD_DARWIN))
SRC += osx/util.mm
endif

HEADER += \
	*.h \
	*/*.h \
	include/libaegisub/*.h \
	include/libaegisub/charsets.def \
	include/libaegisub/cajun/*.h

EXTRA_DIST += osx/util.mm

include ../Makefile.target
-include */*.d
