#!/bin/sh
# USAGE: osx-dmg.sh [Bundle Directory] "[Package Name]"
#
# Amar Takhar <verm@aegisub.org>
#
# -------------------------------------------------------------------------------------
# This script is based on osx-dmg.sh from the Inkscape Project http://www.inkscape.org/
#
# Jean-Olivier Irisson <jo.irisson@gmail.com>
# Michael Wybrow <mjwybrow@users.sourceforge.net>
#
# Copyright (C) 2006-2010
# Released under GNU GPL, read the file 'COPYING' from the Inkscape project for more
# information.

TMP_DMG="temp_dmg"
PKG_DIR="${1}.app"
PKG_NAME="${2}"
PKG_NAME_RW="${1}_rw.dmg"
PKG_NAME_VOLUME="${2}"

if ! test -d "${PKG_DIR}"; then
  echo "\"${PKG_DIR}\" does not exist, please run 'make osx-bundle'"
  exit 1;
fi

rm -rf "${TMP_DMG}" "${PKG_NAME}.dmg"
mkdir -v "${TMP_DMG}"
echo
echo "---- Copying ${1} into ${TMP_DMG}/ ----"
cp -R "${PKG_DIR}" "${TMP_DMG}"

echo
echo "---- Setting up ----"
ln -vsf /Applications "${TMP_DMG}"
mkdir -v "${TMP_DMG}/.background"
cp -v packages/osx_dmg/dmg_background.png "${TMP_DMG}/.background/background.png"
cp -v packages/osx_bundle/Contents/Resources/Aegisub.icns "${TMP_DMG}/.VolumeIcon.icns"

echo
echo "---- Creating image ----"
/usr/bin/hdiutil create -srcfolder "${TMP_DMG}" -volname "${PKG_NAME}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW "${PKG_NAME_RW}" || exit $?

echo
echo "---- Mounting image ----"
DEV_NAME=`/usr/bin/hdiutil attach -readwrite -noverify -noautoopen "${PKG_NAME_RW}" |awk '/Apple_partition_scheme/ {print $1}'` || exit $?
echo "Device name: ${DEV_NAME}"

echo
echo "---- Setting bless -openfolder ----"
bless -openfolder "/Volumes/${PKG_NAME_VOLUME}" || exit $?

echo
echo "---- Setting root icon using SetFile ----"
SetFile -a C "/Volumes/${PKG_NAME_VOLUME}" || exit $?

echo
if test -n "${SET_STYLE}"; then
  echo "---- Running AppleScript to set style ----"
  SCRIPT_TMP=`mktemp /tmp/aegisub_dmg_as.XXX`

  sed -f scripts/osx-bundle.sed packages/osx_dmg/dmg_set_style.applescript > ${SCRIPT_TMP}

  /usr/bin/osacompile -o ${SCRIPT_TMP}.scpt ${SCRIPT_TMP}

  /usr/bin/osascript ${SCRIPT_TMP}.scpt
  open "/Volumes/${PKG_NAME_VOLUME}"

  echo "********************************************************"
  echo "Please move the window to the center of the screen then"
  echo "close it."
  echo "********************************************************"
  echo
  echo "PRESS ENTER WHEN DONE"
  open "/Volumes/${PKG_NAME_VOLUME}"
  read -e DUMB

  hdiutil detach "${DEV_NAME}"

  DEV_NAME=`/usr/bin/hdiutil attach -readwrite -noverify -noautoopen "${PKG_NAME_RW}" |awk '/Apple_partition_scheme/ {print $1}'` || exit $?
  echo "Device name: ${DEV_NAME}"

  cp -v "/Volumes/${PKG_NAME_VOLUME}/.DS_Store" packages/osx_dmg/DS_Store
  SetFile -a v packages/osx_dmg/DS_Store
  hdiutil detach "${DEV_NAME}"

  rm -rf "${TMP_DMG}"  "${PKG_NAME_RW}" ${SCRIPT_TMP}.scpt ${SCRIPT_TMP}
  exit 0
else
  echo "---- Installing DS_Store ----"
  cp -v packages/osx_dmg/DS_Store "/Volumes/${PKG_NAME_VOLUME}/.DS_Store"
fi

echo
echo "---- Detaching ----"
/usr/bin/hdiutil detach "${DEV_NAME}" -force || exit $?

echo
echo "---- Compressing ----"
/usr/bin/hdiutil convert "${PKG_NAME_RW}" -format UDZO -imagekey zlib-level=9 -o "${PKG_NAME}.dmg" || exit $?

echo
echo "---- Removing \"${TMP_DMG}\", \"${PKG_NAME_RW}\" ----"
rm -rf "${TMP_DMG}"  "${PKG_NAME_RW}" || exit $?

echo
echo "Done!"
