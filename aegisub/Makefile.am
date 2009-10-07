AUTOMAKE_OPTIONS = foreign


if HAVE_UNIVCHARDET
univchardet = universalchardet
endif

if WITH_LIBASS
libass = libass
endif

if HAVE_PROVIDER_FFMPEGSOURCE
ffmpegsource = libffms
endif

SUBDIRS = \
	$(ffmpegsource) \
    $(libass) \
	$(univchardet) \
	tools \
	src \
	reporter \
	automation \
	desktop \
	po

osx-bundle:
	@SHELL@ tools/osx-bundle.sh "Aegisub (r@SVN_REVISION@-@DARWIN_ARCH@@PACKAGE_DEBUG@)" @AEGISUB_VERSION_DATA@ DICT_DIR=$(DICT_DIR)


osx-dmg:
	@SHELL@ tools/osx-dmg.sh "Aegisub (r@SVN_REVISION@-@DARWIN_ARCH@@PACKAGE_DEBUG@)" "Aegisub (r@SVN_REVISION@-@DARWIN_ARCH@@PACKAGE_DEBUG@)"

osx-tinderbox-bundle:
	@SHELL@ tools/osx-bundle.sh "$(T_BUNDLE)" @AEGISUB_VERSION_DATA@ DICT_DIR="$(DICT_DIR)"

osx-tinderbox-dmg:
	@SHELL@ tools/osx-dmg.sh "$(T_DMG)" "$(T_BUNDLE)"
	mv  "$(T_DMG).dmg" bundle.dmg

EXTRA_DIST = \
	intltool-extract.in \
	intltool-merge.in   \
	intltool-update.in  \
	config.guess \
	config.sub \
	depcomp \
	install-sh \
	missing \
	INSTALL \
	README \
	svn_revision

# m4macros/
EXTRA_DIST += \
	m4macros/check_gnu_make.m4 \
	m4macros/boost.m4 \
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


CLEANFILES= \
	intltool-extract \
	intltool-merge \
	intltool-update
