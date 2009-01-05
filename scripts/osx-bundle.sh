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
mkdir -v ${PKG_DIR}/Contents/Resources/etc
mkdir -v ${PKG_DIR}/Contents/Resources/etc/fonts
mkdir -v ${PKG_DIR}/Contents/Resources/etc/fonts/conf.d
mkdir -v ${PKG_DIR}/Contents/SharedSupport
mkdir -v ${PKG_DIR}/Contents/SharedSupport/dictionaries

echo
echo "---- Copying Skel Files ----"
find ${SKEL_DIR} -type f -not -regex ".*.svn.*"
cp ${SKEL_DIR}/Contents/Resources/*.icns ${PKG_DIR}/Contents/Resources
cp ${SKEL_DIR}/Contents/Resources/etc/fonts/fonts.* ${PKG_DIR}/Contents/Resources/etc/fonts
cp ${SKEL_DIR}/Contents/Resources/etc/fonts/conf.d/*.conf ${PKG_DIR}/Contents/Resources/etc/fonts/conf.d
cat ${SKEL_DIR}/Contents/Info.plist |sed -f scripts/osx-bundle.sed > ${PKG_DIR}/Contents/Info.plist

echo
echo "---- Copying locale files ----"
# Let Aqua know that aegisub supports english.  English strings are
# internal so we don't need an aegisub.mo file.
mkdir -v ${PKG_DIR}/Contents/Resources/en.lproj

for i in `cat po/LINGUAS`; do
  if test -f "po/${i}.gmo"; then
    mkdir -v ${PKG_DIR}/Contents/Resources/${i}.lproj;
    cp -v po/${i}.gmo ${PKG_DIR}/Contents/Resources/${i}.lproj/aegisub.mo;
  else
    echo "${i}.gmo not found!"
  fi
done

echo
echo "---- Binaries ----"
cp -v aegisub/.libs/aegisub ${PKG_DIR}/Contents/MacOS

echo
echo "---- Libraries ----"
python scripts/osx-fix-libs.py "${PKG_DIR}/Contents/MacOS/Aegisub"

echo
echo "Done Creating ${PKG_DIR}"
