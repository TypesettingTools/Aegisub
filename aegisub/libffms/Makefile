include ../Makefile.inc

CXXFLAGS = -Icore -Iinclude -D__UNIX__ -DFFMS_EXPORTS -DHAVE_STRLCPY -D__STDC_CONSTANT_MACROS -fPIC
CXXFLAGS += $(CFLAGS_LIBAVFORMAT) $(CFLAGS_LIBAVCODEC) $(CFLAGS_LIBAVUTIL) $(CFLAGS_LIBPOSTPROC)

LIB = libffmpegsource_aegisub.a

SRC = \
	src/core/audiosource.cpp \
	src/core/ffms.cpp \
	src/core/indexing.cpp \
	src/core/lavfaudio.cpp \
	src/core/lavfindexer.cpp \
	src/core/lavfvideo.cpp \
	src/core/matroskaaudio.cpp \
	src/core/matroskaindexer.cpp \
	src/core/matroskaparser.c \
	src/core/matroskavideo.cpp \
	src/core/stdiostream.c \
	src/core/utils.cpp \
	src/core/videosource.cpp \
	src/core/wave64writer.cpp

HEADER = \
	src/core/*.h \
	include/*.h

include ../Makefile.target
-include src/core/*.d

