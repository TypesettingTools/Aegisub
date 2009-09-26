#!/bin/sh

if test -z "$1"; then
	echo "You must provide a project name."
	exit;
fi

# Remove from any previous runs
rm -f docs/doxygen/doxygen.log

# Help buildbot find the doxygen.log
ln -s docs/doxygen/doxygen.log

case "$1" in
	"aegisub")
		OUTPUT="source"
    ;;
    "reporter")
		OUTPUT="reporter"
    ;;
esac

cd docs/doxygen
sh -x ./gen.sh $1 /usr/www/docs.aegisub.org/${OUTPUT}

chmod 0644 /usr/www/docs.aegisub.org/${OUTPUT}/*
