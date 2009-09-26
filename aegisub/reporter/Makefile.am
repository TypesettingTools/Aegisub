AUTOMAKE_OPTIONS = foreign
AM_CXXFLAGS =

bin_PROGRAMS = reporter

if PRECOMPILED_HEADER
BUILT_SOURCES = r_pre.h.gch
AM_CXXFLAGS += -include r_pre.h -Winvalid-pch -fpch-deps -fpch-preprocess
nodist_reporter_SOURCES = r_prec.h.gch
endif

reporter_CPPFLAGS = -Iinclude @LIBCURL_CFLAGS@ @WX_CPPFLAGS@
reporter_LDFLAGS = @WX_LIBS@ @LIBCURL_LIBS@

if PRECOMPILED_HEADER
# This doesn't depend on Makefile on purpose, you should already know what you're doing when using this.
r_pre.h.gch: r_pre.h
	@CXX@ @WX_CPPFLAGS@ @CXXFLAGS@ @DEBUG_FLAGS@ r_pre.h
endif


reporter_SOURCES = \
	aegisub.cpp \
	main.cpp \
	name_map.cpp \
	platform.cpp \
	platform_unix.cpp \
	progress.cpp \
	report.cpp \
	upload.cpp \
	view.cpp

if BUILD_BSD
reporter_SOURCES += platform_unix_bsd.cpp
endif

if BUILD_LINUX
reporter_SOURCES += platform_unix_linux.cpp
endif


reporter_SOURCES += \
	*.h \
	include/*.h

