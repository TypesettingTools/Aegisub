noinst_LIBRARIES = libosxutil.a

libosxutil_a_SOURCES = bundledirs.c launch.c

libosxutil_a_CFLAGS = -I/Developer/SDKs/MacOSX10.5.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/LaunchServices.framework/Versions/A/Headers/
noinst_HEADERS = libosxutil.h

CLEANFILES = bundledirs-test

EXTRA_DIST= bundledirs-test.c

bundledirs-test: libosxutil.a
	$(CC) -c bundledirs-test.c
	$(CC) -o bundledirs-test -framework CoreFoundation libosxutil_a-bundledirs.o libosxutil_a-launch.o libosxutil.a 
