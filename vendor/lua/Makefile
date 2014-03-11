include ../../Makefile.inc

LIB = liblua-aegisub.a

CXXFLAGS += -Wno-empty-body -Wno-deprecated-declarations

SRC = \
	src/lapi.cpp \
	src/lauxlib.cpp \
	src/lbaselib.cpp \
	src/lcode.cpp \
	src/ldblib.cpp \
	src/ldebug.cpp \
	src/ldo.cpp \
	src/ldump.cpp \
	src/lfunc.cpp \
	src/lgc.cpp \
	src/linit.cpp \
	src/liolib.cpp \
	src/llex.cpp \
	src/lmathlib.cpp \
	src/lmem.cpp \
	src/loadlib.cpp \
	src/lobject.cpp \
	src/lopcodes.cpp \
	src/loslib.cpp \
	src/lparser.cpp \
	src/lstate.cpp \
	src/lstring.cpp \
	src/lstrlib.cpp \
	src/ltable.cpp \
	src/ltablib.cpp \
	src/ltm.cpp \
	src/lua.cpp \
	src/luac.cpp \
	src/lundump.cpp \
	src/lvm.cpp \
	src/lzio.cpp \
	src/print.cpp

HEADER = src/*.h

include ../../Makefile.target
-include src/*.d

