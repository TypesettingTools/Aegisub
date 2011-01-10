#!/bin/sh
SRCDIR=`pwd`
TMPFILE=`mktemp /tmp/aegisub_dist.XXXXXX`
EXTRA=`find . -name Makefile -or -name wscript`
rm -rf aegisub-pkg

if ! test -d src; then
	echo "Please run this from the parent directory.";
	exit 1;
fi

gmake distfiles \
	|egrep ^/ |sed "s|${SRCDIR}/||" \
	|awk '{print "echo \"aegisub-pkg/"$0"\"\n./install-sh -m 0644 \""$0"\" \"aegisub-pkg/"$0"\""}' \
	> ${TMPFILE}

for i in ${EXTRA}; do
	echo "aegisub-pkg/$i";
	./install-sh -m 0644 $i aegisub-pkg/$i;
done

sh ${TMPFILE}

#rm ${TMPFILE}
