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
