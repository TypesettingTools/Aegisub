#!/bin/sh
# Check that the version in meson.build has an entry in the metainfo release
# list and, if a tag is given, that the tag names the same version.
#
# Usage: tools/check-release-version.sh [vX.Y.Z]
set -eu

srcdir=$(cd "$(dirname "$0")/.." && pwd)
metainfo="$srcdir/packages/desktop/aegisub.metainfo.xml.in.in"

version=$(sed -n "s/^ *version: '\([^']*\)'.*/\1/p" "$srcdir/meson.build" | head -n 1)
if [ -z "$version" ]; then
  echo "Could not read the project version from meson.build" >&2
  exit 1
fi

status=0

latest=$(sed -n 's/^ *<release version="\([^"]*\)".*/\1/p' "$metainfo" | head -n 1)
if [ "$latest" != "$version" ]; then
  echo "meson.build is at $version, but the newest release in $(basename "$metainfo") is ${latest:-missing}" >&2
  status=1
fi

if [ $# -gt 0 ] && [ "$1" != "v$version" ]; then
  echo "Tag $1 does not match the meson.build version $version" >&2
  status=1
fi

if [ $status -eq 0 ]; then
  echo "Release version $version is consistent"
fi
exit $status
