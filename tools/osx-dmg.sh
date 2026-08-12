#!/bin/sh
# USAGE: osx-dmg.sh [Source Directory] [Build Directory] [Version Override]
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

set -e

SRC_DIR="${1}"
BUILD_DIR="${2}"

PKG_DIR="${BUILD_DIR}/Aegisub.app"

if ! test -d "${PKG_DIR}"; then
  echo "\"${PKG_DIR}\" does not exist, please run 'meson compile osx-bundle'"
  exit 1;
fi

if test -z "${AEGISUB_BUNDLE_SIGNATURE+x}" || test -z "${AEGISUB_BUNDLE_SIGNATURE}"; then
  echo "AEGISUB_BUNDLE_SIGNATURE must be set to a Developer ID identity or '-' for an unsigned image" >&2
  exit 1
fi

SIGN_IDENTITY="${AEGISUB_BUNDLE_SIGNATURE}"
SIGN_KEYCHAIN="${AEGISUB_SIGNING_KEYCHAIN:-}"

PKG_NAME="$("${SRC_DIR}/tools/osx-package-name.sh" "${PKG_DIR}" "${3:-}")"
PKG_NAME_VOLUME="${PKG_NAME}"

DMG_TMP_DIR="${BUILD_DIR}/temp_dmg"
DMG_PATH="${BUILD_DIR}/${PKG_NAME}.dmg"
DMG_RW_PATH="${BUILD_DIR}/${PKG_NAME}_rw.dmg"

echo
echo "---- Removing old \"${DMG_TMP_DIR}\", \"${DMG_PATH}\", \"${DMG_RW_PATH}\" ----"
rm -rf "${DMG_TMP_DIR}" "${DMG_PATH}" "${DMG_RW_PATH}"
mkdir -v "${DMG_TMP_DIR}"
echo
echo "---- Copying ${PKG_NAME} into ${DMG_TMP_DIR}/ ----"
cp -R "${PKG_DIR}" "${DMG_TMP_DIR}"

echo
echo "---- Setting up ----"
ln -vsf /Applications "${DMG_TMP_DIR}"
mkdir -v "${DMG_TMP_DIR}/.background"
cp -v "${SRC_DIR}/packages/osx_dmg/dmg_background.png" "${DMG_TMP_DIR}/.background/background.png"
cp -v "${SRC_DIR}/packages/osx_bundle/Contents/Resources/Aegisub.icns" "${DMG_TMP_DIR}/.VolumeIcon.icns"

echo
echo "---- Creating image ----"
/usr/bin/hdiutil create -srcfolder "${DMG_TMP_DIR}" -volname "${PKG_NAME}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW "${DMG_RW_PATH}"

echo
echo "---- Mounting image ----"
DEV_NAME="$(/usr/bin/hdiutil attach -readwrite -noverify -noautoopen "${DMG_RW_PATH}" | awk '/GUID_partition_scheme/ {print $1}')"
echo "Device name: ${DEV_NAME}"

echo
echo "---- Setting root icon using SetFile ----"
SetFile -a C "/Volumes/${PKG_NAME_VOLUME}"

echo
if test -n "${SET_STYLE}"; then
  echo "---- Running AppleScript to set style ----"
  SCRIPT_TMP="$(mktemp /tmp/aegisub_dmg_as.XXX)"

  sed -f "${SRC_DIR}/scripts/osx-bundle.sed" "${SRC_DIR}/packages/osx_dmg/dmg_set_style.applescript" > "${SCRIPT_TMP}"

  /usr/bin/osacompile -o "${SCRIPT_TMP}.scpt" "${SCRIPT_TMP}"

  /usr/bin/osascript "${SCRIPT_TMP}.scpt"
  open "/Volumes/${PKG_NAME_VOLUME}"

  echo "********************************************************"
  echo "Please move the window to the center of the screen then"
  echo "close it."
  echo "********************************************************"
  echo
  echo "PRESS ENTER WHEN DONE"
  open "/Volumes/${PKG_NAME_VOLUME}"
  read -r _

  hdiutil detach "${DEV_NAME}"

  DEV_NAME="$(/usr/bin/hdiutil attach -readwrite -noverify -noautoopen "${DMG_RW_PATH}" | awk '/GUID_partition_scheme/ {print $1}')"
  echo "Device name: ${DEV_NAME}"

  cp -v "/Volumes/${PKG_NAME_VOLUME}/.DS_Store" "${SRC_DIR}/packages/osx_dmg/DS_Store"
  SetFile -a v "${SRC_DIR}/packages/osx_dmg/DS_Store"
  hdiutil detach "${DEV_NAME}"

  rm -rf "${DMG_TMP_DIR}" "${DMG_RW_PATH}" "${SCRIPT_TMP}.scpt" "${SCRIPT_TMP}"
  exit 0
else
  echo "---- Installing DS_Store ----"
  cp -v "${SRC_DIR}/packages/osx_dmg/DS_Store" "/Volumes/${PKG_NAME_VOLUME}/.DS_Store"
fi

echo
echo "---- Detaching ----"
echo /usr/bin/hdiutil detach "${DEV_NAME}" -force
/usr/bin/hdiutil detach "${DEV_NAME}" -force

echo
echo "---- Compressing ----"
/usr/bin/hdiutil convert "${DMG_RW_PATH}" -format UDBZ -imagekey bzip2-level=9 -o "${DMG_PATH}"

echo "---- Removing temp dmg \"${DMG_RW_PATH}\" ----"
rm -rf "${DMG_RW_PATH}"

if test "${SIGN_IDENTITY}" != "-"; then
  echo
  echo "---- Signing image ----"
  if test -n "${SIGN_KEYCHAIN}"; then
    codesign --force --timestamp --keychain "${SIGN_KEYCHAIN}" --sign "${SIGN_IDENTITY}" "${DMG_PATH}"
  else
    codesign --force --timestamp --sign "${SIGN_IDENTITY}" "${DMG_PATH}"
  fi
  codesign --verify --strict --verbose=2 "${DMG_PATH}"
fi

echo
echo "---- Verifying image ----"
/usr/bin/hdiutil verify "${DMG_PATH}"

echo
echo "Done creating \"${DMG_PATH}\""
