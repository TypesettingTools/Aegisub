#!/bin/sh
DIST_NAME="aegisub-snap-r${1}"
UNAME_S=`uname -s`

if test -z "${1}"; then
  echo "You must supply a revision number!"
  exit 1
fi

# On FreeBSD "make" is PMake, so we need to use 'gmake'
if test "${UNAME_S}" = "FreeBSD"; then
  BIN_MAKE="gmake"
  CONFIGURE_ARGS="--with-wx-config=/usr/local/bin/wxgtk2u-2.8-config"
else
  BIN_MAKE="make"
fi

ACLOCAL_FLAGS="-I /home/verm/build/wx/share/aclocal" ./autogen.sh ${CONFIGURE_ARGS} || exit $?

# XXX: Work around ancient versions of intltool (cough DEBIAN)
touch intltool-update.in intltool-merge.in intltool-extract.in

${BIN_MAKE} distdir distdir="${DIST_NAME}" || exit $?

# Put the name of the buildslave into ./slave_info as it may be useful for
# debugging localised buildslave issues if a user is using a snapshot.
echo "${2}" > "${DIST_NAME}/slave_info"

tar cf "${DIST_NAME}.tar" "${DIST_NAME}" || exit $?
bzip2 -v ${DIST_NAME}.tar
mv "${DIST_NAME}.tar.bz2" dist.tar.bz2

