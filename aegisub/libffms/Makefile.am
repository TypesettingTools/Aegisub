AUTOMAKE_OPTIONS = subdir-objects

noinst_LIBRARIES = libffmpegsource_aegisub.a

AM_CPPFLAGS = -I../src -Icore -Iinclude -D__UNIX__ -DFFMS_EXPORTS -DHAVE_STRLCPY @LIBAVFORMAT_CFLAGS@ @LIBAVCODEC_CFLAGS@ @LIBSWSCALE_CFLAGS@ @LIBAVUTIL_CFLAGS@ @LIBPOSTPROC_CFLAGS@


libffmpegsource_aegisub_a_SOURCES = \
	src/core/audiosource.cpp \
	src/core/ffms.cpp \
	src/core/haaliaudio.cpp \
	src/core/haaliindexer.cpp \
	src/core/haalivideo.cpp \
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

libffmpegsource_aegisub_a_SOURCES += \
	src/core/*.h
