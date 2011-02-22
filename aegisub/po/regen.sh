#!/bin/sh -x
# $Id$

find ../src -type f|sed "s|^../src/||" |egrep "\\.h$|\\.cpp$" |sort > files
xgettext --files-from files  --directory=../src --output aegisub.pot --c++ -k_
rm files
for i in `cat LINGUAS`; do msgmerge -N $i.po aegisub.pot > tmp; mv tmp $i.po; done
