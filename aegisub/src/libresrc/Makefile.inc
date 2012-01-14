SRC += \
	libresrc/bitmap.cpp \
	libresrc/default_config.cpp \
	libresrc/libresrc.cpp

HEADER += \
	libresrc/libresrc.h

libresrc/bitmap.cpp libresrc/bitmap.h: ../tools/common-respack
	../tools/common-respack bitmaps/manifest.respack libresrc/bitmap.cpp libresrc/bitmap.h

libresrc/default_config.cpp libresrc/default_config.h: ../tools/common-respack libresrc/*.json
	../tools/common-respack libresrc/manifest.respack libresrc/default_config.cpp libresrc/default_config.h

CLEANFILES += \
	libresrc/bitmap.cpp \
	libresrc/bitmap.h \
	libresrc/default_config.cpp \
	libresrc/default_config.h

GLOBAL_DEPS = libresrc/bitmap.h libresrc/default_config.h
