if HAVE_AUTO3_LUA

AM_CFLAGS = -DAUTO3LIB @LUA50_CFLAGS@
AM_LDFLAGS = @LUA50_LDFLAGS@ @LUALIB50_LDFLAGS@

lib_LTLIBRARIES = libaegisub-auto3-2.1.la
libaegisub_auto3_2_1_la_SOURCES = auto3.c
noinst_HEADERS = auto3.h

endif

EXTRA_DIST= callables.c
