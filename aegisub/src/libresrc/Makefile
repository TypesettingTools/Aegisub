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
	../../tools/common-respack bitmap.cpp ../bitmaps/16 ../bitmaps/24 ../bitmaps/misc/splash.png ../bitmaps/misc/wxicon.png

default_config.cpp: ../../tools/common-respack *.json
	../../tools/common-respack \
		default_config.cpp \
		./default_config.json \
		./default_hotkey.json \
		./default_menu.json \
		./default_mru.json \
		./default_path.json \
		./default_toolbar.json

EXTRA_DIST = *.json

CLEANFILES= \
	bitmap.cpp \
	bitmap.h \
	default_config.cpp \
	default_config.h

include ../../Makefile.target
-include *.d
