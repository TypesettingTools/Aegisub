# $Id$

bin_PROGRAMS = run

run_LDFLAGS= -L../libaegisub -laegisub-2.2 -lgtest -rpath /mnt/free/aegisub/commit/libaegisub/.libs
run_CPPFLAGS= -I../src/include -I../libaegisub/include


run_SOURCES = \
		main.cpp \
		util.cpp \
		libaegisub_access.cpp \
		libaegisub_cajun.cpp \
		libaegisub_util.cpp \
		libaegisub_mru.cpp
		
run_SOURCES += \
	*.h
