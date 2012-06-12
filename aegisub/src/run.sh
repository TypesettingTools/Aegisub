#!/bin/sh
BINDIR="$(dirname "$0")"
BIN="$BINDIR/aegisub-3.0"
LIBDIR="$BINDIR/../libaegisub"

if ! test -x "${BIN}"; then
	echo "${BIN} does not exist or is not executable.";
	exit 1;
fi

case "$1" in
	"gdb")
		GDB="gdb"
		;;
esac

LD_LIBRARY_PATH="$LIBDIR" exec $GDB "./$BIN"
