#!/bin/sh 

# This script does all the magic calls to automake/autoconf and friends
# that are needed to configure a Subversion checkout.  As described in
# the file HACKING you need a couple of extra tools to run this script
# successfully.
#
# If you are compiling from a released tarball you don't need these
# tools and you shouldn't use this script.  Just call ./configure
# directly.

ACLOCAL=${ACLOCAL-aclocal-1.10}
AUTOHEADER=${AUTOHEADER-autoheader}
AUTOMAKE=${AUTOMAKE-automake-1.10}
AUTOCONF=${AUTOCONF-autoconf}


GLIB_REQUIRED_VERSION=2.10.0
AUTOCONF_REQUIRED_VERSION=2.54
AUTOMAKE_REQUIRED_VERSION=1.10

REQUIRED_M4="fontutil.m4 gettext.m4 pkg.m4 iconv.m4"
REQUIRED_M4_WX="wxwin29.m4 wxwin.m4"

PROJECT="aegisub"

srcdir=`pwd`
test -z "$srcdir" && srcdir=.
ORIGDIR=`pwd`
cd $srcdir

test -d src || {
    echo
    echo "You must run this script in the top-level $PROJECT directory."
    echo
    exit 1
}

# The internal echo for /bin/sh on darwin doesn't support -n
# therefore we have to use /bin/echo.
if test `uname -s` = "Darwin"; then
    ECHO_N="/bin/echo -n";
else
    ECHO_N="echo -n";
fi


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


$ECHO_N "checking for autoconf >= $AUTOCONF_REQUIRED_VERSION ... "
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


# 1.11 comes after 1.10 beacuse 1.11 isn't fully supported.
$ECHO_N "checking for automake >= $AUTOMAKE_REQUIRED_VERSION ... "
if ($AUTOMAKE --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=$AUTOMAKE
   ACLOCAL=$ACLOCAL
elif (automake-1.10 --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=automake-1.10
   ACLOCAL=aclocal-1.10
elif (automake-1.11 --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=automake-1.11
   ACLOCAL=aclocal-1.11
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


if test -z "$BIN_AWK"; then
  BIN_AWK=`which awk`
fi

$ECHO_N "checking for AWK ... "
if test -x "$BIN_AWK"; then
  echo $BIN_AWK
else
    echo "not found"
    echo
    echo "  You must have 'awk' installed.  Please set"
    echo "  BIN_AWK or add it to your PATH."
    echo
    DIE=1
fi

if test "$DIE" -eq 1; then
    echo
    echo "Please install/upgrade the missing tools and call me again."
    echo	
    exit 1
fi


if ! test "$1" = "--skip-configure"; then
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

rm -rf autom4te.cache

echo "--- $ACLOCAL ---"
$ACLOCAL $ACLOCAL_FLAGS
RC=$?
if test $RC -ne 0; then
   echo "$ACLOCAL gave errors. Please fix the error conditions and try again."
   exit $RC
fi

echo "--- $AUTOHEADER ---"
$AUTOHEADER || exit $?

echo "--- $AUTOCONF ---"
$AUTOCONF || exit $?

cd $ORIGDIR

if ! test "$1" = "--skip-configure"; then
  echo "--- $srcdir/configure ---"
  echo "Running '$srcdir/configure --enable-maintainer-mode $AUTOGEN_CONFIGURE_ARGS "$@"'"
  echo
  $srcdir/configure --enable-maintainer-mode $AUTOGEN_CONFIGURE_ARGS "$@"
  RC=$?

  echo
  echo
  echo "***********************************************************************"
  echo "*"
  echo "* Please do not ask for support when using the SVN verison of aegisub,"
  echo "* download an official distfile in order to receive support."
  echo "*"
  echo "***********************************************************************"
  echo

  if test $RC -ne 0; then
    echo
    echo "Configure failed or did not finish!"
    exit $RC
  fi

  echo "Now type 'make' to compile $PROJECT."
fi

