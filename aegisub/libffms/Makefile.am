AUTOMAKE_OPTIONS = subdir-objects

noinst_LIBRARIES = libffmpegsource_aegisub.a

AM_CPPFLAGS = -I../src -Icore -Iinclude -D__UNIX__ -DFFMS_EXPORTS -DHAVE_STRLCPY @LIBAVFORMAT_CFLAGS@ @LIBAVCODEC_CFLAGS@ @LIBSWSCALE_CFLAGS@ @LIBAVUTIL_CFLAGS@ @LIBPOSTPROC_CFLAGS@ @BOOST_CPPFLAGS@


libffmpegsource_aegisub_a_SOURCES = \
	src/core/ffaudiosource.cpp \
	src/core/ffhaaliaudio.cpp \
	src/core/ffhaaliindexer.cpp \
	src/core/ffhaalivideo.cpp \
	src/core/fflavfaudio.cpp \
	src/core/fflavfindexer.cpp \
	src/core/fflavfvideo.cpp \
	src/core/ffmatroskaaudio.cpp \
	src/core/ffmatroskaindexer.cpp \
	src/core/ffmatroskavideo.cpp \
	src/core/ffms.cpp \
	src/core/ffvideosource.cpp \
	src/core/indexing.cpp \
	src/core/matroskaparser.c \
	src/core/stdiostream.c \
	src/core/utils.cpp \
	src/core/wave64writer.cpp

libffmpegsource_aegisub_a_SOURCES += \
	src/core/*.h
