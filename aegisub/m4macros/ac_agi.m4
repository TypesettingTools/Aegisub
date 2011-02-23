AC_DEFUN([AC_AGI_COMPILE],[
  aegisub_save_LIBS="$LIBS"
  aegisub_save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$3"
  LIBS="$4"
      AC_CACHE_CHECK(
        [whether $1 works], [agi_cv_with_$2],
        [AC_RUN_IFELSE([$5],
        [eval agi_cv_with_$2="yes"],
        [eval agi_cv_with_$2="no"],
		[if test $? -ne 0; then
			eval agi_cv_with_$2="no";
		else
			eval agi_cv_with_$2="yes";
		fi])
        ])
  CPPFLAGS="$aegisub_save_CPPFLAGS"
  LIBS="$aegisub_save_LIBS"
])


AC_DEFUN([AC_AGI_LINK],[
  aegisub_save_CPPFLAGS="$CPPFLAGS"
  aegisub_save_CXXFLAGS="$CXXFLAGS"
  CPPFLAGS="$4"
  CXXFLAGS="$CPPFLAGS"
  AC_CHECK_HEADER([$3],[agi_cv_header="yes"], [agi_cv_header="no"])
  if test "$agi_cv_header" = "yes"; then
    aegisub_save_LDFLAGS="$LDFLAGS"
    LDFLAGS="$5"
    AC_CACHE_CHECK(
      [whether $1 works], [agi_cv_with_$2],
      [AC_LINK_IFELSE([$6],
      [eval agi_cv_with_$2="yes"],
      [eval agi_cv_with_$2="no"])
    ])
    LDFLAGS="$aegisub_save_LDFLAGS"
  else
    eval agi_cv_with_$2="no"
  fi
  CPPFLAGS="$aegisub_save_CPPFLAGS"
  CXXFLAGS="$aegisub_save_CXXFLAGS"
])
