if BUILD_MINGW
PLATF = win32
else
PLATF = posix
AM_CPPFLAGS = -DCSRI_PATH="\"$(csri_path)\""
endif
lib_LTLIBRARIES = libcsri.la
noinst_HEADERS = csrilib.h posix/csrilib_os.h win32/csrilib_os.h
libcsri_la_SOURCES = list.c wrap.c $(PLATF)/enumerate.c
libcsri_la_CFLAGS = -I$(top_srcdir)/lib/posix -I$(top_srcdir)/include
libcsri_la_LIBADD = ../subhelp/libsubhelp_la-logging.lo
libcsri_la_LDFLAGS = -static

EXTRA_DIST = posix/enumerate.c win32/enumerate.c
