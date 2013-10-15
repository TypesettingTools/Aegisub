include ../Makefile.inc

PROGRAM = common-respack

SRC			+= common-respack.cpp

ifeq (yes, $(BUILD_DARWIN))
osx-bundle-restart-helper: osx-bundle-restart-helper.c
	$(BIN_CC) -o osx-bundle-restart-helper osx-bundle-restart-helper.c
CLEANFILES += osx-bundle-restart-helper
all: osx-bundle-restart-helper
endif

EXTRA_DIST = \
	osx-bundle-restart-helper.c \
	osx-bundle.sh \
	osx-bundle.sed.in \
	osx-dmg.sh \
	osx-fix-libs.py

DISTCLEANFILES= \
	osx-bundle.sed

include ../Makefile.target
