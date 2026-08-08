#!/bin/sh

set -e

PKG_DIR="${1}"
AEGI_VER="${2:-}"

if ! test -d "${PKG_DIR}"; then
  echo "\"${PKG_DIR}\" does not exist" >&2
  exit 1
fi

if test -z "${AEGI_VER}"; then
  AEGI_VER="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' "${PKG_DIR}/Contents/Info.plist")"
fi

# Git branch names may contain characters which are unsafe in a filename.
SAFE_AEGI_VER="$(printf '%s' "${AEGI_VER}" | LC_ALL=C tr -c 'A-Za-z0-9._-' '-' | sed 's/--*/-/g; s/^-//; s/-$//')"
if test -z "${SAFE_AEGI_VER}"; then
  echo "Could not derive a package version from \"${AEGI_VER}\"" >&2
  exit 1
fi

APP_ARCHS="$(lipo -archs "${PKG_DIR}/Contents/MacOS/aegisub")"
case "${APP_ARCHS}" in
  'arm64 x86_64'|'x86_64 arm64') PKG_ARCH="universal2" ;;
  'x86_64') PKG_ARCH="x64" ;;
  *) PKG_ARCH="$(printf '%s' "${APP_ARCHS}" | tr ' ' '-')" ;;
esac

printf 'Aegisub-%s-%s\n' "${SAFE_AEGI_VER}" "${PKG_ARCH}"
