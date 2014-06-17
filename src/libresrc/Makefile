include $(dir $(lastword $(MAKEFILE_LIST)))../../header.mk

LIB += resrc

resrc_CPPFLAGS := -I$(TOP) -I$(d) $(CPPFLAGS_WX)
resrc_OBJ := \
	$(d)bitmap.o \
	$(d)default_config.o \
	$(d)libresrc.o

$(resrc_OBJ): $(d)default_config.h $(d)bitmap.h

RESPACK := cd $(TOP)src/libresrc; $(TOP)vendor/luajit/src/host/minilua $(TOP)tools/respack.lua

$(d)bitmap.cpp $(d)bitmap.h: $(TOP)tools/respack.lua $(d)../bitmaps/button
	$(RESPACK) ../bitmaps/manifest.respack bitmap.cpp bitmap.h

ifeq (yes, $(BUILD_DARWIN))
$(d)default_config.cpp $(d)default_config.h: $(TOP)tools/respack.lua $(d)*.json $(d)osx/*.json
	$(RESPACK) manifest_osx.respack default_config.cpp default_config.h
else
$(d)default_config.cpp $(d)default_config.h: $(TOP)tools/respack.lua $(d)*.json
	$(RESPACK) manifest.respack default_config.cpp default_config.h
endif

CLEANFILES += \
	$(d)bitmap.cpp \
	$(d)bitmap.h \
	$(d)default_config.cpp \
	$(d)default_config.h

DISTCLEANFILES += $(d)default_config_platform.json

include $(TOP)Makefile.target
