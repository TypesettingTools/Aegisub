include ../../Makefile.inc

LIB = libluabins.a

CXXFLAGS += -I../lua/src -DLUABINS_LUABUILTASCPP

SRC = \
	src/fwrite.cpp \
	src/load.cpp \
	src/luabins.cpp \
	src/luainternals.cpp \
	src/save.cpp \
	src/savebuffer.cpp \
	src/write.cpp

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
