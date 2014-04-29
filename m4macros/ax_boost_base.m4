# ===========================================================================
#       http://www.gnu.org/software/autoconf-archive/ax_boost_base.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_BOOST_BASE([MINIMUM-VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#
#   Test for the Boost C++ libraries of a particular version (or newer)
#
#   If no path to the installed boost library is given the macro searchs
#   under /usr, /usr/local, /opt and /opt/local and evaluates the
#   $BOOST_ROOT environment variable. Further documentation is available at
#   <http://randspringer.de/boost/index.html>.
#
#   This macro calls:
#
#     AC_SUBST(BOOST_CPPFLAGS) / AC_SUBST(BOOST_LDFLAGS)
#
#   And sets:
#
#     HAVE_BOOST
#
# LICENSE
#
#   Copyright (c) 2008 Thomas Porschberg <thomas@randspringer.de>
#   Copyright (c) 2009 Peter Adolphs
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 20

AC_DEFUN([AX_BOOST_BASE], [
AC_ARG_WITH([boost],
            [AS_HELP_STRING([--with-boost=PATH], [use Boost library from the specified location])],
            [ac_boost_path="$withval"],
            [ac_boost_path=""])

AC_ARG_WITH([boost-libdir],
            AS_HELP_STRING([--with-boost-libdir=LIB_DIR],
                           [Force given directory for boost libraries.
                            Note that this will override library path detection,
                            so use this parameter only if default library
                            detection fails and you know exactly where your
                            boost libraries are located.]),
    [ac_boost_lib_path="$withval"],
    [ac_boost_lib_path=""])

boost_lib_version_req=ifelse([$1], ,1.20.0,$1)
boost_lib_version_req_shorten=`expr $boost_lib_version_req : '\([[0-9]]*\.[[0-9]]*\)'`
boost_lib_version_req_major=`expr $boost_lib_version_req : '\([[0-9]]*\)'`
boost_lib_version_req_minor=`expr $boost_lib_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
boost_lib_version_req_sub_minor=`expr $boost_lib_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
if test "x$boost_lib_version_req_sub_minor" = "x"; then
    boost_lib_version_req_sub_minor="0"
fi
WANT_BOOST_VERSION=`expr $boost_lib_version_req_major \* 100000 \+  $boost_lib_version_req_minor \* 100 \+ $boost_lib_version_req_sub_minor`
AC_MSG_CHECKING(for boostlib >= $boost_lib_version_req)
succeeded=no

dnl On 64-bit systems check for system libraries in both lib64 and lib.
dnl The former is specified by FHS, but e.g. Debian does not adhere to
dnl this (as it rises problems for generic multi-arch support).
dnl The last entry in the list is chosen by default when no libraries
dnl are found, e.g. when only header-only libraries are installed!
libsubdirs="lib"
ax_arch=`uname -m`
if test $ax_arch = x86_64 -o $ax_arch = ppc64 -o $ax_arch = s390x -o $ax_arch = sparc64; then
    libsubdirs="lib64 lib lib64"
fi

dnl first we check the system location for boost libraries
dnl this location ist chosen if boost libraries are installed with the --layout=system option
dnl or if you install boost with RPM
if test "$ac_boost_path" != ""; then
    BOOST_CPPFLAGS="-I$ac_boost_path/include"
    for ac_boost_path_tmp in $libsubdirs; do
            if test -d "$ac_boost_path"/"$ac_boost_path_tmp" ; then
                    BOOST_LDFLAGS="-L$ac_boost_path/$ac_boost_path_tmp"
                    break
            fi
    done
elif test "$cross_compiling" != yes; then
    for ac_boost_path_tmp in /usr /usr/local /opt /opt/local ; do
        if test -d "$ac_boost_path_tmp/include/boost" && test -r "$ac_boost_path_tmp/include/boost"; then
            for libsubdir in $libsubdirs ; do
                if ls "$ac_boost_path_tmp/$libsubdir/libboost_"* >/dev/null 2>&1 ; then break; fi
            done
            BOOST_LDFLAGS="-L$ac_boost_path_tmp/$libsubdir"
            BOOST_CPPFLAGS="-I$ac_boost_path_tmp/include"
            break;
        fi
    done
fi

dnl overwrite ld flags if we have required special directory with
dnl --with-boost-libdir parameter
if test "$ac_boost_lib_path" != ""; then
   BOOST_LDFLAGS="-L$ac_boost_lib_path"
fi

CPPFLAGS_SAVED="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
export CPPFLAGS

LDFLAGS_SAVED="$LDFLAGS"
LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
export LDFLAGS

AC_REQUIRE([AC_PROG_CXX])
AC_LANG_PUSH(C++)
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
@%:@include <boost/version.hpp>
]], [[
#if BOOST_VERSION >= $WANT_BOOST_VERSION
// Everything is okay
#else
#  error Boost version is too old
#endif
]])],[
    AC_MSG_RESULT(yes)
succeeded=yes
found_system=yes
    ],[
    ])
AC_LANG_POP([C++])

dnl if we found no boost with system layout we search for boost libraries
dnl built and installed without the --layout=system option or for a staged(not installed) version
if test "x$succeeded" != "xyes"; then
    _version=0
    if test "$ac_boost_path" != ""; then
        if test -d "$ac_boost_path" && test -r "$ac_boost_path"; then
            for i in `ls -d $ac_boost_path/include/boost-* 2>/dev/null`; do
                _version_tmp=`echo $i | sed "s#$ac_boost_path##" | sed 's/\/include\/boost-//' | sed 's/_/./'`
                V_CHECK=`expr $_version_tmp \> $_version`
                if test "$V_CHECK" = "1" ; then
                    _version=$_version_tmp
                fi
                VERSION_UNDERSCORE=`echo $_version | sed 's/\./_/'`
                BOOST_CPPFLAGS="-I$ac_boost_path/include/boost-$VERSION_UNDERSCORE"
            done
        fi
    else
        if test "$cross_compiling" != yes; then
            for ac_boost_path in /usr /usr/local /opt /opt/local ; do
                if test -d "$ac_boost_path" && test -r "$ac_boost_path"; then
                    for i in `ls -d $ac_boost_path/include/boost-* 2>/dev/null`; do
                        _version_tmp=`echo $i | sed "s#$ac_boost_path##" | sed 's/\/include\/boost-//' | sed 's/_/./'`
                        V_CHECK=`expr $_version_tmp \> $_version`
                        if test "$V_CHECK" = "1" ; then
                            _version=$_version_tmp
                            best_path=$ac_boost_path
                        fi
                    done
                fi
            done

            VERSION_UNDERSCORE=`echo $_version | sed 's/\./_/'`
            BOOST_CPPFLAGS="-I$best_path/include/boost-$VERSION_UNDERSCORE"
            if test "$ac_boost_lib_path" = ""; then
                for libsubdir in $libsubdirs ; do
                    if ls "$best_path/$libsubdir/libboost_"* >/dev/null 2>&1 ; then break; fi
                done
                BOOST_LDFLAGS="-L$best_path/$libsubdir"
            fi
        fi

        if test "x$BOOST_ROOT" != "x"; then
            for libsubdir in $libsubdirs ; do
                if ls "$BOOST_ROOT/stage/$libsubdir/libboost_"* >/dev/null 2>&1 ; then break; fi
            done
            if test -d "$BOOST_ROOT" && test -r "$BOOST_ROOT" && test -d "$BOOST_ROOT/stage/$libsubdir" && test -r "$BOOST_ROOT/stage/$libsubdir"; then
                version_dir=`expr //$BOOST_ROOT : '.*/\(.*\)'`
                stage_version=`echo $version_dir | sed 's/boost_//' | sed 's/_/./g'`
                    stage_version_shorten=`expr $stage_version : '\([[0-9]]*\.[[0-9]]*\)'`
                V_CHECK=`expr $stage_version_shorten \>\= $_version`
                if test "$V_CHECK" = "1" -a "$ac_boost_lib_path" = "" ; then
                    AC_MSG_NOTICE(We will use a staged boost library from $BOOST_ROOT)
                    BOOST_CPPFLAGS="-I$BOOST_ROOT"
                    BOOST_LDFLAGS="-L$BOOST_ROOT/stage/$libsubdir"
                fi
            fi
        fi
    fi

    CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
    export CPPFLAGS
    LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
    export LDFLAGS

    AC_LANG_PUSH(C++)
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    @%:@include <boost/version.hpp>
    ]], [[
    #if BOOST_VERSION >= $WANT_BOOST_VERSION
    // Everything is okay
    #else
    #  error Boost version is too old
    #endif
    ]])],[
        AC_MSG_RESULT(yes)
    succeeded=yes
    found_system=yes
        ],[
        ])
    AC_LANG_POP([C++])
fi

if test "$succeeded" != "yes" ; then
    if test "$_version" = "0" ; then
        AC_MSG_NOTICE([[We could not detect the boost libraries (version $boost_lib_version_req_shorten or higher). If you have a staged boost library (still not installed) please specify \$BOOST_ROOT in your environment and do not give a PATH to --with-boost option.  If you are sure you have boost installed, then check your version number looking in <boost/version.hpp>. See http://randspringer.de/boost for more documentation.]])
    else
        AC_MSG_NOTICE([Your boost libraries seems too old (version $_version).])
    fi
    # execute ACTION-IF-NOT-FOUND (if present):
    ifelse([$3], , :, [$3])
else
    BOOSTLIBDIR=`echo $BOOST_LDFLAGS | sed -e 's/@<:@^\/@:>@*//'`
    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LDFLAGS)
    # execute ACTION-IF-FOUND (if present):
    ifelse([$2], , :, [$2])
fi

CPPFLAGS="$CPPFLAGS_SAVED"
LDFLAGS="$LDFLAGS_SAVED"
])

AC_DEFUN([AX_BOOST_HEADER_ONLY], [
CPPFLAGS_SAVED="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
export CPPFLAGS

AS_IF([test x$enable_sanity_checks != xno], [
    AC_CACHE_CHECK(
        [whether the boost.$1 library is available],
        ax_cv_boost_$1,
        [AC_LANG_PUSH([C++])
         AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM([[@%:@include <boost/$2>]],
                [[$3;]])],
            [AC_MSG_RESULT(yes)],
            AC_MSG_ERROR([Could not find boost.$1 headers!]))
         AC_LANG_POP([C++])])
])

CPPFLAGS="$CPPFLAGS_SAVED"
])

AC_DEFUN([AX_BOOST_LIB], [
AC_ARG_WITH([boost-$1],
            AS_HELP_STRING([--with-boost-$1=LIBNAME],
                           [Override the name of the library for boost.$1
                           e.g. --with-boost-$1=boost_$1-gcc-mt]),
            [ax_boost_lib="$withval"],
            [ax_boost_lib="boost_$1"])

CPPFLAGS_SAVED="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
export CPPFLAGS

LDFLAGS_SAVED="$LDFLAGS"
LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
export LDFLAGS

AS_IF([test x$enable_sanity_checks != xno], [
    AC_CACHE_CHECK(
        [whether the boost.$1 library is available],
        ax_cv_boost_$1,
        [AC_LANG_PUSH([C++])
         AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM([[@%:@include <boost/$3>]], [[$4;]])],
            [AC_MSG_RESULT(yes)],
            AC_MSG_ERROR([Could not find boost.$1 headers!]))
         AC_LANG_POP([C++])])
    AC_CHECK_LIB($ax_boost_lib, exit, [ax_boost_actual_lib="-l$ax_boost_lib"])
], [ax_boost_actual_lib="-l$ax_boost_lib"])

if test "x$ax_boost_actual_lib" = "x"; then
    for lib in `ls -r $BOOSTLIBDIR/libboost_$1* 2>/dev/null | sed 's,.*/lib,,' | sed 's,\..*,,'`; do
        ax_boost_lib=${lib}
        AC_CHECK_LIB($ax_boost_lib, exit, [ax_boost_actual_lib="-l$ax_boost_lib"; break])
    done
fi

if test "x$ax_boost_actual_lib" = "x"; then
    for lib in `ls -r $BOOSTLIBDIR/boost_$1* 2>/dev/null | sed 's,.*/,,' | sed -e 's,\..*,,'` ; do
        ax_boost_lib=${lib}
        AC_CHECK_LIB($ax_boost_lib, exit, [ax_boost_actual_lib="-l$ax_boost_lib"; break])
    done
fi

if test "x$ax_boost_actual_lib" = "x"; then
    AC_MSG_ERROR(Could not find a working version of the library!)
fi

eval $2="$ax_boost_actual_lib"
AC_SUBST($2)
CPPFLAGS="$CPPFLAGS_SAVED"
LDFLAGS="$LDFLAGS_SAVED"
])

