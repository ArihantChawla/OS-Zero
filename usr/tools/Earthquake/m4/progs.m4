#
# progs.m4 - GNU Autoconf checks for programs.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for informationa about using this software.

# Check for programs.
AC_DEFUN([LIGHTNING_PROGS],
[
    LIGHTNING_CHECK_COMPILER
    AC_CHECK_PROG(AR, ar, ar, false)
    AC_CHECK_PROG(CHMOD, chmod, chmod, true)
    AC_CHECK_PROG(CP, cp, cp, false)
    AC_CHECK_PROG(CTAGS, ctags, ctags, true)
    AC_CHECK_PROG(MKDIR, mkdir, mkdir, false)
    AC_CHECK_PROG(MV, mv, mv, true)
    AC_CHECK_PROG(RM, rm, rm, true)
    AC_CHECK_PROG(SED, sed, sed, false)
    AC_CHECK_PROG(TAR, tar, tar, true)
])

