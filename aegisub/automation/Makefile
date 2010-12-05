include ../Makefile.inc

# share/
DATA_AUTOMATION += \
	autoload/cleantags-autoload.lua \
	autoload/kara-templater.lua \
	autoload/karaoke-auto-leadin.lua \
	autoload/macro-1-edgeblur.lua \
	autoload/macro-2-mkfullwitdh.lua \
	include/cleantags.lua \
	include/karaskel-auto4.lua \
	include/karaskel.lua \
	include/unicode.lua \
	include/utils-auto4.lua \
	include/utils.lua


# share/doc
DATA_GENERAL_DOC = \
	demos/raytracer-test1.ass \
	v4-docs/basic-function-interface.txt \
	v4-docs/configuration-dialogs.txt \
	v4-docs/file-streams.txt \
	v4-docs/misc.txt \
	v4-docs/overview.txt \
	v4-docs/progress-reporting.txt \
	v4-docs/subtitle-data.txt \
	v4-docs/template-scripting-ideas.ass

DATA_AUTOMATION_DOC += \
	demos/future-windy-blur.lua \
	demos/raytracer.lua

all: ;


install: install-share
	@$(BIN_MKDIR) -p $(addprefix $(P_DATA)/automation/, $(dir $(DATA_AUTOMATION)))
	@for i in $(DATA_AUTOMATION); do \
		$(BIN_ECHO) "$(BIN_INSTALL) $$i $(P_DATA)/automation/$$i"; \
		$(BIN_INSTALL) $$i $(P_DATA)/automation/$$i; \
	done

DATA_SHARE_DOC	= $(DATA_GENERAL_DOC) $(DATA_AUTOMATION_DOC)

EXTRA_DIST = \
	$(DATA_AUTOMATION) \
	$(DATA_GENERAL_DOC) \
	$(DATA_AUTOMATION_DOC)

include ../Makefile.target
