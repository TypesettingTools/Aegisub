noinst_LIBRARIES = libass.a

libass_a_CPPFLAGS = @FREETYPE_CFLAGS@ -DHAVE_FONTCONFIG -DUSE_ICONV @FONTCONFIG_CFLAGS@
libass_a_SOURCES = ass.c ass_cache.c ass_font.c ass_fontconfig.c ass_render.c \
                    ass_utils.c ass_bitmap.c ass_library.c mputils.c \
                    ass_bitmap.h ass_cache.h ass_fontconfig.h ass_font.h ass.h \
                    ass_library.h ass_types.h ass_utils.h help_mp.h mputils.h
