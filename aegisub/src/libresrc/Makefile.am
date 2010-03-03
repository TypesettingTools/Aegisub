noinst_LIBRARIES = libresrc.a

libresrc_a_SOURCES = libresrc.cpp libresrc.h
nodist_libresrc_a_SOURCES = bitmap.cpp bitmap.h default_config.cpp default_config.h
libresrc_a_CPPFLAGS = @WX_CPPFLAGS@

BUILT_SOURCES = bitmap.cpp default_config.cpp

bitmap.cpp: ../../tools/common-respack
	../../tools/common-respack bitmap.cpp ../bitmaps/16 ../bitmaps/24 ../bitmaps/misc/splash.png ../bitmaps/misc/wxicon.png

default_config.cpp: ../../tools/common-respack
	../../tools/common-respack default_config.cpp ./default_mru.json

EXTRA_DIST = mru.json

CLEANFILES= \
	bitmap.cpp \
	bitmap.h \
	default_config.cpp \
	default_config.h
