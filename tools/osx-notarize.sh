#!/bin/sh

set -e

SRC_DIR="${1}"
BUILD_DIR="${2}"
VERSION_OVERRIDE="${3:-}"

PKG_DIR="${BUILD_DIR}/Aegisub.app"
PKG_NAME="$("${SRC_DIR}/tools/osx-package-name.sh" "${PKG_DIR}" "${VERSION_OVERRIDE}")"
DMG_PATH="${BUILD_DIR}/${PKG_NAME}.dmg"
NOTARY_PROFILE="${AEGISUB_NOTARY_PROFILE:-}"

if test -z "${NOTARY_PROFILE}"; then
  echo "AEGISUB_NOTARY_PROFILE must name a notarytool Keychain profile"
  exit 1
fi

if ! test -f "${DMG_PATH}"; then
  echo "\"${DMG_PATH}\" does not exist, please run 'meson compile osx-build-dmg'"
  exit 1
fi

codesign --verify --deep --strict --verbose=2 "${PKG_DIR}"
codesign --verify --strict --verbose=2 "${DMG_PATH}"

echo
echo "---- Submitting image for notarization ----"
if test -n "${AEGISUB_NOTARY_KEYCHAIN:-}"; then
  xcrun notarytool submit "${DMG_PATH}" --keychain-profile "${NOTARY_PROFILE}" --keychain "${AEGISUB_NOTARY_KEYCHAIN}" --wait --timeout "${AEGISUB_NOTARY_TIMEOUT:-30m}"
else
  xcrun notarytool submit "${DMG_PATH}" --keychain-profile "${NOTARY_PROFILE}" --wait --timeout "${AEGISUB_NOTARY_TIMEOUT:-30m}"
fi

echo
echo "---- Stapling notarization ticket ----"
xcrun stapler staple "${DMG_PATH}"
xcrun stapler validate "${DMG_PATH}"
spctl --assess --type open --context context:primary-signature --verbose=2 "${DMG_PATH}"

echo
echo "Notarized \"${DMG_PATH}\""
