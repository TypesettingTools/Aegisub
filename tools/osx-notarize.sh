#!/bin/sh

set -e

SRC_DIR="${1}"
BUILD_DIR="${2}"
VERSION_OVERRIDE="${3:-}"

PKG_DIR="${BUILD_DIR}/Aegisub.app"
PKG_NAME="$("${SRC_DIR}/tools/osx-package-name.sh" "${PKG_DIR}" "${VERSION_OVERRIDE}")"
DMG_PATH="${BUILD_DIR}/${PKG_NAME}.dmg"
NOTARY_PROFILE="${AEGISUB_NOTARY_PROFILE:-}"
DEVELOPER_ID_REQUIREMENT='anchor apple generic and certificate leaf[field.1.2.840.113635.100.6.1.13] exists'

verify_timestamp() {
  if ! codesign --display --verbose=4 "${1}" 2>&1 | grep -q '^Timestamp='; then
    echo "\"${1}\" does not have a secure signing timestamp" >&2
    exit 1
  fi
}

verify_app() {
  codesign --verify --deep --strict --verbose=2 -R="${DEVELOPER_ID_REQUIREMENT}" "${1}"
  if ! codesign --display --verbose=4 "${1}" 2>&1 | grep -q '^CodeDirectory .*flags=.*runtime'; then
    echo "\"${1}\" does not have the hardened runtime enabled" >&2
    exit 1
  fi
  if codesign --display --xml --entitlements - "${1}" 2>/dev/null |
     grep -Fq '<key>com.apple.security.cs.disable-library-validation</key>'; then
    echo "\"${1}\" disables hardened-runtime library validation" >&2
    exit 1
  fi
  verify_timestamp "${1}"
}

verify_image() {
  codesign --verify --strict --verbose=2 -R="${DEVELOPER_ID_REQUIREMENT}" "${1}"
  verify_timestamp "${1}"
}

if test -z "${NOTARY_PROFILE}"; then
  echo "AEGISUB_NOTARY_PROFILE must name a notarytool Keychain profile" >&2
  exit 1
fi

if ! test -f "${DMG_PATH}"; then
  echo "\"${DMG_PATH}\" does not exist, please run 'meson compile osx-build-dmg'" >&2
  exit 1
fi

verify_app "${PKG_DIR}"
verify_image "${DMG_PATH}"

# Verify the exact app being submitted, rather than assuming the build-tree
# copy has not changed since the image was made.
VERIFY_MOUNT="$(mktemp -d "${TMPDIR:-/tmp}/aegisub-notary.XXXXXX")"
DMG_MOUNTED=false
cleanup() {
  if test "${DMG_MOUNTED}" = true; then
    hdiutil detach "${VERIFY_MOUNT}" >/dev/null 2>&1 || true
  fi
  rmdir "${VERIFY_MOUNT}" >/dev/null 2>&1 || true
}
trap cleanup EXIT HUP INT TERM

hdiutil attach -readonly -nobrowse -noautoopen -mountpoint "${VERIFY_MOUNT}" "${DMG_PATH}" >/dev/null
DMG_MOUNTED=true
verify_app "${VERIFY_MOUNT}/Aegisub.app"
hdiutil detach "${VERIFY_MOUNT}" >/dev/null
DMG_MOUNTED=false
rmdir "${VERIFY_MOUNT}"
trap - EXIT HUP INT TERM

echo
echo "---- Submitting image for notarization ----"
if test -n "${AEGISUB_NOTARY_KEYCHAIN:-}"; then
  xcrun notarytool submit "${DMG_PATH}" --keychain-profile "${NOTARY_PROFILE}" --keychain "${AEGISUB_NOTARY_KEYCHAIN}" --wait --timeout "${AEGISUB_NOTARY_TIMEOUT:-30m}"
else
  xcrun notarytool submit "${DMG_PATH}" --keychain-profile "${NOTARY_PROFILE}" --wait --timeout "${AEGISUB_NOTARY_TIMEOUT:-30m}"
fi

echo
echo "---- Stapling notarization ticket ----"
# Apple recommends notarizing and stapling only the outermost container when
# distributing nested software such as an app inside a disk image:
# https://developer.apple.com/documentation/xcode/packaging-mac-software-for-distribution
xcrun stapler staple "${DMG_PATH}"
xcrun stapler validate "${DMG_PATH}"
spctl --assess --type open --context context:primary-signature --verbose=2 "${DMG_PATH}"

echo
echo "Notarized \"${DMG_PATH}\""
