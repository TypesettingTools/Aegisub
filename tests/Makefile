include ../Makefile.inc

GTEST_ROOT = ../vendor/googletest

PROGRAM = run

LIBS += -L../libaegisub -laegisub -L../vendor/universalchardet -luniversalchardet $(LIBS_BOOST) $(LIBS_ICU)
LDFLAGS += -Wl,-rpath $(CURDIR)/../libaegisub $(LDFLAGS_ICONV)

CPPFLAGS += -I../src/include -I../libaegisub/include $(CFLAGS_ICONV) -I${GTEST_ROOT} -I${GTEST_ROOT}/include -I./support $(CPPFLAGS_BOOST)
CXXFLAGS += -Wno-unused-value -Wno-sign-compare

PRECOMPILED_HEADER_NAME = ../libaegisub/lagi_pre.h

ifeq (yes, $(BUILD_DARWIN))
LDFLAGS += -framework ApplicationServices -framework Foundation
endif

SRC = \
		support/main.cpp \
		support/util.cpp \
		support/util_unix.cpp \
		tests/access.cpp \
		tests/cajun.cpp \
		tests/color.cpp \
		tests/dialogue_lexer.cpp \
		tests/format.cpp \
		tests/fs.cpp \
		tests/hotkey.cpp \
		tests/iconv.cpp \
		tests/ifind.cpp \
		tests/karaoke_matcher.cpp \
		tests/keyframe.cpp \
		tests/line_iterator.cpp \
		tests/line_wrap.cpp \
		tests/mru.cpp \
		tests/option.cpp \
		tests/path.cpp \
		tests/signals.cpp \
		tests/syntax_highlight.cpp \
		tests/thesaurus.cpp \
		tests/util.cpp \
		tests/uuencode.cpp \
		tests/vfr.cpp \
		tests/word_split.cpp \
		${GTEST_ROOT}/src/gtest-all.cc

HEADER = support/*.h tests/*.h

include ../Makefile.target
-include support/*.d
-include tests/*.d
