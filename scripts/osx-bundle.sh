#!/bin/sh

PKG_DIR=${1}.app
SKEL_DIR="packages/osx_bundle"

if ! test -d packages/osx_bundle; then
  echo
  echo "Make sure you're in the toplevel source directory"
  exit 1;
fi

if test -d ${PKG_DIR}; then
  echo "**** USING OLD ${PKG_DIR} ****"
fi

echo
echo "---- Directory Structure ----"
mkdir -v ${PKG_DIR}
mkdir -v ${PKG_DIR}/Contents
mkdir -v ${PKG_DIR}/Contents/MacOS
mkdir -v ${PKG_DIR}/Contents/Resources

echo
echo "---- Copying Skel Files ----"
cp -v ${SKEL_DIR}/Contents/Resources/* ${PKG_DIR}/Contents/Resources
cp -v ${SKEL_DIR}/Contents/Info.plist ${PKG_DIR}/Contents

echo
echo "---- Binaries ----"
cp -v aegisub/.libs/aegisub ${PKG_DIR}/Contents/MacOS

echo
echo "---- Libraries ----"
python scripts/osx-fix-libs.py "${PKG_DIR}/Contents/MacOS/aegisub"

echo
echo "Done Creating ${PKG_DIR}"
