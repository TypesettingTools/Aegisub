#!/bin/sh
BIN="aegisub-3.0"

if ! test -x ${BIN}; then
	echo "${BIN} does not exist";
	exit 1;
fi

case "$1" in
	"gdb")
		LD_LIBRARY_PATH="../libaegisub" exec gdb ./${BIN}
	;;
	*)
		LD_LIBRARY_PATH="../libaegisub" exec ./${BIN}
	;;
esac

