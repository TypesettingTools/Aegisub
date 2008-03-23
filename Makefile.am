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


SUBDIRS = \
	$(univchardet) \
	$(auto3) \
    $(libass) \
	aegisub \
	automation \
	po \
	m4macros

EXTRA_DIST = \
	intltool-extract.in \
	intltool-merge.in   \
	intltool-update.in  \
	mkinstalldirs \
	config.guess \
	config.sub \
	depcomp \
	install-sh \
	missing \
	mkinstalldirs \
	INSTALL \
	README
