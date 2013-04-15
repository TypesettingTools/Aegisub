include ../Makefile.inc

LIB_SHARED = aegisub.so
LIB_VERSION = 0.1.0

CXXFLAGS += -I../libaegisub/include -I../src $(CXXFLAGS_WX)
CPPFLAGS += $(CPPFLAGS_BOOST)

LIBS := -L../libaegisub -laegisub $(LIBS)
LIBS += $(LIBS_WX) $(LIBS_BOOST) $(LIBS_ICU) $(LIBS_LUA)

SRC += tests/aegisub.cpp ../src/auto4_regex.cpp

test: $(LIB_SHARED)
	moon tests/runner.moon

.PHONY: test

# share/
DATA_AUTOMATION += \
	autoload/clean-info.lua \
	autoload/cleantags-autoload.lua \
	autoload/kara-templater.lua \
	autoload/karaoke-auto-leadin.lua \
	autoload/macro-1-edgeblur.lua \
	autoload/macro-2-mkfullwitdh.lua \
	autoload/select-overlaps.lua \
	autoload/strip-tags.lua \
	include/cleantags.lua \
	include/clipboard.lua \
	include/karaskel-auto4.lua \
	include/karaskel.lua \
	include/re.lua \
	include/unicode.lua \
	include/utils-auto4.lua \
	include/utils.lua

all: ;

install: install-share
	@$(BIN_MKDIR) -p $(addprefix $(DESTDIR)$(P_DATA)/automation/, $(dir $(DATA_AUTOMATION)))
	@for i in $(DATA_AUTOMATION); do \
		$(BIN_ECHO) "$(BIN_INSTALL) $$i $(DESTDIR)$(P_DATA)/automation/$$i"; \
		$(BIN_INSTALL) $$i $(DESTDIR)$(P_DATA)/automation/$$i; \
	done

EXTRA_DIST = $(DATA_AUTOMATION)

include ../Makefile.target
