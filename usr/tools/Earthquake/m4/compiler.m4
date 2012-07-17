#
# compiler.m4 - GNU Autoconf compiler support for Lightning.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

# Check for compiler features.
AC_DEFUN([LIGHTNING_C],
[
    AC_C_CONST
    AC_C_INLINE
    AC_C_CHAR_UNSIGNED

    AC_C_PROTOTYPES

    AC_C_VOLATILE
    if test "$ac_cv_c_volatile" = "yes" ; then
        AC_DEFINE([HAVE_VOLATILE], 1)
    fi
])

AC_DEFUN([LIGHTNING_PROG_CC],
[
    AC_PROG_CC(gcc egcs cc cl)
])

# Check for compiler environment.
AC_DEFUN([LIGHTNING_CHECK_COMPILER],
[
    LIGHTNING_PROG_CC
    AC_PROG_CPP
    AM_PROG_AS
])

# Set compiler flags.
AC_DEFUN([LIGHTNING_SET_COMPILER_FLAGS],
[
    LIGHTNING_SET_CPU_ARCH
    LIGHTNING_SET_CFLAGS
    LIGHTNING_SET_KERNEL_FLAGS
    LIGHTNING_SET_LIB_FLAGS
    LIGHTNING_SET_LIGHTNING_FLAGS
    LIGHTNING_SET_WARNING_FLAGS
])

AC_DEFUN([LIGHTNING_SET_CFLAGS],
[
    # Add compiler flags.
    if test -n "$CPU_CFLAGS" ; then
        CFLAGS="$CPU_CFLAGS $CFLAGS"
    fi

    if test -n "$OPTIMIZE_CFLAGS" ; then
        CFLAGS="$OPTIMIZE_CFLAGS $CFLAGS"
    fi

    if test -n "$EXTRA_CFLAGS" ; then
        CFLAGS="$CFLAGS $EXTRA_CFLAGS"
    fi

    if test -n "$CFLAGS" ; then
        AM_CFLAGS="$AM_CFLAGS $CFLAGS"
    fi

    CCASFLAGS="$AM_CPPFLAGS $AM_CFLAGS"
])

AC_DEFUN([LIGHTNING_SET_KERNEL_FLAGS],
[
    case $CC in
        gcc | tcc)
            LIGHTNING_SET_GCC_KERNEL_FLAGS
            ;;
        *)
            ;;
    esac
])

AC_DEFUN([LIGHTNING_SET_LIB_FLAGS],
[
    case $CC in
        gcc | tcc)
            LIGHTNING_SET_GCC_LIB_FLAGS
            ;;
        *)
            ;;
    esac
])

AC_DEFUN([LIGHTNING_SET_LIGHTNING_FLAGS],
[
    case $CC in
        gcc | tcc)
            LIGHTNING_SET_GCC_LIGHTNING_FLAGS
            ;;
        *)
            ;;
    esac
])

AC_DEFUN([LIGHTNING_SET_WARNING_FLAGS],
[
    case $CC in
        gcc | tcc)
            LIGHTNING_SET_GCC_WARNING_FLAGS
            ;;
        *)
            ;;
    esac
])

