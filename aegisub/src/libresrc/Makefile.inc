SRC += \
	libresrc/bitmap.cpp \
	libresrc/default_config.cpp \
	libresrc/libresrc.cpp

HEADER += \
	libresrc/libresrc.h

libresrc/bitmap.cpp libresrc/bitmap.h: ../tools/common-respack bitmaps/button/*
	../tools/common-respack bitmaps/manifest.respack libresrc/bitmap.cpp libresrc/bitmap.h

ifeq (yes, $(BUILD_DARWIN))
libresrc/default_config.cpp libresrc/default_config.h: ../tools/common-respack libresrc/*.json libresrc/osx/*.json
	../tools/common-respack libresrc/manifest_osx.respack libresrc/default_config.cpp libresrc/default_config.h
else
libresrc/default_config.cpp libresrc/default_config.h: ../tools/common-respack libresrc/*.json
	../tools/common-respack libresrc/manifest.respack libresrc/default_config.cpp libresrc/default_config.h
endif

CLEANFILES += \
	libresrc/bitmap.cpp \
	libresrc/bitmap.h \
	libresrc/default_config.cpp \
	libresrc/default_config.h

DISTCLEANFILES += default_config_platform.json

GLOBAL_DEPS += libresrc/bitmap.h libresrc/default_config.h

EXTRA_DIST += \
	libresrc/*.json \
	libresrc/*.respack
