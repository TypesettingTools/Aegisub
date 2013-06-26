include ../Makefile.inc

.SUFFIXES: .po .mo

PO = \
	ar.po \
	ca.po \
	cs.po \
	da.po \
	de.po \
	el.po \
	es.po \
	eu.po \
	fa.po \
	fi.po \
	fr_FR.po \
	hu.po \
	id.po \
	it.po \
	ja.po \
	ko.po \
	nl.po \
	pl.po \
	pt_BR.po \
	pt_PT.po \
	ru.po \
	sr_RS.po \
	sr_RS@latin.po \
	vi.po \
	zh_CN.po \
	zh_TW.po

MO = $(addsuffix .mo, $(basename $(PO)))

all: $(MO)

%.mo: %.po
	$(BIN_MSGFMT) -o $@ $<

install:
	@$(BIN_MKDIR) -p $(addsuffix /LC_MESSAGES/, $(addprefix $(DESTDIR)$(P_LOCALE)/, $(basename $(PO))))
	@for i in $(basename $(PO)); do \
		echo $(BIN_INSTALL) $$i.mo $(DESTDIR)$(P_LOCALE)/$$i/LC_MESSAGES/$(AEGISUB_CATALOG).mo; \
		$(BIN_INSTALL) $$i.mo $(DESTDIR)$(P_LOCALE)/$$i/LC_MESSAGES/$(AEGISUB_CATALOG).mo; \
	done


CLEANFILES = $(MO)

include ../Makefile.target
