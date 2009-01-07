noinst_LIBRARIES = libffmpegsource2_aegisub.a

AM_CPPFLAGS = -I../aegisub -D__UNIX__ -DHAVE_STRLCPY @FFMPEG_CFLAGS@


libffmpegsource2_aegisub_a_SOURCES = \
	ffaudiosource.cpp \
	ffms.cpp \
	ffvideosource.cpp \
	indexing.cpp \
	MatroskaParser.c \
	stdiostream.c \
	utils.cpp \
	wave64writer.cpp


libffmpegsource2_aegisub_a_SOURCES += \
	MatroskaParser.h \
	avisynth.h \
	ffaudiosource.h \
	ffavisynth.h \
	ffms.h \
	ffpp.h \
	ffswscale.h \
	ffvideosource.h \
	indexing.h \
	stdiostream.h \
	utils.h \
	wave64writer.h

EXTRA_DIST = \
	ffms2.html
