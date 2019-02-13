if BUILD_MINGW
AM_CPPFLAGS = \
	-I$(top_srcdir)/include
lib_LTLIBRARIES = csri_avs.la
csri_avs_la_SOURCES = avisynth.cpp
csri_avs_la_LIBADD = \
	../../lib/libcsri_la-enumerate.lo \
	../../lib/libcsri_la-list.lo \
	../../lib/libcsri_la-wrap.lo \
	../../subhelp/libsubhelp_la-logging.lo
csri_avs_la_LDFLAGS = -avoid-version -no-undefined -module
endif

noinst_HEADERS = avisynth.h
EXTRA_DIST = avisynth.cpp
