# $Id$

bin_PROGRAMS = run

run_LDFLAGS= -L../libaegisub -laegisub-3.0 -lgtest @ICONV_LDFLAGS@ -L../universalchardet -luniversalchardet
run_CPPFLAGS= -I../src/include -I../libaegisub/include


run_SOURCES = \
		main.cpp \
		util.cpp \
		util_unix.cpp \
		libaegisub_access.cpp \
		libaegisub_cajun.cpp \
		libaegisub_iconv.cpp \
		libaegisub_line_iterator.cpp \
		libaegisub_mru.cpp \
		libaegisub_util.cpp \
		libaegisub_vfr.cpp
		
run_SOURCES += \
	*.h
