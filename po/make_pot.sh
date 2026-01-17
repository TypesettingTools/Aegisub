#!/bin/sh
set -e

maybe_append() {
  while read -r msg; do
    msgfile=$(printf '%s' "$msg" | cut -d'|' -f1)
    msgline=$(printf '%s' "$msg" | cut -d'|' -f2)
    msgid=$(printf '%s' "$msg" | cut -d'|' -f3-)

    if ! grep -Fq "msgid $msgid" aegisub.pot; then
      printf "\n#: %s:%s\nmsgid %s\nmsgstr \"\"\n\n" \
        "$msgfile" "$msgline" "$msgid" >> aegisub.pot
    fi
  done
}

find ../src ../src/command -name '*.cpp' -o -name '*.h' \
  | xgettext --files-from=- -o - --c++ --sort-by-file \
             -k_ -kwxTRANSLATE -kSTR_MENU -kSTR_DISP -kSTR_HELP -kCOMMAND_GROUP:5 \
             -kfmt_tl -kfmt_plural:2,3 \
  | sed 's/SOME DESCRIPTIVE TITLE./Aegisub 3.2/' \
  | sed 's/YEAR/2005-2014/' \
  | sed "s/THE PACKAGE'S COPYRIGHT HOLDER/Rodrigo Braz Monteiro, Niels Martin Hansen, Thomas Goyne et. al./" \
  | sed 's/PACKAGE/Aegisub/' \
  | sed 's/VERSION/3.2.0/' \
  | sed 's/FIRST AUTHOR <EMAIL@ADDRESS>/Niels Martin Hansen <nielsm@aegisub.org>/' \
  | sed 's/CHARSET/UTF-8/' \
  > aegisub.pot

for f in default_menu.json default_menu_platform.json osx/default_menu.json; do
    sed '/"text"/!d;s/^.*"text" : \("[^"]\+"\).*$/default_menu.json|0|\1/' ../src/libresrc/"$f" \
      | maybe_append
done

sed '/"text"/!d;s/^.*"text" : \("[^"]\+"\).*$/default_menu.json|0|\1/' ../src/libresrc/osx/default_menu.json \
  | maybe_append

grep '"[A-Za-z ]\+" : {' -n ../src/libresrc/default_hotkey.json \
  | sed 's/^\([0-9]\+:\).*\("[^"]\+"\).*$/default_hotkey.json|\1|\2/' \
  | maybe_append

find ../automation -name '*.lua' -o -name '*.moon' \
  | LC_ALL=C sort \
  | xargs grep 'tr"[^"]*"' -o -n \
  | sed 's/\(.*\):\([0-9]\+\):tr\(".*"\)/\1|\2|\3/' \
  | maybe_append

xgettext ../packages/desktop/aegisub.desktop.in.in \
  --language=Desktop --join-existing --omit-header -o aegisub.pot

xgettext ../packages/desktop/aegisub.metainfo.xml.in.in \
  --language=AppData --join-existing --omit-header -o aegisub.pot

grep '^_[A-Za-z0-9]*=.*' ../packages/win_installer/fragment_strings.iss.in | while read line
do
  printf '%s\n' "$line" \
    | sed 's/[^=]*=\(.*\)/packages\/win_installer\/fragment_strings.iss|1|"\1"/' \
    | maybe_append
done

for lang in $(cat LINGUAS) ; do
  # If using gettext < 0.21, run twice to avoid reversing order of old strings
  # ref: https://savannah.gnu.org/bugs/?58778
  msgmerge --update --backup=none --no-fuzzy-matching "$lang".po aegisub.pot
done
