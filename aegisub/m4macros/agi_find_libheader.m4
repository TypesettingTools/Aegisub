AC_DEFUN([AGI_FIND_HEADER],[
  file=`echo $2 | $as_tr_sh`

  aegisub_save_CPPFLAGS="$CPPFLAGS"

  for dir in $3; do
    vdir=`echo $dir | $as_tr_sh`
    CPPFLAGS="-I$dir"
    AC_CACHE_CHECK(
      [for $2 in $dir],
      [agi_cv_header_${vdir}_${file}],
      [AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([#include <$2>])],
        [eval agi_cv_header_${vdir}_${file}="yes"; found="${dir}"],
        [eval agi_cv_header_${vdir}_${file}="no"; found=""])
    ])

    if test -n "$found"; then
      break;
    fi
  done

  if test -n "$found"; then
    $1_CFLAGS="-I$found"
  fi
  CPPFLAGS="$aegisub_save_CPPFLAGS"
])


AC_DEFUN([AGI_FIND_LIB],[
  aegisub_save_LDFLAGS="$LDFLAGS"

  if test -n $3; then
    LDDIR="-L$3"
  fi

  for lib in $2; do
    vlib=`echo $lib | $as_tr_sh`
    LDFLAGS="$LDDIR -l$lib $4"
    AC_CACHE_CHECK(
      [for -l${lib}],
      [agi_cv_lib_${vlib}],
      [AC_LINK_IFELSE(
        [AC_LANG_PROGRAM()],
        [eval agi_cv_lib_${vlib}="yes"; found="${lib}"],
        [eval agi_cv_lib_${vlib}="no"; found=""])
    ])

    if test -n "$found"; then
      break;
    fi
  done

  if test -n "$found"; then
    $1_LDFLAGS="$LDDIR -l$found"
  fi
  LDFLAGS="$aegisub_save_LDFLAGS"
])
