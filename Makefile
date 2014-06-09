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


EXTRA_DIST = \
	INSTALL \
	LICENCE \
	README \
	acinclude.m4 \
	autogen.sh \
	configure.in \
	acconf.h.in\
	configure \
	build/git_version.h \
	config.log \
	config.sub \
	config.guess \
	install-sh \
	Makefile.inc.in \
	Makefile.target


# m4macros/
EXTRA_DIST += \
	m4macros/ac_agi.m4 \
	m4macros/ac_agi_mdcpucfg.m4 \
	m4macros/ac_flag.m4 \
	m4macros/agi_find_libheader.m4 \
	m4macros/ax_check_gl.m4 \
	m4macros/ax_lang_compiler_ms.m4 \
	m4macros/ax_pthread.m4 \
	m4macros/check_gnu_make.m4

# packages/osx_bundle/
EXTRA_DIST += \
	packages/osx_bundle/Contents/Info.plist \
	packages/osx_bundle/Contents/Resources/*.icns \
	packages/osx_bundle/Contents/Resources/etc/fonts/fonts.conf \
	packages/osx_bundle/Contents/Resources/etc/fonts/fonts.dtd \
	packages/osx_bundle/Contents/Resources/etc/fonts/conf.d/*.conf

# packages/osx_dmg/
EXTRA_DIST += \
	packages/osx_dmg/dmg_background.png \
	packages/osx_dmg/dmg_set_style.applescript


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
