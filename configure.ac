AC_PREREQ(2.57)
AC_INIT([Aegisub], [2.00-alpha],, [aegisub])
AC_CONFIG_AUX_DIR([ac])
AC_CONFIG_SRCDIR([aegisub/main.cpp])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([aegisub/posix/acconf.h])
AC_GNU_SOURCE

AC_CANONICAL_HOST


dnl Checks for programs.
CHECK_GNU_MAKE
AC_PROG_CC
AC_PROG_CXX
AC_LANG(C++)
AC_PROG_RANLIB

AC_PATH_PROGS(CONVERT, [convert], [])
if test -z "$CONVERT"; then
	AC_MSG_FAILURE([Aegisub needs convert (from ImageMagick) to produce xpms from the supplied bitmaps/icons])
fi

dnl Don't replace this with AC_GNU_GETTEXT, we don't want the framework-mess
dnl that macro depends on, just check if the single program we need is
dnl installed on the system. We can probably even do with non-GNU versions.
AC_PATH_PROGS(MSGFMT, [msgfmt], [])
if test -z "$MSGFMT"
then	AC_MSG_FAILURE([Aegisub needs msgfmt from gettext-tools to compile i18n string tables])
fi

ACX_PTHREAD([], [
	AC_MSG_FAILURE([Aegisub needs pthreads])
])
AC_MSG_CHECKING([whether pthreads works])
AC_COMPILE_IFELSE([
#include <stdlib.h>
#include <pthread.h>
int main() { return 0; }
], [
	AC_MSG_RESULT([yes])
], [
	AC_MSG_RESULT([no])
	echo <<EOM
***********************************************************************
* Your pthreads installation seems to be broken.
* Apart from obviously being bad, this can cause strange detection
* errors as well.
* On some Linux distributions, this can be caused by having the package
* libpthread-dev installed. If you have this package, please uninstall
* it now.
***********************************************************************
EOM
	AC_MSG_FAILURE([You need a working pthreads installation.])
])


AC_CHECK_LIB([asound], [snd_pcm_open], [found_audio=yes], [with_alsa=no])
AC_CHECK_LIB([portaudio], [Pa_Initialize], [found_audio=yes], [with_portaudio=no])
AC_CHECK_FUNCS([Pa_GetStreamTime])
PKG_CHECK_MODULES([LIBPULSE], libpulse >= 0.5, [found_audio=yes], [with_pulseaudio=no])
AC_CHECK_LIB([avcodec], [avcodec_init],, [with_ffmpeg=no])
AC_CHECK_LIB([avformat], [av_read_frame],, [with_ffmpeg=no])
if test "$with_ffmpeg" != "no"; then
	found_video_provider=yes
fi
AC_CHECK_LIB([hunspell], [main],, [with_hunspell=no])
AC_CHECK_LIB([ruby1.9], [ruby_init],, [with_ruby=no])

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

AX_CHECK_GL
if test ! "$GL_LIBS"; then
	AC_MSG_WARN([Could not find system GL library])
	AC_MSG_FAILURE([GL support is required to build Aegisub. (Try mesa3d)])
fi
CPPFLAGS="$CPPFLAGS $GL_CFLAGS"
LIBS="$LIBS $GL_LIBS"

AC_MSG_CHECKING([wxWidgets version])
if wxversion=`$WXCONFIG --version`; then
    AC_MSG_RESULT([$wxversion])
else
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([wxWidgets is required. Try --with-wx-config.])
fi

CPPFLAGS="$CPPFLAGS `$WXCONFIG --cppflags | sed -e 's/-fno-exceptions//'` $FREETYPE_CFLAGS"
LIBS="$LIBS `$WXCONFIG --libs std,gl,stc` $FREETYPE_LIBS"

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

AC_MSG_CHECKING([wxWidgets StyledTextCtrl support])
AC_LINK_IFELSE([
#include <wx/stc/stc.h>
int main(int argc, char **argv) {
	wxStyledTextCtrl *canvas;
	return 0;
}
],[
	AC_MSG_RESULT([yes])
],[
	AC_MSG_RESULT([not available])
	AC_MSG_FAILURE([wxWidgets missing StyledTextCtrl])
])

AC_CHECK_LIB([m], [main])
AC_CHECK_LIB([dl], [dlopen])

AC_ARG_WITH([libass], [AS_HELP_STRING([--without-libass],[disable libass @<:@default=check@:>@])],[],[with_libass=check])

AS_IF([test "x$with_libass" != xno], [
	PKG_CHECK_MODULES([LIBASS], libass >= 0.9.1, [
		CPPFLAGS="$CPPFLAGS $LIBASS_CFLAGS"
		LIBS="$LIBS $LIBASS_LIBS"
		AC_DEFINE(HAVE_LIBASS, 1, [found libass via pkg-config])
		AC_CHECK_HEADERS([ass/ass.h], [
			with_libass=yes
		], [	if test "x$with_libass" != xcheck
			then	AC_MSG_FAILURE([--with-libass given, but libass headers not found])
			fi
			with_libass=no
		])
	], [	if test "x$with_libass" != xcheck
		then	AC_MSG_FAILURE([--with-libass given, but libass pkgconfig module found])
		fi
		with_libass=no
	])
])

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
AC_C_FLAG([-O2])
AC_C_FLAG([-Wextra],[AC_C_FLAG([-W])])
AC_C_FLAG([-Wno-unused-parameter])
AC_C_FLAG([-std=gnu99])
AC_CXX_FLAG([-Wall])
AC_CXX_FLAG([-Wextra],[AC_CXX_FLAG([-W])])
AC_CXX_FLAG([-Wno-unused-parameter])
AC_CXX_FLAG([-Wno-long-long])
AC_CXX_FLAG([-O2])
AC_CXX_FLAG([-fpermissive])
AC_CXX_FLAG([-fno-strict-aliasing])
AC_CXX_FLAG([-std=c++98])


AC_PATH_XTRA

AC_HEADER_STDC
AC_HEADER_STDBOOL

AC_CONFIG_SUBDIRS([csri])

AM_CONDITIONAL([USE_LIBASS], [test "$with_libass" != "no" ])
AM_CONDITIONAL([HAVE_ALSA], [test "$with_alsa" != "no"])
AM_CONDITIONAL([HAVE_PORTAUDIO], [test "$with_portaudio" != "no"])
AM_CONDITIONAL([HAVE_PULSEAUDIO], [test "$with_pulseaudio" != "no"])
AM_CONDITIONAL([HAVE_HUNSPELL], [test "$with_hunspell" != "no"])
AM_CONDITIONAL([HAVE_RUBY], [test "$with_ruby" != "no"])
AM_CONDITIONAL([HAVE_FFMPEG], [test "$with_ffmpeg" != "no"])

AC_OUTPUT([
	Makefile
	ac/Makefile
	automation/Makefile
	FexTrackerSource/Makefile
	locale/Makefile
	aegisub/Makefile
	aegisub/bitmaps/Makefile
	aegisub/posix/Makefile
])

if test -z "$found_audio"; then
	echo <<EOM
***********************************************************************
* No supported audio player interface was found on your system.
* If you want audio support in Aegisub you need to install one of
* these libraries:
*  - PulseAudio
*    http://pulseaudio.org/
*  - ALSA (Linux only) (Currently broken)
*    http://www.alsa-project.org/
*  - PortAudio (version 18 only, version 19 is not compatible)
*    http://www.portaudio.com/
***********************************************************************
EOM
fi

if test -z "$found_video_provider"; then
	echo <<EOM
***********************************************************************
* No supported video/audio reader interface was found on your system.
* You will not be able to open any video or audio files in Aegisub
* unless you install a supported video/audio provider.
* You will however still be able to open "dummy" video, ie. a blank,
* virtual video clip with subtitles overlaid.
* Currently we only support one video/audio provider on non-Windows
* systems:
*  - FFmpeg (libavcodec + libavformat)
*    http://ffmpeg.mplayerhq.hu/
***********************************************************************
EOM
fi



