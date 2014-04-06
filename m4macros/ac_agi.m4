AC_DEFUN([AC_AGI_COMPILE],[
  aegisub_save_LIBS="$LIBS"
  aegisub_save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $3"
  LIBS="$LIBS $4"
      AC_CACHE_CHECK(
        [whether $1 works], [agi_cv_with_$2],
        [AC_RUN_IFELSE([AC_LANG_SOURCE([$5])],
                       [eval agi_cv_with_$2="yes"],
                       [eval agi_cv_with_$2="no"],
                       [AS_IF([test $? -ne 0], [eval agi_cv_with_$2="no"], [eval agi_cv_with_$2="yes"])])])
  CPPFLAGS="$aegisub_save_CPPFLAGS"
  LIBS="$aegisub_save_LIBS"
])

AC_DEFUN([AC_AGI_LINK],[
  aegisub_save_LIBS="$LIBS"
  aegisub_save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $4"
  LIBS="$LIBS $5"
  AC_CHECK_HEADER([$3], [agi_cv_header="yes"], [agi_cv_header="no"])
  AS_IF([test "x$agi_cv_header" = xyes],
        [AC_CACHE_CHECK(
          [whether $1 works], [agi_cv_with_$2],
          [AC_LINK_IFELSE([AC_LANG_SOURCE([$6])], [eval agi_cv_with_$2="yes"], [eval agi_cv_with_$2="no"])])]
        [eval agi_cv_with_$2="no"])
  CPPFLAGS="$aegisub_save_CPPFLAGS"
  LIBS="$aegisub_save_LIBS"
])

# An optional dependency which requires pkg-config
# Args: Name, AC_ARG_WITH help string, AC_DEFINE help string
AC_DEFUN([AGI_OPT_PKG], [
  m4_define([varname], m4_bpatsubst([$1], [-.*], []))dnl
  m4_define([upper], m4_translit(varname, [a-z], [A-Z]))dnl

  AC_ARG_WITH(varname, AS_HELP_STRING([--without-][varname], [$2]))
  AS_IF([test x$with_]varname[ = xno],
        varname[_disabled="(disabled)"],
        [PKG_CHECK_MODULES(upper, $1 >= varname[]_required_version, [
          AC_DEFINE([WITH_]upper, 1, $3)
          with_[]varname="yes"
        ], [
            AS_IF([test x$with_]varname[ = xyes],
                  [AC_MSG_FAILURE([--with-]varname[ was specified, but ]varname[ could not be found])])
            with_[]varname="no"
        ])])
  AC_SUBST([with_]varname)
])
