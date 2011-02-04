include ../Makefile.inc

PROGRAM = reporter-3.0
PROGRAM_INSTALL = yes
PRECOMPILED_HEADER_NAME=r_pre.h


#if PRECOMPILED_HEADER
#BUILT_SOURCES = r_pre.h.gch
#AM_CXXFLAGS += -include r_pre.h -Winvalid-pch -fpch-deps -fpch-preprocess
#nodist_reporter_3_0_SOURCES = r_prec.h.gch
#endif

CXXFLAGS += -Iinclude $(CFLAGS_WX) -I../libaegisub/include
LDFLAGS += $(LDFLAGS_WX) $(LDFLAGS_LIBCURL) -L../libaegisub -laegisub-3.0 $(LDFLAGS_UCHARDET) -Wl,-rpath ../libaegisub

ifeq (yes, $(HAVE_PROVIDER_FFMPEGSOURCE))
LDFLAGS += $(LDFLAGS_LIBAVFORMAT) $(LDFLAGS_LIBAVCODEC) $(LDFLAGS_LIBSWSCALE) $(LDFLAGS_LIBAVUTIL) $(LDFLAGS_LIBPOSTPROC)
endif

#if PRECOMPILED_HEADER
## This doesn't depend on Makefile on purpose, you should already know what you're doing when using this.
#r_pre.h.gch: r_pre.h
#	@CXX@ @WX_CPPFLAGS@ @CXXFLAGS@ @DEBUG_FLAGS@ r_pre.h
#DISTCLEANFILES += r_pre.h.gch
#endif


# File-specific flags
upload.o: CXXFLAGS += $(CFLAGS_CURL)

SRC = \
	aegisub.cpp \
	main.cpp \
	platform.cpp \
	platform_unix.cpp \
	progress.cpp \
	report.cpp \
	upload.cpp \
	util.cpp \
	view.cpp


ifeq (yes, $(BUILD_BSD))
SRC += platform_unix_bsd.cpp
endif

ifeq (yes, $(BUILD_LINUX))
SRC += platform_unix_linux.cpp
endif

ifeq (yes, $(BUILD_DARWIN))
SRC += platform_unix_osx.cpp
endif


HEADER = \
	*.h \
	include/*.h

include ../Makefile.target
-include *.d
