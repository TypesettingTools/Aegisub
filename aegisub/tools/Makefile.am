bin_PROGRAMS = common-respack
common_respack_CPPFLAGS = @WX_CPPFLAGS@
common_respack_LDFLAGS = @WX_LIBS@
common_respack_SOURCES = common-respack.cpp

if BUILD_DARWIN
bin_PROGRAMS += osx-bundle-restart-helper
osx_bundle_restart_helper_SOURCES = osx-bundle-restart-helper.c
endif

EXTRA_DIST = \
	osx-bundle-restart-helper.c \
	osx-bundle.sh \
	osx-dmg.sh \
	osx-fix-libs.py
