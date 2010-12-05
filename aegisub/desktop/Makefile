include ../Makefile.inc

applicationsdir = $(DESKTOP_DATADIR)/applications
applications_DATA = aegisub.desktop

iconsscalabledir = $(DESKTOP_DATADIR)/icons/hicolor/scalable/apps
iconsscalable_DATA = scalable/aegisub.svg


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

DESKTOP_FILE = \
	aegisub.desktop

EXTRA_DIST = \
	aegisub.desktop.in \
	$(ICON_HICOLOR) \
	$(ICON_SCALABLE)

install: install-desktop install-icons

install-desktop:
	@$(BIN_MKDIR) -p $(P_DESKTOP)
	$(BIN_INSTALL) $(DESKTOP_FILE) $(P_DESKTOP)/$(DESKTOP_FILE)


install-icons:
	@$(BIN_MKDIR) -p $(addprefix $(P_ICON)/hicolor/, $(dir $(ICON_HICOLOR)))
	@for i in $(ICON_HICOLOR); do \
		$(BIN_ECHO) "$(BIN_INSTALL) $$i $(P_ICON)/hicolor/$$i"; \
		$(BIN_INSTALL) $$i $(P_ICON)/hicolor/$$i; \
	done
	$(BIN_TOUCH) $(P_ICON)/hicolor
	$(BIN_MKDIR) -p $(P_ICON)/hicolor/scalable/apps
	$(BIN_CP) $(ICON_SCALABLE) $(P_ICON)/hicolor/$(ICON_SCALABLE)


CLEANFILES = aegisub.desktop

include ../Makefile.target
