# $Id$
include ../Makefile.inc

PROGRAM = run

LIBS += -L../libaegisub -laegisub  -lgtest -L../universalchardet -luniversalchardet
LDFLAGS += -Wl,-rpath $(CURDIR)/../libaegisub $(LDFLAGS_ICONV)

CPPFLAGS += -I../src/include -I../libaegisub/include $(CFLAGS_ICONV)
CXXFLAGS += -Wno-unused-value

ifeq (yes, $(BUILD_DARWIN))
LDFLAGS += -framework ApplicationServices
endif

SRC = \
		main.cpp \
		util.cpp \
		util_unix.cpp \
		libaegisub_access.cpp \
		libaegisub_cajun.cpp \
		libaegisub_color.cpp \
		libaegisub_dialogue_lexer.cpp \
		libaegisub_hotkey.cpp \
		libaegisub_iconv.cpp \
		libaegisub_keyframe.cpp \
		libaegisub_line_iterator.cpp \
		libaegisub_line_wrap.cpp \
		libaegisub_option.cpp \
		libaegisub_mru.cpp \
		libaegisub_signals.cpp \
		libaegisub_thesaurus.cpp \
		libaegisub_util.cpp \
		libaegisub_vfr.cpp \
		libaegisub_word_split.cpp

HEADER = \
	*.h

include ../Makefile.target
