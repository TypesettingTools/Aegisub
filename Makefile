include Makefile.inc

SUBDIRS += \
	packages/desktop \
	vendor/luajit \
	vendor/universalchardet \
	vendor/luabins \
	libaegisub \
	tools \
	src \
	automation \
	po

all: ;

ifeq (yes, $(BUILD_DARWIN))
osx-bundle:
	$(BIN_SHELL) tools/osx-bundle.sh "$(AEGISUB_COMMAND)" "$(BIN_WX_CONFIG)" DICT_DIR=$(DICT_DIR)

osx-dmg: osx-bundle
	codesign -s 'Mac Developer' --deep Aegisub.app || true
	$(BIN_SHELL) tools/osx-dmg.sh "$(BUILD_VERSION_STRING)"
endif

DISTCLEANFILES += \
	acconf.h \
	configure \
	acconf.h.in~ \
	build/git_version.h \
	Makefile.inc \
	config.log \
	acconf.h.in \
	config.status \
	autom4te.cache \
	aclocal.m4 \

include Makefile.target
