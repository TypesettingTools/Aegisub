include ../Makefile.inc

CXXFLAGS += -DCONFIG_ICONV -DCONFIG_FONTCONFIG $(CFLAGS_FREETYPE) $(CFLAGS_ICONV) $(CFLAGS_FONTCONFIG)

LIB = libass_aegisub.a

SRC = \
	ass.c \
	ass_bitmap.c \
	ass_cache.c \
	ass_drawing.c \
	ass_font.c \
	ass_fontconfig.c \
	ass_library.c \
	ass_parse.c \
	ass_render.c \
	ass_render_api.c \
	ass_strtod.c \
	ass_utils.c

HEADER = *.h

include ../Makefile.target
