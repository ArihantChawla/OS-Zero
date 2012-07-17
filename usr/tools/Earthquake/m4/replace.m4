#
# replace.m4 - GNU Autoconf functionality replacements.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

# Trick autoconf to not check for default headers.
AC_DEFUN([AC_INCLUDES_DEFAULT],
[
    RESURRECTION_MSG_LINE([skipping check for ANSI C headers])
])

