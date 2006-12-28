noinst_LIBRARIES = libposix.a
nodist_libposix_a_SOURCES = res.cpp
CLEANFILES = res.cpp res.h
EXTRA_DIST = genres.sh defines.h

res.h: $(srcdir)/genres.sh $(srcdir)/../res.rc 
	/bin/sh $^
res.cpp: res.h

