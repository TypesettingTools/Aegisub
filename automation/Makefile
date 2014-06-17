include $(dir $(lastword $(MAKEFILE_LIST)))../header.mk

DATA_AUTOMATION := \
	$(wildcard $(d)autoload/*) \
	$(wildcard $(d)demos/*) \
	$(wildcard $(d)include/*.lua) \
	$(wildcard $(d)include/aegisub/*)

DATA_AUTOMATION_REL := $(subst $(d),,$(DATA_AUTOMATION))
DATA_AUTOMATION_INSTALLED = $(addprefix $(DESTDIR)$(P_DATA)/automation/, $(DATA_AUTOMATION_REL))

$(DESTDIR)$(P_DATA)/automation/%: $(d)%
	$(MKDIR_INSTALL)

aegisub-lua_OBJ := $(d)tests/aegisub.o $(TOP)lib/libaegisub.a $(TOP)lib/libluabins.a $(LIBS_LUA)
aegisub-lua_CPPFLAGS := $(CPPFLAGS_BOOST) $(CFLAGS_LUA) -I$(TOP)libaegisub/include -I$(TOP)src $(CXXFLAGS_WX)
aegisub-lua_LIBS := $(LIBS_WX) $(LIBS_BOOST) $(LIBS_ICU)

PROGRAM += $(d)aegisub-lua

test-automation: $(PROGRAM)
	cd $(TOP)automation; LUA=./aegisub-lua busted -p 'moon' tests/modules

test: test-automation

install: $(DATA_AUTOMATION_INSTALLED)

include $(TOP)Makefile.target
