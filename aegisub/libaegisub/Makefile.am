# $Id$
AUTOMAKE_OPTIONS = subdir-objects
AM_CXXFLAGS=
DISTCLEANFILES=

lib_LTLIBRARIES = libaegisub-2.2.la
libaegisub_2_2_la_CPPFLAGS = -I../src/include -Iinclude -I. @WX_CPPFLAGS@

if PRECOMPILED_HEADER
BUILT_SOURCES = lagi_pre.h.gch
AM_CXXFLAGS += -include lagi_pre.h -Winvalid-pch -fpch-deps -fpch-preprocess
nodist_libaegisub_2_2_la_SOURCES = lagi_pre.h.gch
endif

if PRECOMPILED_HEADER
# This doesn't depend on Makefile on purpose, you should already know what you're doing when using this.
lagi_pre.h.gch: lagi_pre.h
	@CXX@ $(libaegisub_2_2_la_CPPFLAGS) @CXXFLAGS@ @DEBUG_FLAGS@ -fPIC -DPIC lagi_pre.h
DISTCLEANFILES += lagi_pre.h.gch
endif

libaegisub_2_2_la_SOURCES = \
	common/mru.cpp \
	common/option.cpp \
	common/option_visit.cpp \
	common/validator.cpp \
	unix/util.cpp \
	unix/io.cpp \
	unix/access.cpp

noinst_HEADERS = *.h

