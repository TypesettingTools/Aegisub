include $(dir $(lastword $(MAKEFILE_LIST)))../header.mk

.SUFFIXES: .po .mo

PO := $(wildcard $(d)*.po)
MO := $(PO:po=mo)

ifeq (yes, $(BUILD_DARWIN))
MO_INSTALLED = $(DESTDIR)$(P_LOCALE)/%.lproj/$(AEGISUB_CATALOG).mo
else
MO_INSTALLED = $(DESTDIR)$(P_LOCALE)/%/LC_MESSAGES/$(AEGISUB_CATALOG).mo
endif

%.mo: %.po
	$(BIN_MSGFMT) -o $@ $<

$(MO_INSTALLED): $(d)%.mo	; $(MKDIR_INSTALL)

all: $(MO)
install: $(patsubst %.mo, $(MO_INSTALLED), $(notdir $(MO)))

CLEANFILES += $(MO)

include $(TOP)Makefile.target
