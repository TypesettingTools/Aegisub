include ../Makefile.inc

applicationsdir = $(DESKTOP_DATADIR)/applications
applications_DATA = aegisub.desktop

iconsscalabledir = $(DESKTOP_DATADIR)/icons/hicolor/scalable/apps
iconsscalable_DATA = scalable/aegisub.svg

aegisub.desktop: aegisub.desktop.template ../po
	intltool-merge --quiet --desktop-style ../po aegisub.desktop.template aegisub.desktop

all: ;

ICON_HICOLOR = \
	16x16/apps/aegisub.png \
	22x22/apps/aegisub.png \
	24x24/apps/aegisub.png \
	32x32/apps/aegisub.png \
	48x48/apps/aegisub.png \
	64x64/apps/aegisub.png

ICON_SCALABLE = \
	scalable/apps/aegisub.svg

DESKTOP_FILE = aegisub.desktop

EXTRA_DIST = \
	aegisub.desktop.template.in \
	aegisub.desktop.template \
	$(ICON_HICOLOR) \
	$(ICON_SCALABLE)

install: install-desktop install-icons

install-desktop: $(DESKTOP_FILE)
	@$(BIN_MKDIR) -p $(DESTDIR)$(P_DESKTOP)
	$(BIN_INSTALL) $(DESKTOP_FILE) $(DESTDIR)$(P_DESKTOP)/$(DESKTOP_FILE)


install-icons:
	@$(BIN_MKDIR) -p $(addprefix $(DESTDIR)$(P_ICON)/hicolor/, $(dir $(ICON_HICOLOR)))
	@for i in $(ICON_HICOLOR); do \
		$(BIN_ECHO) "$(BIN_INSTALL) -m644 $$i $(DESTDIR)$(P_ICON)/hicolor/$$i"; \
		$(BIN_INSTALL) -m644 $$i $(DESTDIR)$(P_ICON)/hicolor/$$i; \
	done
	$(BIN_TOUCH) $(DESTDIR)$(P_ICON)/hicolor
	$(BIN_MKDIR) -p $(DESTDIR)$(P_ICON)/hicolor/scalable/apps
	$(BIN_CP) $(ICON_SCALABLE) $(DESTDIR)$(P_ICON)/hicolor/$(ICON_SCALABLE)


DISTCLEANFILES = aegisub.desktop

include ../Makefile.target
