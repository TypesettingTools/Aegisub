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
AC_DEFUN([AC_PCH_FLAG], [{
	AC_LANG_PUSH(C++)
	ac_cxx_flag_save="$CXXFLAGS"
	ac_cxx_werror_flag_save="$ac_cxx_werror_flag"
	ac_cxx_werror_flag=yes
	CXXFLAGS="$CXXFLAGS -Werror $1"
	AC_MSG_CHECKING([[whether $CXX supports $1]])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM([[]])],
		[
			PCHFLAGS="$PCHFLAGS $1"
			AC_MSG_RESULT([yes])
		],
		[
			AC_MSG_RESULT([no])
			$2
		])
	CXXFLAGS="$ac_cxx_flag_save"
	ac_cxx_werror_flag="$ac_cxx_werror_flag_save"
	AC_LANG_POP(C++)
	}])
