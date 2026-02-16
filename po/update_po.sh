#!/bin/sh
set -e

# Use as './update_po.sh de ja' to only update some po files,
# or as `./update_po.sh` to update all of them.

for lang in ${@:-$(cat LINGUAS)} ; do
  # If using gettext < 0.21, run twice to avoid reversing order of old strings
  # ref: https://savannah.gnu.org/bugs/?58778
  msgmerge --update --backup=none --no-fuzzy-matching "$lang".po aegisub.pot
done
