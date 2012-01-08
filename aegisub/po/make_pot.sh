#!/bin/sh

find ../src ../src/command -name \*.cpp -o -name \*.h | \
    xgettext --files-from=- -o - --c++ -k_ -kSTR_MENU -kSTR_DISP -kSTR_HELP -kwxT | \
    sed 's/SOME DESCRIPTIVE TITLE/Aegisub 3.0.0/' | \
    sed 's/YEAR/2005-2012/' | \
    sed "s/THE PACKAGE'S COPYRIGHT HOLDER/Rodrigo Braz Monteiro, Niels Martin Hansen, Thomas Goyne et. al./" | \
    sed 's/PACKAGE/Aegisub 3.0.0/' > \
    aegisub.pot

sed '/"text"/!d;s/^.*"text" : \("[^"]\+"\).*$/\n#: default_menu.json\nmsgid \1\nmsgstr ""\n/' ../src/libresrc/default_menu.json >> aegisub.pot
