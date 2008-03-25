AC_DEFUN([AC_AGI_COMPILE],[
  aegisub_save_LDFLAGS="$LDFLAGS"
  aegisub_save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$3"
  LDFLAGS="$4"
      AC_CACHE_CHECK(
        [whether $1 works], [agi_with_$2],
        [AC_RUN_IFELSE([$5],
        [eval agi_with_$2="yes"],
        [eval agi_with_$2="no"])
        ])
  CPPFLAGS="$aegisub_save_CPPFLAGS"
  LDFLAGS="$aegisub_save_LDFLAGS"
])


AC_DEFUN([AC_AGI_LINK],[
  aegisub_save_CPPFLAGS="$CPPFLAGS"
  aegisub_save_CXXFLAGS="$CXXFLAGS"
  CPPFLAGS="$4"
  CXXFLAGS="$CPPFLAGS"
  AC_CHECK_HEADER([$3],[agi_header="yes"], [agi_header="no"])
  if test "$agi_header" = "yes"; then
    aegisub_save_LDFLAGS="$LDFLAGS"
    LDFLAGS="$5"
    AC_CACHE_CHECK(
      [whether $1 works], [agi_with_$2],
      [AC_LINK_IFELSE([$6],
      [eval agi_with_$2="yes"],
      [eval agi_with_$2="no"])
    ])
    LDFLAGS="$aegisub_save_LDFLAGS"
  else
    eval agi_with_$2="no"
  fi
  CPPFLAGS="$aegisub_save_CPPFLAGS"
  CXXFLAGS="$aegisub_save_CXXFLAGS"
])
