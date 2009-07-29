#!/bin/sh
#
# $Id$
#

if test -z "$1"; then
	export OUTPUT_DIR="./output"
else
	export OUTPUT_DIR="$1"
fi

mkdir -vp "${OUTPUT_DIR}"
cp -v css.css "${OUTPUT_DIR}"

SRC_PWD=`pwd|sed "s|/docs/doxygen||"`
export SRC_TRIM="${SRC_PWD}/src/"

doxygen doxyfile
