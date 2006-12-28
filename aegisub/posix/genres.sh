#!/bin/sh

RESFILE="$1"

egrep -v "^(#|/|$)" $RESFILE | cut -d ' ' -f 1 | (
	echo "#define static" >&4
	
	echo "#ifndef _RES_H" >&5
	echo "#define _RES_H" >&5
	while read NAME
	do	echo "extern char *${NAME}_xpm[];" >&5
		echo "#include \"../bitmaps/${NAME}_xpm.xpm\"" >&4
	done
	echo "#endif /* _RES_H */" >&5
) 4>res.cpp 5>res.h

