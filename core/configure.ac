AC_PREREQ(2.57)
AC_INIT(aegisub, 1.10)
AC_CONFIG_AUX_DIR(ac)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([posix/acconf.h])

AC_CANONICAL_HOST

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
AC_PROG_RANLIB

AC_C_FLAG([-pthread])
AC_CXX_FLAG([-pthread])
AC_CHECK_LIB([pthread], [pthread_join])

AC_CHECK_HEADERS([wchar.h],,[
	AC_MSG_FAILURE([aegisub needs wide character support, find a wchar.h])
])
PKG_CHECK_MODULES([FREETYPE], freetype2 >= 9.8.3,, [
	AC_MSG_FAILURE([aegisub needs FreeType2, go install it])
])

WXCONFIG=wx-config
AC_ARG_WITH(wx-config,
	AC_HELP_STRING([--with-wx-config=FILE], [Use specific wx-config script to determine wxWidgets configuration. [[wx-config]]]),
[
    if test "$withval" != "yes" -a "$withval" != ""; then
        WXCONFIG=$withval
    fi
])
AC_MSG_CHECKING([wxWidgets version])
if wxversion=`$WXCONFIG --version`; then
    AC_MSG_RESULT([$wxversion])
else
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([wxWidgets is required. Try --with-wx-config.])
fi

CXXFLAGS="$CXXFLAGS `$WXCONFIG --cxxflags | sed -e 's/-fno-exceptions//'`"
LIBS="$LIBS `$WXCONFIG --libs`"

AC_CHECK_LIB([m], [main])
AC_CHECK_LIB([dl], [dlopen])

AC_CHECK_LIB([lua], [lua_call],, [AC_MSG_ERROR([lua not found.])])
AC_CHECK_LIB([lualib], [lua_tostring],, [AC_MSG_ERROR([lualib not found.])])
AC_CHECK_LIB([portaudio], [Pa_Initialize],, [AC_MSG_ERROR([portaudio not found.])])
AC_CHECK_LIB([avcodec], [avcodec_init],, [AC_MSG_ERROR([libavcodec (ffmpeg) not found.])])
AC_CHECK_LIB([avformat], [av_frac_init],, [AC_MSG_ERROR([libavformat (ffmpeg) not found or too old.])])

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
AC_CXX_FLAG([-std=c++98])


AC_PATH_XTRA

AC_HEADER_STDC
AC_HEADER_STDBOOL


AC_OUTPUT([
	Makefile
	ac/Makefile
	posix/Makefile
	bitmaps/Makefile
])

