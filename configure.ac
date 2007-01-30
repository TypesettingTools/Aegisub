AC_PREREQ(2.57)
AC_INIT([aegisub], [2.00-alpha])
AC_CONFIG_AUX_DIR(ac)
AC_CONFIG_SRCDIR([aegisub/main.cpp])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([aegisub/posix/acconf.h])
AC_GNU_SOURCE

AC_CANONICAL_HOST

AM_GNU_GETTEXT

AC_DEFUN([AC_C_FLAG], [{
	AC_LANG_PUSH(C)
	ac_c_flag_save="$CFLAGS"
	CFLAGS="$CFLAGS $1"
	AC_MSG_CHECKING([[whether $CC supports $1]])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM([[]])],
		[AC_MSG_RESULT([yes])],
		[
			CFLAGS="$ac_c_flag_save"
			AC_MSG_RESULT([no])
			$2
		])
	AC_LANG_POP(C)
	}])
AC_DEFUN([AC_CXX_FLAG], [{
	AC_LANG_PUSH(C++)
	ac_cxx_flag_save="$CXXFLAGS"
	CXXFLAGS="$CXXFLAGS $1"
	AC_MSG_CHECKING([[whether $CXX supports $1]])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM([[]])],
		[AC_MSG_RESULT([yes])],
		[
			CXXFLAGS="$ac_cxx_flag_save"
			AC_MSG_RESULT([no])
			$2
		])
	AC_LANG_POP(C++)
	}])


# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_LANG(C++)
AC_PROG_RANLIB

AC_PATH_PROGS(CONVERT, [convert], [])
if test -z "$CONVERT"
then	AC_MSG_FAILURE([aegisub needs convert (from ImageMagick) to produce xpms from the supplied bitmaps/icons])
fi

AC_C_FLAG([-pthread])
AC_CXX_FLAG([-pthread])
AC_CHECK_LIB([pthread], [pthread_join])

AC_CHECK_HEADER([wchar.h],,[
	AC_MSG_FAILURE([aegisub needs wide character support, find a wchar.h])
])
PKG_CHECK_MODULES([FREETYPE], freetype2 >= 9.7.0,, [
	AC_MSG_FAILURE([aegisub needs FreeType2 2.1.9, go install it])
])

WXCONFIG=wx-config
AC_ARG_WITH(wx-config,
	AC_HELP_STRING([--with-wx-config=FILE], [Use specific wx-config script to determine wxWidgets configuration. [[wx-config]]]),
[
    if test "$withval" != "yes" -a "$withval" != ""; then
        WXCONFIG=$withval
    fi
])

AC_CHECK_LIB([GL], [glBegin],,[
	AC_MSG_FAILURE([system OpenGL support missing])
])

AC_MSG_CHECKING([wxWidgets version])
if wxversion=`$WXCONFIG --version`; then
    AC_MSG_RESULT([$wxversion])
else
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([wxWidgets is required. Try --with-wx-config.])
fi

CPPFLAGS="$CPPFLAGS `$WXCONFIG --cppflags | sed -e 's/-fno-exceptions//'` $FREETYPE_CFLAGS"
LIBS="$LIBS `$WXCONFIG --libs` `$WXCONFIG --libs gl` $FREETYPE_LIBS"

AC_MSG_CHECKING([wxWidgets OpenGL support])
AC_LINK_IFELSE([
#include <wx/glcanvas.h>
int main(int argc, char **argv) {
	wxGLCanvas *canvas;
	return 0;
}
],[
	AC_MSG_RESULT([yes])
],[
	AC_MSG_RESULT([not available])
	AC_MSG_FAILURE([wxWidgets OpenGL support missing])
])

AC_ARG_WITH(wxscintilla,
	AC_HELP_STRING([--with-wxscintilla=DIR], [Use wxScintilla includes from DIR/include and libraries from DIR/lib.]),
[
    if test "$withval" != "yes" -a "$withval" != ""; then
        WXSCINTDIR=$withval
	if test "${WXSCINTDIR#/}" == "$WXSCINTDIR";
	then	WXSCINTDIR="`pwd`/$WXSCINTDIR"
	fi
	CPPFLAGS="$CPPFLAGS -I$WXSCINTDIR/include"
	LDFLAGS="$LDFLAGS -L$WXSCINTDIR/lib -Wl,-rpath,$WXSCINTDIR/lib"
    fi
],, [
#include <wx/buffer.h>
])

AC_CHECK_HEADER([wx/wxscintilla.h],, [AC_MSG_ERROR([wxScintilla includes not found.])])
LIBS="$LIBS -lwxscintilla"

AC_CHECK_LIB([m], [main])
AC_CHECK_LIB([dl], [dlopen])

AC_CHECK_LIB([portaudio], [Pa_Initialize],, [AC_MSG_ERROR([portaudio not found.])])
AC_CHECK_LIB([avcodec], [avcodec_init])
AC_CHECK_LIB([avformat], [av_read_frame])

PKG_CHECK_MODULES([ASA], asa >= 0.3.2, [
	CPPFLAGS="$CPPFLAGS $ASA_CFLAGS"
	LIBS="$LIBS $ASA_LIBS"
	AC_DEFINE(HAVE_ASA, 1, [found asa via pkg-config])
], [true])
AC_CHECK_HEADERS([asa/asa.h])

PKG_CHECK_MODULES([LIBASS], libass >= 0.9.1, [
	CPPFLAGS="$CPPFLAGS $LIBASS_CFLAGS"
	LIBS="$LIBS $LIBASS_LIBS"
	AC_DEFINE(HAVE_LIBASS, 1, [found libass via pkg-config])
], [true])
AC_CHECK_HEADERS([ass/ass.h])

PKG_CHECK_MODULES([GLIB], glib-2.0, [
	CPPFLAGS="$CPPFLAGS $GLIB_CFLAGS"
	LIBS="$LIBS $GLIB_LIBS"

	AC_CHECK_LIB([ssa], [SSA_Init])
	AC_CHECK_HEADERS([libssa/libssa.h])
], [
	AC_MSG_WARN([not testing for libssa as it depends on glib-2.0, which was not found])
])

AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE

AC_PROG_INSTALL

AC_MSG_CHECKING([[whether preprocessor supports #pragma once]])
AC_PREPROC_IFELSE(
	[AC_LANG_PROGRAM([[#pragma once]])],
	[
		AC_MSG_RESULT([yes])
		AC_DEFINE([HAVE_PRAGMA_ONCE], [1], [Preprocessor support for #pragma once])
	],
	[AC_MSG_RESULT([no])])

AC_C_FLAG([-Wall])
AC_C_FLAG([-Wextra],[AC_C_FLAG([-W])])
AC_C_FLAG([-Wno-unused-parameter])
AC_C_FLAG([-std=gnu99])
AC_CXX_FLAG([-Wall])
AC_CXX_FLAG([-Wextra],[AC_CXX_FLAG([-W])])
AC_CXX_FLAG([-Wno-unused-parameter])
AC_CXX_FLAG([-Wno-long-long])
AC_CXX_FLAG([-fpermissive])
AC_CXX_FLAG([-std=c++98])


AC_PATH_XTRA

AC_HEADER_STDC
AC_HEADER_STDBOOL


AC_OUTPUT([
	Makefile
	ac/Makefile
	locale/Makefile
	FexTrackerSource/Makefile
	aegisub/Makefile
	aegisub/posix/Makefile
	aegisub/bitmaps/Makefile
])


