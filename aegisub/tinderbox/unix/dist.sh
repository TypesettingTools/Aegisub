#!/bin/sh
DIST_NAME="aegisub-snap-r${1}"
UNAME_S=`uname -s`
CONFIGURE_ARGS="/mnt/devel/build/debug-wx-trunk-r66440//lib/wx/config/gtk2-unicode-2.9"
BIN_MAKE="make"
ACLOCAL_FLAGS="-I /home/verm/build/wx/share/aclocal" ./autogen.sh ${CONFIGURE_ARGS} || exit $?

CONFIGURE_ARGS="/mnt/devel/build/debug-wx-trunk-r66440//lib/wx/config/gtk2-unicode-2.9"

if test -z "${1}"; then
  echo "You must supply a revision number!"
  exit 1
fi

# On FreeBSD "make" is PMake, so we need to use 'gmake'
if test "${UNAME_S}" = "FreeBSD"; then
  BIN_MAKE="gmake"
fi


${BIN_MAKE} distdir distdir="${DIST_NAME}" || exit $?

# Put the name of the buildslave into ./slave_info as it may be useful for
# debugging localised buildslave issues if a user is using a snapshot.
echo "${2}" > "${DIST_NAME}/slave_info"

tar cf "${DIST_NAME}.tar" "${DIST_NAME}" || exit $?
bzip2 -v ${DIST_NAME}.tar
mv "${DIST_NAME}.tar.bz2" dist.tar.bz2
