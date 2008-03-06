if BUILD_MINGW
PLATF = win32
else
PLATF = posix
endif
noinst_LTLIBRARIES = libsubhelp.la
libsubhelp_la_SOURCES = $(PLATF)/openfile.c logging.c
libsubhelp_la_CFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = win32/openfile.c posix/openfile.c
