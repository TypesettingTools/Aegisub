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
