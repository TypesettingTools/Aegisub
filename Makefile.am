AUTOMAKE_OPTIONS = foreign


if HAVE_UNIVCHARDET
univchardet = universalchardet
endif

if HAVE_AUTO3_LUA
auto3 = auto3
endif

if WITH_LIBASS
libass = libass
endif

if HAVE_PROVIDER_FFMPEGSOURCE
ffmpegsource = FFmpegSource2
endif

SUBDIRS = \
	$(univchardet) \
	$(auto3) \
    $(libass) \
	$(ffmpegsource) \
	aegisub \
	automation \
	po \
	desktop

osx-bundle:
	@SHELL@ scripts/osx-bundle.sh @PACKAGE_TARNAME@-@PACKAGE_VERSION@ @AEGISUB_VERSION_DATA@

osx-dmg:
	@SHELL@ scripts/osx-dmg.sh @PACKAGE_TARNAME@-@PACKAGE_VERSION@ "@PACKAGE_TARNAME@ @PACKAGE_VERSION@"

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
	m4macros/acx_pthread.m4 \
	m4macros/ac_agi.m4 \
	m4macros/ax_lang_compiler_ms.m4 \
	m4macros/ac_agi_mdcpucfg.m4 \
	m4macros/ax_openmp.m4 \
	m4macros/ax_check_gl.m4 \
	m4macros/ac_flag.m4

CLEANFILES= \
	intltool-extract \
	intltool-merge \
	intltool-update
