dnl $Id$
dnl Borrowed from the Mozilla Project configure.in

AC_DEFUN([AC_AGI_MDCPUCFG],[
  AC_MSG_CHECKING([for MDCPUCFG setting])

  case "$1" in
    *-beos*)
      HOST_NSPR_MDCPUCFG="md/_beos.cfg"
    ;;

    *cygwin*|*mingw*|*mks*|*msvc*|*wince)
      HOST_NSPR_MDCPUCFG="md/_winnt.cfg"
    ;;

    *-darwin*)
      HOST_NSPR_MDCPUCFG="md/_darwin.cfg"
    ;;

    *-*-freebsd*)
      HOST_NSPR_MDCPUCFG="md/_freebsd.cfg"
    ;;

    *-linux*|*-kfreebsd*-gnu)
      HOST_NSPR_MDCPUCFG="md/_linux.cfg"
    ;;

    *os2*)
      HOST_NSPR_MDCPUCFG="md/_os2.cfg"
    ;;

    *-osf*)
      HOST_NSPR_MDCPUCFG="md/_osf1.cfg"
    ;;

    *-solaris*)
      HOST_NSPR_MDCPUCFG="md/_solaris.cfg"
    ;;

    *)
      HOST_NSPR_MDCPUCFG="unknown"
    ;;
  esac

  AC_MSG_RESULT([$HOST_NSPR_MDCPUCFG])

  if test "$HOST_NSPR_MDCPUCFG" = "unknown"; then
    AC_MSG_NOTICE([Please edit m4macros/ac_agi_mdcpucfg.m4 to add target host.])
  fi
])
