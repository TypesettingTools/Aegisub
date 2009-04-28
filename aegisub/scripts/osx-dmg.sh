#!/bin/sh
# USAGE
# osx-dmg.sh [Bundle Directory] "[Package Name]"
#
# This script is based on osx-dmg.sh from the Inkscape Project http://www.inkscape.org/
#
# Jean-Olivier Irisson <jo.irisson@gmail.com>
# Michael Wybrow <mjwybrow@users.sourceforge.net>
#
# Copyright (C) 2006-2007
# Released under GNU GPL, read the file 'COPYING' for more information

TMP_DMG="temp_dmg"
PKG_DIR="${1}.app"
PKG_NAME="${2}"
PKG_NAME_RW="${1}_rw.dmg"
PKG_NAME_VOLUME="${2}"

if ! test -d "${PKG_DIR}"; then
  echo "${PKG_DIR} does not exist, please run 'make osx-bundle'"
  exit 1;
fi

if ! perl -e 'require Mac::Finder::DSStore' > /dev/null 2>&1; then
  echo
  echo "Perl Mac::Finder::DSStore is required to build a dmg."
  echo "Please get it from http://freehg.org/u/wiml/dsstore/"
  exit 1;
fi

rm -rf ${TMP_DMG} "${PKG_NAME}.dmg"
mkdir -v ${TMP_DMG}
echo
echo "---- Copying ${1} into ${TMP_DMG}/ ----"
cp -R ${PKG_DIR} ${TMP_DMG}

echo
echo "---- Setting up ----"
ln -vsf /Applications "${TMP_DMG}"
mkdir -v ${TMP_DMG}/.background
cp -v packages/osx_dmg/dmg_background.png ${TMP_DMG}/.background/background.png
cp -v packages/osx_bundle/Contents/Resources/Aegisub.icns ${TMP_DMG}/.VolumeIcon.icns

echo
echo "---- Creating image ----"
/usr/bin/hdiutil create -srcfolder "${TMP_DMG}" -volname "${PKG_NAME}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW "${PKG_NAME_RW}"

echo
echo "---- Mounting image ----"
DEV_NAME=`/usr/bin/hdiutil attach -readwrite -noverify -noautoopen "${PKG_NAME_RW}" |awk '/Apple_partition_scheme/ {print $1}'`
echo "Device name: ${DEV_NAME}"

echo
echo "---- Setting bless -openfolder ----"
bless -openfolder "/Volumes/${PKG_NAME_VOLUME}"

echo
echo "---- Setting root icon using SetFile ----"
/usr/bin/SetFile -a C "/Volumes/${PKG_NAME_VOLUME}"

echo
echo "--- Generating /Volumes/${PKG_NAME_VOLUME}/.DS_Store ----"
/usr/bin/perl scripts/osx-dmg-dsstore.pl "/Volumes/${PKG_NAME_VOLUME}/.DS_Store" "${PKG_DIR}" "/Volumes/${PKG_NAME_VOLUME}/.background/background.png"

echo
echo "---- Detaching ----"
/usr/bin/hdiutil detach "${DEV_NAME}"

echo
echo "---- Compressing ----"
/usr/bin/hdiutil convert "${PKG_NAME_RW}" -format UDZO -imagekey zlib-level=9 -o "${PKG_NAME}.dmg"

echo
echo "---- Removing ${TMP_DMG}, ${PKG_NAME_RW} ----"
rm -rf ${TMP_DMG}  ${PKG_NAME_RW}

echo
echo "Done!"
