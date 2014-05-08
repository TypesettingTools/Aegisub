include ../Makefile.inc

ifeq (yes, $(BUILD_DARWIN))
osx-bundle-restart-helper: osx-bundle-restart-helper.c
	$(BIN_CC) -o osx-bundle-restart-helper osx-bundle-restart-helper.c
CLEANFILES += osx-bundle-restart-helper
all: osx-bundle-restart-helper
endif

CXXFLAGS +=  -I../libaegisub/include $(CFLAGS_ICU)
LIBS := -L../libaegisub -laegisub $(LIBS)
LIBS += $(LIBS_BOOST) $(LIBS_ICU)

repack-thes-dict: repack-thes-dict.cpp
	$(BIN_CXX) -o repack-thes-dict repack-thes-dict.cpp  $(CXXFLAGS) $(LIBS)

EXTRA_DIST = \
	osx-bundle-restart-helper.c \
	osx-bundle.sh \
	osx-bundle.sed.in \
	osx-dmg.sh \
	osx-fix-libs.py

DISTCLEANFILES= \
	osx-bundle.sed

include ../Makefile.target
