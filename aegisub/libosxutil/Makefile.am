noinst_LIBRARIES = libosxutil.a

libosxutil_a_SOURCES = bundledirs.c

noinst_HEADERS = libosxutil.h

EXTRA_DIST= callables.c


if BUILD_DARWIN
CLEANFILES = bundledirs-test
bundledirs-test: libosxutil.a
	$(CC) -c bundledirs-test.c
	$(CC) -o bundledirs-test -framework CoreFoundation bundledirs-test.o libosxutil.a 
endif
