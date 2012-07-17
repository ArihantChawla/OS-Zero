#
# host.m4 - GNU Autoconf host configuration for Lightning.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

# Host system checks.

AC_DEFUN([LIGHTNING_CHECK_SYSTEM],
[
    AC_REQUIRE([AC_CANONICAL_HOST])
    AC_REQUIRE([AC_GNU_SOURCE])
    LIGHTNING_CHECK_BUILD_SYSTEM
])

AC_DEFUN([LIGHTNING_CHECK_BUILD_SYSTEM],
[
    AC_MSG_CHECKING([for target system type])
    case $host_os in
        *bsd* | *hpux* | *irix* | *linux* | *svr* | *sysv*)
            lightning_build_system=unix
            ;;
        *)
            lightning_build_system=unknown
            ;;
    esac
    AC_MSG_RESULT([$lightning_build_system])
])

