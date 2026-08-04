#!/bin/sh

set -e

SRC_DIR="${1}"
PKG_DIR="${2}"

if ! test -d "${PKG_DIR}"; then
  echo "\"${PKG_DIR}\" does not exist" >&2
  exit 1
fi

SIGN_IDENTITY="${AEGISUB_BUNDLE_SIGNATURE:--}"
SIGN_KEYCHAIN="${AEGISUB_SIGNING_KEYCHAIN:-}"
ENTITLEMENTS="${AEGISUB_BUNDLE_ENTITLEMENTS:-${SRC_DIR}/packages/osx_bundle/aegisub.entitlements}"

sign_file() {
  if test "${SIGN_IDENTITY}" = "-"; then
    codesign --force --sign - "${1}"
  elif test -n "${SIGN_KEYCHAIN}"; then
    codesign --force --options runtime --timestamp --keychain "${SIGN_KEYCHAIN}" --sign "${SIGN_IDENTITY}" "${1}"
  else
    codesign --force --options runtime --timestamp --sign "${SIGN_IDENTITY}" "${1}"
  fi
}

echo
echo "---- Signing app bundle ----"

# Sign each real Mach-O file once. Library aliases are symlinks to these files
# and do not need (or want) their own signatures.
find "${PKG_DIR}/Contents/MacOS" -type f -print | while IFS= read -r fname; do
  case "$(file -b "${fname}")" in
    Mach-O*) sign_file "${fname}" ;;
  esac
done

if test "${SIGN_IDENTITY}" = "-"; then
  codesign --force --sign - "${PKG_DIR}"
elif test -n "${SIGN_KEYCHAIN}"; then
  codesign --force --options runtime --timestamp --keychain "${SIGN_KEYCHAIN}" --entitlements "${ENTITLEMENTS}" --sign "${SIGN_IDENTITY}" "${PKG_DIR}"
else
  codesign --force --options runtime --timestamp --entitlements "${ENTITLEMENTS}" --sign "${SIGN_IDENTITY}" "${PKG_DIR}"
fi

codesign --verify --deep --strict --verbose=2 "${PKG_DIR}"

if test "${SIGN_IDENTITY}" = "-"; then
  echo "Ad-hoc signed \"${PKG_DIR}\""
else
  echo "Developer ID signed \"${PKG_DIR}\" with ${SIGN_IDENTITY}"
fi
