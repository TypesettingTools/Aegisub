#!/bin/sh

set -e

SRC_DIR="${1}"
BUILD_DIR="${2}"
WX_PREFIX=""
FONTCONFIG_CONF_DIR="${4}"
DICT_DIR="${5}"
MESON_BUILD_OSX_BUNDLE="${6}"

if [ "${MESON_BUILD_OSX_BUNDLE}" != "TRUE" ]; then
  echo "Project not built with \`build_osx_bundle\`"
  echo "Please run \`meson configure -Dbuild_osx_bundle=true\` and rebuild"
  exit 1
fi

PKG_DIR="${BUILD_DIR}/Aegisub.app"
SKEL_DIR="${SRC_DIR}/packages/osx_bundle"

if test -d "${PKG_DIR}"; then
  rm -rf "${PKG_DIR}"
  echo "Removing old Aegisub.app"
fi

echo
echo "---- Directory Structure ----"
mkdir -v "${PKG_DIR}"
mkdir -v "${PKG_DIR}/Contents"
mkdir -v "${PKG_DIR}/Contents/MacOS"
mkdir -v "${PKG_DIR}/Contents/Resources"
mkdir -v "${PKG_DIR}/Contents/SharedSupport"
mkdir -v "${PKG_DIR}/Contents/SharedSupport/dictionaries"

echo
echo "---- Copying Skel Files ----"
if ! test -f "${BUILD_DIR}/osx-bundle.sed"; then
  echo
  echo "NOT FOUND: ${BUILD_DIR}/osx-bundle.sed"
  exit 1
fi

# used by osx-bundle.sed
find "${SRC_DIR}/po" -name *.po | sed 's/.*\/\(.*\)\.po/        <string>\1<\/string>/; s/RS/YU/' > "${BUILD_DIR}/languages"

#find "${SKEL_DIR}" -type f -not -regex ".*.svn.*"
cp -v ${SKEL_DIR}/Contents/Resources/*.icns "${PKG_DIR}/Contents/Resources"
cat "${SKEL_DIR}/Contents/Info.plist" | sed -f "${BUILD_DIR}/osx-bundle.sed" > "${PKG_DIR}/Contents/Info.plist"

rm "${BUILD_DIR}/languages"

echo
echo "---- Installing files ----"
CURRENT_DIR=`pwd`
cd ${BUILD_DIR}
ninja install
cd ${CURRENT_DIR}

echo
echo "---- Copying dictionaries ----"
if test -f "${DICT_DIR}"; then
  cp -v "${DICT_DIR}/*" "${PKG_DIR}/Contents/SharedSupport/dictionaries"
else
  echo "Specified dictionary directory ${DICT_DIR} not found!"
fi

echo
echo "---- Copying Aegisub locale files ----"
# Let Aqua know that aegisub supports english.  English strings are
# internal so we don't need an aegisub.mo file.
mkdir -vp "${PKG_DIR}/Contents/Resources/en.lproj"

# FIXME
# 10.8 wants sr_YU rather than sr_RS
#mv "${PKG_DIR}/Contents/Resources/sr_RS.lproj" "${PKG_DIR}/Contents/Resources/sr_YU.lproj"
#mv "${PKG_DIR}/Contents/Resources/sr_RS@latin.lproj" "${PKG_DIR}/Contents/Resources/sr_YU@latin.lproj"

## TODO: rm those lines
##  xref: [Update and review translations · Issue #132 · TypesettingTools/Aegisub](https://github.com/TypesettingTools/Aegisub/issues/132)
# echo
# echo "---- Copying WX locale files ----"
#
# for i in `ls -1 ${SRC_DIR}/po/*.mo|sed "s|po/\(.*\).mo|\1|"`; do
#   WX_MO="${WX_PREFIX}/share/locale/${i}/LC_MESSAGES/wxstd.mo"
#
#   if ! test -f "${WX_MO}"; then
#     WX_MO="${HOME_DIR}/wxstd/${i}.mo"
#   fi
#
#   if test -f "${WX_MO}"; then
#     cp -v "${WX_MO}" "${PKG_DIR}/Contents/Resources/${i}.lproj/"
#   else
#     echo "WARNING: \"$i\" locale in aegisub but no WX catalog found!"
#   fi
# done

echo
echo "---- Fixing libraries ----"
sudo python "${SRC_DIR}/tools/osx-fix-libs.py" "${PKG_DIR}/Contents/MacOS/aegisub" || exit $?

echo
echo "Done creating \"${PKG_DIR}\""
