include ../../Makefile.inc

LIB = libresrc.a

CPPFLAGS = $(CPPFLAGS_WX)

SRC = \
	bitmap.cpp \
	default_config.cpp \
	libresrc.cpp

HEADER= \
	libresrc.h

all: bitmap.cpp default_config.cpp

bitmap.cpp: ../../tools/common-respack
	../../tools/common-respack ../bitmaps/manifest.respack bitmap.cpp bitmap.h

default_config.cpp: ../../tools/common-respack *.json
	../../tools/common-respack manifest.respack default_config.cpp default_config.h

EXTRA_DIST = *.json

CLEANFILES= \
	bitmap.cpp \
	bitmap.h \
	default_config.cpp \
	default_config.h

include ../../Makefile.target
-include *.d
