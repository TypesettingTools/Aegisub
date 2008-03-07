if HAVE_AUTO3_LUA

AM_CFLAGS = -DAUTO3LIB @LUA50_CPPFLAGS@
AM_LDFLAGS = @LUA50_LDFLAGS@

lib_LTLIBRARIES = libaegisub-auto3.la
libaegisub_auto3_la_SOURCES = auto3.c
noinst_HEADERS = auto3.h

endif

EXTRA_DIST= callables.c
