#!/bin/sh
SRCDIR=`pwd`
MAKE=$1
DISTDIR=$2

rm -rf $DISTDIR

if ! test -d src; then
	echo "Please run this from the parent directory.";
	exit 1;
fi

$MAKE distfiles \
	| grep -E ^/ \
	| sed "s|${SRCDIR}/||" \
	| xargs -I {} ./install-sh -m 0644 "{}" "$DISTDIR/{}"

for i in `find . -name Makefile -or -name wscript`; do
	./install-sh -m 0644 "$i" "${DISTDIR}/$i"
done

chmod +x ${DISTDIR}/configure
