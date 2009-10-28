noinst_LIBRARIES = libass_aegisub.a

AM_CPPFLAGS = @FREETYPE_CFLAGS@ -DCONFIG_ICONV -DCONFIG_FONTCONFIG @FONTCONFIG_CFLAGS@ @ICONV_CFLAGS@

if BUILD_DARWIN
AM_CPPFLAGS += -DBUILD_DARWIN
endif

libass_aegisub_a_SOURCES = \
	ass.c \
	ass_bitmap.c \
	ass_cache.c \
	ass_drawing.c \
	ass_font.c \
	ass_fontconfig.c \
	ass_library.c \
	ass_render.c \
	ass_utils.c \
	ass_parse.c

libass_aegisub_a_SOURCES += \
	*.h
