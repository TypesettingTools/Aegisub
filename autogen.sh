#!/bin/sh 

# This script does all the magic calls to automake/autoconf and friends
# that are needed to configure a Subversion checkout.  As described in
# the file HACKING you need a couple of extra tools to run this script
# successfully.
#
# If you are compiling from a released tarball you don't need these
# tools and you shouldn't use this script.  Just call ./configure
# directly.

ACLOCAL=${ACLOCAL-aclocal-1.9}
AUTOCONF=${AUTOCONF-autoconf}
AUTOHEADER=${AUTOHEADER-autoheader}
AUTOMAKE=${AUTOMAKE-automake-1.9}
LIBTOOLIZE=${LIBTOOLIZE-libtoolize}

GLIB_REQUIRED_VERSION=2.10.0
AUTOCONF_REQUIRED_VERSION=2.54
AUTOMAKE_REQUIRED_VERSION=1.9
INTLTOOL_REQUIRED_VERSION=0.31
LIBTOOL_REQUIRED_VERSION=1.5

REQUIRED_M4="fontutil.m4 wxwin28.m4 glib-gettext.m4 intltool.m4 intl.m4 pkg.m4 iconv.m4"
REQUIRED_M4_WX="wxwin.m4 wxwin28.m4"

PROJECT="aegisub"

srcdir=`pwd`
test -z "$srcdir" && srcdir=.
ORIGDIR=`pwd`
cd $srcdir

test -d aegisub || {
    echo
    echo "You must run this script in the top-level $PROJECT directory."
    echo
    exit 1
}

check_version ()
{
    VERSION_A=$1
    VERSION_B=$2

    save_ifs="$IFS"
    IFS=.
    set dummy $VERSION_A 0 0 0
    MAJOR_A=$2
    MINOR_A=$3
    MICRO_A=$4
    set dummy $VERSION_B 0 0 0
    MAJOR_B=$2
    MINOR_B=$3
    MICRO_B=$4
    IFS="$save_ifs"

    if expr "$MAJOR_A" = "$MAJOR_B" > /dev/null; then
        if expr "$MINOR_A" \> "$MINOR_B" > /dev/null; then
           echo "yes (version $VERSION_A)"
        elif expr "$MINOR_A" = "$MINOR_B" > /dev/null; then
            if expr "$MICRO_A" \>= "$MICRO_B" > /dev/null; then
               echo "yes (version $VERSION_A)"
            else
                echo "Too old (version $VERSION_A)"
                DIE=1
            fi
        else
            echo "Too old (version $VERSION_A)"
            DIE=1
        fi
    elif expr "$MAJOR_A" \> "$MAJOR_B" > /dev/null; then
	echo "Major version might be too new ($VERSION_A)"
    else
	echo "Too old (version $VERSION_A)"
	DIE=1
    fi
}

DIE=0


echo -n "checking for libtool >= $LIBTOOL_REQUIRED_VERSION ... "
if ($LIBTOOLIZE --version) < /dev/null > /dev/null 2>&1; then
   LIBTOOLIZE=$LIBTOOLIZE
elif (glibtoolize --version) < /dev/null > /dev/null 2>&1; then
   LIBTOOLIZE=glibtoolize
else
    echo
    echo "  You must have libtool installed to compile $PROJECT."
    echo "  Install the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    echo
    DIE=1
fi

if test x$LIBTOOLIZE != x; then
    VER=`$LIBTOOLIZE --version \
         | grep libtool | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $LIBTOOL_REQUIRED_VERSION
fi

echo -n "checking for autoconf >= $AUTOCONF_REQUIRED_VERSION ... "
if ($AUTOCONF --version) < /dev/null > /dev/null 2>&1; then
    VER=`$AUTOCONF --version | head -n 1 \
         | grep -iw autoconf | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOCONF_REQUIRED_VERSION
else
    echo
    echo "  You must have autoconf installed to compile $PROJECT."
    echo "  Download the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/autoconf/"
    echo
    DIE=1;
fi


echo -n "checking for automake >= $AUTOMAKE_REQUIRED_VERSION ... "
if ($AUTOMAKE --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=$AUTOMAKE
   ACLOCAL=$ACLOCAL
elif (automake-1.10 --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=automake-1.10
   ACLOCAL=aclocal-1.10
elif (automake-1.9 --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=automake-1.9
   ACLOCAL=aclocal-1.9
else
    echo
    echo "  You must have automake $AUTOMAKE_REQUIRED_VERSION or newer installed to compile $PROJECT."
    echo "  Download the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/automake/"
    echo
    DIE=1
fi

if test x$AUTOMAKE != x; then
    VER=`$AUTOMAKE --version \
         | grep automake | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOMAKE_REQUIRED_VERSION
fi


echo -n "checking for glib-gettextize ... "
if (glib-gettextize --version) < /dev/null > /dev/null 2>&1; then
    VER=`glib-gettextize --version \
         | grep glib-gettextize | sed "s/.* \([0-9.]*\)/\1/"`
    check_version $VER $GLIB_REQUIRED_VERSION
else
    echo
    echo "  You must have glib-gettextize installed to compile $PROJECT."
    echo "  glib-gettextize is part of glib-2.0, so you should already"
    echo "  have it. Make sure it is in your PATH."
    echo
    DIE=1
fi


echo -n "checking for intltool >= $INTLTOOL_REQUIRED_VERSION ... "
if (intltoolize --version) < /dev/null > /dev/null 2>&1; then
    VER=`intltoolize --version \
         | grep intltoolize | sed "s/.* \([0-9.]*\)/\1/"`
    check_version $VER $INTLTOOL_REQUIRED_VERSION
else
    echo
    echo "  You must have intltool installed to compile $PROJECT."
    echo "  Get the latest version from"
    echo "  ftp://ftp.gnome.org/pub/GNOME/sources/intltool/"
    echo
    DIE=1
fi

WHICH_CONVERT=`which convert`
echo -n "checking for ImageMagick 'convert' utility ... "
if test -x "$WHICH_CONVERT"; then
  BIN_CONVERT=$WHICH_CONVERT
  echo $BIN_CONVERT
else
    echo
    echo "  You must have 'convert' installed from the"
    echo "  ImageMagick project."
    echo
    DIE=1
fi

WHICH_AWK=`which awk`
echo -n "checking for AWK ... "
if test -x "$WHICH_AWK"; then
  BIN_AWK=$WHICH_AWK
  echo $BIN_AWK
else
    echo
    echo "  You must have 'convert' installed from the"
    echo "  ImageMagick project."
    echo
    DIE=1
fi

if test "$DIE" -eq 1; then
    echo
    echo "Please install/upgrade the missing tools and call me again."
    echo	
    exit 1
fi


echo
echo "I am going to run ./configure with the following arguments:"
echo
echo "  --enable-maintainer-mode $AUTOGEN_CONFIGURE_ARGS $@"
echo

if test -z "$*"; then
    echo "If you wish to pass additional arguments, please specify them "
    echo "on the $0 command line or set the AUTOGEN_CONFIGURE_ARGS "
    echo "environment variable."
    echo
fi



echo "--- Checking for required M4 files ---"

if test -z "$ACLOCAL_FLAGS"; then
    acdir=`$ACLOCAL --print-ac-dir`

    for file in $REQUIRED_M4; do
	    if [ ! -f "$acdir/$file" ]; then
             echo
             echo "WARNING: aclocal's directory is $acdir, but..."
             echo "         no file $acdir/$file"
             echo "         You may see fatal macro warnings below."
             echo "         If these files are installed in /some/dir, set the "
             echo "         ACLOCAL_FLAGS environment variable to \"-I /some/dir\""
             echo "         or install $acdir/$file."
             echo
        fi
    done


    for file in $REQUIRED_M4_WX; do
	    if [ -f "$acdir/$file" ]; then
           FOUND_M4_WX="yes"
        fi
    done

    if test -z "$FOUND_M4_WX"; then
        echo
        echo "WARNING: aclocal's directory is $acdir, but..."
        echo "         none of: \"$REQUIRED_M4_WX\" were found."
        echo "         You may see fatal macro warnings below."
        echo "         If these files are installed in /some/dir, set the "
        echo "         ACLOCAL_FLAGS environment variable to \"-I /some/dir\""
        echo "         or install ONE OF: \"$REQUIRED_M4_WX\" in $acdir."
        echo "NOTE:    These are the same files under different names."
        echo
    fi
fi



# XXX: This is a kludge until I sort out the config/includes situation.
touch ${srcdir}/aegisub/posix/config.h

# BMP -> XPM via src/res.rc
cat ${srcdir}/aegisub/res.rc | ${BIN_AWK} -f ${srcdir}/aegisub/bitmaps/genxpm.awk > ${srcdir}/aegisub/bitmaps/Makefile.bitmaps
cd ${srcdir}/aegisub/bitmaps
make -f Makefile.bitmaps
cd ${srcdir}

awk '/BITMAP/ { image[count] = $1; ++count} END { printf("EXTRA_DIST= \\\n	wxicon_xpm.xpm"); for (v in image) printf(" \\\n	%s_xpm.xpm", image[v])}' \
  ${srcdir}/aegisub/res.rc \
  > ${srcdir}/aegisub/bitmaps/Makefile.am

# XXX: This needs replacing.
cd ${srcdir}/aegisub/posix
sh genres.sh ${srcdir}/aegisub/res.rc
cd ${srcdir}

rm -rf autom4te.cache

echo "--- $ACLOCAL ---"
$ACLOCAL $ACLOCAL_FLAGS
RC=$?
if test $RC -ne 0; then
   echo "$ACLOCAL gave errors. Please fix the error conditions and try again."
   exit $RC
fi

echo "--- $LIBTOOLIZE ---"
$LIBTOOLIZE --force || exit $?

echo "--- $AUTOHEADER ---"
$AUTOHEADER || exit $?

echo "--- $AUTOMAKE ---"
$AUTOMAKE --add-missing || exit $?

echo "--- $AUTOCONF ---"
$AUTOCONF || exit $?

echo "--- glib-gettextizeL ---"
glib-gettextize --force || exit $?

echo "--- intltoolize ---"
intltoolize --force --automake || exit $?

cd $ORIGDIR

echo "--- $srcdir/configure ---"
$srcdir/configure --enable-maintainer-mode $AUTOGEN_CONFIGURE_ARGS "$@"
RC=$?
if test $RC -ne 0; then
  echo
  echo "Configure failed or did not finish!"
  exit $RC
fi

echo
echo
echo "***********************************************************************"
echo "*"
echo "* Please do not ask for support when using the SVN verison of aegisub,"
echo "* download an official distfile in order to receive support."
echo "*"
echo "***********************************************************************"
echo
echo "Now type 'make' to compile the $PROJECT."

