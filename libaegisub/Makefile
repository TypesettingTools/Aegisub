include ../Makefile.inc

LIB = libaegisub.a

CXXFLAGS += -Iinclude -I../src -I.. -fPIC
CPPFLAGS += $(CPPFLAGS_BOOST) $(CFLAGS_LUA)

PRECOMPILED_HEADER_NAME = lagi_pre.h
lagi_pre.h.gch: CXXFLAGS := $(CXXFLAGS)

common/charset_conv.o: CXXFLAGS += $(CFLAGS_ICONV)
common/parser.o: CXXFLAGS += -ftemplate-depth=256
unix/path.o: CXXFLAGS += -DP_DATA=\"$(P_DATA)\" -DP_DOC=\"$(P_DOC)\" -DP_LOCALE=\"$(P_LOCALE)\"

SRC += \
	common/parser.cpp \
	ass/dialogue_parser.cpp \
	common/cajun/elements.cpp \
	common/cajun/reader.cpp \
	common/cajun/writer.cpp \
	common/calltip_provider.cpp \
	common/character_count.cpp \
	common/charset.cpp \
	common/charset_6937.cpp \
	common/charset_conv.cpp \
	common/color.cpp \
	common/dispatch.cpp \
	common/file_mapping.cpp \
	common/fs.cpp \
	common/hotkey.cpp \
	common/io.cpp \
	common/json.cpp \
	common/kana_table.cpp \
	common/karaoke_matcher.cpp \
	common/keyframe.cpp \
	common/log.cpp \
	common/mru.cpp \
	common/option.cpp \
	common/option_visit.cpp \
	common/path.cpp \
	common/thesaurus.cpp \
	common/util.cpp \
	common/vfr.cpp \
	common/ycbcr_conv.cpp \
	lua/modules.cpp \
	lua/modules/lfs.cpp \
	lua/modules/lpeg.c \
	lua/modules/re.cpp \
	lua/modules/unicode.cpp \
	lua/script_reader.cpp \
	lua/utils.cpp \
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
-include lua/*/*.d
-include common/*/*.d
