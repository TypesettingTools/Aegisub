SRC += \
	libresrc/bitmap.cpp \
	libresrc/default_config.cpp \
	libresrc/libresrc.cpp

libresrc/bitmap.cpp libresrc/bitmap.h: ../tools/respack.lua bitmaps/button/*
	../tools/respack.lua bitmaps/manifest.respack libresrc/bitmap.cpp libresrc/bitmap.h

ifeq (yes, $(BUILD_DARWIN))
libresrc/default_config.cpp libresrc/default_config.h: ../tools/respack.lua libresrc/*.json libresrc/osx/*.json
	../tools/respack.lua libresrc/manifest_osx.respack libresrc/default_config.cpp libresrc/default_config.h
else
libresrc/default_config.cpp libresrc/default_config.h: ../tools/respack.lua libresrc/*.json
	../tools/respack.lua libresrc/manifest.respack libresrc/default_config.cpp libresrc/default_config.h
endif

CLEANFILES += \
	libresrc/bitmap.cpp \
	libresrc/bitmap.h \
	libresrc/default_config.cpp \
	libresrc/default_config.h

DISTCLEANFILES += default_config_platform.json

GLOBAL_DEPS += libresrc/bitmap.h libresrc/default_config.h

