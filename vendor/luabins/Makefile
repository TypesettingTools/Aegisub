include ../../Makefile.inc

LIB = libluabins.a

CPPFLAGS += -I../lua/src

SRC = \
	src/fwrite.c \
	src/load.c \
	src/luabins.c \
	src/luainternals.c \
	src/save.c \
	src/savebuffer.c \
	src/write.c

HEADER = \
	src/fwrite.h \
	src/luabins.h \
	src/luaheaders.h \
	src/luainternals.h \
	src/savebuffer.h \
	src/saveload.h \
	src/write.h

EXTRA_DIST = \
	src/lualess.c \
	src/lualess.h \
	etc/benchmark.lua \
	etc/checkfmt.lua \
	etc/dataset.lua \
	etc/tolua.lua \
	etc/toluabins.lua \
	AUTHORS \
	COPYRIGHT \
	HISTORY \
	README.md \
	TODO

include ../../Makefile.target
-include src/*.d
