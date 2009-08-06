noinst_LIBRARIES = libresrc.a

nodist_libresrc_a_SOURCES = libresrc.cpp
libresrc_a_CPPFLAGS = @WX_CPPFLAGS@

libresrc.cpp: ../../tools/common-respack
	../../tools/common-respack libresrc.cpp ../bitmaps/16 ../bitmaps/24 ../bitmaps/misc/splash.png ../bitmaps/misc/wxicon.png

CLEANFILES= libresrc.cpp libresrc.h
