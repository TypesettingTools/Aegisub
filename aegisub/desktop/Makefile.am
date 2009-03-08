
if DESKTOP_DATADIR

applicationsdir = $(DESKTOP_DATADIR)/applications
applications_DATA = aegisub.desktop

iconsscalabledir = $(DESKTOP_DATADIR)/icons/hicolor/scalable/apps
iconsscalable_DATA = scalable/aegisub.svg

install-data-hook:
	touch "$(DESTDIR)$(DESKTOP_DATADIR)/icons/hicolor"


icons16dir = $(DESKTOP_DATADIR)/icons/hicolor/16x16/apps
icons16_DATA = 16x16/aegisub.png

icons22dir = $(DESKTOP_DATADIR)/icons/hicolor/22x22/apps
icons22_DATA = 22x22/aegisub.png

icons24dir = $(DESKTOP_DATADIR)/icons/hicolor/24x24/apps
icons24_DATA = 24x24/aegisub.png

icons32dir = $(DESKTOP_DATADIR)/icons/hicolor/32x32/apps
icons32_DATA = 32x32/aegisub.png

icons48dir = $(DESKTOP_DATADIR)/icons/hicolor/48x48/apps
icons48_DATA = 48x48/aegisub.png

icons64dir = $(DESKTOP_DATADIR)/icons/hicolor/64x64/apps
icons64_DATA = 64x64/aegisub.png

endif


EXTRA_DIST = \
	aegisub.desktop.in \
	16x16/aegisub.png \
	22x22/aegisub.png \
	24x24/aegisub.png \
	32x32/aegisub.png \
	48x48/aegisub.png \
	64x64/aegisub.png \
	scalable/aegisub.svg

DISTCLEANFILES = aegisub.desktop
