LANGS=de es fr it ko pt_BR ru
localedir = $(datadir)/locale

%.gmo: %/aegisub.po
	msgfmt -o $@ $<

all-am: $(foreach lang,$(LANGS),$(lang).gmo)

install-data-am: all-am
	for lang in $(LANGS); do \
	  $(install_sh_DATA) $${lang}.gmo $(DESTDIR)$(localedir)/$${lang}/LC_MESSAGES/aegisub.mo; \
	done

