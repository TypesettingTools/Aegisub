include ../../Makefile.inc

LIB = libluabins.a

CPPFLAGS += -I../luajit/include

SRC = \
	src/fwrite.c \
	src/load.c \
	src/luabins.c \
	src/luainternals.c \
	src/save.c \
	src/savebuffer.c \
	src/write.c

include ../../Makefile.target
-include src/*.d
