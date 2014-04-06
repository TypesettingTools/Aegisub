#!/bin/sh

for i in `cat LINGUAS`; do
  echo -n "$i	";
  msgfmt --verbose -o /dev/null $i 2>&1 \
    |awk '{ TOTAL= $1 + $4 + $7; PERCENT = ($1 / TOTAL) * 100; print "Total Strings: " TOTAL", " PERCENT"% Translated"}';
done
