include $(dir $(lastword $(MAKEFILE_LIST)))../../header.mk
DESKTOP_SRC := $(d)

ICONS = $(sort $(wildcard $(d)*.png)) $(sort $(wildcard $(d)*.svg))
ICONS_INSTALLED = $(DESTDIR)$(P_ICON)/hicolor/%/apps/aegisub.

DESKTOP_FILE := $(d)aegisub.desktop
DESKTOP_FILE_PO := $(d)../../po
DESKTOP_FILE_INSTALLED = $(DESTDIR)$(P_DESKTOP)/$(notdir $(DESKTOP_FILE))

DISTCLEANFILES += $(DESKTOP_FILE)

APPDATA_FILE := $(d)aegisub.appdata.xml
APPDATA_FILE_PO := $(d)../../po
APPDATA_FILE_INSTALLED = $(DESTDIR)$(P_APPDATA)/$(notdir $(APPDATA_FILE))

DISTCLEANFILES += $(APPDATA_FILE)

%.desktop: %.desktop.template $(DESKTOP_FILE_PO)
	intltool-merge --quiet --desktop-style $(DESKTOP_FILE_PO) $< $@

%.appdata.xml: %.appdata.xml.template $(APPDATA_FILE_PO)
	intltool-merge --quiet --xml-style $(APPDATA_FILE_PO) $< $@

$(ICONS_INSTALLED)png: $(d)%.png	; $(MKDIR_INSTALL)
$(ICONS_INSTALLED)svg: $(d)%.svg	; $(MKDIR_INSTALL)
$(DESKTOP_FILE_INSTALLED): $(DESKTOP_FILE)	; $(MKDIR_INSTALL)
$(APPDATA_FILE_INSTALLED): $(APPDATA_FILE)	; $(MKDIR_INSTALL)

ifneq (yes, $(BUILD_DARWIN))
install: \
	$(APPDATA_FILE_INSTALLED) \
	$(DESKTOP_FILE_INSTALLED) \
	$(patsubst %.png, $(ICONS_INSTALLED)png, $(patsubst %.svg, $(ICONS_INSTALLED)svg, $(notdir $(ICONS))))
endif

include $(TOP)Makefile.target
