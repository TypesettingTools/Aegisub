include ../Makefile.inc

ifeq (yes, $(BUILD_DARWIN))
osx-bundle-restart-helper: osx-bundle-restart-helper.m
	$(BIN_CC) -o osx-bundle-restart-helper -fmodules osx-bundle-restart-helper.m
CLEANFILES += osx-bundle-restart-helper
all: osx-bundle-restart-helper
endif

CXXFLAGS +=  -I../libaegisub/include $(CFLAGS_ICU)
LIBS := -L../libaegisub -laegisub $(LIBS) $(LIBS_BOOST) $(LIBS_ICU)

repack-thes-dict: repack-thes-dict.cpp
	$(BIN_CXX) -o repack-thes-dict repack-thes-dict.cpp  $(CXXFLAGS) $(LIBS)

include ../Makefile.target
