#
# util.m4 - GNU Autoconf utilities.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

#
# Utility macros
#

#
# m4 macros
#

# Print messages that line up.
# LIGHTNING_MSG_INFO(LEFT, RIGHT, [RIGHTCOLUMN=34])
m4_define([LIGHTNING_MSG_INFO],
[dnl
    m4_pushdef([LIGHTNING_prefix], m4_default([$3], [                                 ]))dnl
    m4_pushdef([LIGHTNING_format], [%-]m4_eval(m4_len(LIGHTNING_prefix) - 3)[s ])dnl
    echo "m4_text_wrap([$2], LIGHTNING_prefix, m4_format(LIGHTNING_format, [$1]))"dnl
    m4_popdef([LIGHTNING_prefix])dnl
    m4_popdef([LIGHTNING_format])dnl
])

#
# Autoconf/shell macros
#

# Print a message without newline in the end
AC_DEFUN([LIGHTNING_MSG], [echo $ECHO_N $1])

# Print a message with newline in the end
AC_DEFUN([LIGHTNING_MSG_LINE],
[
    if test "$quiet" != "yes" ; then
        echo $1
    fi
])

AC_DEFUN([LIGHTNING_TRACE_CALL],
[
    if test "$lightning_configure_trace" = "yes" ; then
        echo "TRACE: calling [$1]"
    fi
])

AC_DEFUN([LIGHTNING_TRACE_VAR],
[
    if test "$lightning_configure_trace" = "yes" ; then
        echo "TRACE: $1 = '$$1'"
    fi
])

AC_DEFUN([LIGHTNING_PRINT_CONFIGURE_OPTS],
[
    if test -n "$lightning_configure_opts" ; then
        LIGHTNING_MSG_INFO([configure options:], [$lightning_configure_opts])
    fi
])

AC_DEFUN([LIGHTNING_PRINT_CONFIGURE_INFO],
[
    LIGHTNING_MSG_LINE
    if test "$lightning_configure_info" = "yes" ; then
        LIGHTNING_PRINT_CONFIGURE_OPTS
        LIGHTNING_MSG_LINE
        LIGHTNING_MSG_LINE(["SRC:      $LIGHTNING_DISTRIBUTION_ROOT"])
        LIGHTNING_MSG_LINE(["PACKAGE:  $PACKAGE $VERSION"])
        LIGHTNING_MSG_LINE(["HOST:     $host"])
        LIGHTNING_MSG_LINE(["----"])
        LIGHTNING_MSG_LINE(["ARCH:     $CPU_ARCHITECTURE"])
        LIGHTNING_MSG_LINE(["CC:       $CC"])
        LIGHTNING_MSG_LINE(["CFLAGS:   $CFLAGS"])
        LIGHTNING_MSG_LINE(["----"])
        LIGHTNING_MSG_LINE(["CPP:      $CPP"])
        LIGHTNING_MSG_LINE(["CPPFLAGS: $CPPFLAGS"])
        LIGHTNING_MSG_LINE(["----"])
        LIGHTNING_MSG_LINE(["AS:       $CCAS"])
        LIGHTNING_MSG_LINE(["ASFLAGS:  $CCASFLAGS"])
        LIGHTNING_MSG_LINE(["----"])
        LIGHTNING_MSG_LINE(["PREFIX:   $prefix"])
        LIGHTNING_MSG_LINE(["HEADERS:  $pkgincludedir"])
        LIGHTNING_MSG_LINE(["DATA:     $pkgdatadir"])
    elif test "$quiet" != "yes" ; then
        LIGHTNING_MSG_LINE(["PACKAGE:  $PACKAGE $VERSION"])
        LIGHTNING_MSG_LINE(["ARCH:     $CPU_ARCHITECTURE"])
        LIGHTNING_MSG_LINE(["CC:       $CC $CFLAGS"])
        LIGHTNING_MSG_LINE(["----"])
        LIGHTNING_MSG_LINE(["PREFIX:   $prefix"])
        LIGHTNING_MSG_LINE(["HEADERS:  $pkgincludedir"])
        LIGHTNING_MSG_LINE(["DATA:     $pkgdatadir"])
    fi
    LIGHTNING_MSG_LINE
    LIGHTNING_MSG_LINE([Type 'make' to build [$PACKAGE].])
])

