include ../Makefile.inc

.SUFFIXES: .po .mo

PO = \
	ca.po \
	cs.po \
	da.po \
	de.po \
	el.po \
	es.po \
	fi.po \
	fr_FR.po \
	hu.po \
	it.po \
	ja.po \
	ko.po \
	pt_BR.po \
	ru.po \
	vi.po \
	zh_CN.po \
	zh_TW.po

MO = $(addsuffix .mo, $(basename $(PO)))

all: $(MO)

%.mo: %.po
	$(BIN_MSGFMT) -o $@ $<

install:
	@$(BIN_MKDIR) -p $(addsuffix /LC_MESSAGES/, $(addprefix $(P_LOCALE)/, $(basename $(PO))))
	@for i in $(basename $(PO)); do \
		echo $(BIN_INSTALL) $$i.mo $(P_LOCALE)/$$i/LC_MESSAGES/$(GETTEXT_PACKAGE).mo; \
		$(BIN_INSTALL) $$i.mo $(P_LOCALE)/$$i/LC_MESSAGES/$(GETTEXT_PACKAGE).mo; \
	done


CLEANFILES = $(MO)

include ../Makefile.target
