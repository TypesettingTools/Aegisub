#!/bin/sh

RESFILE="$1"
SRCDIR="$2"

cd "$SRCDIR"

cat <<EOF
all: bmp2xpm wxicon_xpm.xpm

.PHONY: all bmp2xpm

srcdir=${SRCDIR}

wxicon_xpm.xpm: \$(srcdir)/icon.ico
	convert \$(srcdir)/'icon.ico[2]' wxicon_xpm.xpm

EOF

XPMNAMES=""

for I in *.bmp
do
	DNAME="`grep "bitmaps/$I" $RESFILE | cut -d ' ' -f 1`"
	echo -e "${DNAME}_xpm.xpm: \$(srcdir)/$I
\tconvert -transparent \\#c0c0c0 \$(srcdir)/$I ${DNAME}_xpm.xpm
"
	XPMNAMES="${XPMNAMES} ${DNAME}_xpm.xpm"
done

echo "bmp2xpm: $XPMNAMES"

