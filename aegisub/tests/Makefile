# $Id$
include ../Makefile.inc

PROGRAM = run

LDFLAGS		+= -L../libaegisub -laegisub-3.0 -rpath $(CURDIR)/../libaegisub -lgtest -L../universalchardet -luniversalchardet
LDFLAGS		+= $(LDFLAGS_ICONV)

CPPFLAGS	+= -I../src/include -I../libaegisub/include $(CFLAGS_ICONV)


ifdef $(BUILD_DARWIN)
LDFLAGS += -framework ApplicationServices
endif

SRC = \
		main.cpp \
		util.cpp \
		util_unix.cpp \
		libaegisub_access.cpp \
		libaegisub_cajun.cpp \
		libaegisub_iconv.cpp \
		libaegisub_keyframe.cpp \
		libaegisub_line_iterator.cpp \
		libaegisub_mru.cpp \
		libaegisub_signals.cpp \
		libaegisub_util.cpp \
		libaegisub_vfr.cpp
		
HEADER = \
	*.h

include ../Makefile.target
