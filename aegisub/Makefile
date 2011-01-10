include Makefile.inc

ifeq (yes, $(WITH_LIBASS))
SUBDIRS += libass
endif

ifeq (yes, $(HAVE_PROVIDER_FFMPEGSOURCE))
SUBDIRS += libffms
endif

SUBDIRS += \
	universalchardet \
	libaegisub \
	tools \
	src \
	reporter \
	automation \
	desktop \
	po

all: ;

ifeq (yes, $(BUILD_DARWIN))
osx-bundle:
	$(BIN_SHELL) tools/osx-bundle.sh "$(BUNDLE_STRING)" $(AEGISUB_VERSION_DATA) DICT_DIR=$(DICT_DIR)

osx-dmg:
	$(BIN_SHELL) tools/osx-dmg.sh "$(BUNDLE_STRING)" "$(DMG_STRING)"

osx-tinderbox-bundle:
	$(BIN_SHELL) tools/osx-bundle.sh "$(T_BUNDLE)" $(AEGISUB_VERSION_DATA) DICT_DIR="$(DICT_DIR)"

osx-tinderbox-dmg:
	$(BIN_SHELL) tools/osx-dmg.sh "$(T_DMG)" "$(T_BUNDLE)"
	$(BIN_MV)  "$(T_DMG).dmg" bundle.dmg
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
	svn_revision \
	config.log \
	install-sh \
	Makefile.inc.in \
	Makefile.target


# m4macros/
EXTRA_DIST += \
	m4macros/check_gnu_make.m4 \
	m4macros/acx_pthread.m4 \
	m4macros/ac_agi.m4 \
	m4macros/ax_lang_compiler_ms.m4 \
	m4macros/ac_agi_mdcpucfg.m4 \
	m4macros/ax_openmp.m4 \
	m4macros/ax_check_gl.m4 \
	m4macros/ac_flag.m4

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
	svn_revision \
	Makefile.inc \
	config.log \
	acconf.h.in \
	config.status \
	autom4te.cache \
	aclocal.m4 \

include Makefile.target
