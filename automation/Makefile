include ../Makefile.inc

PRECOMPILED_HEADER_NAME = ../libaegisub/lagi_pre.h

PROGRAM = aegisub-lua

CXXFLAGS += -I../libaegisub/include -I../src -I ../vendor/lua/src $(CXXFLAGS_WX)
CPPFLAGS += $(CPPFLAGS_BOOST)

LIBS := -L../libaegisub -laegisub  -L../vendor/lua -llua-aegisub $(LIBS)
LIBS += ../vendor/luabins/libluabins.a
LIBS += $(LIBS_WX) $(LIBS_BOOST) $(LIBS_ICU)

SRC += tests/aegisub.cpp

test: $(PROGRAM)
	LUA=./aegisub-lua busted -p 'moon' tests/modules

.PHONY: test

# share/
DATA_AUTOMATION += \
	autoload/clean-info.lua \
	autoload/cleantags-autoload.lua \
	autoload/kara-templater.lua \
	autoload/karaoke-auto-leadin.lua \
	autoload/macro-1-edgeblur.lua \
	autoload/macro-2-mkfullwitdh.lua \
	autoload/select-overlaps.moon \
	autoload/strip-tags.lua \
	demos/future-windy-blur.lua \
	demos/raytracer.lua \
	include/aegisub/clipboard.lua \
	include/aegisub/re.moon \
	include/aegisub/unicode.moon \
	include/aegisub/util.moon \
	include/cleantags.lua \
	include/clipboard.lua \
	include/karaskel-auto4.lua \
	include/karaskel.lua \
	include/moonscript.lua \
	include/re.lua \
	include/unicode.lua \
	include/utils-auto4.lua \
	include/utils.lua

all: ;

install: install-share
	@$(BIN_MKDIR) -p $(addprefix $(DESTDIR)$(P_DATA)/automation/, $(dir $(DATA_AUTOMATION)))
	@for i in $(DATA_AUTOMATION); do \
		$(BIN_ECHO) "$(BIN_INSTALL) -m644 $$i $(DESTDIR)$(P_DATA)/automation/$$i"; \
		$(BIN_INSTALL) -m644 $$i $(DESTDIR)$(P_DATA)/automation/$$i; \
	done

EXTRA_DIST = $(DATA_AUTOMATION)

include ../Makefile.target
-include tests/*.d
