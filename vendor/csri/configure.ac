AC_PREREQ(2.57)
AC_INIT(csri, 0.1.0)
AC_CONFIG_AUX_DIR(ac)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([include/acconf.h])

# Checks for programs.
AC_PROG_CC

AC_PROG_LIBTOOL
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE

AC_PROG_INSTALL

AC_CHECK_HEADERS([png.h])
AC_CHECK_LIB([z], [inflate])
AC_CHECK_LIB([m], [pow])
AC_CHECK_LIB([png], [png_init_io])

AC_CPP_PRAGMA_ONCE
AC_C_FLAG([-Wall])
AC_C_FLAG([-Wextra],[AC_C_FLAG([-W])])
AC_C_FLAG([-Wno-unused-parameter])
AC_C_FLAG([-Winvalid-pch])
AC_C_FLAG([-pedantic])
AC_C_FLAG([-std=c99],[AC_C_FLAG([-c99])])
AC_ARG_ENABLE([werror],
	AC_HELP_STRING([--enable-werror], [compile with -Werror (for developers)]),
	[	if test "$enableval" == "yes"
		then	AC_C_FLAG([-Werror])
		fi])
AC_GCC_VISIBILITY([internal])

AC_PATH_XTRA
platform=""
case $host_os in
mingw*)	# LoadLibraryEx + GetProcAddress
	platform="win32"
	;;
*)	AC_SEARCH_LIBS([dlopen], [dl], [], [
		AC_MSG_FAILURE([dlopen not found - unsupported dynamic loader architecture / operating system])
	])
esac

AM_CONDITIONAL([BUILD_MINGW], [test "$platform" == "win32" ])


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([stdlib.h string.h sys/ioctl.h sys/time.h sys/wait.h sys/mman.h unistd.h libgen.h getopt.h])
AC_CHECK_FUNCS([getopt_long])
#
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.
#AC_FUNC_FORK
#AC_PROG_GCC_TRADITIONAL
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([memset strdup strerror strtoul])

AC_ARG_WITH([libass],[
	AS_HELP_STRING([--with-libass],[build libass backend @<:@default=auto@:>@])
],[],[	with_libass=auto
])

libass=no
AS_IF([test "$with_libass" != no],[
	ac_cflags_save="$CFLAGS"
	ac_ldflags_save="$LDFLAGS"

	if test "$with_libass" != auto -a "$with_libass" != yes
	then	LIBASS_CFLAGS="-I$with_libass/include"
		LIBASS_LIBS="-L$with_libass/lib -lass"
	else	PKG_CHECK_MODULES([LIBASS], libass >= 0.9.1,[],[
			AC_MSG_WARN([libass not found via pkg-config])
			LIBASS_LIBS="-lass"
		])
	fi
	CFLAGS="$CFLAGS $LIBASS_CFLAGS"
	LDFLAGS="$LDFLAGS $LIBASS_LIBS"
		
	AC_CHECK_LIB([ass], [ass_library_init], [libass_lib="yes"])
	AC_CHECK_HEADER([ass/ass.h], [libass_hdr="yes"])

	if test "$libass_lib" == yes -a "$libass_hdr" == yes
	then	libass=yes
	else	if "$with_libass" != auto
		then	AC_MSG_FAILURE([libass requested but not found])
		fi
	fi
	CFLAGS="$ac_cflags_save"
	LDFLAGS="$ac_ldflags_save"
])
AM_CONDITIONAL([BUILD_LIBASS], [test "$libass" == "yes" ])
AC_SUBST([LIBASS_CFLAGS])
AC_SUBST([LIBASS_LIBS])

csrilibdir="${libdir}/csri"
AC_SUBST([csrilibdir])

csri_path="${csrilibdir}:/usr/lib/csri:/usr/local/lib/csri:~/.csri/lib"
AC_ARG_WITH(csri-libpath,
	AC_HELP_STRING([--with-csri-libpath=PATH1:PATH2],
		[Look for CSRI renderers in the specified directories. Separate directory names with colons. Tildes are expanded at run-time. [[$csri_path]]]),
[
    if test "$withval" != "yes" -a "$withval" != "" -a "$withval" != "no"
    then csri_path="$withval"
    fi
])
AC_SUBST([csri_path], ["$csri_path"])

AC_CONFIG_FILES([csri.pc])

echo ""
echo "      using search path: ${csri_path}"
echo "building libass wrapper: ${libass}"
echo ""
AC_OUTPUT([
	ac/Makefile
	include/Makefile
	subhelp/Makefile
	lib/Makefile
	frontends/Makefile
	frontends/avisynth25/Makefile
	frontends/cmdline/Makefile
	backends/Makefile
	backends/libass/Makefile
	Makefile
])

