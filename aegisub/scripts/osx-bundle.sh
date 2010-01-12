#!/bin/sh

PKG_DIR=${1}.app
SKEL_DIR="packages/osx_bundle"
AEGISUB_VERSION_DATA="${2}"
AEGISUB_BIN="aegisub-${AEGISUB_VERSION_DATA}"
SRCDIR=`pwd`
HOME_DIR=`echo ~`
WX_CONFIG="wx-config"
WX_PREFIX=`${WX_CONFIG} --prefix`

if test -z "${CC}"; then
  CC="cc"
fi

if ! test -d packages/osx_bundle; then
  echo
  echo "Make sure you're in the toplevel source directory"
  exit 1;
fi

if test -d "${PKG_DIR}"; then
  echo "**** USING OLD ${PKG_DIR} ****"
fi

echo
echo "---- Directory Structure ----"
mkdir -v "${PKG_DIR}"
mkdir -v "${PKG_DIR}/Contents"
mkdir -v "${PKG_DIR}/Contents/MacOS"
mkdir -v "${PKG_DIR}/Contents/Resources"
mkdir -v "${PKG_DIR}/Contents/Resources/etc"
mkdir -v "${PKG_DIR}/Contents/Resources/etc/fonts"
mkdir -v "${PKG_DIR}/Contents/Resources/etc/fonts/conf.d"
mkdir -v "${PKG_DIR}/Contents/SharedSupport"
mkdir -v "${PKG_DIR}/Contents/SharedSupport/dictionaries"

echo
echo "---- Copying Skel Files ----"
find ${SKEL_DIR} -type f -not -regex ".*.svn.*"
cp ${SKEL_DIR}/Contents/Resources/*.icns "${PKG_DIR}/Contents/Resources"
cp ${SKEL_DIR}/Contents/Resources/etc/fonts/fonts.dtd "${PKG_DIR}/Contents/Resources/etc/fonts"
cat ${SKEL_DIR}/Contents/Resources/etc/fonts/fonts.conf |sed -f scripts/osx-bundle.sed > "${PKG_DIR}/Contents/Resources/etc/fonts/fonts.conf"
cp ${SKEL_DIR}/Contents/Resources/etc/fonts/conf.d/*.conf "${PKG_DIR}/Contents/Resources/etc/fonts/conf.d"
cat ${SKEL_DIR}/Contents/Info.plist |sed -f scripts/osx-bundle.sed > "${PKG_DIR}/Contents/Info.plist"


echo
echo "---- Copying dictionaries ----"
if test -z "${DICT_DIR}"; then
  DICT_DIR="${HOME_DIR}/dict"
fi

if test -d "${DICT_DIR}"; then
  cp -v ${DICT_DIR}/* "${PKG_DIR}/Contents/SharedSupport/dictionaries"
else
  echo "WARNING: Dictionaries not found, please set $$DICT_DIR to a directiory"
  echo "         where the *.aff and *.dic files can be found"
fi


echo
echo "---- Copying automation/ files ----"
cd automation
make install \
	aegisubdatadir="../${PKG_DIR}/Contents/SharedSupport" \
	aegisubdocdir="../${PKG_DIR}/Contents/SharedSupport/doc"
cd "${SRCDIR}"


echo
echo "---- Copying Aegisub locale files ----"
# Let Aqua know that aegisub supports english.  English strings are
# internal so we don't need an aegisub.mo file.
mkdir -vp "${PKG_DIR}/Contents/Resources/en.lproj"

for i in `cat po/LINGUAS`; do
  if test -f "po/${i}.gmo"; then
    mkdir -p "${PKG_DIR}/Contents/Resources/${i}.lproj";
    cp -v po/${i}.gmo "${PKG_DIR}/Contents/Resources/${i}.lproj/aegisub.mo";
  else
    echo "${i}.gmo not found!"
	exit 1
  fi;
done


echo
echo "---- Copying WX locale files ----"

for i in `cat po/LINGUAS`; do
  if test -f "${WX_PREFIX}/share/locale/${i}/LC_MESSAGES/wxstd.mo"; then
	cp -v "${WX_PREFIX}/share/locale/${i}/LC_MESSAGES/wxstd.mo" "${PKG_DIR}/Contents/Resources/${i}.lproj/";
  else
	echo "WARNING: \"$i\" locale in aegisub but no WX catalog found!";
  fi;
done


echo
echo "---- Binaries ----"

# XXX: Fix me
# I'm not sure of the exact reason but libtool likes creating the binary in
# either '.' or '.libs', the file in the parent is just a script that loads
# the binary from .libs.  On ocassion it'll stop using the script and just
# create the binary, this isn't the best way to check which one to copy but
# it's a good enough hack until then.

if test src/${AEGISUB_BIN} -nt src/.libs/${AEGISUB_BIN}; then
  cp -v src/${AEGISUB_BIN} "${PKG_DIR}/Contents/MacOS/aegisub"
else
  cp -v src/.libs/${AEGISUB_BIN} "${PKG_DIR}/Contents/MacOS/aegisub" || exit $?
fi

echo cc -o "${PKG_DIR}/Contents/MacOS/restart-helper scripts/osx-bundle-restart-helper.c"
${CC} -o "${PKG_DIR}/Contents/MacOS/restart-helper" scripts/osx-bundle-restart-helper.c || exit $?

echo
echo "---- Libraries ----"
python scripts/osx-fix-libs.py "${PKG_DIR}/Contents/MacOS/aegisub" || exit $?

echo
echo "Done Creating \"${PKG_DIR}\""
