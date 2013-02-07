include ../Makefile.inc

LIB = libaegisub.a

CXXFLAGS += -Iinclude -I../src -I.. -DLAGI -fPIC -Wno-variadic-macros
CPPFLAGS += $(CPPFLAGS_BOOST)

PRECOMPILED_HEADER_NAME = lagi_pre.h
lagi_pre.h.gch: CXXFLAGS := $(CXXFLAGS)

common/charset_conv.o: CXXFLAGS += $(CFLAGS_ICONV)
unix/path.o: CXXFLAGS += -DP_DATA=\"$(P_DATA)\" -DP_DOC=\"$(P_DOC)\" -DP_LOCALE=\"$(P_LOCALE)\"

SRC += \
	ass/dialogue_parser.cpp \
	ass/uuencode.cpp \
	common/cajun/elements.cpp \
	common/cajun/reader.cpp \
	common/cajun/writer.cpp \
	common/calltip_provider.cpp \
	common/charset.cpp \
	common/charset_6937.cpp \
	common/charset_conv.cpp \
	common/color.cpp \
	common/dispatch.cpp \
	common/fs.cpp \
	common/hotkey.cpp \
	common/io.cpp \
	common/json.cpp \
	common/keyframe.cpp \
	common/log.cpp \
	common/mru.cpp \
	common/option.cpp \
	common/option_visit.cpp \
	common/parser.cpp \
	common/path.cpp \
	common/thesaurus.cpp \
	common/util.cpp \
	common/vfr.cpp \
	unix/access.cpp \
	unix/fs.cpp \
	unix/log.cpp \
	unix/path.cpp \
	unix/util.cpp

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
