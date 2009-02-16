noinst_LIBRARIES = libosxutil.a

libosxutil_a_SOURCES = bundledirs.c

noinst_HEADERS = libosxutil.h

CLEANFILES = bundledirs-test

EXTRA_DIST= bundledirs-test.c

bundledirs-test: libosxutil.a
	$(CC) -c bundledirs-test.c
	$(CC) -o bundledirs-test -framework CoreFoundation bundledirs-test.o libosxutil.a 
