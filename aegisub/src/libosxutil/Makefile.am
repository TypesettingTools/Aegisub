noinst_LIBRARIES = libosxutil.a

libosxutil_a_SOURCES = bundledirs.c launch.c

libosxutil_a_CFLAGS = @CXXFLAGS_OSX@
noinst_HEADERS = libosxutil.h

CLEANFILES = bundledirs-test

EXTRA_DIST= bundledirs-test.c

bundledirs-test: libosxutil.a
	$(CC) -c bundledirs-test.c
	$(CC) -o bundledirs-test -framework CoreFoundation bundledirs-test.c libosxutil.a 
