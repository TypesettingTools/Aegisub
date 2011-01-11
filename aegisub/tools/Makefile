include ../Makefile.inc

PROGRAM = common-respack

SRC			+= common-respack.cpp

ifdef $(BUILD_DARWIN)
bin_PROGRAMS += osx-bundle-restart-helper
osx_bundle_restart_helper_SOURCES = osx-bundle-restart-helper.c
endif


EXTRA_DIST = \
	osx-bundle-restart-helper.c \
	osx-bundle.sh \
	osx-dmg.sh \
	osx-fix-libs.py

CLEANFILES= \
	osx-bundle.sed

include ../Makefile.target
