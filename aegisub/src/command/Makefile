include ../../Makefile.inc

CXXFLAGS += -I../../ -I.. -I. -I../../libaegisub/include -I../include -DAEGISUB

CXXFLAGS += $(CFLAGS_WX)
PRECOMPILED_HEADER_NAME=../agi_pre.h


LIB = aegisub_command.a

SRC = \
	app.cpp \
	audio.cpp \
	automation.cpp \
	edit.cpp \
	grid.cpp \
	help.cpp \
	keyframe.cpp \
	medusa.cpp \
	menu.cpp \
	recent.cpp \
	subtitle.cpp \
	time.cpp \
	timecode.cpp \
	tool.cpp \
	video.cpp

SRC += \
	icon.cpp \
	command.cpp

HEADER = *.h

include ../../Makefile.target
