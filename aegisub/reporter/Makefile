include ../Makefile.inc

PROGRAM = reporter-3.0
PROGRAM_INSTALL = yes
PRECOMPILED_HEADER_NAME=r_pre.h

CXXFLAGS += -Iinclude $(CFLAGS_WX) -I../libaegisub/include
LIBS += $(LIBS_WX) $(LIBS_LIBCURL) -L../libaegisub -laegisub-3.0 $(LIBS_UCHARDET) $(LIBS_GL)
LDFLAGS += -Wl,-rpath ../libaegisub

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
